// Checks the bridge between original and ported code (src/dv_cpu_infra.h) without the ROM:
// a small made-up program with the game's wait_vblank and an NMI handler at the game's
// addresses, run with and without ported routines, with NMIs placed in each of the ways the
// harness uses.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dv_cpu_infra.h"
#include "nes_hw.h"

static int failures;

#define CHECK(cond)                                                    \
  do {                                                                 \
    if(!(cond)) {                                                      \
      fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #cond); \
      failures++;                                                      \
    }                                                                  \
  } while(0)

static uint8_t prg[kPrgSize], chr[kChrSize];

static void put(uint16_t addr, const uint8_t *bytes, size_t n) {
  memcpy(prg + 0x1C000 + (addr - 0xC000), bytes, n);  // the fixed banks at $C000-$FFFF
}

// The program. Each frame it counts $10 up to $30 in a loop (where a lag NMI can come), waits
// for vblank, then counts frames in $00.
static void make_rom(void) {
  static const uint8_t kMain[] = {
    0xA2, 0xFE,        // E000 ldx #$FE
    0x9A,              // E002 txs
    0xA9, 0x00,        // E003 lda #0
    0x85, 0x10,        // E005 sta $10
    0xE6, 0x10,        // E007 inc $10
    0xA5, 0x10,        // E009 lda $10
    0xC9, 0x30,        // E00B cmp #$30
    0xD0, 0xF8,        // E00D bne $E007
    0x20, 0x9C, 0xEB,  // E00F jsr wait_vblank
    0xE6, 0x00,        // E012 inc $00
    0x4C, 0x03, 0xE0,  // E014 jmp $E003
  };
  static const uint8_t kWaitVblank[] = {  // as in the game
    0xA5, 0xF0, 0x09, 0x02, 0x85, 0xF0, 0xA9, 0x02, 0x25, 0xF0, 0xD0, 0xFC, 0x60,
  };
  static const uint8_t kNmi[] = {
    0x48,              // EC9A pha
    0xA5, 0xF0,        //      lda system_flags
    0x29, 0xFD,        //      and #$FD: the wait is over
    0x85, 0xF0,        //      sta system_flags
    0xE6, 0x25,        //      inc frame_counter
    0x68,              //      pla
    0x4C, 0x23, 0xED,  //      jmp $ED23
  };
  // A second program at $E200 whose count is a subroutine ($E100) calling another ($E110).
  static const uint8_t kMain2[] = {
    0xA2, 0xFE, 0x9A,  // E200 ldx #$FE, txs
    0x20, 0x00, 0xE1,  // E203 jsr $E100
    0x20, 0x9C, 0xEB,  // E206 jsr wait_vblank
    0xE6, 0x00,        // E209 inc $00
    0x4C, 0x03, 0xE2,  // E20B jmp $E203
  };
  static const uint8_t kOuter[] = {0x20, 0x10, 0xE1, 0x60};  // E100 jsr $E110, rts
  static const uint8_t kInner[] = {
    0xA9, 0x00, 0x85, 0x10,  // E110 lda #0, sta $10
    0xE6, 0x10,              // E114 inc $10
    0xA5, 0x10, 0xC9, 0x30,  //      lda $10, cmp #$30
    0xD0, 0xF8, 0x60,        //      bne $E114, rts
  };
  // A third at $E300, calling $E120: pushes X and Y, waits, counts Y down, pulls them back.
  static const uint8_t kMain3[] = {
    0xA2, 0xFE, 0x9A,  // E300 ldx #$FE, txs
    0x20, 0x20, 0xE1,  // E303 jsr $E120
    0xE6, 0x00,        // E306 inc $00
    0x4C, 0x03, 0xE3,  // E308 jmp $E303
  };
  static const uint8_t kWaitsInside[] = {
    0x8A, 0x48, 0x98, 0x48,  // E120 txa, pha, tya, pha
    0x20, 0x9C, 0xEB,        // E124 jsr wait_vblank
    0xA0, 0x20,              // E127 ldy #$20
    0x88, 0xD0, 0xFD,        // E129 dey, bne $E129
    0x68, 0xA8, 0x68, 0xAA,  // E12C pla, tay, pla, tax
    0x60,                    // E130 rts
  };
  static const uint8_t kRti[] = {0x40};
  static const uint8_t kVectors[] = {0x9A, 0xEC, 0x00, 0xE0, 0x00, 0xE0};
  memset(prg, 0, sizeof(prg));
  put(0xE000, kMain, sizeof(kMain));
  put(0xE200, kMain2, sizeof(kMain2));
  put(0xE100, kOuter, sizeof(kOuter));
  put(0xE110, kInner, sizeof(kInner));
  put(0xE300, kMain3, sizeof(kMain3));
  put(0xE120, kWaitsInside, sizeof(kWaitsInside));
  put(0xEB9C, kWaitVblank, sizeof(kWaitVblank));
  put(0xEC9A, kNmi, sizeof(kNmi));
  put(0xED23, kRti, sizeof(kRti));
  put(0xFFFA, kVectors, sizeof(kVectors));
}

static void power_on(void) {
  nes_init(prg, chr);
  g_nes.cpu.pc = 0xE000;
  g_nes.cpu.s = 0xFD;
  g_nes.cpu.p = kFlagI;
  nes_power_on(0);
  CHECK(emu_start() == 0);
}

// The reference's memories at its next NMI, as the test predicts them.
static uint8_t ref_ram[0x800], ref_sram[0x2000], ref_ciram[0x800], ref_pal[0x20], ref_oam[0x100];

static EmuNmi nmi(uint16_t return_pc, uint8_t a, uint8_t sp) {
  EmuNmi n = {0, 0, return_pc, a, 0xFE, 0x00, sp, ref_ram, ref_sram, ref_ciram, ref_pal, ref_oam};
  return n;
}

static int run(const EmuNmi *n, EmuTaken *t) {
  char err[256];
  if(emu_run_to_nmi(n, t, err, sizeof(err))) {
    fprintf(stderr, "emu_run_to_nmi: %s\n", err);
    return -1;
  }
  return 0;
}

// A lag frame: the reference's NMI came at the inc $10 when $10 was $1F, one frame after the
// last NMI (taken in the wait).
static EmuNmi lag_nmi(void) {
  memcpy(ref_ram, g_nes.ram, sizeof(ref_ram));
  ref_ram[0xF0] &= ~0x02;
  ref_ram[0x25]++;
  ref_ram[0x00]++;
  ref_ram[0x10] = 0x1F;
  return nmi(0xE007, 0x1F, 0xFB);
}

static void test_pass_through(void) {
  power_on();
  EmuTaken t;
  EmuNmi wait = nmi(0xEBA4, 0x02, 0xF9);
  CHECK(run(&wait, &t) == 0);
  CHECK(t.place == kPlaceWait6502 && t.compare && t.compare_regs && t.compare_sp);
  CHECK(g_nes.ram[0x10] == 0x30 && g_nes.ram[0x00] == 0);
  CHECK(t.stack_top == g_nes.cpu.s);

  EmuNmi lag = lag_nmi();
  CHECK(run(&lag, &t) == 0);
  CHECK(t.place == kPlaceExact && t.compare_regs && t.compare_sp);
  CHECK(g_nes.ram[0x10] == 0x1F && g_nes.ram[0x00] == 1 && g_nes.ram[0x25] == 1);

  CHECK(run(&wait, &t) == 0);
  CHECK(t.place == kPlaceWait6502);
  CHECK(g_nes.ram[0x10] == 0x30 && g_nes.ram[0x25] == 2);

  EmuSnapshot snap;
  CHECK(emu_save(&snap) == 0);

  // A lag NMI whose state never comes, in code that is all original: an error.
  memcpy(ref_ram, g_nes.ram, sizeof(ref_ram));
  ref_ram[0x10] = 0x99;
  EmuNmi never = nmi(0xE007, 0x1F, 0xFB);
  char err[256];
  CHECK(emu_run_to_nmi(&never, &t, err, sizeof(err)) == -1);
  CHECK(strstr(err, "never reached") != NULL);
  // A replay puts the state back and carries on.
  CHECK(emu_load(&snap) == 0);
  CHECK(run(&wait, &t) == 0);
  CHECK(t.place == kPlaceWait6502 && g_nes.ram[0x00] == 2);
}

// Ported wait_vblank: waits in C.
static uint16_t c_wait_vblank(void) {
  emu_take_late_nmis();
  g_nes.ram[0xF0] |= 0x02;
  while(g_nes.ram[0xF0] & 0x02) {
    emu_wait_for_nmi(0xEBA4);
  }
  emu_set_a(0);
  return kEmuRts;
}

// Ported counting loop at $E003: $10 goes to $30 at once.
static uint16_t c_count(void) {
  g_nes.ram[0x10] = 0x30;
  emu_set_a(0x30);
  return 0xE00F;
}

// Ported jsr wait_vblank at $E00F: original wait_vblank called from C.
static uint16_t c_call_wait(void) {
  emu_call(0xEB9C);
  return 0xE012;
}

// Ported $E100 that does $E110 inline. A lag NMI in either runs the original $E100 instead:
// with emu_call that would land on this hook again, without end.
static uint16_t c_outer(void) {
  if(emu_lag_in(0xE100) || emu_lag_in(0xE110)) {
    emu_call_original(0xE100);
    return kEmuRts;
  }
  g_nes.ram[0x10] = 0x30;
  emu_set_a(0x30);
  return kEmuRts;
}

// Ported $E120: pushes as the original does, and after its wait carries on as original code
// when the reference's next NMI is in its own code.
static uint16_t c_waits_inside(void) {
  Cpu6502 *c = &g_nes.cpu;
  if(emu_lag_in(0xE120)) {
    emu_call_original(0xE120);
    return kEmuRts;
  }
  uint8_t x = c->x, y = c->y;
  emu_push(x);
  emu_push(y);
  c_wait_vblank();
  if(emu_lag_in(0xE120)) {
    emu_resume_original(0xE127, 2);
    return kEmuRts;
  }
  emu_pull();
  emu_pull();
  c->y = y;
  c->x = x;
  emu_set_a(x);
  return kEmuRts;
}

// The same with the wait in C, ending with the point for lag NMIs in its own code.
static uint16_t c_wait_inline(void) {
  c_wait_vblank();
  emu_lag_behind(0xE00F);
  return 0xE012;
}

static void test_ported(void) {
  // The counting loop ported, its range covering the loop: a lag NMI inside it makes the C
  // side run the original loop for that call, and the NMI is placed by state.
  static const EmuHook kHooks[] = {
    {"wait_vblank", 0xEB9C, 0xEBA9, 7, c_wait_vblank},
    {"count", 0xE003, 0xE00F, 7, c_count},
  };
  emu_set_hooks(kHooks, 2);
  power_on();
  EmuTaken t;
  EmuNmi wait = nmi(0xEBA4, 0x02, 0xF9);
  CHECK(run(&wait, &t) == 0);
  CHECK(t.place == kPlaceWaitC && t.compare && !t.compare_regs && !t.compare_sp);
  CHECK(t.stack_top == 0xFC);  // the stack pointer when wait_vblank was entered
  EmuSnapshot snap;
  CHECK(emu_save(&snap) == -1);  // ported code is on the game's call stack
  EmuNmi lag = lag_nmi();
  CHECK(run(&lag, &t) == 0);
  CHECK(t.place == kPlaceExact && t.compare_regs);
  CHECK(g_nes.ram[0x10] == 0x1F);
  CHECK(run(&wait, &t) == 0);
  CHECK(t.place == kPlaceWaitC && g_nes.ram[0x10] == 0x30);

  // A wait-loop NMI while the reference waits elsewhere: an error.
  EmuNmi other = nmi(0xEBB4, 0x08, 0xF9);
  char err[256];
  CHECK(emu_run_to_nmi(&other, &t, err, sizeof(err)) == -1);

  // The loop's range left out: the 6502 never runs it, so the NMI comes late, in the wait.
  static const EmuHook kShortHooks[] = {
    {"wait_vblank", 0xEB9C, 0xEBA9, 7, c_wait_vblank},
    {"count", 0xE003, 0xE005, 7, c_count},
  };
  emu_set_hooks(kShortHooks, 2);
  power_on();
  CHECK(run(&wait, &t) == 0);
  lag = lag_nmi();
  CHECK(run(&lag, &t) == 0);
  CHECK(t.place == kPlaceLate && !t.compare);
  CHECK(strstr(emu_late_note(), "never ran $E007") != NULL);
  CHECK(g_nes.ram[0x00] == 1 && g_nes.ram[0x10] == 0x30 && g_nes.ram[0x25] == 1);
  CHECK(run(&wait, &t) == 0);
  CHECK(t.place == kPlaceWaitC && g_nes.ram[0x25] == 2);

  // The loop counted as ported code done inline: where the C side's memories become the
  // reference's (here in the wait, $10 already $30), the NMI is taken and compared.
  static const EmuHook kInlineHooks[] = {
    {"wait_vblank", 0xEB9C, 0xEBA9, 7, c_wait_vblank},
    {"count", 0xE003, 0xE005, 7, c_count},
    {"count_loop", 0xE005, 0xE00F, 7, NULL},
  };
  emu_set_hooks(kInlineHooks, 3);
  power_on();
  CHECK(run(&wait, &t) == 0);
  lag = lag_nmi();
  ref_ram[0x10] = 0x30;
  CHECK(run(&lag, &t) == 0);
  CHECK(t.place == kPlaceMemory && t.compare && !t.compare_regs);
  CHECK(run(&wait, &t) == 0);
  CHECK(t.place == kPlaceWaitC && g_nes.ram[0x25] == 2);

  // A lag NMI in the code of a ported routine that was already waiting in C when the NMI
  // before it came: taken at the routine's end, where the memories match.
  static const EmuHook kBehindHooks[] = {
    {"wait_inline", 0xE00F, 0xE012, 7, c_wait_inline},
  };
  emu_set_hooks(kBehindHooks, 1);
  power_on();
  CHECK(run(&wait, &t) == 0);
  CHECK(t.place == kPlaceWaitC);
  memcpy(ref_ram, g_nes.ram, sizeof(ref_ram));
  ref_ram[0xF0] &= ~0x02;
  ref_ram[0x25]++;
  EmuNmi tail = nmi(0xE011, 0x00, 0xFB);
  CHECK(run(&tail, &t) == 0);
  CHECK(t.place == kPlaceMemory && t.compare);
  CHECK(run(&wait, &t) == 0);
  CHECK(t.place == kPlaceWaitC && g_nes.ram[0x00] == 1);

  // A lag NMI in original code a ported routine does inline: the original runs instead.
  static const EmuHook kOuterHooks[] = {
    {"wait_vblank", 0xEB9C, 0xEBA9, 7, c_wait_vblank},
    {"outer", 0xE100, 0xE104, 7, c_outer},
    {"inner", 0xE110, 0xE11D, 7, NULL},
  };
  emu_set_hooks(kOuterHooks, 3);
  power_on();
  g_nes.cpu.pc = 0xE200;
  CHECK(run(&wait, &t) == 0);
  CHECK(t.place == kPlaceWaitC);
  lag = lag_nmi();
  lag.return_pc = 0xE114;
  CHECK(run(&lag, &t) == 0);
  CHECK(t.place == kPlaceExact && g_nes.ram[0x10] == 0x1F);
  CHECK(run(&wait, &t) == 0);
  CHECK(t.place == kPlaceWaitC && g_nes.ram[0x10] == 0x30 && g_nes.ram[0x00] == 1);

  // A lag NMI in a ported routine's own code after a wait inside it: the rest of the routine
  // runs as original code, and the NMI is placed by state.
  static const EmuHook kInsideHooks[] = {
    {"wait_vblank", 0xEB9C, 0xEBA9, 7, c_wait_vblank},
    {"waits_inside", 0xE120, 0xE131, 7, c_waits_inside},
  };
  emu_set_hooks(kInsideHooks, 2);
  power_on();
  g_nes.cpu.pc = 0xE300;
  CHECK(run(&wait, &t) == 0);
  CHECK(t.place == kPlaceWaitC);
  memcpy(ref_ram, g_nes.ram, sizeof(ref_ram));
  ref_ram[0xF0] &= ~0x02;
  ref_ram[0x25]++;
  EmuNmi inside = nmi(0xE129, 0x00, 0xF5);
  inside.y = 0x10;
  CHECK(run(&inside, &t) == 0);
  CHECK(t.place == kPlaceExact && g_nes.cpu.y == 0x10);
  CHECK(run(&wait, &t) == 0);
  // The stack balanced again: return address, X and Y, and the NMI's three bytes below $FE.
  CHECK(t.place == kPlaceWaitC && g_nes.ram[0x00] == 1 && g_nes.cpu.s == 0xF7);

  // Ported code calling original code that waits (nested in the game coroutine).
  static const EmuHook kCallHooks[] = {
    {"call_wait", 0xE00F, 0xE012, 7, c_call_wait},
  };
  emu_set_hooks(kCallHooks, 1);
  power_on();
  for(int i = 0; i < 3; i++) {
    CHECK(run(&wait, &t) == 0);
    CHECK(t.place == kPlaceWait6502 && t.compare_regs && !t.compare_sp);
    CHECK(g_nes.ram[0x00] == i);
  }
  emu_set_hooks(NULL, 0);
}

// A ported NMI handler, doing what the program's original does.
static uint16_t c_nmi(void) {
  Cpu6502 *c = &g_nes.cpu;
  emu_push(c->a);
  g_nes.ram[0xF0] &= ~0x02;
  g_nes.ram[0x25]++;
  c->a = emu_pull();
  return kEmuRti;
}

// The same with a byte too many, for the checks to find.
static uint16_t c_nmi_wrong(void) {
  g_nes.ram[0x40]++;
  return c_nmi();
}

static uint64_t count_cycles;

// Ported counting loop, noting when the game carried on after the handler.
static uint16_t c_count_timed(void) {
  count_cycles = g_nes.cpu.cycles;
  return c_count();
}

static void test_ported_nmi(void) {
  // The handler in C: it ends the 6502's wait, and returns where the NMI came from with the
  // stack balanced and the cycle count at the reference's.
  static const EmuHook kHooks[] = {
    {"nmi", 0xEC9A, 0xED24, 7, c_nmi},
    {"count", 0xE003, 0xE00F, 7, c_count_timed},
  };
  emu_set_hooks(kHooks, 2);
  power_on();
  EmuTaken t;
  EmuNmi wait = nmi(0xEBA4, 0x02, 0xF9);
  CHECK(run(&wait, &t) == 0);
  CHECK(t.place == kPlaceWait6502 && t.compare_regs && t.compare_sp);
  CHECK(g_nes.cpu.s == 0xF9 && g_nes.ram[0x25] == 0);
  wait.rti_cycle = 2000000;
  count_cycles = 0;
  CHECK(run(&wait, &t) == 0);
  CHECK(t.place == kPlaceWait6502 && t.compare_regs && t.compare_sp);
  CHECK(g_nes.cpu.s == 0xF9 && g_nes.ram[0x25] == 1 && g_nes.ram[0x00] == 1);
  CHECK(count_cycles >= 2000006 && count_cycles < 2000100);

  // The port checks run the handler's original to its RTI, and compare.
  unsigned long checked, differed, checked0, differed0;
  emu_set_port_check(1);
  emu_port_check_counts(&checked0, &differed0);
  CHECK(run(&wait, &t) == 0);
  emu_port_check_counts(&checked, &differed);
  CHECK(checked > checked0 && differed == differed0);
  EmuFuzzCounts counts;
  emu_fuzz_port(0, 100, 1, 0, &counts);
  CHECK(counts.compared == 100 && counts.differed == 0);

  static const EmuHook kWrongHooks[] = {{"nmi", 0xEC9A, 0xED24, 7, c_nmi_wrong}};
  emu_set_hooks(kWrongHooks, 1);
  emu_fuzz_port(0, 100, 1, 0, &counts);
  CHECK(counts.compared == 100 && counts.differed == 100);
  emu_port_check_counts(&checked0, &differed0);
  CHECK(run(&wait, &t) == 0);
  emu_port_check_counts(&checked, &differed);
  CHECK(checked > checked0 && differed > differed0);
  emu_set_port_check(0);
  emu_set_hooks(NULL, 0);
}

// The wrong $E120: right until its wait, then Y isn't counted down.
static uint16_t c_waits_inside_wrong(void) {
  Cpu6502 *c = &g_nes.cpu;
  uint8_t x = c->x, y = c->y;
  emu_push(x);
  emu_push(y);
  c_wait_vblank();
  emu_pull();
  emu_pull();
  c->y = y;
  c->x = x;
  emu_set_a(0);
  return kEmuRts;
}

// A routine that always waits: without NMIs in the check it can't be compared, with them it can,
// right or wrong.
static void test_fuzz_through_waits(void) {
  static const EmuHook kHooks[] = {
    {"waits_inside", 0xE120, 0xE131, 7, c_waits_inside},
    {"wait_vblank", 0xEB9C, 0xEBA9, 7, c_wait_vblank},
  };
  emu_set_hooks(kHooks, 2);
  power_on();
  EmuFuzzCounts counts;
  emu_fuzz_port(0, 100, 1, 0, &counts);
  CHECK(counts.compared == 0 && counts.skipped == 100);
  emu_fuzz_port(0, 100, 1, 1, &counts);
  CHECK(counts.compared == 100 && counts.differed == 0);

  static const EmuHook kWrongHooks[] = {
    {"waits_inside", 0xE120, 0xE131, 7, c_waits_inside_wrong},
    {"wait_vblank", 0xEB9C, 0xEBA9, 7, c_wait_vblank},
  };
  emu_set_hooks(kWrongHooks, 2);
  emu_fuzz_port(0, 100, 1, 1, &counts);
  CHECK(counts.compared == 100 && counts.differed == 100);
  emu_set_hooks(NULL, 0);
}

// What ran is marked, by the way each branch went, and a fuzz sample's marks only stay if it was compared.
static void test_coverage(void) {
  static uint8_t flags[kPrgSize];
  enum { kBank7 = 0x1C000 - 0xC000 };
  memset(flags, 0, sizeof(flags));
  emu_set_hooks(NULL, 0);
  power_on();
  emu_set_coverage(flags);
  EmuTaken t;
  EmuNmi wait = nmi(0xEBA4, 0x02, 0xF9);
  CHECK(run(&wait, &t) == 0);
  CHECK(flags[kBank7 + 0xE000] == kCoverRan);
  CHECK(flags[kBank7 + 0xE001] == 0);  // an operand
  CHECK(flags[kBank7 + 0xE00D] == (kCoverRan | kCoverBranched | kCoverFellThrough));  // bne $E007, both ways
  CHECK(flags[kBank7 + 0xE012] == 0);  // after the wait, which hasn't ended

  static const EmuHook kHooks[] = {
    {"waits_inside", 0xE120, 0xE131, 7, c_waits_inside},
    {"wait_vblank", 0xEB9C, 0xEBA9, 7, c_wait_vblank},
  };
  emu_set_hooks(kHooks, 2);
  power_on();
  memset(flags, 0, sizeof(flags));
  emu_set_coverage(flags);
  EmuFuzzCounts counts;
  emu_fuzz_port(0, 100, 1, 0, &counts);  // all skipped: the original waits
  CHECK(counts.compared == 0 && flags[kBank7 + 0xE120] == 0);
  emu_fuzz_port(0, 100, 1, 1, &counts);  // through waits: all compared
  CHECK(counts.compared == 100 && flags[kBank7 + 0xE120] == kCoverRan);
  CHECK(flags[kBank7 + 0xE12A] == (kCoverRan | kCoverBranched | kCoverFellThrough));
  emu_set_coverage(NULL);
  emu_set_hooks(NULL, 0);
}

int main(void) {
  make_rom();
  test_pass_through();
  test_ported();
  test_ported_nmi();
  test_fuzz_through_waits();
  test_coverage();
  if(failures) {
    fprintf(stderr, "test_bridge: %d checks failed\n", failures);
    return 1;
  }
  printf("test_bridge: all checks passed\n");
  return 0;
}
