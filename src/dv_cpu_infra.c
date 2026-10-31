// The game on the C side: original code, ported routines and where NMIs go; see dv_cpu_infra.h.
#include "dv_cpu_infra.h"

#include <setjmp.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "coro.h"
#include "cpu6502.h"
#include "variables.h"

enum {
  // Where in bank 7 the routines fuzz_help_routine helps are.
  kFirstHotspotRoutine = 0xC2A8,  // click_picture
  kFirstSpecialScene = 0xC55D,    // run_special_scene
  kFirstWait = 0xC7E4,            // wait_180_frames
  kFirstSaveRoutine = 0xC800,     // load_game
  kAfterSaveRoutines = 0xCB08,    // pick_default_command
  kFirstSoundRoutine = 0xF3F5,    // sound_reset
  kAfterSoundRoutines = 0xFADA,   // sound_jump_table
  kMaxCycles = 300000,   // ten frames of 6502 code: give up if the NMI's place never comes
  kRecentSize = 64,
  kSentinelPc = 0x0000,  // emu_call's routine returns here (the game never runs code there)
  kGameStack = 1 << 20,
  kHaltRom = 6 * 0x4000 + 0x32C6,  // 6:B2C6, the jmp to itself after the ending
  kCheckLimit = 100000,   // a port check gives up on originals that run more cycles (never
                          // return), and on runs that call ported routines more often (C takes
                          // no cycles)
  kCheckLimitWaits = 1000000,  // the same for a check that runs NMIs through its waits
  kFuzzGiveUp = 20,       // samples through waits with none finished before a routine is left
  kCheckReports = 3,      // differences printed per routine
  kMaxPortedDepth = 500,  // the C stack would run out long before the 6502's wraps
  kFreeRunFrameCycles = 29781,  // the game on its own: 6502 code gets this much in a frame (a glitch's)
  kSpinGuards = 200000,   // ported routines called with no wait before a frame passes anyway (the game
                          // on its own), for a loop with no wait that emu_spin isn't in; a real frame
                          // of work is far less, since at most about 2,500 fit in one on the console
};

static Coro *game;
static int failed;
static char fail_msg[256];

static EmuNmi target;   // the reference's next NMI
static int target_set;  // the game hasn't reached its place yet
static int target_wait; // it came in a wait loop
static EmuPlace place;  // where the game took it
static uint64_t run_start;
static int near_count;     // times the 6502 was at the interrupted instruction, looking for it
static Cpu6502 near_cpu;   // the last of them
static char late_note[200];

static void (*frame_hook)(void);  // emu_set_frame_hook
static int free_run;       // emu_run_frame: no reference, so the NMI comes when the game waits
static int nmi_pending;    // the game has waited, with the NMI on
static int jammed;         // the game on its own: the 6502 ran an opcode that freezes it
static uint64_t frame_start_cycle;  // the game on its own: g_nes.cpu.cycles as this frame began
static unsigned guards_since_wait, spin_passes;
static unsigned long bad_indexes;
static int in_nmi;      // between taking an NMI and its RTI
static int in_handler;  // running the NMI handler
static int ported_depth;  // ported routines on the game's call stack
static uint8_t outer_sp;  // the 6502's stack pointer when the outermost of them was entered
static int c_ran;       // ported code ran since the NMI handler

static const EmuHook *hooks;
static size_t hook_count;
static uint32_t *hook_rom;   // where each hook's routine is in PRG ROM
static uint16_t *hook_next;  // 1 + the next hook at the same CPU address, 0: none
static uint16_t hook_first[0x10000];  // 1 + the first hook at each CPU address, 0: none

static EmuTraceEntry recent[kRecentSize];  // the last instructions, for error reports
static unsigned recent_pos;
static int tracing;
static EmuTraceEntry *trace_buf;
static size_t trace_len, trace_cap;

static int port_check;        // emu_set_port_check
static int in_check;          // running an original on a copy of the state, for a check
static jmp_buf check_abort;   // out of that run: 1 it waits, 2 it runs too long, 3 so does C
static unsigned long check_work;  // ported routines called in that run
static uint64_t check_start;      // the CPU cycle it started at
static unsigned long check_limit = kCheckLimit;  // how much of either before it gives up
static int check_nmis;        // the run's waits run a made-up NMI instead of giving up
static int unfair_read;  // a port read through a pointer what differs from what the original reads
static int sample_astray;  // a sample's stack went into the data below kStackFloor, or it
                          // jumped through a made-up index (emu_note_bad_index)
static uint8_t *check_skip;   // per hook: its original never returns by RTS, so isn't checked
static unsigned *check_reports;  // per hook: differences printed
static unsigned long *hook_calls;  // per hook: calls of the port (or its original instead)
static unsigned long checks, check_diffs;
static unsigned long waits_taken, nmis_taken;
static unsigned long originals_run, originals_resumed;  // for lag NMIs in ported code

static int in_game(void) {
  return game && !in_handler && !in_check && coro_current() == game;
}

// Stops the C side with a reason. Inside the game it returns only once emu_load has put back
// an earlier state (for a replay); until then the game stays parked here.
static void fail(const char *fmt, ...) {
  if(!failed) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(fail_msg, sizeof(fail_msg), fmt, ap);
    va_end(ap);
    failed = 1;
  }
  while(failed && in_game()) {
    coro_yield();
  }
}

static void record(const Cpu6502 *c, const char *ported) {
  EmuTraceEntry e = {c->cycles, c->pc, c->a, c->x, c->y, c->s, c->p, g_nes.bank_regs[6], ported};
  recent[recent_pos++ % kRecentSize] = e;
  if(tracing) {
    if(trace_len == trace_cap) {
      trace_cap = trace_cap ? trace_cap * 2 : 4096;
      trace_buf = realloc(trace_buf, trace_cap * sizeof(*trace_buf));
    }
    trace_buf[trace_len++] = e;
  }
}

void emu_push(uint8_t v) {
  cpu_bus_write(0x100 | g_nes.cpu.s--, v);
}

uint8_t emu_pull(void) {
  return cpu_bus_read(0x100 | ++g_nes.cpu.s);
}

// ---- Where the NMI goes --------------------------------------------------------------------

int emu_is_wait_loop(uint16_t pc) {
  // wait_vblank after it sets the flag, its spin; the spin on nmi_requests; and the jmp to
  // itself where ending_sequence (6:B2C6) stops.
  return (pc >= 0xEBA2 && pc <= 0xEBA7) || (pc >= 0xEBB4 && pc <= 0xEBB7) || pc == 0xB2C6;
}

// Which wait an NMI returning to pc came in (0: none).
static int wait_kind(uint16_t pc) {
  return pc >= 0xEBA2 && pc <= 0xEBA7 ? 1 : pc >= 0xEBB4 && pc <= 0xEBB7 ? 2 : pc == 0xB2C6 ? 3 : 0;
}

static int same_registers(void) {
  const Cpu6502 *c = &g_nes.cpu;
  // Under ported code the stack pointers differ, and so does X after TSX (stack_push,
  // stack_pop): X also counts if it is as far from the stack pointer on both sides.
  int same_x = c->x == target.x ||
               (ported_depth && (uint8_t)(c->x - c->s) == (uint8_t)(target.x - target.sp - 3));
  return c->pc == target.return_pc && c->a == target.a && same_x && c->y == target.y &&
         (ported_depth || (uint8_t)(c->s - 3) == target.sp);
}

// The 6502 waits in the same wait loop as the reference, and will stay in it until the NMI.
static int waiting_like(void) {
  const Cpu6502 *c = &g_nes.cpu;
  if(!same_registers()) {
    return 0;
  }
  switch(wait_kind(c->pc)) {
    case 1: return system_flags & 0x02;  // wait_vblank
    case 2: return c->a & nmi_requests;  // the spin on nmi_requests
    default: return 1;                   // the jmp to itself at the end of the game
  }
}

// The highest stack page byte (in $0100 + kStackFloor ...) that isn't compared: below the stack
// pointer, or under ported code.
static uint8_t stack_top(void) {
  return ported_depth ? outer_sp : g_nes.cpu.s;
}

// The C side's memories are the reference's at its NMI, except stack bytes that don't count
// (dead, ported code's, or the NMI's own).
static int same_memories(void) {
  uint8_t ram[sizeof(g_nes.ram)];
  memcpy(ram, g_nes.ram, sizeof(ram));
  for(int i = kStackFloor; i <= stack_top(); i++) {
    ram[0x100 + i] = target.ram[0x100 + i];
  }
  for(int k = 1; k <= 3; k++) {
    ram[0x100 + (uint8_t)(target.sp + k)] = target.ram[0x100 + (uint8_t)(target.sp + k)];
  }
  return !memcmp(ram, target.ram, sizeof(ram)) &&
         !memcmp(g_nes.sram, target.sram, sizeof(g_nes.sram)) &&
         !memcmp(g_nes.ciram, target.ciram, sizeof(g_nes.ciram)) &&
         !memcmp(g_nes.palette, target.palette, sizeof(g_nes.palette)) &&
         !memcmp(g_nes.oam, target.oam, sizeof(g_nes.oam));
}

// The 6502 is where the reference was when the NMI interrupted it: same instruction, registers
// and memories.
static int at_reference_state(void) {
  return same_registers() && same_memories();
}

static int in_bank(const EmuHook *h, uint16_t pc) {
  return pc >= 0xC000 ? h->bank == 7 : nes_prg_offset(pc) == h->bank * 0x4000u + (pc & 0x3FFF);
}

// The reference's next NMI is a lag frame, and the game (not the NMI handler) is running.
static int lag_pending(void) {
  return in_game() && target_set && !target_wait;
}

// The reference's next NMI is a lag frame inside the original code of h.
static int lag_inside(const EmuHook *h) {
  return lag_pending() && target.return_pc >= h->addr && target.return_pc < h->end &&
         in_bank(h, target.return_pc);
}

// The reference's NMI interrupted code that is ported here.
static int lag_in_ported_code(void) {
  for(size_t i = 0; i < hook_count; i++) {
    if(lag_inside(&hooks[i])) {
      return 1;
    }
  }
  return 0;
}

static void take(EmuPlace p);
static const EmuHook *find_hook(uint16_t pc);

// On the way into a wait: everything before is behind the reference's NMI, so if that
// interrupted ported code and the memories match, the NMI is taken here.
static void c_point(void) {
  while(lag_pending() && lag_in_ported_code() && same_memories()) {
    take(kPlaceMemory);
  }
}

// The ported routine (or inline code) at addr, if any.
static const EmuHook *routine_at(uint16_t addr) {
  for(size_t i = 0; i < hook_count; i++) {
    if(hooks[i].addr == addr && in_bank(&hooks[i], addr)) {
      return &hooks[i];
    }
  }
  return NULL;
}

int emu_lag_in(uint16_t addr) {
  if(in_check && ++check_work > check_limit) {
    longjmp(check_abort, 3);  // a loop in C that wouldn't end (a made-up state, say)
  }
  if(free_run && ++guards_since_wait > kSpinGuards && in_game()) {
    guards_since_wait = 0;
    coro_yield();
  }
  if(hook_first[addr] && !in_check) {
    hook_calls[hook_first[addr] - 1]++;  // a call of the port, from C or through its hook
  }
  const EmuHook *h = lag_pending() ? routine_at(addr) : NULL;
  return h && lag_inside(h);
}

unsigned long emu_hook_calls(size_t index) {
  return index < hook_count ? hook_calls[index] : 0;
}

void emu_lag_behind(uint16_t addr) {
  const EmuHook *h = lag_pending() ? routine_at(addr) : NULL;
  while(h && lag_inside(h) && same_memories()) {
    take(kPlaceMemory);
  }
}

// The 6502 is on its way into a wait that will block, and the reference's NMI (a lag frame)
// isn't in the instructions before the wait starts: the NMI's place is behind.
static int at_blocking_wait(void) {
  uint16_t pc = g_nes.cpu.pc, rp = target.return_pc;
  switch(pc) {
    case 0xEB9C: return !(rp >= 0xEB9C && rp < 0xEBA2);  // wait_vblank
    case 0xEBAE: return (nmi_requests & 0x08) && !(rp >= 0xEBAE && rp < 0xEBB2);
    case 0xEBB2: return nmi_requests && rp != 0xEBB2;  // wait_nmi_work_done
    case 0xB2C6: return nes_prg_offset(pc) == kHaltRom;
    default: return 0;
  }
}

// Hands the game over to emu_run_to_nmi, which takes the NMI here and runs its handler.
static void take(EmuPlace p) {
  place = p;
  target_set = 0;
  nmis_taken++;
  waits_taken += p == kPlaceWait6502 || p == kPlaceWaitC;
  if((p == kPlaceLate || p == kPlaceMemory) && !near_count) {
    snprintf(late_note, sizeof(late_note),
             "the 6502 never ran $%04X: that code is ported, or the C side went elsewhere",
             target.return_pc);
  } else if(p == kPlaceLate || p == kPlaceMemory) {
    const Cpu6502 *c = &near_cpu;
    snprintf(late_note, sizeof(late_note),
             "the 6502 ran $%04X %d times, the last with a=%02X x=%02X y=%02X sp=%02X (the "
             "reference a=%02X x=%02X y=%02X sp=%02X)%s",
             target.return_pc, near_count, c->a, c->x, c->y, (uint8_t)(c->s - 3), target.a,
             target.x, target.y, target.sp,
             c->a == target.a && c->x == target.x && c->y == target.y ? ", memory differing" : "");
  }
  coro_yield();
}

const char *emu_late_note(void) {
  return late_note;
}

// At an instruction boundary in the game: takes the reference's NMIs that go here. Returns 1
// after a failure that a replay (emu_load) has since undone: the state is another one now.
static int nmi_point(void) {
  const Cpu6502 *c = &g_nes.cpu;
  for(;;) {
    if(!target_set) {
      return 0;
    }
    if(target_wait) {
      if(!waiting_like()) {
        break;
      }
      take(kPlaceWait6502);
      continue;
    }
    if(c->pc == target.return_pc) {
      near_count++;  // for late_note
      near_cpu = *c;
      if(at_reference_state()) {
        take(kPlaceExact);
        continue;
      }
    }
    // A ported wait here takes the NMI itself (emu_take_late_nmis).
    if(!at_blocking_wait() || (hook_first[c->pc] && find_hook(c->pc))) {
      break;
    }
    if(!c_ran) {
      fail("never reached the reference's state where its NMI interrupted the game at $%04X "
           "(a=%02X x=%02X y=%02X sp=%02X); it is waiting at $%04X instead",
           target.return_pc, target.a, target.x, target.y, target.sp, c->pc);
      return 1;
    }
    take(lag_in_ported_code() && same_memories() ? kPlaceMemory : kPlaceLate);
  }
  if(c->cycles - run_start > kMaxCycles) {
    fail("never got to the reference's NMI (it %s $%04X with a=%02X x=%02X y=%02X); ten frames "
         "on, the C side is at $%04X with a=%02X x=%02X y=%02X",
         target_wait ? "waits at" : "interrupted the game at", target.return_pc, target.a,
         target.x, target.y, c->pc, c->a, c->x, c->y);
    return 1;
  }
  return 0;
}

static uint16_t step(void);

// A wait for the NMI in a check that runs them (emu_fuzz_port with waits): the NMI comes at once,
// with the handler that's in use, and the A button changes, so that loops waiting for a press
// and a release end. If the original and the port wait alike they see the same NMIs; if not,
// frame_counter and much else will differ.
static void made_up_nmi(void) {
  Cpu6502 *c = &g_nes.cpu;
  uint16_t pc = c->pc;
  uint8_t s = c->s;
  cpu_nmi(c);
  while(!failed && !(c->pc == pc && c->s == s)) {
    if(c->cycles - check_start > check_limit) {
      longjmp(check_abort, 2);
    }
    step();
  }
  if(failed) {
    longjmp(check_abort, 1);  // made-up data ran into an opcode that freezes the 6502
  }
  g_nes.pad[0] ^= 0x01;
}

void emu_spin(unsigned passes_per_frame) {
  if(free_run && in_game() && ++spin_passes >= passes_per_frame) {
    spin_passes = 0;
    guards_since_wait = 0;
    coro_yield();
  }
}

void emu_take_late_nmis(void) {
  if(in_check) {
    if(!check_nmis) {
      longjmp(check_abort, 1);  // the original being checked waits: no check this time
    }
    return;
  }
  c_point();
  while(lag_pending()) {
    take(kPlaceLate);
  }
}

void emu_wait_for_nmi(uint16_t spin) {
  if(in_check) {
    if(!check_nmis) {
      longjmp(check_abort, 1);
    }
    made_up_nmi();
    return;
  }
  if(!in_game()) {
    fail("ported code waited for the NMI outside the game (in the NMI handler?)");
    return;
  }
  if(free_run) {
    guards_since_wait = 0;
    coro_yield();
    return;
  }
  if(!target_wait) {
    fail("the reference's NMI interrupted the game at $%04X, but the C side waits in its "
         "$%04X loop",
         target.return_pc, spin);
  } else if(wait_kind(target.return_pc) != wait_kind(spin)) {
    fail("the reference waits at $%04X, the C side in its $%04X loop", target.return_pc, spin);
  }
  take(kPlaceWaitC);
}

// ---- Running code --------------------------------------------------------------------------

void emu_set_hooks(const EmuHook *list, size_t count) {
  for(size_t i = 0; i < hook_count; i++) {
    hook_first[hooks[i].addr] = 0;
  }
  hooks = list;
  hook_count = count;
  hook_rom = realloc(hook_rom, (count + 1) * sizeof(*hook_rom));
  hook_next = realloc(hook_next, (count + 1) * sizeof(*hook_next));
  free(check_skip);
  free(check_reports);
  free(hook_calls);
  check_skip = calloc(count + 1, sizeof(*check_skip));
  check_reports = calloc(count + 1, sizeof(*check_reports));
  hook_calls = calloc(count + 1, sizeof(*hook_calls));
  for(size_t i = count; i-- > 0;) {
    const EmuHook *h = &list[i];
    hook_rom[i] = (uint32_t)h->bank * 0x4000 + (h->addr & 0x3FFF);
    if(h->fn) {
      hook_next[i] = hook_first[h->addr];
      hook_first[h->addr] = (uint16_t)(i + 1);
    }
  }
}

static const EmuHook *find_hook(uint16_t pc) {
  for(unsigned i = hook_first[pc]; i; i = hook_next[i - 1]) {
    if(hook_rom[i - 1] == nes_prg_offset(pc)) {
      return &hooks[i - 1];
    }
  }
  return NULL;
}

// ---- Checking ports against their originals (emu_set_port_check) --------------------------

// Whether h's original ends with RTI, so the flags lie on top of its return address, which is
// the address itself (an RTS adds 1 to what a JSR pushed).
static int ends_with_rti(const EmuHook *h) {
  return h->addr == kNmiHandler;
}

// Runs the original of h from its first instruction until it returns past its caller's
// return address, with the ported routines it calls in C as usual.
static void run_original(const EmuHook *h) {
  Cpu6502 *c = &g_nes.cpu;
  uint8_t s0 = c->s;
  int rti = ends_with_rti(h);
  uint8_t s1 = (uint8_t)(s0 + 2 + rti);  // the stack pointer after it returns
  uint16_t ret = (uint16_t)((g_nes.ram[0x100 | (uint8_t)(s1 - 1)] |
                             g_nes.ram[0x100 | s1] << 8) + !rti);
  check_start = c->cycles;
  if(cpu_step(c)) {  // the original's first instruction, where the hook is
    longjmp(check_abort, 1);
  }
  while(!(c->pc == ret && c->s == s1)) {
    if(failed || emu_is_wait_loop(c->pc)) {
      longjmp(check_abort, 1);
    }
    sample_astray |= c->s < kStackFloor;
    if(c->cycles - check_start > check_limit) {
      longjmp(check_abort, 2);
    }
    step();
  }
  // Made-up data ran into an unknown opcode in original code a port called, and the port's caller
  // happened to end up where the original returns to.
  if(failed) {
    longjmp(check_abort, 1);
  }
}

// Runs h's original on a copy of the state and puts back the state; *out gets the result.
// Returns 0 if the original waited for the NMI or never returned.
static int check_original(const EmuHook *h, Nes *out) {
  static Nes saved;
  saved = g_nes;
  int depth = ported_depth, ran = c_ran;
  uint8_t osp = outer_sp;
  volatile int ok = 0;
  in_check = 1;
  check_work = 0;
  switch(setjmp(check_abort)) {
    case 0:
      run_original(h);
      *out = g_nes;
      ok = 1;
      break;
    case 2:
      check_skip[h - hooks] = 1;
      break;
    default:
      break;
  }
  in_check = 0;
  failed = 0;
  g_nes = saved;
  ported_depth = depth;
  c_ran = ran;
  outer_sp = osp;
  return ok;
}

// Describes in `what` how the state after the port (g_nes) differs from the state after the
// original (*o); returns 0 if it doesn't.
static size_t port_diff(const Nes *o, int waited, char *what, size_t size) {
  const Nes *n = &g_nes;
  size_t len = 0;
  what[0] = 0;
#define DIFF(...) \
  (len < size ? len += (size_t)snprintf(what + len, size - len, __VA_ARGS__) : 0)
  if(waited) {
    DIFF(" (the port waited for the NMI, the original doesn't)");
  }
  const Cpu6502 *c = &n->cpu, *e = &o->cpu;
  uint8_t flags = kFlagN | kFlagZ | kFlagC;
  if(c->a != e->a) DIFF(" a=%02X/%02X", c->a, e->a);
  if(c->x != e->x) DIFF(" x=%02X/%02X", c->x, e->x);
  if(c->y != e->y) DIFF(" y=%02X/%02X", c->y, e->y);
  if(c->s != e->s) DIFF(" sp=%02X/%02X", c->s, e->s);
  if(c->pc != e->pc) DIFF(" pc=%04X/%04X", c->pc, e->pc);
  if((c->p ^ e->p) & flags) DIFF(" p=%02X/%02X", c->p & flags, e->p & flags);
  int count = 0;
  for(int a = 0; a < 0x800; a++) {
    int dead = a >= 0x100 + kStackFloor && a <= 0x100 + c->s;
    if(!dead && n->ram[a] != o->ram[a] && count++ < 8) {
      DIFF(" $%04X=%02X/%02X", a, n->ram[a], o->ram[a]);
    }
  }
  const struct {
    const char *name;
    const uint8_t *mine, *theirs;
    size_t size;
  } kMems[] = {
    {"sram", n->sram, o->sram, sizeof(n->sram)},
    {"nametables", n->ciram, o->ciram, sizeof(n->ciram)},
    {"palette", n->palette, o->palette, sizeof(n->palette)},
    {"oam", n->oam, o->oam, sizeof(n->oam)},
    {"mmc3 banks", n->bank_regs, o->bank_regs, sizeof(n->bank_regs)},
  };
  for(size_t i = 0; i < sizeof(kMems) / sizeof(kMems[0]); i++) {
    if(memcmp(kMems[i].mine, kMems[i].theirs, kMems[i].size)) {
      DIFF(" %s", kMems[i].name);
    }
  }
  if(n->bank_select != o->bank_select || n->mirroring != o->mirroring ||
     n->ram_protect != o->ram_protect) {
    DIFF(" mmc3 registers");
  }
  if(n->ppu_ctrl != o->ppu_ctrl || n->ppu_mask != o->ppu_mask || n->oam_addr != o->oam_addr ||
     n->vram_addr != o->vram_addr || n->vram_temp != o->vram_temp || n->fine_x != o->fine_x ||
     n->write_toggle != o->write_toggle || n->ppu_latch != o->ppu_latch) {
    DIFF(" ppu registers");
  }
  if(memcmp(n->pad_shift, o->pad_shift, sizeof(n->pad_shift)) || n->pad_strobe != o->pad_strobe) {
    DIFF(" controllers");
  }
  // Entry by entry: NesAccess has a padding byte, whatever the stack held where it was built.
  int apu_differs = n->apu_count != o->apu_count;
  for(uint32_t i = 0; i < n->apu_count && i < kApuLogMax && !apu_differs; i++) {
    apu_differs = n->apu_log[i].addr != o->apu_log[i].addr ||
                  n->apu_log[i].value != o->apu_log[i].value;
  }
  if(apu_differs) {
    DIFF(" apu writes");
  }
  if(n->ppu_read_pos != o->ppu_read_pos) {
    DIFF(" ppu reads");
  }
#undef DIFF
  return len;
}

// Prints how the state after the port differs from the state after the original.
static void compare_port(const EmuHook *h, const Cpu6502 *in, const Nes *o, int waited) {
  char what[600];
  size_t len = port_diff(o, waited, what, sizeof(what));
  checks++;
  if(len) {
    check_diffs++;
    if(check_reports[h - hooks]++ < kCheckReports) {
      fprintf(stderr, "dv: port check: %s (called with a=%02X x=%02X y=%02X) differs, "
                      "port/original:%s\n",
              h->name, in->a, in->x, in->y, what);
    }
  }
}

void emu_set_port_check(int on) {
  port_check = on;
}

void emu_port_check_counts(unsigned long *checked, unsigned long *differed) {
  *checked = checks;
  *differed = check_diffs;
}

// What the RTS (kEmuRts) or RTI (kEmuRti) ending a ported routine does.
static void return_from(uint16_t how) {
  Cpu6502 *c = &g_nes.cpu;
  if(how == kEmuRti) {
    c->p = emu_pull() & ~(kFlagB | kFlagU);
  }
  uint8_t lo = emu_pull();
  c->pc = (uint16_t)((lo | emu_pull() << 8) + (how == kEmuRts));
  c->cycles += 6;
}

static uint32_t fuzz_rng;

static uint8_t fuzz_byte(void) {
  fuzz_rng ^= fuzz_rng << 13;  // xorshift32
  fuzz_rng ^= fuzz_rng >> 17;
  fuzz_rng ^= fuzz_rng << 5;
  return (uint8_t)(fuzz_rng >> 24);
}

// For a fuzz through waits, what isn't under test made sane, so that the NMIs run clean. A
// made-up VRAM queue mostly never ends, nor does a sprite list without its end mark. Made-up
// sound data reads the stack through its pointers, where the original and the port differ.
static void fuzz_quiet_sound(void) {
  memset(sound_channels, 0, 0xD8);
  repeat_sound = 0;
  sound_table_ptr[0] = 0x00;
  sound_table_ptr[1] = 0x80;
}

static void fuzz_quiet_nmis(void) {
  fuzz_quiet_sound();
  stack_index = 0xFF;
  vram_queue_len = 0;
  object_sprite_list[0] = 0xFF;
}

// A room with its hotspots in bank 2 and the cursor on one of them, in one of the scenes their
// checks look for half the time, so that the checks run with real kinds and ids. No command half the
// time, and a real script table, for pick_default_command.
static void fuzz_hotspot_state(const EmuHook *h) {
  static const uint8_t kScenes[] = {0x00, 0x12, 0x40, 0x43, 0x44, 0x46, 0x49, 0x4C, 0x5D,
                                    0x5E, 0x67, 0x68, 0x6C, 0x6D, 0x73, 0x77, 0x78, 0x7A};
  g_nes.bank_regs[6] = 0x04;
  g_nes.bank_regs[7] = 0x05;
  current_bank = 0x02;
  stack_index = (uint8_t)(0xE0 | (fuzz_byte() & 0x1F));
  fuzz_quiet_sound();
  room = fuzz_byte() % 78;
  uint16_t list = (uint16_t)(cpu_bus_read((uint16_t)(0x9C00 + room * 2)) |
                             cpu_bus_read((uint16_t)(0x9C01 + room * 2)) << 8);
  int count = 0;
  while(count < 64 && cpu_bus_read((uint16_t)(list + count * 6)) != 0xFF) {
    count++;
  }
  if(count) {
    uint16_t record = (uint16_t)(list + fuzz_byte() % count * 6);
    uint8_t x1 = cpu_bus_read(record), x2 = cpu_bus_read((uint16_t)(record + 1));
    uint8_t y1 = cpu_bus_read((uint16_t)(record + 2)), y2 = cpu_bus_read((uint16_t)(record + 3));
    cursor_x = (uint8_t)(x1 + (x2 > x1 ? fuzz_byte() % (x2 - x1) : 0));
    cursor_y = (uint8_t)(y1 + (y2 > y1 ? fuzz_byte() % (y2 - y1) : 0));
  }
  if(fuzz_byte() & 1) {
    scene = kScenes[fuzz_byte() % sizeof(kScenes)];
  }
  if(fuzz_byte() & 1) {
    command = 0xFF;
  }
  if(strcmp(h->name, "collect_room_sprites")) {
    script_table = fuzz_byte() % 6;
  }
}

// Save RAM on, a real file and copy, with the pointers save_copy_address leaves for it. Half the time
// that copy is good (its state, signature and sum), with the sum so far right, and the same for
// the backup. Otherwise these mostly see an empty file, or write who knows where.
static void fuzz_save_state(void) {
  int file = fuzz_byte() % 3;
  int copy = fuzz_byte() & 1 ? 0 : fuzz_byte() % 4;
  unsigned base = 0x10u + file * 0x800u + copy * 0x200u;
  save_file = (uint8_t)file;
  g_ram[0x64] = (uint8_t)copy;
  g_nes.ram_protect = 0x80;
  data_ptr[0] = (uint8_t)base;
  data_ptr[1] = (uint8_t)(0x60 + (base >> 8));
  g_ram[0x57] = (uint8_t)(base + 0x120);
  g_ram[0x58] = (uint8_t)(0x60 + ((base + 0x120) >> 8));
  g_ram[0x59] = (uint8_t)(base + 0x126);
  g_ram[0x5A] = (uint8_t)(0x60 + ((base + 0x126) >> 8));
  g_ram[0x5B] = (uint8_t)base;
  g_ram[0x5C] = (uint8_t)(0x62 + (base >> 8));
  if(fuzz_byte() & 1) {
    uint8_t *state = g_nes.sram + base;
    unsigned sum = 0;
    for(int i = 0; i < 0x110; i++) {
      state[i] = fuzz_byte();
      sum += state[i];
    }
    memcpy(state + 0x120, "DEJAVU", 6);
    state[0x126] = (uint8_t)sum;
    state[0x127] = (uint8_t)(sum >> 8);
    g_ram[0x65] = (uint8_t)sum;
    g_ram[0x66] = (uint8_t)(sum >> 8);
  }
  if(fuzz_byte() & 1) {
    uint8_t *backup = g_nes.sram + 0x1E00;
    unsigned sum = 0;
    for(int i = 0; i < 0x110; i++) {
      backup[i] = fuzz_byte();
      sum += backup[i];
    }
    backup[0x136] = (uint8_t)sum;
    backup[0x137] = (uint8_t)(sum >> 8);
  }
}

// The sound driver's routines with its data mapped, the loop indexes in their slots' stacks and, most
// of the time, a real slot in X. Half the time a real sound to start, and all slots playing, due for
// a command, from somewhere in the real streams. Random bytes rarely get as far as a note.
static void fuzz_sound_state(void) {
  static const uint8_t kSlots[] = {0x00, 0x18, 0x30, 0x48, 0x60, 0x78, 0x90, 0xA8, 0xC0};
  Cpu6502 *c = &g_nes.cpu;
  g_nes.bank_regs[6] = 0x0A;
  g_nes.bank_regs[7] = 0x0B;
  current_bank = 5;
  sound_table_ptr[0] = 0x00;
  sound_table_ptr[1] = 0x80;
  for(int i = 0; i < 9; i++) {
    sound_channels[kSlots[i] + 15] = (uint8_t)(i << 4 | (fuzz_byte() & 0x0C));
    sound_channels[kSlots[i] + 21] = fuzz_byte() & 3;  // the triangle's mode: a random one is rarely 1 or 2
  }
  if(fuzz_byte() & 7) {
    c->x = kSlots[fuzz_byte() % 9];
  }
  if(fuzz_byte() & 1) {
    uint8_t sound = fuzz_byte() % 50;
    c->a = (uint8_t)(sound + 1);
    g_ram[0x44] = sound;
    g_ram[0x45] = cpu_bus_read((uint16_t)(0x8000 + sound * 2));
    g_ram[0x46] = cpu_bus_read((uint16_t)(0x8001 + sound * 2));
    for(int i = 0; i < 9; i++) {
      uint16_t stream = (uint16_t)(0x8064 + (fuzz_byte() << 5 | (fuzz_byte() & 0x1F)) % 0x199C);
      sound_channels[kSlots[i]] |= 0x80;
      sound_channels[kSlots[i] + 1] = (uint8_t)stream;
      sound_channels[kSlots[i] + 2] = (uint8_t)(stream >> 8);
      sound_channels[kSlots[i] + 5] = 0;
      sound_channels[kSlots[i] + 6] = 0;
      sound_channels[kSlots[i] + 7] = 1;
      sound_channels[kSlots[i] + 8] = 0;
    }
  }
}

// For routines a random state almost never gets anywhere in, and which no movie runs, a state with
// just enough sense for their code to run: the target on the container stack, whose search
// otherwise never ends, and a switch key on one of the tables. Likewise a real layout, special
// scene, hotspot or save file.
static void fuzz_help_routine(const EmuHook *h) {
  Cpu6502 *c = &g_nes.cpu;
  if(!strcmp(h->name, "close_container") || !strcmp(h->name, "op_47")) {
    int entry = (fuzz_byte() % 7) * 2;
    container_stack[entry] = script_table;
    container_stack[entry + 1] = object_id;
  } else if(!strcmp(h->name, "op_32_body") || !strcmp(h->name, "op_32")) {
    switch_key[0] = (uint8_t)(0x02 + fuzz_byte() % 3);
  } else if(!strcmp(h->name, "draw_layout_number")) {
    c->a = fuzz_byte() % 16;
  } else if(h->addr >= kFirstHotspotRoutine && h->addr < kFirstSpecialScene && h->bank == 7) {
    fuzz_hotspot_state(h);
  } else if(h->addr >= kFirstSpecialScene && h->addr < kFirstWait && h->bank == 7) {
    special_scene = fuzz_byte() % 27;
  } else if(h->addr >= kFirstSaveRoutine && h->addr < kAfterSaveRoutines && h->bank == 7) {
    fuzz_save_state();
  } else if(h->addr >= kFirstSoundRoutine && h->addr < kAfterSoundRoutines && h->bank == 7) {
    fuzz_sound_state();
  }
}

static const char *fuzz_trace_name;
static unsigned fuzz_trace_sample;

void emu_set_fuzz_trace(const char *name, unsigned sample) {
  fuzz_trace_name = name;
  fuzz_trace_sample = sample;
}

// Prints what ran since the trace was last cleared, and goes on tracing what runs next.
static void print_fuzz_trace(const char *routine, unsigned sample, int waits, const char *which) {
  fprintf(stderr, "dv: port fuzz trace: %s, sample %u%s, %s:\n", routine, sample,
          waits ? " (through waits)" : "", which);
  for(size_t i = 0; i < trace_len; i++) {
    const EmuTraceEntry *t = &trace_buf[i];
    if(t->ported) {
      fprintf(stderr, "  %04X ported: %s\n", t->pc, t->ported);
    } else {
      fprintf(stderr, "  %04X a=%02X x=%02X y=%02X s=%02X p=%02X r6=%02X\n", t->pc, t->a, t->x,
              t->y, t->s, t->p, t->bank);
    }
  }
  trace_len = 0;
}

void emu_fuzz_port(size_t index, unsigned samples, uint32_t seed, int waits,
                   EmuFuzzCounts *counts) {
  static Nes base, start, original;
  const EmuHook *h = &hooks[index];
  Cpu6502 *c = &g_nes.cpu;
  memset(counts, 0, sizeof(*counts));
  if(!h->fn) {
    return;
  }
  base = g_nes;
  int depth = ported_depth, ran = c_ran;
  uint8_t osp = outer_sp;
  check_nmis = waits;
  const unsigned long limit = waits ? kCheckLimitWaits : kCheckLimit;
  fuzz_rng = seed * 2654435761u | 1;
  for(unsigned i = 0; i < samples; i++) {
    // A routine that never gets anywhere on made-up data (main, RLE data) costs the most.
    if(waits && i == kFuzzGiveUp && !counts->compared) {
      counts->skipped += samples - i;
      break;
    }
    g_nes = base;
    for(int a = 0; a < 0x800; a++) {
      g_nes.ram[a] = fuzz_byte();
    }
    // The game never sets PRG mode 1 (bit 6), which swaps out the code at $C000-$DFFF under the
    // original while the port carries on.
    bank_select_shadow &= 0xBF;
    // A routine in a switched bank runs with that bank mapped and current_bank saying so, and
    // the bank stack where it lives ($07E0-$07FF, above the sound channels, with room for a
    // few pushes), as the far call that got there leaves them, and with sound quiet. Otherwise
    // the next pop_bank (play_sound's, or the NMI's sound_update's) maps some other bank under
    // the original, while the port carries on in C: press_a differed in 172 of 173 samples with
    // a made-up current_bank, and a few times more with a made-up stack_index or sound
    // pointers, when sound data overwrote the bank it had pushed.
    if(h->bank != 7) {
      g_nes.bank_regs[6] = (uint8_t)(h->bank << 1);
      g_nes.bank_regs[7] = (uint8_t)((h->bank << 1) + 1);
      current_bank = h->bank;
      stack_index = (uint8_t)(0xE0 | (fuzz_byte() & 0x1F));
      fuzz_quiet_sound();
    }
    if(waits) {
      fuzz_quiet_nmis();
    }
    c->a = fuzz_byte();
    c->x = fuzz_byte();
    c->y = fuzz_byte();
    c->p = fuzz_byte() & ~(kFlagB | kFlagU);
    fuzz_help_routine(h);
    c->s = 0xF0;
    if(ends_with_rti(h)) {  // as an NMI does, from $0000
      emu_push(0x00);
      emu_push(0x00);
      emu_push(c->p);
    } else {  // as emu_call does: the routine returns to $0000
      emu_push(0xFF);
      emu_push(0xFF);
    }
    c->pc = h->addr;
    start = g_nes;
    volatile int phase = 0;  // 1: the original ran; 2: the port ran too; 3: not comparable
    volatile int why = 0;    // what stopped the port, after the original ran
    in_check = 1;
    check_work = 0;
    check_limit = limit;
    sample_astray = 0;
    int trace = fuzz_trace_name && i == fuzz_trace_sample && !strcmp(fuzz_trace_name, h->name);
    if(trace) {
      trace_len = 0;
      tracing = 1;
    }
    switch(setjmp(check_abort)) {
      case 0: {
        run_original(h);
        if(trace) {
          print_fuzz_trace(h->name, i, waits, "the original");
        }
        original = g_nes;
        phase = 1;
        g_nes = start;
        check_work = 0;
        check_start = c->cycles;
        // The original's run decides what is too long. The port's own guards count as work where the
        // original's own code doesn't, so it gets more room: print_message once ended at 996,728 under
        // the original and passed the limit of 1,000,000 under the port.
        check_limit = limit * 2;
        unfair_read = 0;
        uint16_t next = h->fn();
        if(failed) {
          longjmp(check_abort, 1);
        }
        if(next != kEmuRts && next != kEmuRti) {
          phase = 3;  // it carries on elsewhere, as jump_table does
          break;
        }
        return_from(next);
        // It returned past its own caller (the script opcodes that end the script), into
        // made-up stack bytes. The original went on through random code from there.
        unfair_read |= c->s != 0xF0;
        phase = 2;
        break;
      }
      case 1:  // waited for the NMI
        break;
      default:  // ran too long
        why = 1;
        break;
    }
    if(trace) {
      print_fuzz_trace(h->name, i, waits, phase ? "the port" : "the original, which waited");
      tracing = 0;
    }
    in_check = 0;
    failed = 0;
    ported_depth = depth;
    c_ran = ran;
    outer_sp = osp;
    // The stack below kStackFloor is the sound driver's loop stacks, compared as data. A made-up
    // script full of opcodes past the table gets there: each resumes as original code, whose JMP
    // script_loop comes back through the hook, a level deeper. Real scripts have none. And past a
    // jump table's end, code runs from whatever the bytes there say, on a stack that differs
    // under C frames: nothing to learn from comparing that.
    if(phase == 0 || phase == 3 || unfair_read || sample_astray) {
      counts->skipped++;
      continue;
    }
    char what[600];
    size_t len = phase == 1 && why ? (size_t)snprintf(what, sizeof(what), " (the port didn't "
                                                      "return, the original did)")
                                   : port_diff(&original, phase == 1, what, sizeof(what));
    counts->compared++;
    if(len && counts->differed++ < kCheckReports) {
      fprintf(stderr, "dv: port fuzz: %s (sample %u, called with a=%02X x=%02X y=%02X) differs, "
                      "port/original:%s\n",
              h->name, i, start.cpu.a, start.cpu.x, start.cpu.y, what);
    }
  }
  g_nes = base;
  check_nmis = 0;
  check_limit = kCheckLimit;
}

// Runs a ported routine in place of the original at pc, then carries on as it says.
// Returns its kEmuRts / kEmuRti / jump address.
static uint16_t run_hook(const EmuHook *h) {
  Cpu6502 *c = &g_nes.cpu;
  record(c, h->name);
  if(!in_check) {
    hook_calls[h - hooks]++;  // also for ports without a guard (main, jump_table, ...)
  }
  Nes *expected = NULL;
  Cpu6502 in = *c;
  unsigned long nmis = nmis_taken, waits = waits_taken;
  if(port_check && !in_check && !check_skip[h - hooks]) {
    expected = malloc(sizeof(Nes));
    if(!check_original(h, expected)) {
      free(expected);
      expected = NULL;
    }
  }
  if(ported_depth++ == 0) {
    outer_sp = c->s;
  }
  if(ported_depth > kMaxPortedDepth) {
    fail("ported routines nest %d deep (in %s): a ported routine calling itself without end?",
         ported_depth, h->name);
  }
  c_ran = 1;
  uint16_t next = h->fn();
  c_ran = 1;
  ported_depth--;
  if(next == kEmuRts || next == kEmuRti) {
    return_from(next);
  } else {
    c->pc = next;
    check_skip[h - hooks] = 1;  // it doesn't return by RTS: the check can't tell when it ends
  }
  // An NMI taken meanwhile for a lag frame makes the two runs incomparable.
  if(expected && (nmis == nmis_taken || waits != waits_taken)) {
    compare_port(h, &in, expected, waits != waits_taken);
  }
  free(expected);
  return next;
}

// The 6502 ran an opcode that freezes it, which takes a glitch. The game on its own freezes as the
// console does, for good: frames still pass, with no NMI, so the window stays alive. With a reference
// or in a check it's a failure, as nothing can line up with that.
static void cpu_jammed(const Cpu6502 *c) {
  if(free_run && !in_check && in_game()) {
    jammed = 1;
    for(;;) {
      coro_yield();
    }
  }
  fail("the 6502 froze on opcode $%02X at $%04X", cpu_bus_read(c->pc), c->pc);
}

// Runs one instruction, or a ported routine in place of the code at pc. Returns what a ported
// routine returned, or kEmuRts for an instruction. The original runs instead of a ported
// routine when the reference's next NMI is a lag frame inside it.
static uint16_t step(void) {
  Cpu6502 *c = &g_nes.cpu;
  const EmuHook *h = hook_first[c->pc] ? find_hook(c->pc) : NULL;
  if(h && !lag_inside(h)) {
    return run_hook(h);
  }
  originals_run += h != NULL;
  record(c, NULL);
  if(cpu_step(c)) {
    cpu_jammed(c);
  }
  return kEmuRts;
}

// One instruction of original code, even where a ported routine is.
static void step_original(void) {
  Cpu6502 *c = &g_nes.cpu;
  record(c, NULL);
  if(cpu_step(c)) {
    cpu_jammed(c);
  }
}

// Whether addr is the stack page, or a mirror of it. A made-up pointer that gets there sees or changes
// what differs between an original and a port: the original's JSRs leave return addresses there,
// which a port doesn't push, so everything on the stack lies elsewhere.
static int in_stack_page(uint16_t addr) {
  unsigned ram = addr < 0x2000 ? addr & 0x7FFu : 0xFFFFu;
  return ram >= 0x100u + kStackFloor && ram <= 0x1FFu;
}

// In a check, before the 6502 runs an instruction: the same for original code a port calls, when it
// reads or writes through a pointer, LDA (zp),Y and the like. Any such use makes the sample unfair.
static void note_pointer_use(const Cpu6502 *c) {
  if(c->pc >= 0x8000 && (g_nes.prg[nes_prg_offset(c->pc)] & 0x1F) == 0x11) {
    uint8_t zp = g_nes.prg[nes_prg_offset((uint16_t)(c->pc + 1))];
    uint16_t base = (uint16_t)(g_nes.ram[zp] | g_nes.ram[(uint8_t)(zp + 1)] << 8);
    unfair_read |= in_stack_page((uint16_t)(base + c->y));
  }
}

// How exec stops, with stop_s the stack pointer before any return address was pushed.
enum {
  kExecCall,      // at stop_pc with the stack pointer back at stop_s
  kExecJump,      // when the stack pointer rises above stop_s
  kExecUnwinding  // whichever comes first
};

// Runs 6502 code from g_nes.cpu.pc: the game (stop_s < 0: never returns), or a call or jump from
// C, until it stops as `mode` says. With original_first, the first instruction is the
// original's even if a ported routine is there.
static void exec(uint16_t stop_pc, int stop_s, int original_first, int mode) {
  const Cpu6502 *c = &g_nes.cpu;
  while(!failed) {
    if((mode != kExecJump && c->pc == stop_pc && c->s == stop_s) ||
       (mode != kExecCall && c->s > stop_s)) {
      return;
    }
    // In a check, original code that a port called has returned past the port: made-up data
    // sent it astray (a sound pointer into a jump table, say), and an RTS further up took it to
    // the port's caller. The original can do that; the port can't follow, so this sample isn't
    // fair to compare.
    if(in_check && stop_s >= 0 && c->s > stop_s) {
      unfair_read = 1;
      longjmp(check_abort, 1);
    }
    if(in_game() && nmi_point()) {
      continue;
    }
    // On the game's own, 6502 code only runs for a glitch, and may never reach a wait: as on the
    // console, the frame ends anyway, and the NMI comes where it is.
    if(free_run && in_game() && c->cycles - frame_start_cycle >= kFreeRunFrameCycles) {
      guards_since_wait = 0;
      coro_yield();
      continue;
    }
    if(in_check && c->cycles - check_start > check_limit) {
      longjmp(check_abort, 2);  // original code a port called during a check runs on and on
    }
    sample_astray |= in_check && c->s < kStackFloor;
    if(in_check) {
      note_pointer_use(c);
    }
    if(original_first) {
      original_first = 0;
      step_original();
    } else {
      step();
    }
  }
}

// From the NMI handler's entry to the end of its RTI, in 6502 code or in a ported handler.
// Either way the cycle count ends as the reference's: the handler's sprite-0 wait takes the
// C side a different time, or none.
static void run_nmi_handler(void) {
  Cpu6502 *c = &g_nes.cpu;
  in_handler = 1;
  while(!failed) {
    if(c->pc == kNmiRti) {
      if(target.rti_cycle) {
        c->cycles = target.rti_cycle;
      }
      step();
      break;
    }
    if(step() == kEmuRti) {
      if(target.rti_cycle) {
        c->cycles = target.rti_cycle + 6;
      }
      Cpu6502 rti = *c;
      rti.pc = kNmiRti;
      record(&rti, "nmi's RTI");  // a replay's trace starts here, as with original code
      break;
    }
  }
  in_handler = 0;
  in_nmi = 0;
  if(frame_hook && !failed) {
    frame_hook();
  }
}

void emu_set_frame_hook(void (*hook)(void)) {
  frame_hook = hook;
}

int emu_run_frame(char *err, size_t err_size) {
  Cpu6502 *c = &g_nes.cpu;
  free_run = 1;
  if(!failed) {
    if(nmi_pending && !jammed) {
      cpu_nmi(c);
      in_nmi = 1;
      run_nmi_handler();
    } else if(frame_hook) {
      frame_hook();
    }
  }
  if(!failed) {
    frame_start_cycle = c->cycles;
    coro_resume(game);
    nmi_pending = g_nes.ppu_ctrl & 0x80;
  }
  if(failed) {
    snprintf(err, err_size, "%s", fail_msg);
    return -1;
  }
  return 0;
}

static void game_main(void *arg) {
  (void)arg;
  exec(kSentinelPc, -1, 0, kExecCall);
  fail("the game ended");
}

int emu_start(void) {
  coro_destroy(game);
  game = coro_create(game_main, NULL, kGameStack);
  failed = 0;
  jammed = 0;
  in_nmi = in_handler = 0;
  ported_depth = c_ran = 0;
  target_set = 0;
  return game ? 0 : -1;
}

int emu_run_to_nmi(const EmuNmi *n, EmuTaken *t, char *err, size_t err_size) {
  Cpu6502 *c = &g_nes.cpu;
  trace_len = 0;
  target = *n;
  target_wait = emu_is_wait_loop(n->return_pc);
  if(!failed && in_nmi) {
    run_nmi_handler();
  }
  if(!failed) {
    c_ran = 0;
    near_count = 0;
    target_set = 1;
    run_start = c->cycles;
    coro_resume(game);
  }
  if(failed) {
    snprintf(err, err_size, "%s", fail_msg);
    return -1;
  }
  cpu_nmi(c);
  in_nmi = 1;
  c->cycles = n->cycle;
  nes_set_vblank(n->cycle - 10);  // roughly: the NMI follows vblank by one instruction
  t->place = place;
  t->compare = place != kPlaceLate;
  t->compare_regs = place == kPlaceWait6502 || place == kPlaceExact;  // stopped in 6502 code
  t->compare_sp = t->compare_regs && !ported_depth;
  t->stack_top = stack_top();
  return 0;
}

static void call(uint16_t addr, uint16_t ret, int original) {
  Cpu6502 *c = &g_nes.cpu;
  uint8_t s0 = c->s;
  uint16_t pushed = (uint16_t)(ret - 1);  // as JSR pushes its last byte's address
  emu_push(pushed >> 8);
  emu_push(pushed & 0xFF);
  c->pc = addr;
  c->cycles += 6;
  c_ran = 1;
  exec(ret, s0, original, kExecCall);
  c_ran = 1;
}

void emu_call(uint16_t addr) {
  call(addr, kSentinelPc, 0);
}

void emu_jump(uint16_t addr) {
  Cpu6502 *c = &g_nes.cpu;
  uint8_t s0 = c->s;
  c->pc = addr;
  c->cycles += 3;
  c_ran = 1;
  exec(0, s0, 0, kExecJump);
  c->s = s0;
  c_ran = 1;
}

void emu_note_bad_index(void) {
  sample_astray |= in_check;
  bad_indexes += !in_check;
}

unsigned long emu_bad_indexes(void) {
  return bad_indexes;
}

int emu_call_may_unwind(uint16_t addr, uint16_t ret) {
  Cpu6502 *c = &g_nes.cpu;
  uint8_t s0 = c->s;
  uint16_t pushed = (uint16_t)(ret - 1);
  emu_push(pushed >> 8);
  emu_push(pushed & 0xFF);
  c->pc = addr;
  c->cycles += 6;
  c_ran = 1;
  exec(ret, s0, 0, kExecUnwinding);
  c_ran = 1;
  if(c->s > s0) {
    c->s = s0;
    return 1;
  }
  return 0;
}

void emu_call_original(uint16_t addr) {
  originals_run++;
  call(addr, kSentinelPc, 1);
}

void emu_original_counts(unsigned long *run, unsigned long *resumed) {
  *run = originals_run;
  *resumed = originals_resumed;
}

void emu_resume_original(uint16_t addr, int pushed) {
  Cpu6502 *c = &g_nes.cpu;
  originals_resumed++;
  uint8_t saved[8];
  for(int i = 0; i < pushed; i++) {
    saved[i] = emu_pull();
  }
  // As if a JSR had called the routine: its return address under what it pushed.
  uint8_t s0 = c->s;
  uint16_t ret = (uint16_t)(kSentinelPc - 1);
  emu_push(ret >> 8);
  emu_push(ret & 0xFF);
  for(int i = pushed; i-- > 0;) {
    emu_push(saved[i]);
  }
  c->pc = addr;
  c_ran = 1;
  exec(kSentinelPc, s0, 1, kExecUnwinding);
  c_ran = 1;
}

uint8_t emu_adc(uint8_t a, uint8_t m) {
  Cpu6502 *c = &g_nes.cpu;
  unsigned sum = a + m + (c->p & kFlagC);
  uint8_t r = (uint8_t)sum;
  emu_set_flag(kFlagC, sum >> 8);
  emu_set_flag(kFlagV, ~(a ^ m) & (a ^ r) & 0x80);
  emu_set_nz(r);
  return r;
}

uint8_t emu_sbc(uint8_t a, uint8_t m) {
  return emu_adc(a, (uint8_t)~m);
}

void emu_count_work(unsigned n) {
  if(in_check && (check_work += n) > check_limit) {
    longjmp(check_abort, 3);
  }
}

uint8_t emu_read_ind_y(uint8_t zp, uint8_t y) {
  uint8_t lo = cpu_bus_read(zp);
  uint8_t hi = cpu_bus_read((uint8_t)(zp + 1));
  uint16_t addr = (uint16_t)((lo | hi << 8) + y);
  // And what $2002 reads depends on the cycle count, which C doesn't keep.
  unfair_read |= in_stack_page(addr) || addr == 0x2002;
  return cpu_bus_read(addr);
}

void emu_note_loop_stack(uint16_t addr) {
  unfair_read |= addr >= 0x1A0 && addr < 0x200;
}

void emu_write_ind_y(uint8_t zp, uint8_t y, uint8_t v) {
  uint8_t lo = cpu_bus_read(zp);
  uint8_t hi = cpu_bus_read((uint8_t)(zp + 1));
  uint16_t addr = (uint16_t)((lo | hi << 8) + y);
  unfair_read |= in_stack_page(addr) || addr >= 0x8000;
  cpu_bus_write(addr, v);
}

void emu_set_stack(uint8_t s) {
  g_nes.cpu.s = s;
  outer_sp = s;
}

void emu_cmp(uint8_t r, uint8_t m) {
  emu_set_flag(kFlagC, r >= m);
  emu_set_nz((uint8_t)(r - m));
}

void emu_set_nz(uint8_t v) {
  Cpu6502 *c = &g_nes.cpu;
  c->p = (c->p & ~(kFlagN | kFlagZ)) | (v & kFlagN) | (v ? 0 : kFlagZ);
}

void emu_set_flag(uint8_t flag, int on) {
  Cpu6502 *c = &g_nes.cpu;
  c->p = (uint8_t)((c->p & ~flag) | (on ? flag : 0));
}

void emu_set_a(uint8_t a) {
  g_nes.cpu.a = a;
  emu_set_nz(a);
}

void emu_call_a(uint16_t addr, uint8_t a) {
  emu_set_a(a);
  emu_call(addr);
}

// ---- Diagnosis -----------------------------------------------------------------------------

void emu_print_trace(FILE *f, int count) {
  if(count > kRecentSize) {
    count = kRecentSize;
  }
  if((unsigned)count > recent_pos) {
    count = (int)recent_pos;
  }
  for(unsigned i = recent_pos - count; i != recent_pos; i++) {
    const EmuTraceEntry *t = &recent[i % kRecentSize];
    if(t->ported) {
      fprintf(f, "  %10llu  %04X  ported: %s\n", (unsigned long long)t->cycles, t->pc, t->ported);
      continue;
    }
    fprintf(f, "  %10llu  %04X  a=%02X x=%02X y=%02X s=%02X p=%02X  r6=%02X\n",
            (unsigned long long)t->cycles, t->pc, t->a, t->x, t->y, t->s, t->p, t->bank);
  }
}

void emu_set_trace(int on) {
  tracing = on;
}

size_t emu_trace(const EmuTraceEntry **entries) {
  *entries = trace_buf;
  return trace_len;
}

int emu_save(EmuSnapshot *s) {
  if(ported_depth) {
    return -1;
  }
  s->nes = g_nes;
  s->in_nmi = in_nmi;
  return 0;
}

int emu_load(const EmuSnapshot *s) {
  if(ported_depth) {
    return -1;
  }
  g_nes = s->nes;
  in_nmi = s->in_nmi;
  failed = 0;
  return 0;
}
