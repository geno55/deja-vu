// Deja Vu (NES) native port: the game in C, a picture from the PPU's state, sound from our own APU, and
// your own ROM for the data.
//
// usage: dejavu [ROM] [--scale N] [--save FILE] [--frames N] [--screenshot FILE.bmp] [--audio-wav FILE]
//               [--no-window] [--movie FILE.inp] [--extract-assets]
//
// ROM        your Deja Vu (USA) dump. You only need it once: the game keeps what it needs from it in
//            dejavu_assets.dat next to the program. Without one the game looks for that file there and
//            here, then for "Deja Vu (USA).nes" here and there.
// --scale    the window's size, in times the picture (default: 3)
// --save     the save file, the game's 8 KB of save RAM (default: saves/dejavu.sav next to the program,
//            or dejavu.sav here if there's one and that isn't there yet). An emulator's .sav of this game
//            works as it is (docs/saves.md)
// --frames   stop after this many frames
// --screenshot  when stopping after --frames, write what the window shows (or with no window, the
//            picture) to this BMP file
// --audio-wav  write the sound to this WAV file as well (tools/compare_audio.py compares it)
// --no-window  run with no window and at full speed, for testing
// --movie    play these buttons, not yours: an .inp file, one line per frame, UDLRTSBA
// --extract-assets  make dejavu_assets.dat from the dump, and stop
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apu.h"
#include "assets.h"
#include "funcs.h"
#include "nes_hw.h"
#include "platform.h"
#include "ppu_render.h"
#include "save_file.h"

enum {
  kSaveCheckFrames = 60,  // how often to see if the save RAM has changed
  kDefaultScale = 3,
  kAudioRate = 48000,
  kFrameCycles = 29780,     // CPU cycles a frame, and half a cycle more
};

#define kDefaultRomName "Deja Vu (USA).nes"

_Static_assert(sizeof(g_nes.sram) == kSaveSize, "the save file is the save RAM");

static uint16_t picture[kScreenWidth * kScreenHeight];
static uint32_t palette_rgb[kPpuColors];
static uint32_t picture_rgb[kScreenWidth * kScreenHeight];
static int16_t audio[4096];
static GameData game_data;
static int show_errors;  // playing in a window: an error that stops the game has to be seen

static void draw_picture(void) {
  ppu_render_frame(picture);
}

// Says what stopped the game. Whoever double-clicked the program would never see the console's text.
static void report_error(const char *message) {
  fprintf(stderr, "dejavu: %s\n", message);
  if(show_errors) {
    platform_show_error(message);
  }
}

// Where the program is, from how it was started: "dir/name", or "" if it was only "name".
static void program_dir(const char *argv0, char *dir, size_t size) {
  const char *end = strrchr(argv0, '/');
#ifdef _WIN32
  const char *back = strrchr(argv0, '\\');
  end = back && (!end || back > end) ? back : end;
#endif
  size_t len = end ? (size_t)(end - argv0) + 1 : 0;
  snprintf(dir, size, "%.*s", (int)len, argv0);
}

// Gets the game's data from the dump named, or else the pack or a dump where they are usually kept. A
// dump is kept as a pack next to the program unless a good one is there already. Returns 0, or -1
// after saying why on stderr.
static int load_game_data(const char *rom_path, const char *dir) {
  char pack[1024], err[1200], beside[1024], message[1400];
  snprintf(pack, sizeof(pack), "%s%s", dir, kAssetsPackName);
  AssetsResult beside_program = assets_read(pack, &game_data, err, sizeof(err));
  if(beside_program == kAssetsBad) {
    fprintf(stderr, "dejavu: %s; a dump will replace it\n", err);
  }
  int have_pack = beside_program == kAssetsOk;
  if(rom_path) {
    AssetsResult r = assets_read(rom_path, &game_data, err, sizeof(err));
    if(r != kAssetsOk) {
      snprintf(message, sizeof(message), "%s: this needs your own Deja Vu (USA) ROM", err);
      report_error(message);
      return -1;
    }
  } else if(!have_pack) {
    // Here as well as beside the program, for a pack or a dump someone has moved.
    AssetsResult r = assets_read(kAssetsPackName, &game_data, err, sizeof(err));
    if(r == kAssetsOk) {
      return 0;
    }
    if(r == kAssetsBad) {
      fprintf(stderr, "dejavu: %s; a dump will replace it\n", err);
    }
    snprintf(beside, sizeof(beside), "%s%s", dir, kDefaultRomName);
    r = assets_read(kDefaultRomName, &game_data, err, sizeof(err));
    if(r == kAssetsMissing) {
      r = assets_read(beside, &game_data, err, sizeof(err));
    }
    if(r == kAssetsMissing) {
      snprintf(message, sizeof(message),
               "this needs your own Deja Vu (USA) ROM. Put it next to the program, named \"%s\", and start "
               "the game again; it keeps what it needs in %s. Or name it once: dejavu path/to/rom.nes",
               kDefaultRomName, pack);
      report_error(message);
      return -1;
    }
    if(r == kAssetsBad) {
      report_error(err);
      return -1;
    }
  }
  if(!have_pack) {
    // Not being able to write it costs nothing but needing the dump next time.
    if(assets_write_pack(pack, &game_data, err, sizeof(err))) {
      fprintf(stderr, "dejavu: %s\n", err);
    } else {
      printf("dejavu: kept the game's data in %s; you don't need the ROM from now on\n", pack);
    }
  }
  return 0;
}

// Where the save goes unless --save says: a folder of its own next to the program, as zelda3 has it. A
// dejavu.sav here, from before there was one, stays in use until there's a save in the folder.
static const char *default_save_path(const char *dir) {
  static char path[1024];
  snprintf(path, sizeof(path), "%ssaves", dir);
  char file[1100];
  snprintf(file, sizeof(file), "%s/dejavu.sav", path);
  FILE *f = fopen(file, "rb");
  if(!f && (f = fopen("dejavu.sav", "rb"))) {
    fclose(f);
    return "dejavu.sav";
  }
  if(f) {
    fclose(f);
  }
  save_file_make_dir(path);  // if it can't be made, the first write says so
  snprintf(path, sizeof(path), "%.1000s", file);
  return path;
}

// The buttons of an .inp file, a byte per frame (lines as tools/movie_inputs.py writes them).
static uint8_t *load_movie(const char *path, long *count) {
  static const char kOrder[] = "UDLRTSBA";
  static const uint8_t kBits[8] = {0x10, 0x20, 0x40, 0x80, 0x08, 0x04, 0x02, 0x01};
  FILE *f = fopen(path, "r");
  if(!f) {
    return NULL;
  }
  uint8_t *pads = NULL;
  long cap = 0, n = 0;
  char line[64];
  while(fgets(line, sizeof(line), f)) {
    if(n == cap) {
      cap = cap ? cap * 2 : 4096;
      pads = realloc(pads, (size_t)cap);
    }
    uint8_t pad = 0;
    for(int i = 0; i < 8 && line[i] && line[i] != '\n'; i++) {
      if(line[i] == kOrder[i]) {
        pad |= kBits[i];
      }
    }
    pads[n++] = pad;
  }
  fclose(f);
  *count = n;
  return pads;
}

// Says what went wrong with the save file; the game carries on, and the next try may work.
static void update_save(const char *path) {
  char err[1200];
  if(save_file_update(path, g_nes.sram, err, sizeof(err))) {
    fprintf(stderr, "dejavu: %s\n", err);
  }
}

static void write_wav_header(FILE *f, uint32_t samples) {
  uint32_t data = samples * 2;
  uint8_t h[44] = "RIFF____WAVEfmt ";
  uint32_t values[] = {36 + data, 16, 1 | 1u << 16, kAudioRate, kAudioRate * 2, 2 | 16u << 16};
  memcpy(h + 4, &values[0], 4);
  memcpy(h + 16, &values[1], 20);
  memcpy(h + 36, "data", 4);
  memcpy(h + 40, &data, 4);
  fseek(f, 0, SEEK_SET);
  fwrite(h, 1, sizeof(h), f);
}

// The picture as a BMP file, for --screenshot with no window: 32-bit pixels, bottom row first.
static void write_screenshot(const char *path) {
  FILE *f = fopen(path, "wb");
  if(!f) {
    fprintf(stderr, "dejavu: can't write %s\n", path);
    return;
  }
  uint32_t pixels = kScreenWidth * kScreenHeight * 4;
  uint32_t file[3] = {54 + pixels, 0, 54};
  uint32_t info[10] = {40, kScreenWidth, kScreenHeight, 1 | 32u << 16, 0, pixels};
  fwrite("BM", 1, 2, f);
  fwrite(file, sizeof(file), 1, f);
  fwrite(info, sizeof(info), 1, f);
  for(int y = kScreenHeight - 1; y >= 0; y--) {
    fwrite(picture_rgb + y * kScreenWidth, sizeof(picture_rgb[0]), kScreenWidth, f);
  }
  fclose(f);
}

static void usage(FILE *f) {
  fprintf(f, "usage: dejavu [ROM] [--scale N] [--save FILE] [--frames N] [--screenshot FILE.bmp] "
             "[--audio-wav FILE] [--no-window] [--movie FILE.inp] [--extract-assets]\n"
             "keys: arrows, X = A, Z = B, Enter = Start, right Shift = Select; or a game controller\n");
}

int main(int argc, char **argv) {
  const char *rom_path = NULL, *save_path = NULL, *screenshot = NULL;
  const char *wav_path = NULL, *movie_path = NULL;
  long frames = -1;
  int scale = kDefaultScale, window = 1, extract_only = 0;
  for(int i = 1; i < argc; i++) {
    const char *arg = argv[i];
    if(!strcmp(arg, "--help")) {
      usage(stdout);
      return 0;
    }
    if(!strcmp(arg, "--no-window")) {
      window = 0;
    } else if(!strcmp(arg, "--extract-assets")) {
      extract_only = 1;
    } else if(arg[0] != '-') {
      rom_path = arg;
    } else if(i + 1 >= argc) {
      usage(stderr);
      return 2;
    } else if(!strcmp(arg, "--scale")) {
      scale = atoi(argv[++i]);
    } else if(!strcmp(arg, "--save")) {
      save_path = argv[++i];
    } else if(!strcmp(arg, "--frames")) {
      frames = atol(argv[++i]);
    } else if(!strcmp(arg, "--screenshot")) {
      screenshot = argv[++i];
    } else if(!strcmp(arg, "--audio-wav")) {
      wav_path = argv[++i];
    } else if(!strcmp(arg, "--movie")) {
      movie_path = argv[++i];
    } else {
      usage(stderr);
      return 2;
    }
  }
  if(scale < 1) {
    usage(stderr);
    return 2;
  }

  show_errors = window && !extract_only;

  char dir[1024];
  program_dir(argv[0], dir, sizeof(dir));
  if(load_game_data(rom_path, dir)) {
    return 1;
  }
  if(extract_only) {
    return 0;
  }
  long movie_frames = 0;
  uint8_t *movie = NULL;
  if(movie_path && !(movie = load_movie(movie_path, &movie_frames))) {
    fprintf(stderr, "dejavu: can't read %s\n", movie_path);
    return 1;
  }

  if(dv_boot(game_data.prg, game_data.chr)) {
    report_error("can't start the game's coroutine");
    return 1;
  }
  if(!save_path) {
    save_path = default_save_path(dir);
  }
  char err[1200];
  if(save_file_load(save_path, g_nes.sram, err, sizeof(err))) {
    report_error(err);
    return 1;
  }
  emu_set_frame_hook(draw_picture);
  ppu_make_palette(palette_rgb);
  if(window && platform_open(scale)) {
    report_error("can't open the window");
    return 1;
  }
  if(window) {
    usage(stdout);
    platform_open_audio(kAudioRate);
  }
  apu_reset(kAudioRate, audio, sizeof(audio) / sizeof(audio[0]));
  FILE *wav = wav_path ? fopen(wav_path, "wb") : NULL;
  uint32_t wav_samples = 0;
  if(wav) {
    write_wav_header(wav, 0);
  }

  int status = 0;
  uint8_t pad = 0;
  for(long frame = 0; frames < 0 || frame < frames; frame++) {
    g_nes.pad[0] = movie ? (frame < movie_frames ? movie[frame] : 0) : pad;
    if(emu_run_frame(err, sizeof(err))) {
      char message[1300];
      snprintf(message, sizeof(message), "frame %ld: %s", frame, err);
      report_error(message);
      status = 1;
      break;
    }
    // The driver writes the sound registers in the NMI, at the start of the frame, whatever else the
    // game does in it. (The log can't overflow: the driver writes at most 17 a frame.)
    for(uint32_t i = 0; i < g_nes.apu_count && i < kApuLogMax; i++) {
      apu_write(g_nes.apu_log[i].addr, g_nes.apu_log[i].value);
    }
    g_nes.apu_count = 0;
    apu_run(kFrameCycles + (uint32_t)(frame & 1));
    size_t samples = apu_take_samples();
    if(window) {
      platform_queue_audio(audio, samples);
    }
    if(wav) {
      fwrite(audio, sizeof(audio[0]), samples, wav);
      wav_samples += (uint32_t)samples;
    }
    if(frame % kSaveCheckFrames == 0) {
      update_save(save_path);
    }
    int last = frame + 1 == frames;
    if(window || (last && screenshot)) {
      for(int i = 0; i < kScreenWidth * kScreenHeight; i++) {
        picture_rgb[i] = palette_rgb[picture[i] & (kPpuColors - 1)];
      }
    }
    if(window && !platform_frame(picture_rgb, &pad, last ? screenshot : NULL)) {
      break;
    }
    if(!window && last && screenshot) {
      write_screenshot(screenshot);
    }
  }
  update_save(save_path);
  if(wav) {
    write_wav_header(wav, wav_samples);
    fclose(wav);
  }
  if(window) {
    platform_close();
  }
  free(movie);
  return status;
}
