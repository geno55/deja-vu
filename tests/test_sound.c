// Checks the ported sound driver (src/sound.c) against the original on the ROM's real sound data,
// without the emulator: every sound played alone from silence to its end, or until music that repeats
// has started again, then a long random mix of sounds over each other, stops and deferred updates. The original runs as 6502 code, the port as C,
// and after every call the registers, RAM (the stack aside), mapper and APU writes must be the same.
// It also lists the driver's instructions the real data never runs, which only the fuzz compares.
//
// usage: test_sound ROM
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "funcs.h"
#include "nes_hw.h"
#include "variables.h"

enum {
  kEntries = 50,         // in sound_table
  kMaxFrames = 400000,   // a sound that hasn't ended or repeated by then fails the test
  kAfterEnd = 30,        // frames to go on after a sound has ended, or repeating music has restarted
  kMixFrames = 400000,   // about 2 hours
  kMaxWrites = 64,       // APU writes per call
  kFirstCode = 0xF3F5,   // sound_reset
  kAfterCode = 0xFADA,   // sound_jump_table
  kJumpList = 0xF46D,    // the list after the JSR in sound_start_by_type
  kJumpListEnd = 0xF477,
  kRestart = 0xF449,     // sound_check_repeat's jsr start_sound: the music comes round again
  kMaxReports = 8,
};

// What a call can change.
typedef struct Side {
  Cpu6502 cpu;
  uint8_t ram[0x800];
  uint8_t bank_select, bank_regs[8];
  uint32_t apu_count;
  NesAccess apu[kMaxWrites];
} Side;

static Side original, port;
static uint8_t covered[0x10000];
static unsigned long differences, compares, writes;

static void load(const Side *s) {
  g_nes.cpu = s->cpu;
  memcpy(g_nes.ram, s->ram, sizeof(s->ram));
  g_nes.bank_select = s->bank_select;
  memcpy(g_nes.bank_regs, s->bank_regs, sizeof(s->bank_regs));
  g_nes.apu_count = 0;
}

static void save(Side *s) {
  s->cpu = g_nes.cpu;
  memcpy(s->ram, g_nes.ram, sizeof(s->ram));
  s->bank_select = g_nes.bank_select;
  memcpy(s->bank_regs, g_nes.bank_regs, sizeof(s->bank_regs));
  s->apu_count = g_nes.apu_count;
  for(uint32_t i = 0; i < g_nes.apu_count && i < kMaxWrites; i++) {
    s->apu[i] = g_nes.apu_log[i];
  }
}

// The original at addr on the 6502, until its RTS comes back.
static void run_original(uint16_t addr) {
  Cpu6502 *c = &g_nes.cpu;
  uint8_t s0 = c->s;
  emu_push(0xFF);
  emu_push(0xFF);
  c->pc = addr;
  while(c->pc != 0 || c->s != s0) {
    covered[c->pc] = 1;
    if(cpu_step(c)) {
      fprintf(stderr, "test_sound: unknown opcode at $%04X\n", c->pc);
      exit(1);
    }
  }
}

// Reports how the two sides differ after `what`.
static void compare(const char *what, int sound, unsigned long frame) {
  const Cpu6502 *o = &original.cpu, *p = &port.cpu;
  char text[300] = "";
  size_t len = 0;
#define DIFF(...) (len < sizeof(text) ? len += (size_t)snprintf(text + len, sizeof(text) - len, __VA_ARGS__) : 0)
  uint8_t flags = kFlagN | kFlagZ | kFlagC;
  if(p->a != o->a) DIFF(" a=%02X/%02X", p->a, o->a);
  if(p->x != o->x) DIFF(" x=%02X/%02X", p->x, o->x);
  if(p->y != o->y) DIFF(" y=%02X/%02X", p->y, o->y);
  if(p->s != o->s) DIFF(" s=%02X/%02X", p->s, o->s);
  if((p->p ^ o->p) & flags) DIFF(" p=%02X/%02X", p->p & flags, o->p & flags);
  int shown = 0;
  for(int a = 0; a < 0x800; a++) {
    int stack = a >= 0x1A0 && a < 0x200;  // above the sound driver's loop stacks
    if(!stack && port.ram[a] != original.ram[a] && shown++ < 6) {
      DIFF(" $%04X=%02X/%02X", a, port.ram[a], original.ram[a]);
    }
  }
  if(port.bank_select != original.bank_select ||
     memcmp(port.bank_regs, original.bank_regs, sizeof(port.bank_regs))) {
    DIFF(" mapper");
  }
  if(port.apu_count != original.apu_count) {
    DIFF(" apu writes %lu/%lu", (unsigned long)port.apu_count, (unsigned long)original.apu_count);
  } else {
    for(uint32_t i = 0; i < port.apu_count && i < kMaxWrites; i++) {
      if(port.apu[i].addr != original.apu[i].addr || port.apu[i].value != original.apu[i].value) {
        DIFF(" apu write %lu: $%04X=%02X/$%04X=%02X", (unsigned long)i, port.apu[i].addr,
             port.apu[i].value, original.apu[i].addr, original.apu[i].value);
        break;
      }
    }
  }
#undef DIFF
  compares++;
  writes += original.apu_count;
  if(len && differences++ < kMaxReports) {
    fprintf(stderr, "test_sound: %s, sound %d, frame %lu, port/original:%s\n", what, sound, frame, text);
  }
}

static void both_play(uint8_t sound, unsigned long frame) {
  load(&original);
  emu_set_a(sound);
  run_original(kPlaySound);
  save(&original);
  load(&port);
  play_sound(sound);
  save(&port);
  compare("play_sound", sound, frame);
}

static void both_update(int sound, unsigned long frame) {
  load(&original);
  run_original(kSoundUpdate);
  save(&original);
  load(&port);
  sound_update();
  save(&port);
  compare("sound_update", sound, frame);
}

// Silence, in a state the game could be in, on both sides.
static void reset_sides(void) {
  memset(&original, 0, sizeof(original));
  original.cpu.s = 0xFD;
  original.bank_regs[6] = 12;  // bank 6, where the game's code plays sounds from
  original.bank_regs[7] = 13;
  original.ram[0x3C] = 0x80;   // sound_table_ptr
  load(&original);
  current_bank = 6;
  stack_index = 0xFF;
  save(&original);
  port = original;
}

static int any_channel_on(void) {
  for(int slot = 0; slot < 0xD8; slot += 0x18) {
    if(original.ram[0x700 + slot] & 0x80) {
      return 1;
    }
  }
  return 0;
}

// Only entries with a real header: entry 8 points into instrument data.
static int is_sound(int sound) {
  load(&original);
  g_nes.bank_regs[6] = 10;
  g_nes.bank_regs[7] = 11;
  uint16_t header = (uint16_t)(cpu_bus_read((uint16_t)(0x8000 + (sound - 1) * 2)) |
                               cpu_bus_read((uint16_t)(0x8001 + (sound - 1) * 2)) << 8);
  return cpu_bus_read(header) < 5;
}

static uint32_t rng = 0x1985ABCD;

static uint32_t next_random32(void) {
  rng ^= rng << 13;
  rng ^= rng >> 17;
  rng ^= rng << 5;
  return rng;
}

// The bytes of 6502 instructions at addr.
static int instruction_length(uint8_t op) {
  if(op == 0x20 || (op & 0x0F) >= 0x0C || (op & 0x1F) == 0x19) return 3;
  if((op & 0x0F) == 0x08 || (op & 0x0F) == 0x0A || op == 0x00 || op == 0x40 || op == 0x60) return 1;
  return 2;
}

static void report_coverage(void) {
  int run = 0, total = 0;
  printf("test_sound: never run on the real data:");
  load(&original);
  for(unsigned a = kFirstCode, first = 0; a < kAfterCode;) {
    if(a == kJumpList) {
      a = kJumpListEnd;
      continue;
    }
    total++;
    run += covered[a];
    if(!covered[a] && !first) {
      first = a;
    }
    a += (unsigned)instruction_length(cpu_bus_read((uint16_t)a));
    if(first && (a >= kAfterCode || covered[a])) {
      printf(" $%04X-$%04X", first, a - 1);
      first = 0;
    }
  }
  printf("\ntest_sound: %d of the driver's %d instructions run\n", run, total);
}

int main(int argc, char **argv) {
  if(argc != 2) {
    fprintf(stderr, "usage: test_sound ROM\n");
    return 2;
  }
  FILE *f = fopen(argv[1], "rb");
  if(!f) {
    fprintf(stderr, "test_sound: can't read %s\n", argv[1]);
    return 2;
  }
  static uint8_t rom[16 + kPrgSize + kChrSize];
  size_t got = fread(rom, 1, sizeof(rom), f);
  fclose(f);
  if(got != sizeof(rom)) {
    fprintf(stderr, "test_sound: %s isn't the size of the game's ROM\n", argv[1]);
    return 2;
  }
  nes_init(rom + 16, rom + 16 + kPrgSize);

  // Every sound alone.
  int sounds = 0, repeating = 0, unfinished = 0;
  unsigned long frames = 0, longest = 0;
  reset_sides();
  for(int sound = 1; sound <= kEntries; sound++) {
    if(!is_sound(sound)) {
      continue;
    }
    sounds++;
    reset_sides();
    covered[kRestart] = 0;
    both_play((uint8_t)sound, 0);
    unsigned long frame = 1;
    for(int after = 0; frame < kMaxFrames && after < kAfterEnd; frame++) {
      both_update(sound, frame);
      after = any_channel_on() && !covered[kRestart] ? 0 : after + 1;
    }
    repeating += covered[kRestart];
    unfinished += frame >= kMaxFrames;
    longest = frame > longest ? frame : longest;
    frames += frame;
  }
  covered[kRestart] = repeating != 0;
  printf("test_sound: %d sounds played alone, %lu frames, the longest %lu; %d are music that repeats, "
         "all played until they did; %d didn't end\n",
         sounds, frames, longest, repeating, unfinished);

  // A random mix, with the game busy now and then: in play_sound or switching banks, an NMI only
  // asks for a double step next time.
  int played[kEntries], count = 0;
  for(int sound = 1; sound <= kEntries; sound++) {
    if(is_sound(sound)) {
      played[count++] = sound;
    }
  }
  reset_sides();
  for(unsigned long frame = 0; frame < kMixFrames; frame++) {
    uint32_t r = next_random32();
    if(!(r & 0x7F)) {
      both_play(r & 0x700 ? (uint8_t)played[(r >> 16) % (unsigned)count] : 0, frame);
    }
    int busy = (r >> 12 & 0x1F) == 0;
    if(busy) {
      original.ram[0xF0] |= r & 0x800 ? 0x04 : 0x08;
      port.ram[0xF0] = original.ram[0xF0];
    }
    both_update(-1, frame);
    if(busy) {
      original.ram[0xF0] &= 0xF3;
      port.ram[0xF0] = original.ram[0xF0];
    }
  }
  printf("test_sound: %d frames of mixed sounds\n", kMixFrames);
  report_coverage();
  printf("test_sound: %lu calls compared, %lu APU writes, %lu different\n", compares, writes,
         differences);
  return differences != 0 || unfinished != 0;
}
