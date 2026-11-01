// dv_harness: plays an input movie on the reference (the original game in Mesen2), logs its
// state at every NMI entry, and compares the C side with it.
//
// usage: dv_harness --movie FILE.inp [--offset K] [--frames N] [--log FILE]
//                   [--rewind-check K] [--compare] [--keep-going] [--pass-through]
//                   [--emulate NAME[,NAME...]] [--check-ports] [--rom ROM] [--home DIR]
//
// --movie    an .inp file from tools/movie_inputs.py: one line per frame, UDLRTSBA.
//            Input poll p gets line p - 1 + K (0-based), exactly as tools/mesen/record_cdl.lua
//            plays movies, so movies that sync there sync here.
// --frames   NMIs to run (default: the movie's length minus K, plus 300 for the ending).
// --log      one line per NMI entry; tools/mesen/record_cdl.lua writes the same format with
//            DV_NMI_LOG (run_mesen.py --log-nmi), so the two can be compared with diff:
//              nmi polls return_pc a= x= y= p= sp= cyc= ram= sram= nt= pal= oam= apu=n:hash
//            Memory hashes are FNV-1a 32 of the whole memory; apu= is the count and hash of
//            the (addr, value) APU register writes since the previous NMI entry.
// --rewind-check K  every K NMIs, save a state, run K NMIs, load it, run them again and check
//            every line repeats exactly (it tests the save states the harness rewinds with).
// --compare  run the C side too (src/dv_cpu_infra.h: the original code on the C side's console,
//            with the routines ported so far in place of theirs) and check it against the
//            reference at every NMI entry: registers, the five memories and the exact list of
//            APU writes. What can't mean the same on both sides isn't compared: registers and
//            the stack where ported code is running, the stack below the stack pointer, and NMIs
//            the C side took late because they interrupted ported code. The first NMI that
//            differs is reported, with the addresses, their values on both sides and before,
//            and the C side's last instructions; then the run stops.
// --keep-going  after a difference, copy the reference's memories to the C side and go on.
// --pass-through  run every routine as original code, none ported.
// --emulate NAME[,NAME...]  run these ported routines (funcs.h has the names) as original code
//            where original code calls them; ported code calling them still gets the C version.
// --check-ports  with --compare: also check every call of a ported routine from original code
//            against its original, run first on a copy of the state (emu_set_port_check in
//            src/dv_cpu_infra.h). Differences don't stop the run; the summary counts them.
// --fuzz-ports N  with --compare: before the movie, check every ported routine against its
//            original on N made-up states (emu_fuzz_port), for paths the movies never take.
//            Then, with N / 10 samples each, those that waited for the NMI, with NMIs run
//            through the waits. With --frames 0 it does only that.
// --fuzz-only NAME[,NAME...]  with --fuzz-ports: only these routines.
// --fuzz-trace NAME:SAMPLE  with --fuzz-ports: prints every instruction of that sample of that
//            routine, as the report of a difference names it, under the original and the port.
// --sweep-scripts FILE  with --compare: after the movie's --frames, run script code from each place FILE lists (hex
//            addresses in bank 3, from tools/run_checks.py) on the state it has reached, the original and the port
//            from the same state (emu_sweep_scripts).
// --sweep-passes N  how many times over; all but the first with random flags. The default is 4.
// --state-out FILE  when it stops, write the reference's memories to this file: RAM, save RAM, nametables, palette
//            and OAM, as dejavu --state-out does for the game on its own.
// --coverage FILE  write a record of what original code ran under comparison (emu_set_coverage), a byte
//            for each byte of PRG, for tools/coverage.py.
// --free-run  with --compare: the C side runs on its own (emu_run_frame), not told where NMIs go, and
//            the run stops at the first difference. An NMI in the game's spins on the controller
//            isn't compared, as the reference is partway through a read there; the C side spins too,
//            and they stay in step. It can only match up to the first real lag NMI, as it has none.
//            That shows it is the same game, as far as that.
// --apu      with --compare: also run the APU (src/apu.h) on the reference's own register writes, at
//            their CPU cycles, and compare its state with the reference's at every NMI. So this tests
//            the chip, not the game's timing. Differences don't stop the run.
// --audio-wav PREFIX  with --apu: also write the sound of both, PREFIX_c.wav and PREFIX_ref.wav, for
//            tools/compare_audio.py.
// --check-palette  compare our palette's RGB (ppu_make_palette) with Mesen2's default, which it reads
//            from Mesen2's source in external/ as it runs, and stop. No movie needed. The picture's
//            comparison is of palette entries, so it can't see this.
// --picture  with --compare: also compare the picture (src/ppu_render.h) with the reference's at
//            every frame, pixel by pixel, as palette colors. Differences don't stop the run.
// --picture-dump DIR  with --picture: writes DIR/diffs.txt, a line per frame that differs, and the
//            first frame of each run of them, both sides, for tools/frame_png.py.
// --sav FILE  start from this save file: Mesen loads it as it would any save file, and with --compare
//            the C side starts with what Mesen loaded (docs/saves.md).
// --sav-out FILE  at the end, keep the save file Mesen writes as it powers off.
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dv_cpu_infra.h"
#include "funcs.h"
#include "apu.h"
#include "nes_hw.h"
#include "ppu_render.h"
#include "ref.h"

#ifndef DV_DEFAULT_ROM
#define DV_DEFAULT_ROM "Deja Vu (USA).nes"
#endif
#ifndef DV_DEFAULT_HOME
#define DV_DEFAULT_HOME "build/harness/mesen-home"
#endif

typedef struct Movie {
  uint8_t *pads;
  uint32_t count;
  uint32_t offset;
} Movie;

static uint8_t movie_input(void *ctx, uint32_t poll) {
  const Movie *m = ctx;
  int64_t line = (int64_t)poll - 1 + m->offset;
  return line >= 0 && line < m->count ? m->pads[line] : 0;
}

static int load_movie(const char *path, Movie *m) {
  static const char kOrder[] = "UDLRTSBA";
  static const uint8_t kBits[8] = {kPadUp, kPadDown, kPadLeft, kPadRight,
                                   kPadStart, kPadSelect, kPadB, kPadA};
  FILE *f = fopen(path, "r");
  if(!f) {
    return -1;
  }
  uint32_t cap = 0;
  char line[64];
  while(fgets(line, sizeof(line), f)) {
    if(m->count == cap) {
      cap = cap ? cap * 2 : 4096;
      m->pads = realloc(m->pads, cap);
    }
    uint8_t pad = 0;
    for(int i = 0; i < 8 && line[i] && line[i] != '\n'; i++) {
      if(line[i] == kOrder[i]) {
        pad |= kBits[i];
      }
    }
    m->pads[m->count++] = pad;
  }
  fclose(f);
  return 0;
}

static uint32_t fnv1a(const uint8_t *p, size_t n, uint32_t h) {
  while(n--) {
    h = (h ^ *p++) * 16777619u;
  }
  return h;
}

static void format_line(Ref *r, const RefStop *s, char *out, size_t size) {
  uint32_t hash[kRefMemCount];
  for(int i = 0; i < kRefMemCount; i++) {
    size_t n;
    const uint8_t *mem = ref_mem(r, i, &n);
    hash[i] = fnv1a(mem, n, 2166136261u);
  }
  const RefAccess *w;
  size_t nw = ref_apu_writes(r, &w);
  uint32_t apu = 2166136261u;
  for(size_t i = 0; i < nw; i++) {
    uint8_t b[3] = {(uint8_t)w[i].addr, (uint8_t)(w[i].addr >> 8), w[i].value};
    apu = fnv1a(b, 3, apu);
  }
  snprintf(out, size,
           "%u %u %04X a=%02X x=%02X y=%02X p=%02X sp=%02X cyc=%llu ram=%08X sram=%08X "
           "nt=%08X pal=%08X oam=%08X apu=%u:%08X",
           s->nmi, s->polls, s->return_pc, s->a, s->x, s->y, s->p, s->sp,
           (unsigned long long)s->cycle, hash[kRefRam], hash[kRefSram], hash[kRefCiram],
           hash[kRefPalette], hash[kRefOam], (unsigned)nw, apu);
}

// The NMI came while the game was idle in one of its two wait loops, which the NMI ends:
// wait_vblank (7:EB9C) after it set the flag at $EBA0, returning to $EBA2 or into the spin
// at $EBA4-$EBA7; or wait_nmi_work_done / wait_vram_queue_empty, spinning on nmi_requests at
// $EBB4-$EBB7. Every other NMI interrupted game code (a lag frame).
static int waiting_for_nmi(uint16_t pc) {
  return (pc >= 0xEBA2 && pc <= 0xEBA7) || (pc >= 0xEBB4 && pc <= 0xEBB7);
}

// How far the game got (the same measure as tools/run_mesen.py).
static void print_progress(Ref *r) {
  const uint8_t *ram = ref_mem(r, kRefRam, NULL);
  int bits = 0, items = 0;
  for(int a = 0x420; a < 0x4B0; a++) {
    for(int b = ram[a]; b; b &= b - 1) {
      bits++;
    }
  }
  for(int a = 0x4C0; a < 0x4F0; a++) {
    items += ram[a] != 0xFF;
  }
  printf("dv_harness: %d story/object flag bits set, %d address book + inventory slots used\n",
         bits, items);
}

// ---- The C side ----------------------------------------------------------------------------

static uint8_t *c_rom;  // the ROM file, for the C side's PRG and CHR

static uint8_t *const kCMem[kRefMemCount] = {
  g_nes.ram, g_nes.sram, g_nes.ciram, g_nes.palette, g_nes.oam,
};
static const size_t kCMemSize[kRefMemCount] = {
  sizeof(g_nes.ram), sizeof(g_nes.sram), sizeof(g_nes.ciram), sizeof(g_nes.palette),
  sizeof(g_nes.oam),
};
static const char *const kMemName[kRefMemCount] = {"ram", "sram", "nametables", "palette", "oam"};
static const uint16_t kMemBase[kRefMemCount] = {0x0000, 0x6000, 0x2000, 0x3F00, 0x0000};

// Powers on the C side in the reference's power-on state.
static int c_power_on(Ref *r, const char *rom) {
  FILE *f = fopen(rom, "rb");
  if(!f) {
    fprintf(stderr, "dv_harness: can't read %s\n", rom);
    return -1;
  }
  c_rom = malloc(16 + kPrgSize + kChrSize + 1);
  size_t n = fread(c_rom, 1, 16 + kPrgSize + kChrSize + 1, f);
  fclose(f);
  if(n != 16 + kPrgSize + kChrSize) {
    fprintf(stderr, "dv_harness: %s is not 16 + 128 KB + 256 KB\n", rom);
    return -1;
  }
  nes_init(c_rom + 16, c_rom + 16 + kPrgSize);
  for(int i = 0; i < kRefMemCount; i++) {
    size_t size;
    const uint8_t *mem = ref_mem(r, i, &size);
    if(size != kCMemSize[i]) {
      fprintf(stderr, "dv_harness: the reference's %s is %zu bytes, the C side's %zu\n",
              kMemName[i], size, kCMemSize[i]);
      return -1;
    }
    memcpy(kCMem[i], mem, size);
  }
  RefCpu cpu;
  ref_cpu(r, &cpu);
  g_nes.cpu = (Cpu6502){0, cpu.pc, cpu.a, cpu.x, cpu.y, cpu.sp, cpu.p & ~(kFlagB | kFlagU)};
  nes_power_on(cpu.cycle);
  if(emu_start()) {
    fprintf(stderr, "dv_harness: can't start the game's coroutine\n");
    return -1;
  }
  return 0;
}

// Whether name is one of a comma-separated list.
static int name_in_list(const char *name, const char *list) {
  size_t len = strlen(name);
  for(const char *p = list; *p; p += strcspn(p, ",") + (p[strcspn(p, ",")] == ',')) {
    if(strcspn(p, ",") == len && !strncmp(p, name, len)) {
      return 1;
    }
  }
  return 0;
}

// The ported routines to use: all of them but those named in `emulate` (comma-separated).
static size_t c_choose_ported(const char *emulate, EmuHook *out) {
  size_t n = 0;
  for(size_t i = 0; i < kPortedRoutineCount; i++) {
    const char *name = kPortedRoutines[i].name;
    size_t len = strlen(name);
    int skip = 0;
    for(const char *p = emulate; p && *p; p += strcspn(p, ",") + (p[strcspn(p, ",")] == ',')) {
      skip |= strcspn(p, ",") == len && !strncmp(p, name, len);
    }
    if(!skip) {
      out[n++] = kPortedRoutines[i];
    }
  }
  return n;
}

// Whether every name in `emulate` is a ported routine.
static int c_known_names(const char *emulate) {
  for(const char *p = emulate; *p; p += strcspn(p, ",") + (p[strcspn(p, ",")] == ',')) {
    size_t len = strcspn(p, ","), i = 0;
    while(i < kPortedRoutineCount &&
          !(strlen(kPortedRoutines[i].name) == len && !strncmp(p, kPortedRoutines[i].name, len))) {
      i++;
    }
    if(i == kPortedRoutineCount) {
      fprintf(stderr, "dv_harness: --emulate: %.*s isn't a ported routine; they are:", (int)len, p);
      for(i = 0; i < kPortedRoutineCount; i++) {
        fprintf(stderr, " %s", kPortedRoutines[i].name);
      }
      fprintf(stderr, "\n");
      return 0;
    }
  }
  return 1;
}

static const char *place_name(EmuPlace p) {
  switch(p) {
    case kPlaceWait6502: return "waiting in the same loop";
    case kPlaceWaitC: return "waiting in ported code";
    case kPlaceExact: return "the reference's state";
    case kPlaceMemory: return "the reference's memories, in ported code";
    default: return "taking it late";
  }
}

// Whether a RAM address isn't compared at this NMI: the stack below the stack pointer, or
// under ported code.
static int c_ram_skipped(size_t a, const EmuTaken *t) {
  return a >= 0x100u + kStackFloor && a <= 0x100u + t->stack_top;
}

// Hands the C side the reference's reads of PPU registers other than $2002 itself for the frame
// about to run (see nes_set_ppu_reads).
static void c_give_ppu_reads(Ref *r) {
  static NesAccess *buf;
  static size_t cap;
  const RefAccess *reads;
  size_t n = ref_ppu_reads(r, &reads);
  if(n > cap) {
    cap = n * 2;
    buf = realloc(buf, cap * sizeof(*buf));
  }
  for(size_t i = 0; i < n; i++) {
    buf[i] = (NesAccess){reads[i].addr, reads[i].value};
  }
  nes_set_ppu_reads(buf, n);
}

// Prints the heading of a difference report before its first line.
static void report(int *diffs, const RefStop *s, const EmuTaken *t, int late_before) {
  if((*diffs)++) {
    return;
  }
  fprintf(stderr,
          "dv_harness: NMI %u differs (the reference returns to $%04X, after cycle %llu; "
          "the C side placed it by %s):\n",
          s->nmi, s->return_pc, (unsigned long long)s->cycle, place_name(t->place));
  if(late_before) {
    fprintf(stderr, "  (the C side took NMI %u late, so the difference may come from where that "
                    "NMI ran; %s)\n",
            s->nmi - 1, emu_late_note());
  }
}

// Compares the C side with the reference at this NMI entry. `before` holds the reference's
// memories at the previous NMI. Returns the number of differences, after printing them.
static int c_compare(Ref *r, const RefStop *s, const EmuTaken *t, int late_before,
                     uint8_t *const before[kRefMemCount]) {
  int diffs = 0;
  const Cpu6502 *c = &g_nes.cpu;
  // Without the stack pointer, X after TSX differs as the stack pointers do: then X counts
  // if it is as far from the stack pointer on both sides (as the C side places NMIs).
  int same_x = c->x == s->x ||
               (!t->compare_sp && (uint8_t)(c->x - c->s) == (uint8_t)(s->x - s->sp));
  if(t->compare_regs &&
     (c->a != s->a || !same_x || c->y != s->y || (t->compare_sp && c->s != s->sp))) {
    report(&diffs, s, t, late_before);
    fprintf(stderr, "  registers: C side a=%02X x=%02X y=%02X sp=%02X, "
                    "reference a=%02X x=%02X y=%02X sp=%02X%s\n",
            c->a, c->x, c->y, c->s, s->a, s->x, s->y, s->sp,
            t->compare_sp ? "" : " (sp not compared: ported code is on the stack)");
  }
  for(int i = 0; i < kRefMemCount; i++) {
    size_t size;
    const uint8_t *ref = ref_mem(r, i, &size);
    int count = 0;
    for(size_t a = 0; a < size; a++) {
      if(i == kRefRam && c_ram_skipped(a, t)) {
        continue;
      }
      if(kCMem[i][a] != ref[a] && count++ < 16) {
        report(&diffs, s, t, late_before);
        fprintf(stderr, "  %-10s $%04X: C side %02X, reference %02X (before %02X)\n", kMemName[i],
                (unsigned)(kMemBase[i] + a), kCMem[i][a], ref[a], before[i][a]);
      }
    }
    if(count > 16) {
      fprintf(stderr, "  %-10s ... %d bytes differ in all\n", kMemName[i], count);
    }
  }
  const RefAccess *w;
  size_t nw = ref_apu_writes(r, &w);
  size_t nc = g_nes.apu_count;
  for(size_t i = 0; i < nw || i < nc; i++) {
    int in_ref = i < nw, in_c = i < nc && i < kApuLogMax;
    if(in_ref && in_c && w[i].addr == g_nes.apu_log[i].addr && w[i].value == g_nes.apu_log[i].value) {
      continue;
    }
    report(&diffs, s, t, late_before);
    fprintf(stderr, "  apu: %zu writes on the C side, %zu on the reference; #%zu is ", nc, nw, i);
    if(in_c) {
      fprintf(stderr, "$%04X=%02X on the C side, ", g_nes.apu_log[i].addr, g_nes.apu_log[i].value);
    } else {
      fprintf(stderr, "missing on the C side, ");
    }
    if(in_ref) {
      fprintf(stderr, "$%04X=%02X on the reference\n", w[i].addr, w[i].value);
    } else {
      fprintf(stderr, "missing on the reference\n");
    }
    break;
  }
  const RefAccess *pr;
  size_t npr = ref_ppu_reads(r, &pr);
  if(g_nes.ppu_read_pos != npr || g_nes.ppu_read_miss) {
    report(&diffs, s, t, late_before);
    fprintf(stderr, "  ppu reads (but $2002): %zu on the C side, %zu on the reference",
            g_nes.ppu_read_pos, npr);
    if(g_nes.ppu_read_miss) {
      fprintf(stderr, "; the C side read $%04X out of turn", g_nes.ppu_read_miss);
    }
    fprintf(stderr, "\n");
  }
  if(diffs) {
    fprintf(stderr, "  the C side's last instructions (cycle, pc, registers, R6):\n");
    emu_print_trace(stderr, 24);
  }
  return diffs;
}

static void print_ref_entry(const char *who, const RefCpu *e) {
  fprintf(stderr, "    %-9s %10llu  %04X  a=%02X x=%02X y=%02X s=%02X p=%02X\n", who,
          (unsigned long long)e->cycle, e->pc, e->a, e->x, e->y, e->sp, e->p & 0xCF);
}

static void print_c_entry(const char *who, const EmuTraceEntry *e) {
  fprintf(stderr, "    %-9s %10llu  %04X  a=%02X x=%02X y=%02X s=%02X p=%02X\n", who,
          (unsigned long long)e->cycles, e->pc, e->a, e->x, e->y, e->s, e->p);
}

// After a failure at an NMI: runs the frame before it again on both sides from their states
// at the previous NMI, tracing every instruction, and prints where the two first part. The
// comparison starts at the RTI ending the previous NMI: inside the NMI handler the C side
// waits for sprite 0 a different number of times, and its cycle count is set at the RTI.
// Where the C side ran a ported routine, the reference's instructions are skipped up to the
// one the C side carried on with, and cycle counts aren't compared after that.
static void replay(Ref *r, const uint8_t *ref_state, size_t ref_size, const EmuSnapshot *c_state,
                   uint8_t pad, const EmuNmi *target) {
  RefStop s;
  if(ref_load_state(r, ref_state, ref_size, &s)) {
    fprintf(stderr, "dv_harness: replay: the reference wouldn't load its state\n");
    return;
  }
  ref_set_trace(r, 1);
  ref_run_to_nmi(r, &s);
  ref_set_trace(r, 0);
  if(emu_load(c_state)) {
    fprintf(stderr, "dv_harness: no replay: the C side's state can't be put back while ported "
                    "code is on the game's call stack\n");
    return;
  }
  g_nes.pad[0] = pad;
  c_give_ppu_reads(r);
  emu_set_trace(1);
  char err[256];
  EmuTaken taken;
  emu_run_to_nmi(target, &taken, err, sizeof(err));
  emu_set_trace(0);

  const RefCpu *rt;
  const EmuTraceEntry *ct;
  size_t rn = ref_trace(r, &rt), cn = emu_trace(&ct), ri = 0, ci = 0;
  while(ri < rn && rt[ri].pc != kNmiRti) {
    ri++;
  }
  while(ci < cn && ct[ci].pc != kNmiRti) {
    ci++;
  }
  if(ri == rn || ci == cn) {
    ri = ci = 0;  // the frame from power-on: compare from the start
  }
  size_t start = ri, agree = 0;
  int cycles = 1;
  for(;;) {
    int skip = 0;
    while(ci < cn && ct[ci].ported) {
      ci++, skip = 1, cycles = 0;
    }
    while(skip && ci < cn && ri < rn && !(rt[ri].pc == ct[ci].pc && rt[ri].sp == ct[ci].s)) {
      ri++;
    }
    if(ri == rn || ci == cn || rt[ri].pc != ct[ci].pc || rt[ri].a != ct[ci].a ||
       rt[ri].x != ct[ci].x || rt[ri].y != ct[ci].y || rt[ri].sp != ct[ci].s ||
       (rt[ri].p & 0xCF) != ct[ci].p || (cycles && rt[ri].cycle != ct[ci].cycles)) {
      break;
    }
    ri++, ci++, agree++;
  }
  fprintf(stderr, "dv_harness: replayed the frame with traces: %zu instructions on the reference, "
                  "%zu on the C side; %zu agree from the RTI",
          rn, cn, agree);
  if(ri == rn || ci == cn) {
    fprintf(stderr, ", until %s stops\n", ri == rn ? "the reference" : "the C side");
  } else {
    fprintf(stderr, ", then they part:\n");
  }
  for(size_t k = ri > start + 6 ? ri - 6 : start; k < ri; k++) {
    print_ref_entry("both", &rt[k]);
  }
  for(size_t k = 0; k < 4; k++) {
    if(ri + k < rn) {
      print_ref_entry("reference", &rt[ri + k]);
    }
    if(ci + k < cn) {
      if(ct[ci + k].ported) {
        fprintf(stderr, "    %-9s             %04X  ported: %s\n", "C side", ct[ci + k].pc,
                ct[ci + k].ported);
      } else {
        print_c_entry("C side", &ct[ci + k]);
      }
    }
  }
}

// ---- The picture ---------------------------------------------------------------------------

enum {
  kPictureReports = 10,  // differences printed
  kPictureDumps = 100,   // frames written
};

static uint16_t c_picture[kScreenWidth * kScreenHeight];
static int c_picture_ready;
static const char *picture_dir;
static FILE *picture_diffs;
static long picture_compared, picture_same, picture_gaps, picture_gaps_same, picture_dumps;
static long picture_with_reads, picture_with_data_reads, picture_diff_reads;
static int picture_differed;  // the frame before did
static uint32_t picture_prev_frame;
static Nes picture_at_start;  // the console as the frame began

static void c_render(void) {
  ppu_render_frame(c_picture);
  c_picture_ready = 1;
  picture_at_start = g_nes;
}

// What the game changed for the PPU since the frame began: the renderer draws a frame from how things
// are as it begins, so a change while the original draws it shows as a difference.
static void describe_changes(char *what, size_t size) {
  const Nes *a = &picture_at_start, *b = &g_nes;
  size_t len = 0;
  what[0] = 0;
#define CHANGED(cond, name) \
  if((cond) && len < size) len += (size_t)snprintf(what + len, size - len, " %s", name)
  CHANGED(memcmp(a->bank_regs, b->bank_regs, 6) || (a->bank_select ^ b->bank_select) & 0x80, "chr-banks");
  CHANGED(a->mirroring != b->mirroring, "mirroring");
  CHANGED(a->ppu_mask != b->ppu_mask, "ppumask");
  CHANGED(memcmp(a->palette, b->palette, sizeof(a->palette)), "palette");
  CHANGED(memcmp(a->ciram, b->ciram, sizeof(a->ciram)), "nametables");
  CHANGED(memcmp(a->oam, b->oam, sizeof(a->oam)), "oam");
  CHANGED(a->ppu_ctrl != b->ppu_ctrl, "ppuctrl");
  CHANGED(a->vram_temp != b->vram_temp || a->fine_x != b->fine_x, "scroll");
#undef CHANGED
  if(!len) {
    snprintf(what, size, " nothing");
  }
}

static void write_pixels(const char *name, uint32_t nmi, const uint16_t *pixels) {
  char path[512];
  snprintf(path, sizeof(path), "%s/%06u_%s.raw", picture_dir, nmi, name);
  FILE *f = fopen(path, "wb");
  if(f) {
    fwrite(pixels, sizeof(pixels[0]), kScreenWidth * kScreenHeight, f);
    fclose(f);
  }
}

// The frame the reference finished at this NMI, against the C side's from the NMI before. Where the
// NMI was off for a while (frames passed without one), the C side's is older than that, and it counts
// apart.
static void compare_picture(Ref *r, const RefStop *s) {
  if(!c_picture_ready) {
    return;
  }
  int gap = s->ppu_frame != picture_prev_frame + 1;
  picture_prev_frame = s->ppu_frame;
  const uint16_t *ref = ref_screen(r);
  long count = 0;
  int first_x = 0, first_y = 0, min_y = kScreenHeight, max_y = -1, min_x = kScreenWidth, max_x = -1;
  for(int y = 0; y < kScreenHeight; y++) {
    for(int x = 0; x < kScreenWidth; x++) {
      if(c_picture[y * kScreenWidth + x] != ref[y * kScreenWidth + x]) {
        if(!count) {
          first_x = x, first_y = y;
        }
        count++;
        min_y = y < min_y ? y : min_y, max_y = y;
        min_x = x < min_x ? x : min_x, max_x = x > max_x ? x : max_x;
      }
    }
  }
  // A glitch that reads the PPU's registers while it draws (TAS #7044): $2007 knocks its scroll
  // counters off, which only a PPU run dot by dot would show.
  const RefAccess *reads;
  size_t read_count = ref_ppu_reads(r, &reads), data_reads = 0;
  for(size_t i = 0; i < read_count; i++) {
    data_reads += (reads[i].addr & 7) == 7;
  }
  picture_with_reads += read_count != 0;
  picture_with_data_reads += data_reads != 0;
  picture_gaps += gap;
  picture_gaps_same += gap && !count;
  picture_compared += !gap;
  picture_same += !gap && !count;
  if(count) {
    int i = first_y * kScreenWidth + first_x;
    char changed[100];
    describe_changes(changed, sizeof(changed));
    picture_diff_reads += !gap && data_reads != 0;
    if(!gap && (picture_compared - picture_same) <= kPictureReports) {
      fprintf(stderr, "dv_harness: picture, NMI %u: %ld pixels differ, x %d-%d, y %d-%d; first at "
                      "(%d, %d): C side %03X, reference %03X; changed since the frame began:%s\n",
              s->nmi, count, min_x, max_x, min_y, max_y, first_x, first_y, c_picture[i], ref[i],
              changed);
    }
    if(picture_diffs) {
      fprintf(picture_diffs, "%u frame=%u gap=%d lag=%d return=%04X count=%ld x=%d-%d y=%d-%d first=%d,%d "
                             "c=%03X ref=%03X ppu_reads=%zu of_2007=%zu changed:%s\n",
              s->nmi, s->ppu_frame, gap, !waiting_for_nmi(s->return_pc), s->return_pc, count, min_x,
              max_x, min_y, max_y, first_x, first_y, c_picture[i], ref[i], read_count, data_reads,
              changed);
    }
    if(picture_dir && !picture_differed && picture_dumps < kPictureDumps) {
      picture_dumps++;
      write_pixels("c", s->nmi, c_picture);
      write_pixels("ref", s->nmi, ref);
    }
  }
  picture_differed = count != 0;
}

// ---- The APU -------------------------------------------------------------------------------

enum {
  kApuReports = 12,       // differences printed
  kAudioRate = 48000,     // Mesen2's default
  kAudioBuffer = 4096,    // samples between NMIs; a frame with the NMI off for a long stretch writes more
  // Where the C APU is at a write or an NMI entry, as against the CPU cycle Mesen2 reports then.
  kApuWriteOffset = 1,
  kApuNmiOffset = 1,
};

static int16_t apu_samples[kAudioBuffer];
static FILE *apu_wav;
static uint32_t apu_wav_samples;
static uint64_t apu_compared, apu_differed, apu_field_diffs;

static void apu_write_wav_header(void) {
  uint32_t data = apu_wav_samples * 2;
  uint8_t h[44] = "RIFF____WAVEfmt ";
  uint32_t values[] = {36 + data, 16, 1 | 1u << 16, kAudioRate, kAudioRate * 2, 2 | 16u << 16, 0, data};
  memcpy(h + 4, &values[0], 4);
  memcpy(h + 16, &values[1], 20);
  memcpy(h + 36, "data", 4);
  memcpy(h + 40, &data, 4);
  fseek(apu_wav, 0, SEEK_SET);
  fwrite(h, 1, sizeof(h), apu_wav);
}

static void apu_flush_samples(void) {
  size_t count = apu_take_samples();
  if(apu_wav) {
    fwrite(apu_samples, 2, count, apu_wav);
    apu_wav_samples += (uint32_t)count;
  }
}

static void apu_run_to(uint64_t cycle) {
  while(g_apu.cycle < cycle) {
    uint64_t left = cycle - g_apu.cycle;
    apu_run(left > 256 ? 256 : (uint32_t)left);
    if(g_apu.sample_count > kAudioBuffer / 2) {
      apu_flush_samples();
    }
  }
}

// The reference's writes since the last NMI, at their cycles, then both side by side at this NMI.
static void compare_apu(Ref *r, const RefStop *s) {
  const RefAccess *writes;
  size_t count = ref_apu_writes(r, &writes);
  const uint64_t *cycles = ref_apu_write_cycles(r);
  for(size_t i = 0; i < count; i++) {
    apu_run_to(cycles[i] + kApuWriteOffset);
    apu_write(writes[i].addr, writes[i].value);
  }
  apu_run_to(s->cycle + kApuNmiOffset);
  apu_flush_samples();

  RefApu ref;
  ref_apu_state(r, &ref);
  char what[400] = "";
  size_t len = 0;
#define FIELD(name, mine, theirs)                                                               \
  if((unsigned)(mine) != (unsigned)(theirs) && len < sizeof(what)) {                            \
    len += (size_t)snprintf(what + len, sizeof(what) - len, " %s=%X/%X", name, (unsigned)(mine), \
                            (unsigned)(theirs));                                                 \
    apu_field_diffs++;                                                                          \
  }
  for(int i = 0; i < 2; i++) {
    const ApuPulse *p = &g_apu.pulse[i];
    const RefApuChannel *c = &ref.channel[i];
    static const char *const kNames[2][20] = {
      {"p1.duty", "p1.pos", "p1.period", "p1.timer", "p1.sweep_on", "p1.negate", "p1.sweep_period",
       "p1.shift", "p1.enabled", "p1.length", "p1.halt", "p1.start", "p1.loop", "p1.const", "p1.vol",
       "p1.divider", "p1.decay"},
      {"p2.duty", "p2.pos", "p2.period", "p2.timer", "p2.sweep_on", "p2.negate", "p2.sweep_period",
       "p2.shift", "p2.enabled", "p2.length", "p2.halt", "p2.start", "p2.loop", "p2.const", "p2.vol",
       "p2.divider", "p2.decay"},
    };
    const char *const *n = kNames[i];
    FIELD(n[0], p->duty, c->duty) FIELD(n[1], p->duty_pos, c->duty_pos)
    FIELD(n[2], p->period, c->period) FIELD(n[3], p->timer / 2, c->timer)
    FIELD(n[4], p->sweep_on, c->sweep_on) FIELD(n[5], p->sweep_negate, c->sweep_negate)
    FIELD(n[6], p->sweep_period, c->sweep_period ? c->sweep_period - 1 : 0) FIELD(n[7], p->sweep_shift, c->sweep_shift)
    FIELD(n[8], p->length.enabled, c->enabled) FIELD(n[9], p->length.counter, c->length)
    FIELD(n[10], p->length.halt, c->halt) FIELD(n[11], p->envelope.start, c->env_start)
    FIELD(n[12], p->envelope.loop, c->env_loop) FIELD(n[13], p->envelope.constant, c->env_constant)
    FIELD(n[14], p->envelope.volume, c->env_volume) FIELD(n[15], p->envelope.divider, c->env_divider)
    FIELD(n[16], p->envelope.decay, c->env_decay)
  }
  const ApuTriangle *t = &g_apu.triangle;
  const RefApuChannel *rt = &ref.channel[2];
  FIELD("t.period", t->period, rt->period) FIELD("t.timer", t->timer, rt->timer)
  FIELD("t.step", t->step, rt->step) FIELD("t.linear", t->linear_counter, rt->linear_counter)
  FIELD("t.reload_value", t->linear_reload_value, rt->linear_reload_value)
  FIELD("t.reload", t->linear_reload, rt->linear_reload) FIELD("t.enabled", t->length.enabled, rt->enabled)
  FIELD("t.length", t->length.counter, rt->length) FIELD("t.halt", t->length.halt, rt->halt)
  const ApuNoise *o = &g_apu.noise;
  const RefApuChannel *rn = &ref.channel[3];
  FIELD("n.period", o->period, rn->period) FIELD("n.timer", o->timer, rn->timer)
  FIELD("n.shift", o->shift, rn->shift) FIELD("n.mode", o->mode, rn->mode)
  FIELD("n.enabled", o->length.enabled, rn->enabled) FIELD("n.length", o->length.counter, rn->length)
  FIELD("n.halt", o->length.halt, rn->halt) FIELD("n.start", o->envelope.start, rn->env_start)
  FIELD("n.loop", o->envelope.loop, rn->env_loop) FIELD("n.const", o->envelope.constant, rn->env_constant)
  FIELD("n.vol", o->envelope.volume, rn->env_volume) FIELD("n.divider", o->envelope.divider, rn->env_divider)
  FIELD("n.decay", o->envelope.decay, rn->env_decay)
  FIELD("five_step", g_apu.five_step, ref.five_step)
#undef FIELD
  apu_compared++;
  if(len && apu_differed++ < kApuReports) {
    fprintf(stderr, "dv_harness: apu, NMI %u (cycle %llu), mine/reference:%s\n", s->nmi,
            (unsigned long long)s->cycle, what);
  }
}

// ---- The palette's colors -------------------------------------------------------------------

enum {
  kPaletteWorst = 4,    // most a channel of a color may be off, of 255
  kPaletteBufferSize = 64 * 1024,
};

// Mesen2's default palette, the 64 colors its UI starts with, read from its source (never copied here).
// Returns 0, or -1 if it can't find them.
static int read_mesen_palette(uint32_t colors[64]) {
  static const char kMarker[] = "UserPalette { get; set; } = new UInt32[64] {";
  static char text[kPaletteBufferSize];
  FILE *f = fopen(DV_MESEN_DIR "/UI/Config/NesConfig.cs", "rb");
  if(!f) {
    return -1;
  }
  size_t got = fread(text, 1, sizeof(text) - 1, f);
  fclose(f);
  text[got] = 0;
  const char *p = strstr(text, kMarker);
  if(!p) {
    return -1;
  }
  p += sizeof(kMarker) - 1;
  for(int i = 0; i < 64; i++) {
    p = strstr(p, "0x");
    if(!p) {
      return -1;
    }
    colors[i] = (uint32_t)strtoul(p, NULL, 16) & 0xFFFFFF;
    p += 2;
  }
  return 0;
}

static int check_palette(void) {
  uint32_t theirs[64], mine[kPpuColors];
  if(read_mesen_palette(theirs)) {
    fprintf(stderr, "dv_harness: can't read the palette from %s/UI/Config/NesConfig.cs\n", DV_MESEN_DIR);
    return 1;
  }
  ppu_make_palette(mine);
  long total = 0;
  int worst = 0, worst_color = 0;
  for(int c = 0; c < 64; c++) {
    for(int shift = 0; shift <= 16; shift += 8) {
      int d = abs((int)(mine[c] >> shift & 255) - (int)(theirs[c] >> shift & 255));
      total += d;
      if(d > worst) {
        worst = d;
        worst_color = c;
      }
    }
  }
  printf("dv_harness: palette: against Mesen2's default, off by %.2f of 255 on average; the worst is "
         "$%02X, by %d: ours %06X, Mesen2's %06X\n",
         (double)total / (64 * 3), worst_color, worst, mine[worst_color], theirs[worst_color]);
  return worst > kPaletteWorst;
}

static void usage(void) {
  fprintf(stderr, "usage: dv_harness --movie FILE.inp [--offset K] [--frames N] [--log FILE]\n"
                  "                  [--rewind-check K] [--compare] [--keep-going]\n"
                  "                  [--pass-through] [--emulate NAME[,NAME...]]\n"
                  "                  [--check-ports] [--fuzz-ports N] [--fuzz-only NAME[,NAME...]]\n"
                  "                  [--fuzz-trace NAME:SAMPLE] [--picture] [--picture-dump DIR]\n"
                  "                  [--apu] [--audio-wav PREFIX] [--check-palette] [--coverage FILE]\n"
                  "                  [--sweep-scripts FILE] [--sweep-passes N] [--state-out FILE]\n"
                  "                  [--sav FILE] [--sav-out FILE] [--rom ROM] [--home DIR]\n");
  exit(2);
}

int main(int argc, char **argv) {
  const char *rom = DV_DEFAULT_ROM, *home = DV_DEFAULT_HOME;
  const char *movie_path = NULL, *log_path = NULL, *emulate = "", *fuzz_only = NULL, *coverage_path = NULL;
  long frames = -1, rewind = 0, fuzz = 0, sweep_passes = 4;
  const char *sweep_path = NULL, *state_out = NULL;
  int compare = 0, keep_going = 0, pass_through = 0, check_ports = 0, picture = 0, free_run = 0;
  int apu_check = 0;
  const char *audio_prefix = NULL, *sav_path = NULL, *sav_out = NULL;
  Movie movie = {0};
  for(int i = 1; i < argc; i++) {
    const char *arg = argv[i];
    if(!strcmp(arg, "--compare")) {
      compare = 1;
      continue;
    }
    if(!strcmp(arg, "--check-ports")) {
      check_ports = 1;
      continue;
    }
    if(!strcmp(arg, "--keep-going")) {
      keep_going = 1;
      continue;
    }
    if(!strcmp(arg, "--pass-through")) {
      pass_through = 1;
      continue;
    }
    if(!strcmp(arg, "--picture")) {
      picture = 1;
      continue;
    }
    if(!strcmp(arg, "--free-run")) {
      free_run = 1;
      continue;
    }
    if(!strcmp(arg, "--apu")) {
      apu_check = 1;
      continue;
    }
    if(!strcmp(arg, "--check-palette")) {
      return check_palette();
    }
    if(i + 1 >= argc) {
      usage();
    }
    const char *val = argv[++i];
    if(!strcmp(arg, "--rom")) rom = val;
    else if(!strcmp(arg, "--home")) home = val;
    else if(!strcmp(arg, "--movie")) movie_path = val;
    else if(!strcmp(arg, "--offset")) movie.offset = (uint32_t)atol(val);
    else if(!strcmp(arg, "--frames")) frames = atol(val);
    else if(!strcmp(arg, "--log")) log_path = val;
    else if(!strcmp(arg, "--rewind-check")) rewind = atol(val);
    else if(!strcmp(arg, "--emulate")) emulate = val;
    else if(!strcmp(arg, "--fuzz-ports")) fuzz = atol(val);
    else if(!strcmp(arg, "--fuzz-only")) fuzz_only = val;
    else if(!strcmp(arg, "--sweep-scripts")) sweep_path = val;
    else if(!strcmp(arg, "--state-out")) state_out = val;
    else if(!strcmp(arg, "--sweep-passes")) sweep_passes = atol(val);
    else if(!strcmp(arg, "--coverage")) coverage_path = val;
    else if(!strcmp(arg, "--picture-dump")) picture_dir = val;
    else if(!strcmp(arg, "--sav")) sav_path = val;
    else if(!strcmp(arg, "--sav-out")) sav_out = val;
    else if(!strcmp(arg, "--audio-wav")) audio_prefix = val;
    else if(!strcmp(arg, "--fuzz-trace")) {
      char *colon = strchr(val, ':');
      if(!colon) usage();
      *colon = 0;
      emu_set_fuzz_trace(val, (unsigned)atol(colon + 1));
    }
    else usage();
  }
  if(!movie_path || (compare && rewind) || (fuzz && !compare) || (picture && !compare) ||
     (apu_check && !compare) || (audio_prefix && !apu_check)) {
    usage();  // rewinding while comparing would need the C side rewound too: not yet
  }
  if(!c_known_names(emulate)) {
    return 2;
  }
  EmuHook *ported = malloc(sizeof(EmuHook) * kPortedRoutineCount);
  size_t ported_count = pass_through ? 0 : c_choose_ported(emulate, ported);
  emu_set_hooks(ported, ported_count);
  emu_set_port_check(check_ports);
  uint8_t *coverage = coverage_path ? calloc(kPrgSize, 1) : NULL;
  emu_set_coverage(coverage);
  if(picture) {
    emu_set_frame_hook(c_render);
    if(picture_dir) {
      char path[512];
      snprintf(path, sizeof(path), "%s/diffs.txt", picture_dir);
      picture_diffs = fopen(path, "w");
    }
  }
  if(load_movie(movie_path, &movie)) {
    fprintf(stderr, "dv_harness: can't read %s\n", movie_path);
    return 1;
  }
  if(frames < 0) {
    frames = (long)movie.count - (long)movie.offset + 300;
  }
  FILE *log = NULL;
  if(log_path && !(log = fopen(log_path, "w"))) {
    fprintf(stderr, "dv_harness: can't write %s\n", log_path);
    return 1;
  }

  char err[256];
  Ref *r = ref_open(rom, home, sav_path, movie_input, &movie, err, sizeof(err));
  if(!r) {
    fprintf(stderr, "dv_harness: %s\n", err);
    return 1;
  }

  if(apu_check) {
    apu_reset(kAudioRate, apu_samples, kAudioBuffer);
    if(audio_prefix) {
      char path[512];
      snprintf(path, sizeof(path), "%s_c.wav", audio_prefix);
      apu_wav = fopen(path, "wb");
      if(apu_wav) {
        fseek(apu_wav, 44, SEEK_SET);
      }
      snprintf(path, sizeof(path), "%s_ref.wav", audio_prefix);
      ref_record_audio(r, path);
    }
  }

  // The reference's memories at the previous NMI, for "before" in difference reports.
  uint8_t *before[kRefMemCount] = {0};
  if(compare) {
    if(c_power_on(r, rom)) {
      ref_close(r, NULL);
      return 1;
    }
    for(int i = 0; i < kRefMemCount; i++) {
      before[i] = malloc(kCMemSize[i]);
      memcpy(before[i], kCMem[i], kCMemSize[i]);
    }
  }
  int status = 0;
  if(fuzz > 0) {
    uint8_t *waited = calloc(ported_count + 1, 1);
    for(int waits = 0; waits <= 1; waits++) {
      unsigned long compared = 0, skipped = 0, differed = 0;
      int uncovered = 0;
      for(size_t i = 0; i < ported_count; i++) {
        // A far call with a made-up id calls whatever its table entry points to, which may play
        // with the stack, and a made-up jump_table entry of 0 or 1 reads as a return; the movies
        // cover the real ones.
        if(!ported[i].fn || !strncmp(ported[i].name, "far_call_", 9) ||
           !strcmp(ported[i].name, "jump_table") || (waits && !waited[i]) ||
           (fuzz_only && !name_in_list(ported[i].name, fuzz_only))) {
          continue;
        }
        EmuFuzzCounts counts;
        clock_t started = clock();
        unsigned samples = (unsigned)(waits ? fuzz / 10 : fuzz);
        emu_fuzz_port(i, samples, fnv1a((const uint8_t *)ported[i].name, strlen(ported[i].name), 2166136261u),
                      waits, &counts);
        fprintf(stderr, "dv_harness: port fuzz%s: %-30s %6lu compared %6lu skipped %4lu differed "
                        "(%.1f s)\n",
                waits ? " through waits" : "", ported[i].name, counts.compared, counts.skipped,
                counts.differed, (double)(clock() - started) / CLOCKS_PER_SEC);
        compared += counts.compared;
        skipped += counts.skipped;
        differed += counts.differed;
        uncovered += !counts.compared;
        waited[i] = counts.skipped != 0;
      }
      printf("dv_harness: port fuzz%s: %lu samples compared, %lu skipped (the original %s), "
             "%lu differed; %d routines with no sample compared\n",
             waits ? " through waits" : "", compared, skipped,
             waits ? "didn't return or read the stack" : "waited or didn't return", differed,
             uncovered);
      status |= differed != 0;
    }
    free(waited);
  }

  struct timespec t0, t1;
  timespec_get(&t0, TIME_UTC);
  char line[256];
  char (*span)[256] = rewind > 0 ? malloc((size_t)rewind * sizeof(*span)) : NULL;
  uint8_t *state = NULL;
  size_t state_size = 0;
  long lag = 0, rewinds = 0, rewind_errors = 0, matched = 0, compared = 0;
  long places[kPlaceLate + 1] = {0};
  int late_before = 0;  // the C side took the previous NMI late
  uint32_t prev_polls = 0;
  uint8_t *prev_state = NULL;  // both sides at the previous NMI, when comparing
  size_t prev_state_size = 0;
  EmuSnapshot *prev_c = compare ? malloc(sizeof(EmuSnapshot)) : NULL;
  RefStop stop = {0}, saved = {0};
  if(compare) {
    size_t routines = 0;
    for(size_t i = 0; i < ported_count; i++) {
      routines += ported[i].fn != NULL;  // the rest is original code they do inline
    }
    printf("dv_harness: the C side runs %zu ported routines in place of the original code\n",
           routines);
  }
  for(long n = 0; n < frames; n++) {
    if(ref_run_to_nmi(r, &stop)) {
      fprintf(stderr, "dv_harness: emulation stopped after %u NMIs\n", stop.nmi);
      status = 1;
      break;
    }
    format_line(r, &stop, line, sizeof(line));
    lag += !waiting_for_nmi(stop.return_pc);
    if(log) {
      fprintf(log, "%s\n", line);
    }
    if(compare) {
      // Between two NMIs the game sees the buttons from the poll before the first.
      uint8_t pad = prev_polls ? movie_input(&movie, prev_polls - 1) : 0;
      g_nes.pad[0] = pad;
      c_give_ppu_reads(r);
      prev_polls = stop.polls;
      EmuNmi target = {stop.cycle, stop.rti_cycle, stop.return_pc, stop.a, stop.x, stop.y, stop.sp,
                       ref_mem(r, kRefRam, NULL), ref_mem(r, kRefSram, NULL),
                       ref_mem(r, kRefCiram, NULL), ref_mem(r, kRefPalette, NULL),
                       ref_mem(r, kRefOam, NULL)};
      EmuTaken taken = {0};
      int ran;
      if(free_run) {
        ran = emu_run_frame(err, sizeof(err)) == 0;
        taken.place = kPlaceWaitC;
        taken.compare = 1;
        taken.stack_top = 0xFF;  // the stack under ported code isn't compared
        if(!waiting_for_nmi(stop.return_pc)) {
          // read_joypads and read_joypads_once, get_input and read_input's helpers.
          int spin = (stop.return_pc >= 0xEB41 && stop.return_pc < 0xEB9C) ||
                     (stop.return_pc >= 0x958D && stop.return_pc < 0x9630);
          if(!spin) {
            printf("dv_harness: NMI %u is a lag NMI in the reference, at $%04X: as far as the C side on "
                   "its own can match\n", stop.nmi, stop.return_pc);
            break;
          }
          taken.compare = 0;
        }
      } else {
        ran = emu_run_to_nmi(&target, &taken, err, sizeof(err)) == 0;
      }
      if(!ran) {
        fprintf(stderr, "dv_harness: NMI %u: the C side %s\n  its last instructions:\n", stop.nmi, err);
        emu_print_trace(stderr, 24);
      } else {
        places[taken.place]++;
        compared += taken.compare;
      }
      int same = ran && (!taken.compare || !c_compare(r, &stop, &taken, late_before, before));
      if(picture && ran) {
        compare_picture(r, &stop);
      }
      if(apu_check) {
        compare_apu(r, &stop);
      }
      late_before = ran && !taken.compare;
      if(!same && prev_state) {
        replay(r, prev_state, prev_state_size, prev_c, pad, &target);
      }
      if(same) {
        matched += taken.compare;
      } else if(!ran || !keep_going) {
        status = 1;
        break;
      } else {
        // Carry on from the reference's state, but leave the C side's own stack alone.
        status = 1;
        for(int i = 0; i < kRefMemCount; i++) {
          const uint8_t *ref = ref_mem(r, i, NULL);
          for(size_t a = 0; a < kCMemSize[i]; a++) {
            if(i != kRefRam || !c_ram_skipped(a, &taken)) {
              kCMem[i][a] = ref[a];
            }
          }
        }
        Cpu6502 *c = &g_nes.cpu;
        if(taken.compare_regs) {
          c->a = stop.a, c->y = stop.y;
          c->p = (stop.p & ~(kFlagB | kFlagU)) | kFlagI;
        }
        if(taken.compare_sp) {
          c->x = stop.x, c->s = stop.sp;  // X may be relative to the stack pointer otherwise
        }
      }
      g_nes.apu_count = 0;
      for(int i = 0; i < kRefMemCount; i++) {
        memcpy(before[i], ref_mem(r, i, NULL), kCMemSize[i]);
      }
      // Both sides as they are now, to replay the next frame if it goes wrong. The C side's
      // state can't be saved while ported code is on the game's call stack.
      free(prev_state);
      prev_state = NULL;
      if(emu_save(prev_c) == 0) {
        prev_state = ref_save_state(r, &prev_state_size);
      }
    }
    if(!rewind) {
      continue;
    }
    if(state) {
      long k = (long)(stop.nmi - saved.nmi) - 1;
      memcpy(span[k], line, sizeof(line));
      if(k + 1 == rewind) {
        // Back to the saved NMI; the same input must give the same K lines.
        if(ref_load_state(r, state, state_size, &stop) || stop.nmi != saved.nmi) {
          fprintf(stderr, "dv_harness: loading the state from NMI %u failed\n", saved.nmi);
          status = 1;
          break;
        }
        for(long j = 0; j < rewind; j++) {
          ref_run_to_nmi(r, &stop);
          format_line(r, &stop, line, sizeof(line));
          if(strcmp(line, span[j]) && rewind_errors++ < 10) {
            fprintf(stderr, "dv_harness: replay after a rewind differs:\n  first  %s\n  replay %s\n",
                    span[j], line);
          }
        }
        rewinds++;
        free(state);
        state = NULL;
      }
    }
    if(!state && stop.nmi % rewind == 0) {
      state = ref_save_state(r, &state_size);
      saved = stop;
    }
  }
  timespec_get(&t1, TIME_UTC);
  double secs = (double)(t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;

  printf("dv_harness: %u NMIs, %u input polls, %ld interrupted game code, %.1f s (%.0f NMIs/s)\n",
         stop.nmi, stop.polls, lag, secs, secs > 0 ? stop.nmi / secs : 0.0);
  if(rewind) {
    printf("dv_harness: %ld rewinds of %ld NMIs, %ld replayed lines differ\n", rewinds, rewind,
           rewind_errors);
    status |= rewind_errors != 0;
  }
  if(compare) {
    printf("dv_harness: NMIs placed %s %ld, %s %ld, at %s %ld, at %s %ld, %s %ld\n",
           place_name(kPlaceWait6502), places[kPlaceWait6502], place_name(kPlaceWaitC),
           places[kPlaceWaitC], place_name(kPlaceExact), places[kPlaceExact],
           place_name(kPlaceMemory), places[kPlaceMemory], place_name(kPlaceLate),
           places[kPlaceLate]);
    printf("dv_harness: the C side matched the reference at %ld of %ld NMIs compared (%u in all)\n",
           matched, compared, stop.nmi);
    unsigned long run, resumed;
    emu_original_counts(&run, &resumed);
    printf("dv_harness: for lag NMIs in ported code, %lu calls ran as original code, and %lu "
           "calls went on as original code after a wait\n",
           run, resumed);
    int unused = 0;
    for(size_t i = 0; i < ported_count; i++) {
      if(ported[i].fn && !emu_hook_calls(i)) {
        printf("%s %s", unused++ ? "," : "dv_harness: ported routines that never ran:",
               ported[i].name);
      }
    }
    if(unused) {
      printf("\n");
    }
  }
  if(check_ports) {
    unsigned long checked, differed;
    emu_port_check_counts(&checked, &differed);
    printf("dv_harness: port checks: %lu calls checked against the original, %lu differed\n",
           checked, differed);
    status |= differed != 0;
  }
  if(apu_check) {
    printf("dv_harness: apu: %llu NMIs compared, %llu differ (%llu fields in all)\n",
           (unsigned long long)apu_compared, (unsigned long long)apu_differed,
           (unsigned long long)apu_field_diffs);
    status |= apu_differed != 0;
    if(apu_wav) {
      apu_write_wav_header();
      fclose(apu_wav);
    }
  }
  if(picture) {
    printf("dv_harness: picture: %ld of %ld frames the same; of %ld after frames with no NMI, %ld\n",
           picture_same, picture_compared, picture_gaps, picture_gaps_same);
    printf("dv_harness: picture: %ld frames had reads of PPU registers (glitches), %ld of $2007; of the "
           "%ld that differ, %ld had reads of $2007\n",
           picture_with_reads, picture_with_data_reads, picture_compared - picture_same,
           picture_diff_reads);
    status |= picture_same != picture_compared;
    if(picture_diffs) {
      fclose(picture_diffs);
    }
  }
  if(state_out) {
    FILE *f = fopen(state_out, "wb");
    for(int i = 0; f && i < kRefMemCount; i++) {
      size_t size;
      const uint8_t *mem = ref_mem(r, i, &size);
      fwrite(mem, 1, size, f);
    }
    if(f) {
      fclose(f);
    } else {
      fprintf(stderr, "dv_harness: can't write %s\n", state_out);
      status = 1;
    }
  }
  if(sweep_path && compare) {
    static uint16_t sweep_list[0x4000];
    unsigned count = 0, place;
    FILE *f = fopen(sweep_path, "r");
    while(f && count < 0x4000 && fscanf(f, "%x", &place) == 1) {
      sweep_list[count++] = (uint16_t)place;
    }
    if(f) {
      fclose(f);
    }
    for(size_t i = 0; count && i < ported_count; i++) {
      if(!strcmp(ported[i].name, "script_loop")) {
        EmuFuzzCounts counts;
        emu_sweep_scripts(i, sweep_list, count, (unsigned)sweep_passes, 1, &counts);
        printf("dv_harness: script sweep: %lu samples compared, %lu skipped, %lu differed\n", counts.compared,
               counts.skipped, counts.differed);
        status |= counts.differed != 0;
      }
    }
    if(!count) {
      fprintf(stderr, "dv_harness: no places in %s\n", sweep_path);
      status = 1;
    }
  }
  if(coverage) {
    FILE *f = fopen(coverage_path, "wb");
    if(!f || fwrite(coverage, 1, kPrgSize, f) != kPrgSize) {
      fprintf(stderr, "dv_harness: can't write %s\n", coverage_path);
      status = 1;
    }
    if(f) {
      fclose(f);
    }
    emu_set_coverage(NULL);
    free(coverage);
  }
  print_progress(r);
  ref_close(r, sav_out);
  if(log) {
    fclose(log);
  }
  for(int i = 0; i < kRefMemCount; i++) {
    free(before[i]);
  }
  free(c_rom);
  free(ported);
  free(prev_state);
  free(prev_c);
  free(state);
  free(span);
  free(movie.pads);
  return status;
}
