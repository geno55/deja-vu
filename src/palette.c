// Palette fades: palette_buffer stepped a brightness level ($10) at a
// time, uploaded by the NMI, five frames per step.
#include "funcs.h"
#include "variables.h"

// fade_in_palette (7:F29E): from the darkest level of the 32-colour palette at $1E/$1F up to
// it in three steps. Keeps Y; returns A = Y.
void fade_in_palette(void) {
  RUN_ORIGINAL_IF_LAG(kFadeInPalette);
  Cpu6502 *c = &g_nes.cpu;
  static const uint16_t kAfterWait[4] = {0xF2B1, 0xF2B7, 0xF2BD, 0xF2C3};
  uint8_t y = c->y;
  emu_set_a(y);
  emu_push(y);
  for(int i = 0; i < 0x20; i++) {
    palette_buffer[i] = emu_read_ind_y(0x1E, (uint8_t)i) & 0x0F;
  }
  c->a = palette_buffer[0x1F];
  c->y = 0x20;
  emu_cmp(0x20, 0x20);
  for(int i = 0; i < 4; i++) {
    palette_fade_wait();
    RESUME_ORIGINAL_IF_LAG(kFadeInPalette, kAfterWait[i], 1);
    if(i < 3) {
      palette_brighten_step();
    }
  }
  emu_pull();
  c->y = y;
  emu_set_a(y);
}

// palette_brighten_step (7:F2C6): each colour not yet at the target ($1E/$1F) a level up.
void palette_brighten_step(void) {
  RUN_ORIGINAL_IF_LAG(kPaletteBrightenStep);
  Cpu6502 *c = &g_nes.cpu;
  uint8_t a = 0;
  for(int i = 0; i < 0x20; i++) {
    a = palette_buffer[i];
    uint8_t target = emu_read_ind_y(0x1E, (uint8_t)i);
    emu_cmp(a, target);
    if(a != target) {
      emu_set_flag(kFlagC, 0);
      a = emu_adc(a, 0x10);
      palette_buffer[i] = a;
    }
  }
  c->a = a;
  c->y = 0x20;
  emu_cmp(0x20, 0x20);
}

// fade_out_palette (7:F2DB): palette_buffer down to black ($0F) in four steps. Keeps X; returns
// A = X.
void fade_out_palette(void) {
  RUN_ORIGINAL_IF_LAG(kFadeOutPalette);
  Cpu6502 *c = &g_nes.cpu;
  static const uint16_t kAfterWait[4] = {0xF2E3, 0xF2E9, 0xF2EF, 0xF2F5};
  uint8_t x = c->x;
  emu_set_a(x);
  emu_push(x);
  for(int i = 0; i < 4; i++) {
    palette_darken_step();
    palette_fade_wait();
    RESUME_ORIGINAL_IF_LAG(kFadeOutPalette, kAfterWait[i], 1);
  }
  emu_pull();
  c->x = x;
  emu_set_a(x);
}

// palette_darken_step (7:F2F8): each colour a level down; from the darkest level to black.
void palette_darken_step(void) {
  RUN_ORIGINAL_IF_LAG(kPaletteDarkenStep);
  Cpu6502 *c = &g_nes.cpu;
  uint8_t a = 0;
  for(int i = 0; i < 0x20; i++) {
    a = palette_buffer[i];
    emu_cmp(a, 0x0F);
    if(a == 0x0F) {
      continue;
    }
    emu_set_nz(a & 0xF0);
    if(!(a & 0xF0)) {
      a = 0x0F;
      palette_buffer[i] = a;
      continue;
    }
    emu_set_flag(kFlagC, 1);
    a = emu_sbc(a, 0x10);
    palette_buffer[i] = a;
  }
  c->a = a;
  c->x = 0x20;
  emu_cmp(0x20, 0x20);
}

// palette_fade_wait (7:F31C): asks the NMI to upload palette_buffer, and waits five frames.
void palette_fade_wait(void) {
  RUN_ORIGINAL_IF_LAG(kPaletteFadeWait);
  request_palette_upload();
  for(int i = 0; i < 5; i++) {
    wait_vblank();
  }
}
