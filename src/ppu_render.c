// The picture; see ppu_render.h. From the NESdev wiki's description of the PPU's rendering: its own
// code, drawing a line at a time rather than a dot at a time.
#include "ppu_render.h"

#include <math.h>

#include "nes_hw.h"

enum {
  kCtrlNametable = 0x03,
  kCtrlSpriteTable = 0x08,
  kCtrlBgTable = 0x10,
  kCtrlTallSprites = 0x20,
  kMaskGray = 0x01,
  kMaskBgLeft = 0x02,
  kMaskSpritesLeft = 0x04,
  kMaskBg = 0x08,
  kMaskSprites = 0x10,
  kSpritesPerLine = 8,
};

// A byte of CHR ROM through the MMC3's banks, in the pattern tables' address space.
static uint8_t chr_byte(unsigned addr) {
  addr &= 0x1FFF;
  if(g_nes.bank_select & 0x80) {
    addr ^= 0x1000;  // CHR mode 1 swaps the 2 KB and the 1 KB banks
  }
  unsigned bank;
  if(addr < 0x1000) {
    bank = (g_nes.bank_regs[addr >> 11] & 0xFEu) | (addr >> 10 & 1);
  } else {
    bank = g_nes.bank_regs[2 + ((addr - 0x1000) >> 10)];
  }
  return g_nes.chr[bank * 0x400 + (addr & 0x3FF)];
}

static uint8_t nametable_byte(unsigned table, unsigned offset) {
  unsigned page = g_nes.mirroring ? table >> 1 : table & 1;
  return g_nes.ciram[page * 0x400 + offset];
}

// One line of background, from the v register: color bits 0-1 and palette bits 2-3 per pixel.
static void draw_background_line(uint16_t v, uint8_t ctrl, uint8_t *line) {
  unsigned coarse_x = v & 0x1F, coarse_y = v >> 5 & 0x1F, fine_y = v >> 12 & 7;
  unsigned table = v >> 10 & 3;
  unsigned pattern = ctrl & kCtrlBgTable ? 0x1000 : 0;
  int x = -(int)g_nes.fine_x;
  while(x < kScreenWidth) {
    unsigned tile = nametable_byte(table, coarse_y * 32 + coarse_x);
    unsigned attr = nametable_byte(table, 0x3C0 + (coarse_y >> 2) * 8 + (coarse_x >> 2));
    attr = attr >> ((coarse_y & 2) * 2 + (coarse_x & 2)) & 3;
    unsigned lo = chr_byte(pattern + tile * 16 + fine_y);
    unsigned hi = chr_byte(pattern + tile * 16 + 8 + fine_y);
    for(int bit = 7; bit >= 0; bit--, x++) {
      if(x >= 0 && x < kScreenWidth) {
        unsigned color = (lo >> bit & 1) | (hi >> bit & 1) << 1;
        line[x] = (uint8_t)(color ? color | attr << 2 : 0);
      }
    }
    if(++coarse_x == 32) {
      coarse_x = 0;
      table ^= 1;
    }
  }
}

// The sprites on a line, the first in OAM on top: color bits 0-1, palette bits 2-3, 0x10 behind the
// background, 0x20 sprite 0. Returns whether any is there.
static int draw_sprite_line(int y, uint8_t ctrl, uint8_t *line) {
  int height = ctrl & kCtrlTallSprites ? 16 : 8;
  int found = 0;
  for(int i = 0; i < kScreenWidth; i++) {
    line[i] = 0;
  }
  for(int i = 0; i < 64 && found < kSpritesPerLine; i++) {
    const uint8_t *s = &g_nes.oam[i * 4];
    int row = y - 1 - s[0];  // a sprite shows the line after its Y
    if(row < 0 || row >= height) {
      continue;
    }
    found++;
    if(s[2] & 0x80) {
      row = height - 1 - row;
    }
    unsigned tile = s[1], pattern = ctrl & kCtrlSpriteTable ? 0x1000 : 0;
    if(height == 16) {
      pattern = tile & 1 ? 0x1000 : 0;
      tile &= 0xFE;
      if(row >= 8) {
        tile++;
        row -= 8;
      }
    }
    unsigned lo = chr_byte(pattern + tile * 16 + (unsigned)row);
    unsigned hi = chr_byte(pattern + tile * 16 + 8 + (unsigned)row);
    for(int col = 0; col < 8; col++) {
      int x = s[3] + col;
      int bit = s[2] & 0x40 ? col : 7 - col;
      unsigned color = (lo >> bit & 1) | (hi >> bit & 1) << 1;
      if(x < kScreenWidth && color && !line[x]) {
        line[x] = (uint8_t)(color | (s[2] & 3) << 2 | (s[2] & 0x20 ? 0x10 : 0) | (i == 0 ? 0x20 : 0));
      }
    }
  }
  return found;
}

// v's fine Y and coarse Y, a line on. Row 29 is the last of a nametable; a scroll that puts Y in
// the attributes (30, 31) wraps without changing table.
static uint16_t next_line_v(uint16_t v) {
  if((v & 0x7000) != 0x7000) {
    return (uint16_t)(v + 0x1000);
  }
  v &= 0x0FFF;
  unsigned coarse_y = v >> 5 & 0x1F;
  if(coarse_y == 29) {
    coarse_y = 0;
    v ^= 0x0800;
  } else if(coarse_y == 31) {
    coarse_y = 0;
  } else {
    coarse_y++;
  }
  return (uint16_t)((v & ~0x03E0) | coarse_y << 5);
}

void ppu_render_frame(uint16_t *pixels) {
  uint8_t mask = g_nes.ppu_mask;
  uint16_t emphasis = (uint16_t)((mask & 0xE0) << 1);
  uint8_t color_mask = mask & kMaskGray ? 0x30 : 0x3F;
  if(!(mask & (kMaskBg | kMaskSprites))) {
    // With rendering off the PPU shows the backdrop, or the palette entry v points at.
    uint16_t v = g_nes.vram_addr & 0x3FFF;
    uint8_t color = g_nes.palette[v >= 0x3F00 ? v & 0x1F : 0];
    for(int i = 0; i < kScreenWidth * kScreenHeight; i++) {
      pixels[i] = (uint16_t)((color & color_mask) | emphasis);
    }
    return;
  }
  uint8_t ctrl = g_nes.split_on ? g_nes.split_upper_ctrl : g_nes.ppu_ctrl;
  uint16_t t = (uint16_t)((g_nes.vram_temp & ~0x0C00) | (ctrl & kCtrlNametable) << 10);
  uint16_t v = t;
  int split_pending = g_nes.split_on;
  for(int y = 0; y < kScreenHeight; y++, pixels += kScreenWidth) {
    uint8_t bg[kScreenWidth], sprites[kScreenWidth];
    v = (uint16_t)((v & ~0x041F) | (t & 0x041F));
    draw_background_line(v, ctrl, bg);
    draw_sprite_line(y, ctrl, sprites);
    int hit = 0;
    for(int x = 0; x < kScreenWidth; x++) {
      int b = mask & kMaskBg && (x >= 8 || mask & kMaskBgLeft) ? bg[x] : 0;
      int s = mask & kMaskSprites && (x >= 8 || mask & kMaskSpritesLeft) ? sprites[x] : 0;
      hit |= (s & 0x20) && b && x != 255;
      unsigned entry = s & 3 && !(b && s & 0x10) ? 0x10u | (s & 0x0F) : (unsigned)b;
      if(!(entry & 3)) {
        entry = 0;
      }
      pixels[x] = (uint16_t)((g_nes.palette[entry] & color_mask) | emphasis);
    }
    v = next_line_v(v);
    if(hit && split_pending) {
      split_pending = 0;
      ctrl = g_nes.ppu_ctrl;
      t = g_nes.vram_temp;
    }
  }
}

// A channel's level from YIQ, 0-255.
static uint32_t to_byte(double v) {
  return v <= 0 ? 0 : v >= 1 ? 255 : (uint32_t)(v * 255 + 0.5);
}

void ppu_make_palette(uint32_t *rgb) {
  static const double kLow[4] = {0.350, 0.518, 0.962, 1.550};
  static const double kHigh[4] = {1.094, 1.506, 1.962, 1.962};
  static const double kBlack = 0.518, kWhite = 1.962, kAttenuate = 0.746;
  static const double kHuePhase = 4;  // where the color burst falls, in the 12 phases of a pixel
  static const double kSaturation = 1.56;
  static const double kPi = 3.14159265358979323846;
  for(int value = 0; value < kPpuColors; value++) {
    int hue = value & 15, level = value >> 4 & 3, emphasis = value >> 6;
    double low = kLow[level], high = kHigh[level];
    if(hue == 0) {
      low = high;
    } else if(hue > 12) {
      high = low;
    }
    double y = 0, i = 0, q = 0;
    for(int phase = 0; phase < 12; phase++) {
      double signal = (hue + phase) % 12 < 6 ? high : low;
      if(hue > 13) {
        signal = kBlack;
      }
      // Emphasis dims the signal during the phases of the other two colors.
      if(hue < 14 && (((emphasis & 1) && (0x0C + phase) % 12 < 6) ||
                      ((emphasis & 2) && (0x04 + phase) % 12 < 6) ||
                      ((emphasis & 4) && (0x08 + phase) % 12 < 6))) {
        signal *= kAttenuate;
      }
      double v = (signal - kBlack) / (kWhite - kBlack);
      y += v;
      i += v * cos(kPi * (phase + kHuePhase) / 6);
      q += v * sin(kPi * (phase + kHuePhase) / 6);
    }
    y /= 12, i *= kSaturation / 12, q *= kSaturation / 12;
    rgb[value] = to_byte(y + 0.946882 * i + 0.623557 * q) << 16 |
                 to_byte(y - 0.274788 * i - 0.635691 * q) << 8 |
                 to_byte(y - 1.108545 * i + 1.709007 * q);
  }
}
