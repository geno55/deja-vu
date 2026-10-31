// The reference side of the comparison harness: the original game running in Mesen2's
// emulation core, stopped at every NMI entry.
//
// Mesen2 runs on its own thread. A debugger breakpoint on the NMI handler parks that thread
// at each NMI entry until the next ref_run_to_nmi(), so everything here is only called while
// the reference is parked, and the memory it returns is stable until then.
// Only built with DV_HARNESS: the implementation links Mesen2, which is GPL-3.0.
#ifndef DV_REF_H
#define DV_REF_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Ref Ref;

// Memory compared at each NMI entry.
enum {
  kRefRam,      // 2 KB work RAM ($0000-$07FF)
  kRefSram,     // 8 KB battery-backed save RAM ($6000-$7FFF)
  kRefCiram,    // 2 KB nametable RAM inside the console
  kRefPalette,  // 32 bytes of palette RAM
  kRefOam,      // 256 bytes of sprite RAM
  kRefMemCount,
};

// Controller 1 buttons, as bits.
enum {
  kPadA = 0x01, kPadB = 0x02, kPadSelect = 0x04, kPadStart = 0x08,
  kPadUp = 0x10, kPadDown = 0x20, kPadLeft = 0x40, kPadRight = 0x80,
};

// Returns the buttons held for input poll number `poll` (0 = the first poll after power-on).
// Mesen polls the controllers once per frame, on scanline 241, just before the NMI.
// Called on Mesen's thread while ref_run_to_nmi() waits.
typedef uint8_t RefInputFn(void *ctx, uint32_t poll);

// A read or write of a hardware register.
typedef struct RefAccess {
  uint16_t addr;
  uint8_t value;
} RefAccess;

// Where the reference stopped: the first instruction of the NMI handler, not yet run.
typedef struct RefStop {
  uint32_t nmi;        // NMIs taken since power-on; the first is 1
  uint32_t polls;      // input polls so far
  uint32_t ppu_frame;  // Mesen's PPU frame counter
  uint64_t cycle;      // CPU cycles since power-on
  uint64_t rti_cycle;  // the cycle at which the previous NMI's RTI started (0: none yet)
  uint16_t return_pc;  // the address the NMI will return to
  uint8_t a, x, y, p, sp;  // registers at NMI entry (p and the return address are on the stack)
} RefStop;

// The CPU's registers where the reference is parked (before the first instruction, or at an
// NMI entry).
typedef struct RefCpu {
  uint64_t cycle;
  uint16_t pc;
  uint8_t a, x, y, p, sp;
} RefCpu;

// Powers on the ROM in a fresh Mesen2 core and parks it before the first instruction.
// home_dir is Mesen's home folder for this run (its save files go there; any .sav for the ROM is
// deleted first, and sav_path, if not NULL, is put in its place, for Mesen to load as it would any
// save file). Returns NULL and fills err on failure.
Ref *ref_open(const char *rom_path, const char *home_dir, const char *sav_path, RefInputFn *input,
              void *input_ctx, char *err, size_t err_size);
// sav_out, if not NULL, gets the save file Mesen writes as it powers off.
void ref_close(Ref *r, const char *sav_out);

// Runs until the next NMI entry. Returns 0, or -1 if emulation stopped.
int ref_run_to_nmi(Ref *r, RefStop *stop);

// One of the kRef* memories; *size gets its length.
const uint8_t *ref_mem(Ref *r, int which, size_t *size);

void ref_cpu(Ref *r, RefCpu *cpu);

// The frame the PPU has just finished, at an NMI entry: 256 x 240 pixels by rows, each a palette color
// (6 bits) with PPUMASK's emphasis bits at 6-8.
const uint16_t *ref_screen(Ref *r);

// Instruction tracing, for finding where the C side goes another way. While on, every
// instruction is recorded with the registers before it runs (slow: a debugger break each).
// ref_trace returns the ones run since the previous NMI entry, that NMI's first instruction
// included.
void ref_set_trace(Ref *r, int on);
size_t ref_trace(Ref *r, const RefCpu **entries);

// The APU register writes ($4000-$4013, $4015, $4017) since the reference was last parked,
// in order.
size_t ref_apu_writes(Ref *r, const RefAccess **writes);

// The CPU cycle of each of ref_apu_writes' writes.
const uint64_t *ref_apu_write_cycles(Ref *r);

// One channel of the APU, where the reference is parked. What a channel doesn't have stays 0.
typedef struct RefApuChannel {
  uint8_t duty, duty_pos;  // pulse
  uint8_t sweep_on, sweep_negate, sweep_period, sweep_shift;  // pulse
  uint16_t period, timer;  // the pulse's timer in units of 2 CPU cycles, the others' in CPU cycles
  uint8_t enabled, length, halt;
  uint8_t env_start, env_loop, env_constant, env_volume, env_divider, env_decay;  // pulse, noise
  uint8_t step, linear_counter, linear_reload_value, linear_reload;  // triangle
  uint16_t shift;  // noise
  uint8_t mode;    // noise
} RefApuChannel;

typedef struct RefApu {
  RefApuChannel channel[4];  // pulse 1, pulse 2, triangle, noise
  uint8_t five_step, frame_step;
} RefApu;

void ref_apu_state(Ref *r, RefApu *apu);

// Records the reference's sound to a WAV file, from now until ref_close.
void ref_record_audio(Ref *r, const char *path);

// The reads of PPU registers other than $2002 itself (its mirrors included) since the
// reference was last parked, in order (dummy reads left out). The game's own code makes none;
// glitches can, through a stray pointer, and what they return depends on the PPU's exact
// position and open-bus decay, which the C side doesn't model. So it takes them from here.
size_t ref_ppu_reads(Ref *r, const RefAccess **reads);

// Save states in memory, taken at an NMI entry. ref_save_state returns a malloc'd buffer
// (free it) and its size in *size. After ref_load_state the reference is back at that NMI
// entry, described in *stop; it returns 0, or -1 if Mesen rejects the data.
uint8_t *ref_save_state(Ref *r, size_t *size);
int ref_load_state(Ref *r, const uint8_t *data, size_t size, RefStop *stop);

#ifdef __cplusplus
}
#endif

#endif  // DV_REF_H
