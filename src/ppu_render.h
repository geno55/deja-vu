// The picture: one frame drawn from the PPU's state in g_nes, as the 2C02 would.
#ifndef DV_PPU_RENDER_H
#define DV_PPU_RENDER_H

#include <stdint.h>

enum {
  kScreenWidth = 256,
  kScreenHeight = 240,
};

// Draws the frame the PPU shows from here to the next vblank, kScreenWidth * kScreenHeight pixels by
// rows. A pixel is a palette color (6 bits) with PPUMASK's emphasis bits at 6-8, as Mesen2 keeps
// them, so the two can be compared without colors coming into it. Call it after the NMI handler.
//
// It draws the two screens of the sprite-0 split from the PPUCTRL values nmi() leaves
// (g_nes.split_on, g_nes.split_upper_ctrl), changing over on the line after the hit.
void ppu_render_frame(uint16_t *pixels);

// The RGB of each pixel value, 0x00RRGGBB: computed from the NTSC signal the PPU puts out, with the
// levels on the NESdev wiki and the saturation that brings it to within 3 levels of 255 of Mesen2's
// default palette (dv_harness --check-palette).
enum { kPpuColors = 512 };
void ppu_make_palette(uint32_t *rgb);

#endif  // DV_PPU_RENDER_H
