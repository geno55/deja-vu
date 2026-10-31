// Runs the game on its own, as the native program will: no reference, no 6502 needed in normal play.
// A monkey presses random buttons for a long time, which walks through the title, the file menus and
// the game's commands. It checks that the game keeps going: a C loop with no wait would hang it (the
// test's timeout), and an error stops it. It also says how far it got, so a run that sits at the title
// is seen for what it is.
//
// usage: test_game ROM [FRAMES]
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "funcs.h"
#include "nes_hw.h"
#include "ppu_render.h"
#include "variables.h"

enum {
  kDefaultFrames = 50000,  // about a quarter of an hour of play, in about 12 seconds
  kSeed = 0x1985ABCD,
};

static uint16_t picture[kScreenWidth * kScreenHeight];

static void render(void) {
  ppu_render_frame(picture);
}

static uint32_t rng = kSeed;

static uint32_t next_random32(void) {
  rng ^= rng << 13;
  rng ^= rng >> 17;
  rng ^= rng << 5;
  return rng;
}

int main(int argc, char **argv) {
  if(argc < 2) {
    fprintf(stderr, "usage: test_game ROM [FRAMES]\n");
    return 2;
  }
  FILE *f = fopen(argv[1], "rb");
  if(!f) {
    fprintf(stderr, "test_game: can't read %s\n", argv[1]);
    return 2;
  }
  static uint8_t rom[16 + kPrgSize + kChrSize];
  size_t got = fread(rom, 1, sizeof(rom), f);
  fclose(f);
  if(got != sizeof(rom)) {
    fprintf(stderr, "test_game: %s isn't the size of the game's ROM\n", argv[1]);
    return 2;
  }
  long frames = argc > 2 ? atol(argv[2]) : kDefaultFrames;

  emu_set_frame_hook(render);
  if(dv_boot(rom + 16, rom + 16 + kPrgSize)) {
    fprintf(stderr, "test_game: can't start the game's coroutine\n");
    return 1;
  }

  static uint8_t room_seen[256], picture_seen[0x10000];
  int rooms = 0, pictures = 0;
  long frame = 0;
  uint8_t pad = 0;
  unsigned hold = 0;
  char err[256];
  for(; frame < frames; frame++) {
    // Buttons held for a while, as a hand does, mostly the d-pad and A.
    if(!hold) {
      uint32_t r = next_random32();
      hold = 1 + (r >> 8 & 31);
      pad = 0;
      switch(r & 7) {
        case 0: pad = 0x10; break;  // up
        case 1: pad = 0x20; break;  // down
        case 2: pad = 0x40; break;  // left
        case 3: pad = 0x80; break;  // right
        case 4:
        case 5: pad = 0x01; hold = 2; break;  // A
        case 6: pad = 0x02; hold = 2; break;  // B
        default: pad = (r >> 16 & 15) ? 0 : 0x08; break;  // start, rarely
      }
    }
    hold--;
    g_nes.pad[0] = pad;
    if(emu_run_frame(err, sizeof(err))) {
      fprintf(stderr, "test_game: frame %ld: %s\n", frame, err);
      return 1;
    }
    g_nes.apu_count = 0;
    if(!room_seen[room]) {
      room_seen[room] = 1;
      rooms++;
    }
    unsigned hash = 2166136261u;
    for(int i = 0; i < kScreenWidth * kScreenHeight; i += 97) {
      hash = (hash ^ picture[i]) * 16777619u;
    }
    hash = (hash ^ hash >> 16) & 0xFFFF;
    if(!picture_seen[hash]) {
      picture_seen[hash] = 1;
      pictures++;
    }
  }
  unsigned long run, resumed;
  emu_original_counts(&run, &resumed);
  int ran = 0;
  for(size_t i = 0; i < kPortedRoutineCount; i++) {
    ran += kPortedRoutines[i].fn && emu_hook_calls(i);
  }
  printf("test_game: %ld frames; %d different pictures (by a hash of some pixels), %d rooms; "
         "%d ported routines ran; %lu calls ran as original code, %lu indexes past a table\n",
         frame, pictures, rooms, ran, run + resumed, emu_bad_indexes());
  return 0;
}
