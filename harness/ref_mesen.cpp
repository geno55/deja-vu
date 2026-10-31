// The reference (ref.h) on top of Mesen2's emulation core, the same C++ classes its UI
// drives through InteropDLL. Built only with DV_HARNESS: Mesen2 is GPL-3.0.
//
// How it stays in step with the harness:
// - Mesen always emulates on a thread of its own. We power on paused with the debugger
//   attached (what Mesen's test runner does), so that thread breaks before the first
//   instruction.
// - Debugger breaks are reported through a CodeBreak notification sent on the emulation
//   thread. Our listener handles it there: for an APU write breakpoint it logs the write;
//   for the NMI breakpoint it parks the thread until the next ref_run_to_nmi(). Either way it
//   resumes the debugger before returning, so Mesen never falls into its 10 ms sleep loop.
// - Input comes from an IInputProvider, the hook Mesen's movie player uses, so each poll gets
//   the harness's buttons the same way a Lua emu.setInput() in the inputPolled event does.
#include "pch.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/SettingTypes.h"
#include "Shared/SaveStateManager.h"
#include "Shared/NotificationManager.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/BaseControlManager.h"
#include "Shared/MemoryType.h"
#include "Shared/CpuType.h"
#include "Shared/DebuggerRequest.h"
#include "Shared/Interfaces/INotificationListener.h"
#include "Shared/Interfaces/IInputProvider.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/Breakpoint.h"
#include "NES/NesConsole.h"
#include "NES/NesCpu.h"
#include "NES/BaseNesPpu.h"
#include "NES/APU/NesApu.h"
#include "Shared/Audio/SoundMixer.h"
#include "NES/Input/NesController.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/VirtualFile.h"

#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <mutex>

#include "ref.h"

namespace {

constexpr uint16_t kNmiHandler = 0xEC9A;  // nmi (7:EC9A); the vector at $FFFA is checked
constexpr uint16_t kNmiRti = 0xED23;      // the RTI ending it (nmi_done + 17); checked too

// Layout of Mesen2's Breakpoint class (Debugger/Breakpoint.h), whose fields are private.
// InteropDLL's SetBreakpoints takes the same layout from the UI (UI/Interop/InteropBreakpoint.cs).
struct BreakpointFields {
  uint32_t id;
  CpuType cpu_type;
  MemoryType memory_type;
  BreakpointTypeFlags type;
  int32_t start_addr;
  int32_t end_addr;
  bool enabled;
  bool mark_event;
  bool ignore_dummy_operations;
  char condition[1000];
};
static_assert(sizeof(BreakpointFields) == sizeof(Breakpoint), "Mesen2's Breakpoint layout changed");

enum BreakpointId { kBpExec, kBpApu, kBpPpuRead };

// NesController's button bits, in the order of ref.h's kPad* bits.
constexpr uint8_t kPadButtons[8] = {
  NesController::A, NesController::B, NesController::Select, NesController::Start,
  NesController::Up, NesController::Down, NesController::Left, NesController::Right,
};

// What the harness stores in a save state besides Mesen's own data.
struct StateHeader {
  char magic[4];
  RefStop stop;
};

}  // namespace

struct Ref : INotificationListener, IInputProvider {
  std::unique_ptr<Emulator> emu;
  shared_ptr<Ref> self;  // the notification manager holds listeners as weak_ptr
  RefInputFn *input = nullptr;
  void *input_ctx = nullptr;

  // Shared with the emulation thread; guarded by mu.
  std::mutex mu;
  std::condition_variable cv;
  bool parked = false;    // the emulation thread waits in Park()
  bool quitting = false;  // ref_close: stop parking
  bool started = false;   // the first break (before the first instruction) has happened
  std::string battery;    // Mesen's save file for this ROM

  // Used by one side at a time: the emulation thread while it runs, the harness while the
  // emulation thread is parked (ref_load_state rewrites polls, nmis and stop).
  Debugger *debugger = nullptr;
  NesConsole *console = nullptr;
  uint32_t polls = 0;
  uint32_t nmis = 0;
  uint64_t rti_cycle = 0;
  std::vector<RefAccess> apu, ppu_reads;
  std::vector<uint64_t> apu_cycles;
  bool recording = false;
  std::vector<RefCpu> trace;
  bool tracing = false, want_tracing = false;  // the harness sets want_tracing while parked
  RefStop stop = {};

  void ProcessNotification(ConsoleNotificationType type, void *parameter) override {
    if(type != ConsoleNotificationType::CodeBreak) {
      return;
    }
    BreakEvent *evt = static_cast<BreakEvent *>(parameter);
    if(!started) {
      started = true;
      Setup();
      Park();
    } else if(evt->Source == BreakSource::Breakpoint) {
      // One break per access, whichever breakpoint matched: dispatch on what it was.
      // Execute breakpoints also match operand fetches; only opcode fetches count here.
      const MemoryOperationInfo &op = evt->Operation;
      if(op.Type == MemoryOperationType::Write) {
        apu.push_back({(uint16_t)op.Address, (uint8_t)op.Value});
        apu_cycles.push_back(console->GetCpu()->GetState().CycleCount);
      } else if(op.Type == MemoryOperationType::Read) {
        ppu_reads.push_back({(uint16_t)op.Address, (uint8_t)op.Value});
      } else if(op.Type != MemoryOperationType::ExecOpCode) {
        // an operand
      } else if(op.Address == kNmiHandler) {
        nmis++;
        Capture();
        Park();
        if(want_tracing) {
          trace.push_back(Cpu());  // the next stretch starts with this instruction
        }
      } else {
        if(tracing) {
          trace.push_back(Cpu());
        }
        if(op.Address == kNmiRti) {
          rti_cycle = console->GetCpu()->GetState().CycleCount;
        }
      }
    }
    if(want_tracing != tracing) {
      tracing = want_tracing;
      SetBreakpoints();
    }
    debugger->Run();  // clears the break, so SleepUntilResume returns at once
  }

  RefCpu Cpu() {
    const NesCpuState &s = console->GetCpu()->GetState();
    return {s.CycleCount, s.PC, s.A, s.X, s.Y, s.PS, s.SP};
  }

  bool SetInput(BaseControlDevice *device) override {
    if(device->GetPort() != 0 || !device->HasControllerType(ControllerType::NesController)) {
      return false;
    }
    uint8_t pad = input ? input(input_ctx, polls) : 0;
    polls++;
    for(int bit = 0; bit < 8; bit++) {
      if(pad & (1 << bit)) {
        device->SetBitValue(kPadButtons[bit], true);
      }
    }
    return true;
  }

  // On the emulation thread, before the first instruction runs.
  void Setup() {
    debugger = emu->GetDebugger(false).GetDebugger();
    console = dynamic_cast<NesConsole *>(emu->GetConsoleUnsafe());
    console->GetControlManager()->RegisterInputProvider(this);
    SetBreakpoints();
  }

  // On the emulation thread. The NMI handler's first instruction (NMI entry) and its RTI
  // (whose cycle lets the C side count cycles from the same point), or every instruction
  // while tracing; writes to the APU registers, $4000-$4013, $4015 and $4017 ($4014 is
  // OAM DMA and $4016 the controller strobe); and reads of PPU registers except $2002 itself
  // (a condition, so the game's many $2002 reads don't each stop Mesen; its mirrors count).
  void SetBreakpoints() {
    const BreakpointFields exec = {kBpExec, CpuType::Nes, MemoryType::NesMemory,
                                   BreakpointTypeFlags::Execute, 0, 0, true};
    const BreakpointFields apu_write = {kBpApu, CpuType::Nes, MemoryType::NesMemory,
                                        BreakpointTypeFlags::Write, 0x4000, 0x4013, true};
    BreakpointFields bp[6] = {exec, exec, apu_write, apu_write, apu_write};
    bp[0].start_addr = bp[0].end_addr = kNmiHandler;
    bp[1].start_addr = tracing ? 0x0000 : kNmiRti;
    bp[1].end_addr = tracing ? 0xFFFF : kNmiRti;
    bp[3].start_addr = bp[3].end_addr = 0x4015;
    bp[4].start_addr = bp[4].end_addr = 0x4017;
    bp[5] = {kBpPpuRead, CpuType::Nes, MemoryType::NesMemory, BreakpointTypeFlags::Read,
             0x2000, 0x3FFF, true};
    snprintf(bp[5].condition, sizeof(bp[5].condition), "address != 8194");  // $2002
    debugger->SetBreakpoints(reinterpret_cast<Breakpoint *>(bp), 6);
  }

  void Capture() {
    NesCpuState &cpu = console->GetCpu()->GetState();
    const uint8_t *ram = static_cast<uint8_t *>(emu->GetMemory(MemoryType::NesInternalRam).Memory);
    stop.nmi = nmis;
    stop.rti_cycle = rti_cycle;
    stop.polls = polls;
    stop.ppu_frame = console->GetPpu()->GetFrameCount();
    stop.cycle = cpu.CycleCount;
    stop.a = cpu.A;
    stop.x = cpu.X;
    stop.y = cpu.Y;
    stop.sp = cpu.SP;
    stop.p = ram[0x100 + (uint8_t)(cpu.SP + 1)];
    stop.return_pc = ram[0x100 + (uint8_t)(cpu.SP + 2)] | ram[0x100 + (uint8_t)(cpu.SP + 3)] << 8;
  }

  void Park() {
    std::unique_lock<std::mutex> lock(mu);
    if(quitting) {
      return;
    }
    parked = true;
    cv.notify_all();
    cv.wait(lock, [this] { return !parked || quitting; });
  }

  // Harness side: wait until the emulation thread parks, or emulation ends.
  bool WaitParked() {
    std::unique_lock<std::mutex> lock(mu);
    while(!parked) {
      if(cv.wait_for(lock, std::chrono::milliseconds(200), [this] { return parked; })) {
        break;
      }
      if(!emu->IsRunning()) {
        return false;
      }
    }
    return true;
  }
};

static void set_err(char *err, size_t size, const char *msg) {
  if(err && size) {
    snprintf(err, size, "%s", msg);
  }
}

extern "C" Ref *ref_open(const char *rom_path, const char *home_dir, const char *sav_path,
                         RefInputFn *input, void *input_ctx, char *err, size_t err_size) {
  namespace fs = std::filesystem;
  std::error_code ec;
  fs::create_directories(fs::path(home_dir) / "Saves", ec);
  fs::path rom = fs::absolute(rom_path, ec);
  // A save file from an earlier run would make this one start differently.
  fs::path battery = fs::path(home_dir) / "Saves" / rom.filename().replace_extension(".sav");
  fs::remove(battery, ec);
  if(sav_path && !fs::copy_file(sav_path, battery, ec)) {
    set_err(err, err_size, "can't copy the save file to Mesen's save folder");
    return nullptr;
  }
  FolderUtilities::SetHomeFolder(fs::absolute(home_dir).string());

  shared_ptr<Ref> r = std::make_shared<Ref>();
  r->self = r;
  r->battery = battery.string();
  r->input = input;
  r->input_ctx = input_ctx;
  r->emu.reset(new Emulator());
  Emulator *emu = r->emu.get();
  emu->Initialize(false);

  // Mesen's defaults, except what these runs need to be repeatable and quick. The RAM fill
  // and the two standard controllers match the settings tools/run_mesen.py recordings use.
  EmuSettings *settings = emu->GetSettings();
  NesConfig nes = settings->GetNesConfig();
  nes.RamPowerOnState = RamState::AllZeros;
  nes.AutoConfigureInput = false;
  nes.Port1.Type = ControllerType::NesController;
  nes.Port2.Type = ControllerType::NesController;
  nes.ExpPort.Type = ControllerType::None;
  settings->SetNesConfig(nes);
  PreferencesConfig prefs = settings->GetPreferences();
  prefs.AutoSaveStateDelay = 0;
  prefs.RewindBufferSize = 0;
  prefs.DisableGameSelectionScreen = true;
  settings->SetPreferences(prefs);
  settings->SetFlag(EmulationFlags::ConsoleMode);  // attach the debugger when the ROM loads

  emu->GetNotificationManager()->RegisterNotificationListener(r);
  emu->Pause();  // so the debugger breaks before the first instruction
  if(!emu->LoadRom(VirtualFile(rom.string()), VirtualFile())) {
    set_err(err, err_size, "Mesen2 could not load the ROM");
    ref_close(r.get(), nullptr);
    return nullptr;
  }
  // Only now: loading a ROM resets the rewind manager, which clears this flag.
  settings->SetFlag(EmulationFlags::MaximumSpeed);
  if(!r->WaitParked()) {
    set_err(err, err_size, "Mesen2 stopped before the first instruction");
    ref_close(r.get(), nullptr);
    return nullptr;
  }
  // The fixed bank is the last 16 KB of PRG ROM, so $C000-$FFFF is at offset $1C000.
  const ConsoleMemoryInfo prg = emu->GetMemory(MemoryType::NesPrgRom);
  const uint8_t *fixed = static_cast<uint8_t *>(prg.Memory) + 0x1C000 - 0xC000;
  if(prg.Size != 0x20000 || (fixed[0xFFFA] | fixed[0xFFFB] << 8) != kNmiHandler ||
     fixed[kNmiRti] != 0x40) {
    set_err(err, err_size, "no NMI handler at $EC9A ending with RTI at $ED23; is this Deja Vu (USA)?");
    ref_close(r.get(), nullptr);
    return nullptr;
  }
  return r.get();
}

extern "C" void ref_close(Ref *r, const char *sav_out) {
  if(r->recording) {
    r->emu->GetSoundMixer()->StopRecording();
  }
  if(!r) {
    return;
  }
  {
    std::lock_guard<std::mutex> lock(r->mu);
    r->quitting = true;
    r->parked = false;
  }
  r->cv.notify_all();
  // Only when asked does Mesen write its save file, as it does when it powers off.
  r->emu->Stop(false, true, sav_out != nullptr);
  if(sav_out) {
    std::error_code ec;
    std::filesystem::copy_file(r->battery, sav_out, std::filesystem::copy_options::overwrite_existing,
                               ec);
    if(ec) {
      fprintf(stderr, "ref_close: can't copy %s to %s\n", r->battery.c_str(), sav_out);
    }
  }
  r->emu->Release();
  r->emu.reset();
  r->self.reset();  // frees r
}

extern "C" int ref_run_to_nmi(Ref *r, RefStop *stop) {
  {
    std::lock_guard<std::mutex> lock(r->mu);
    r->apu.clear();
  r->apu_cycles.clear();
    r->ppu_reads.clear();
    r->trace.clear();
    r->parked = false;
  }
  r->cv.notify_all();
  if(!r->WaitParked()) {
    return -1;
  }
  if(stop) {
    *stop = r->stop;
  }
  return 0;
}

extern "C" const uint8_t *ref_mem(Ref *r, int which, size_t *size) {
  static const MemoryType kTypes[kRefMemCount] = {
    MemoryType::NesInternalRam, MemoryType::NesSaveRam, MemoryType::NesNametableRam,
    MemoryType::NesPaletteRam, MemoryType::NesSpriteRam,
  };
  ConsoleMemoryInfo mem = r->emu->GetMemory(kTypes[which]);
  if(size) {
    *size = mem.Size;
  }
  return static_cast<const uint8_t *>(mem.Memory);
}

extern "C" const uint16_t *ref_screen(Ref *r) {
  return r->console->GetPpu()->GetScreenBuffer(false);
}

extern "C" void ref_cpu(Ref *r, RefCpu *cpu) {
  *cpu = r->Cpu();
}

extern "C" void ref_set_trace(Ref *r, int on) {
  r->want_tracing = on != 0;
}

extern "C" size_t ref_trace(Ref *r, const RefCpu **entries) {
  *entries = r->trace.data();
  return r->trace.size();
}

extern "C" const uint64_t *ref_apu_write_cycles(Ref *r) {
  return r->apu_cycles.data();
}

static void copy_envelope(RefApuChannel *c, const ApuEnvelopeState &e) {
  c->env_start = e.StartFlag;
  c->env_loop = e.Loop;
  c->env_constant = e.ConstantVolume;
  c->env_volume = e.Volume;
  c->env_divider = e.Divider;
  c->env_decay = e.Counter;
}

static void copy_length(RefApuChannel *c, const ApuLengthCounterState &l, bool enabled) {
  c->enabled = enabled;
  c->length = l.Counter;
  c->halt = l.Halt;
}

extern "C" void ref_apu_state(Ref *r, RefApu *apu) {
  NesApu *chip = r->console->GetApu();
  chip->Run();  // it only catches up when it has to
  ApuState s = chip->GetState();
  const ApuSquareState *square[2] = {&s.Square1, &s.Square2};
  memset(apu, 0, sizeof(*apu));
  for(int i = 0; i < 2; i++) {
    RefApuChannel *c = &apu->channel[i];
    c->duty = square[i]->Duty;
    c->duty_pos = square[i]->DutyPosition;
    c->sweep_on = square[i]->SweepEnabled;
    c->sweep_negate = square[i]->SweepNegate;
    c->sweep_period = square[i]->SweepPeriod;
    c->sweep_shift = square[i]->SweepShift;
    c->period = square[i]->Period;
    c->timer = square[i]->Timer;
    copy_length(c, square[i]->LengthCounter, square[i]->Enabled);
    copy_envelope(c, square[i]->Envelope);
  }
  RefApuChannel *t = &apu->channel[2];
  t->period = s.Triangle.Period;
  t->timer = s.Triangle.Timer;
  t->step = s.Triangle.SequencePosition;
  t->linear_counter = s.Triangle.LinearCounter;
  t->linear_reload_value = s.Triangle.LinearCounterReload;
  t->linear_reload = s.Triangle.LinearReloadFlag;
  copy_length(t, s.Triangle.LengthCounter, s.Triangle.Enabled);
  RefApuChannel *n = &apu->channel[3];
  n->period = s.Noise.Period;
  n->timer = s.Noise.Timer;
  n->shift = s.Noise.ShiftRegister;
  n->mode = s.Noise.ModeFlag;
  copy_length(n, s.Noise.LengthCounter, s.Noise.Enabled);
  copy_envelope(n, s.Noise.Envelope);
  apu->five_step = s.FrameCounter.FiveStepMode;
  apu->frame_step = s.FrameCounter.SequencePosition;
}

extern "C" void ref_record_audio(Ref *r, const char *path) {
  // With no UI to set them, the channels' volumes are 0, and the sound is silence.
  NesConfig cfg = r->emu->GetSettings()->GetNesConfig();
  for(uint32_t &volume : cfg.ChannelVolumes) {
    volume = 100;
  }
  r->emu->GetSettings()->SetNesConfig(cfg);
  r->emu->GetSoundMixer()->StartRecording(path);
  r->recording = true;
}

extern "C" size_t ref_apu_writes(Ref *r, const RefAccess **writes) {
  *writes = r->apu.data();
  return r->apu.size();
}

extern "C" size_t ref_ppu_reads(Ref *r, const RefAccess **reads) {
  *reads = r->ppu_reads.data();
  return r->ppu_reads.size();
}

extern "C" uint8_t *ref_save_state(Ref *r, size_t *size) {
  std::stringstream ss;
  StateHeader header = {{'D', 'V', 'R', '1'}, r->stop};
  ss.write(reinterpret_cast<const char *>(&header), sizeof(header));
  ss.write(reinterpret_cast<const char *>(&r->nmis), sizeof(r->nmis));
  ss.write(reinterpret_cast<const char *>(&r->polls), sizeof(r->polls));
  r->emu->Serialize(ss, false, 0);
  std::string data = ss.str();
  uint8_t *out = static_cast<uint8_t *>(malloc(data.size()));
  memcpy(out, data.data(), data.size());
  *size = data.size();
  return out;
}

extern "C" int ref_load_state(Ref *r, const uint8_t *data, size_t size, RefStop *stop) {
  StateHeader header;
  const size_t fixed = sizeof(header) + 2 * sizeof(uint32_t);
  if(size < fixed || memcmp(data, "DVR1", 4) != 0) {
    return -1;
  }
  memcpy(&header, data, sizeof(header));
  std::stringstream ss(std::string(reinterpret_cast<const char *>(data) + fixed, size - fixed));
  if(r->emu->Deserialize(ss, SaveStateManager::FileFormatVersion, false, std::nullopt, false) !=
     DeserializeResult::Success) {
    return -1;
  }
  memcpy(&r->nmis, data + sizeof(header), sizeof(r->nmis));
  memcpy(&r->polls, data + sizeof(header) + sizeof(r->nmis), sizeof(r->polls));
  r->stop = header.stop;
  r->rti_cycle = header.stop.rti_cycle;
  r->apu.clear();
  r->apu_cycles.clear();
  r->ppu_reads.clear();
  r->trace.clear();
  if(stop) {
    *stop = r->stop;
  }
  return 0;
}
