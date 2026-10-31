// The game's original code on the C side (nes_hw.h's console), and the bridge between it and
// ported C code (zelda3's RunEmulatedFunc).
//
// The game runs as a coroutine (coro.h). It starts at the reset vector as 6502
// code. Wherever the 6502 reaches the address of a ported routine (a hook), the C version runs
// instead; C code calls original routines with emu_call. Waiting for the NMI is where the
// game yields: the 6502 spinning in one of the game's wait loops, or ported code calling
// emu_wait_for_nmi.
//
// The reference decides where each NMI goes (EmuNmi), and emu_run_to_nmi runs the game to the
// matching place:
// - The reference's NMI came while the game waited in a wait loop: the C side takes it once
//   it waits in the same one (in 6502 code with the same registers, or in ported code).
// - It interrupted game code (a lag frame): the C side takes it where its 6502 is at the same
//   instruction with the same registers and the same memories as the reference was. That works
//   wherever the interrupted code runs as 6502 code, whatever ran before it.
// - If the interrupted code is ported, the C side can't stop inside it. Then:
//   1. A ported routine starts with a guard (emu_lag_in). If the reference's next NMI is
//      inside it, this call runs the original code instead, and the NMI is placed by state.
//   2. Otherwise the call was already under way in C, past a wait inside it. The routine
//      ends with emu_lag_behind, where the C side takes the NMI if its memories are the
//      reference's (kPlaceMemory; the harness compares memories). Not just because the call
//      ended: it may be a nested call of the same routine, the reference's place still ahead.
//   3. Failing that, it takes the NMI on the way into the next wait for the NMI, before the
//      wait changes anything: by memories if they match, otherwise late (kPlaceLate). The
//      harness doesn't compare at a late NMI, only at the next one that lines up. (If the
//      ported code changed what the NMI does in between, that next comparison shows it.)
//   Memories are matched only at such points, where the NMI's place may be behind; earlier,
//   code that writes no memory would look like the right place.
//
// So that original code run in place of a ported routine, or called from one, sees what it
// would have seen, ported routines leave the registers as the original does, however they
// are called: they set the input registers from their arguments, and the outputs at the end.
#ifndef DV_CPU_INFRA_H
#define DV_CPU_INFRA_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "nes_hw.h"

enum {
  kNmiHandler = 0xEC9A,  // nmi
  kNmiRti = 0xED23,      // the RTI that ends it (nmi_done + 17)
  // Stack page bytes from $0100 + kStackFloor up are the 6502 stack; below are data
  // (chr_bank_shadows, and from $0110 the sound driver's loop stacks: 16 bytes for each of
  // channel slots 0-6, sound_channels+15 holding the index).
  kStackFloor = 0x80,
};

// The reference's view of one NMI, for emu_run_to_nmi.
typedef struct EmuNmi {
  uint64_t cycle;      // CPU cycle at NMI entry
  uint64_t rti_cycle;  // CPU cycle at the start of the RTI ending the previous NMI (0: none)
  uint16_t return_pc;  // where the NMI interrupted the game
  uint8_t a, x, y, sp; // registers at NMI entry
  // The reference's memories at NMI entry (as nes_hw.h lays them out), for finding where an
  // NMI that interrupted game code goes.
  const uint8_t *ram, *sram, *ciram, *palette, *oam;
} EmuNmi;

// How the C side took an NMI.
typedef enum EmuPlace {
  kPlaceWait6502,  // waiting in the same 6502 wait loop as the reference, same registers
  kPlaceWaitC,     // waiting in ported code, in the wait the reference waited in
  kPlaceExact,     // a lag frame: at the instruction, registers and memories where the
                   // reference was interrupted
  kPlaceMemory,    // a lag frame that interrupted ported code: where the C side's memories
                   // are the reference's (registers don't count)
  kPlaceLate,      // a lag frame that interrupted ported code: at the next wait instead
} EmuPlace;

// What emu_run_to_nmi did, and what of the C side's state means the same as the reference's.
typedef struct EmuTaken {
  EmuPlace place;
  int compare;       // 0 for kPlaceLate: the two sides aren't at the same point
  int compare_regs;  // a, x, y mean the same (the C side stopped in 6502 code)
  int compare_sp;    // and the stack pointer (no ported routine is on the 6502 stack)
  // Stack page bytes $0100 + kStackFloor ... $0100 + stack_top are dead (below the stack
  // pointer) or belong to ported code, which doesn't push what the original pushed.
  uint8_t stack_top;
} EmuTaken;

// Starts the game on the C side: a coroutine that runs from g_nes.cpu (the reset vector at
// power-on). Returns 0, or -1 if the coroutine can't be made.
int emu_start(void);

// Runs from where the C side is (power-on, or the previous NMI's entry: first its handler)
// to this NMI, and takes it. Returns 0, or -1 with a reason in err; after an error the C side
// can't go on.
int emu_run_to_nmi(const EmuNmi *nmi, EmuTaken *taken, char *err, size_t err_size);

// Called each time the NMI handler has finished, as the PPU starts on the frame (the picture).
void emu_set_frame_hook(void (*hook)(void));

// The game on its own, with no reference to say where NMIs go: the NMI handler of the frame before
// (if the game has the NMI on), then the game until it next waits. Lag frames don't arise: C takes no
// time, so every NMI comes at a wait. The frame hook runs as the handler ends, or here if the NMI is
// off. After emu_start, with the reset vector in g_nes.cpu.pc. Returns 0, or -1 with a reason in err;
// after an error the game can't go on.
int emu_run_frame(char *err, size_t err_size);

// In a loop that spins without waiting for the NMI (get_input, reading the controller): on the
// console the NMI comes anyway. In the game on its own, a frame passes every `tenths_of_passes` / 10
// calls, counted from the last wait: 97 for 9.7 calls a frame. With a reference, it does nothing, as the
// reference says where the NMI goes.
void emu_spin(unsigned tenths_of_passes);

// How many times a port went on as original code for an index past a table's end
// (emu_note_bad_index), outside checks: real play shouldn't.
unsigned long emu_bad_indexes(void);

// Whether an NMI that returns to pc came while the game was idle in a wait loop.
int emu_is_wait_loop(uint16_t pc);

// ---- Ported routines -----------------------------------------------------------------------

// A ported routine standing in for original code. When the 6502 reaches addr in this bank (by
// JSR, JMP or falling through), fn runs instead; it finds its inputs in g_nes.cpu and leaves its
// results there as the original does. fn returns where the 6502 carries on: kEmuRts (as the
// routine's RTS would), kEmuRti (the NMI handler's RTI), or an address to jump to. Except when
// the reference's next NMI is a lag frame in [addr, end): then the original runs.
typedef uint16_t EmuHookFn(void);
enum { kEmuRts = 0, kEmuRti = 1 };
typedef struct EmuHook {
  const char *name;
  uint16_t addr, end;  // the original's code, [addr, end)
  uint8_t bank;  // the 16 KB PRG bank (0-7) it lives in; 7 is the fixed bank at $C000
  EmuHookFn *fn; // NULL for original code a ported routine does inline (only its range counts)
} EmuHook;

// The ported routines to use (the array must stay alive). None: pass-through mode.
void emu_set_hooks(const EmuHook *hooks, size_t count);

// How often hooks[index] was called so far (counted at its guard, emu_lag_in, so from C or
// from original code), to tell which ported routines a run exercised.
unsigned long emu_hook_calls(size_t index);

// From C: runs the original routine at addr on the 6502 with the registers in g_nes.cpu, until
// its RTS; its results are left there. The routine may wait for NMIs if the game coroutine
// called it. Not for routines that read data placed after their JSR (jump_table and friends).
void emu_call(uint16_t addr);

// emu_call with A loaded as LDA #a would (the N and Z flags too).
void emu_call_a(uint16_t addr, uint8_t a);

// From C, for a ported routine that ends by jumping to original code (JMP, or jump_table): runs
// it with the stack as the original has it there, no return address of its own, until its RTS
// takes the stack pointer above where it was (it returns from the ported routine), then puts
// the stack pointer back so the port returns as usual. With emu_call the callee would run two
// bytes lower on the stack, which code that keeps the stack pointer shows (the fuzz found
// special_command_by_room storing it). Only for code that returns by RTS, not reading the
// return address above it.
void emu_jump(uint16_t addr);

// A ported routine found an index past its jump table's end: real code never does, but TAS #7044's
// glitches might, so it goes on as original code. A check of a port skips such a sample.
void emu_note_bad_index(void);

// emu_call_from for original code that may end its caller too: a script opcode that does PLA, PLA,
// JMP script_end drops the return address of its dispatch, and script_end's RTS then returns from
// the whole script. Runs until the routine returns to ret (returning 0), or until the stack
// pointer rises above where it was (returning 1, with the stack pointer put back).
int emu_call_may_unwind(uint16_t addr, uint16_t ret);

// emu_call for running the original of a ported routine (see emu_lag_in): the routine at addr
// runs as original code even though a ported routine is there. Ported routines it calls are
// still the C versions.
void emu_call_original(uint16_t addr);

// At the start of a ported routine (or of original code it does inline) at addr, with the
// original's inputs in the registers: whether to run the original this time instead,
// because the reference's next NMI is a lag frame inside it. If so, the routine calls
// emu_call_original(addr) and returns what the original left (RUN_ORIGINAL_IF_LAG in funcs.h).
int emu_lag_in(uint16_t addr);

// At the end of a ported routine that can wait for the NMI, for its own code at addr (and
// for each piece of original code it does inline): takes the reference's next NMI now if it
// was a lag frame there, which is behind by now.
void emu_lag_behind(uint16_t addr);

// In a ported routine, after a call that may have waited for the NMI, where the original is at
// addr and has pushed `pushed` bytes (which the port has pushed too, with emu_push): runs the
// rest of the routine as original code, until it returns. For when the reference's next NMI
// is a lag frame in the routine's own code after the wait (RESUME_ORIGINAL_IF_LAG in
// funcs.h): the C side can then still stop at its exact place. If that code ends the script
// (see emu_call_may_unwind), this returns as soon as it has returned past this call, and the
// stack pointer shows it.
void emu_resume_original(uint16_t addr, int pushed);

// The 6502's arithmetic, for ported code that leaves the flags as the original does: ADC and
// SBC with the carry in g_nes.cpu.p (setting N, Z, C and V), and CMP (N, Z and C).
uint8_t emu_adc(uint8_t a, uint8_t m);
uint8_t emu_sbc(uint8_t a, uint8_t m);
void emu_cmp(uint8_t r, uint8_t m);

// In a C loop that a made-up state could keep going forever: counts n units of work, and a
// check of a port gives up on a run that does too much. Calling ported routines counts too, so
// only a loop that calls none needs this. The limit is at least 100,000: keep real runs well
// under it.
void emu_count_work(unsigned n);

// The byte LDA (zp),Y reads, with the 6502's bus reads (the pointer, then the byte): an
// unmapped address returns what is left on the bus, the pointer's high byte.
uint8_t emu_read_ind_y(uint8_t zp, uint8_t y);

// STA (zp),Y. In a check, a made-up pointer into the stack or the mapper makes the sample unfair: the
// stack is laid out differently under a port (return addresses), and a mapper write can swap out the
// code the original is running. emu_read_ind_y treats the stack the same way.
void emu_write_ind_y(uint8_t zp, uint8_t y, uint8_t v);

// The sound driver's loop stacks lie in the stack page, up to $019F. In a check, a made-up index past
// them reaches the stack, where the original has return addresses and a port hasn't.
void emu_note_loop_stack(uint16_t addr);

// TXS for a ported routine that throws the whole stack away (reset): nothing on it counts any more.
void emu_set_stack(uint8_t s);

// Sets A as LDA would, with N and Z.
void emu_set_a(uint8_t a);

// Sets N and Z from a value, as loads and transfers do.
void emu_set_nz(uint8_t v);

// Sets or clears one flag (kFlagC, kFlagV, ...).
void emu_set_flag(uint8_t flag, int on);

// The 6502's stack, for ported routines that pull their own return address (data after the
// JSR) or push what the original pushes.
void emu_push(uint8_t v);
uint8_t emu_pull(void);

// From ported code in the game, on the way into a wait for the NMI that will block, before the
// wait changes anything: takes the NMIs that came while ported code ran (lag frames).
void emu_take_late_nmis(void);

// From ported code in the game: waits here for the next NMI and returns after its handler
// ran. spin is the original wait loop this stands for, where the reference's NMIs return
// ($EBA4 for wait_vblank, $EBB4 for the nmi_requests waits).
void emu_wait_for_nmi(uint16_t spin);

// ---- Diagnosis -----------------------------------------------------------------------------

// Why the last NMI taken late couldn't be placed exactly: whether the 6502 ever ran the
// instruction the reference was interrupted at, and with which registers.
const char *emu_late_note(void);

// Checks every call of a ported routine from original code against the original: the
// original runs first on a copy of the state (with the ported routines it calls in C), then
// the port runs for real, and the two results are compared: registers (N, Z and C of the
// flags), memories (not the stack below the stack pointer), mapper and PPU registers,
// controllers, APU writes. Differences go to stderr, a few per routine. A call whose original
// waits for the NMI can't run twice, so it isn't checked; nor are routines whose original
// never returns by RTS (main, jump_table, set_chr_banks_inline). The NMI handler is checked to
// its RTI. About half as fast.
void emu_set_port_check(int on);
void emu_port_check_counts(unsigned long *checked, unsigned long *differed);

// Checks hooks[index] against its original on made-up states, for paths a movie never takes:
// each of `samples` states has random RAM and registers (from seed) and the rest as the C side
// is now. The original runs first, then the port, from the same state, and the results are
// compared as for emu_set_port_check. Samples where the original waits for the NMI or doesn't
// return are skipped, and so are those where a made-up pointer reaches the stack page (the original
// has return addresses there, which a port doesn't push) or reads $2002 (it depends on the cycle
// count, which C doesn't keep). A port that waits or doesn't return where its original does counts
// as a difference.
// With `waits`, a wait for the NMI doesn't skip the sample: the NMI runs at once, on both, and
// the A button changes (made_up_nmi), so routines that always wait get compared too, with what
// isn't under test (sound, sprites, the VRAM queue) sane at the start and a higher limit on
// how long a run may take. It needs the waits ported: a 6502 wait loop still skips the sample.
// Differences go to stderr (a few per routine). The C side's state is put back after.
typedef struct EmuFuzzCounts {
  unsigned long compared, skipped, differed;
} EmuFuzzCounts;
void emu_fuzz_port(size_t index, unsigned samples, uint32_t seed, int waits,
                   EmuFuzzCounts *counts);

// Prints every instruction of one sample of emu_fuzz_port, under the original and the port, to
// stderr (in each pass that has that sample). The name must stay alive.
void emu_set_fuzz_trace(const char *name, unsigned sample);

// How often a ported routine ran as original code because the reference's next NMI was a lag
// frame in it: whole calls, and the rest of a call after a wait (emu_resume_original).
void emu_original_counts(unsigned long *run, unsigned long *resumed);

// Prints the last `count` instructions run (up to 64), and the ported routines run among them.
void emu_print_trace(FILE *f, int count);

// An instruction, with the registers before it ran; or a ported routine that ran instead.
typedef struct EmuTraceEntry {
  uint64_t cycles;
  uint16_t pc;
  uint8_t a, x, y, s, p;
  uint8_t bank;        // MMC3 R6: which 8 KB bank is at $8000
  const char *ported;  // the ported routine run at pc, or NULL for an instruction
} EmuTraceEntry;

// While on, every instruction emu_run_to_nmi runs is kept; emu_trace returns those of the
// last call.
void emu_set_trace(int on);
size_t emu_trace(const EmuTraceEntry **entries);

// The whole C side's state, to replay a frame. Only while no ported code is on the game's
// call stack: emu_save and emu_load return -1 otherwise (the coroutine's C stack can't be
// saved).
typedef struct EmuSnapshot {
  Nes nes;
  int in_nmi;
} EmuSnapshot;
int emu_save(EmuSnapshot *s);
int emu_load(const EmuSnapshot *s);

#endif  // DV_CPU_INFRA_H
