// Whole nametables, written straight to the PPU with rendering and the NMI off: filling them
// with one tile, and the run-length-coded screens (title, dialogs, logo,
// ending) that four routines in banks 2 and 6 draw.
#include "funcs.h"
#include "variables.h"

// clear_nametables (7:EE65): fills both nametables with tile A, rendering off, then turns
// rendering back on (it runs into rendering_on).
void clear_nametables(uint8_t tile) {
  emu_set_a(tile);
  RUN_ORIGINAL_IF_LAG(kClearNametables);
  rendering_off(tile);
  fill_nametable_2000();
  fill_nametable_2nd();
  rendering_on();
}

// rendering_on (7:EE6E): turns the NMI back on, waits a frame with the sprites going up, then
// restores PPUMASK, the scroll, and X and Y as rendering_off kept them.
void rendering_on(void) {
  RUN_ORIGINAL_IF_LAG(kRenderingOn);
  Cpu6502 *c = &g_nes.cpu;
  enable_nmi();
  request_oam_dma();
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kRenderingOn, 0xEE77, 0);
  cpu_bus_write(0x2001, ppumask_shadow);
  c->x = g_ram[0x11];
  c->y = g_ram[0x12];
  cpu_bus_write(0x2005, ppuscroll_x);
  cpu_bus_write(0x2005, ppuscroll_y);
  emu_set_a(ppuscroll_y);
}

// rendering_off (7:EE9A): keeps A, X and Y in $10, $11 and $12, turns the NMI off and
// background and sprites off (in PPUMASK, not in ppumask_shadow).
void rendering_off(uint8_t tile) {
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(tile);
  RUN_ORIGINAL_IF_LAG(kRenderingOff);
  g_ram[0x10] = tile;
  g_ram[0x11] = c->x;
  g_ram[0x12] = c->y;
  disable_nmi();
  uint8_t mask = ppumask_shadow & 0xE7;
  cpu_bus_write(0x2001, mask);
  emu_set_a(mask);
}

// The tiles of the nametable at VRAM $hh00 (not its attributes) set to tile $10.
static void fill_nametable(uint8_t hi) {
  Cpu6502 *c = &g_nes.cpu;
  cpu_bus_write(0x2006, hi);
  cpu_bus_write(0x2006, 0x00);
  uint8_t tile = g_ram[0x10];
  for(int i = 0; i < 0x3C0; i++) {
    cpu_bus_write(0x2007, tile);
  }
  c->a = tile;
  c->x = 0xC0;
  c->y = 0x03;
  emu_cmp(0xC0, 0xC0);
}

// fill_nametable_2000 (7:EEAB), which jumps into fill_nametable_2nd's loop.
void fill_nametable_2000(void) {
  if(emu_lag_in(kFillNametable2000) || emu_lag_in(kFillNametable2nd)) {
    emu_call_original(kFillNametable2000);
    return;
  }
  cpu_bus_read(0x2002);
  emu_set_a(0x20);
  fill_nametable(0x20);
}

// fill_nametable_2nd (7:EEB2): the second nametable, $2400 or (horizontal mirroring) $2800.
void fill_nametable_2nd(void) {
  RUN_ORIGINAL_IF_LAG(kFillNametable2nd);
  cpu_bus_read(0x2002);
  fill_nametable(mirroring_mode & 0x08 ? 0x28 : 0x24);
}

// ---- Run-length-coded screens -------------------------------------------------------------

// rle_write_2000 (7:EA47): writes tile A to PPUDATA, counting the position in $1E/$1F; past $3C0
// (the attribute table) the byte goes to the attribute buffer at $1C too. Keeps A and X.
void rle_write_2000(uint8_t v) {
  emu_set_a(v);
  RUN_ORIGINAL_IF_LAG(kRleWrite2000);
  cpu_bus_write(0x2007, v);
  g_ram[0x1D] = v;
  emu_cmp(g_ram[0x1F], 0x03);
  if(g_ram[0x1F] == 0x03) {
    emu_cmp(g_ram[0x1E], 0xC0);
    if(g_ram[0x1E] >= 0xC0) {
      attr_buffer_2000[g_ram[0x1C]] = g_ram[0x1D];
      g_ram[0x1C]++;
    }
  }
  add16(g_ram + 0x1E, 0x01);
  emu_set_a(v);
}

// rle_to_ppu_2000 (7:EA18): writes the run-length-coded data at inline_ptr to PPUDATA. A byte is
// itself; $FF v n is n copies of v (256 for n = 0); $FF $FF n is n copies of $FF; $FF $FF $00 ends
// it.
void rle_to_ppu_2000(void) {
  RUN_ORIGINAL_IF_LAG(kRleToPpu2000);
  Cpu6502 *c = &g_nes.cpu;
  for(;;) {
    uint8_t v = read_inline_byte();
    emu_cmp(v, 0xFF);
    if(v != 0xFF) {
      rle_write_2000(v);
      continue;
    }
    v = read_inline_byte();
    emu_cmp(v, 0xFF);
    if(v == 0xFF) {
      uint8_t n = read_inline_byte();
      emu_cmp(n, 0x00);
      if(!n) {
        return;
      }
      c->x = n;
      emu_set_a(0xFF);
    } else {
      c->x = read_inline_byte();
      emu_set_a(v);
    }
    do {
      rle_write_2000(v);
      c->x--;
      emu_set_nz(c->x);
    } while(c->x);
  }
}

// draw_rle_screen_2000 (7:E9D5), for original code only: the screen's data follows the JSR, and it
// returns to the caller's caller. Black palette and rendering off, the nametable written with the
// NMI off, then rendering, sprites and the palette back on. Keeps X and Y; returns A = X.
uint16_t draw_rle_screen_2000(void) {
  Cpu6502 *c = &g_nes.cpu;
  inline_ptr[0] = emu_pull();
  inline_ptr[1] = emu_pull();
  uint8_t x = c->x, y = c->y;
  emu_push(x);
  emu_push(y);
  emu_set_a(y);
  request_black_palette();
  wait_nmi_work_done();
  if(emu_lag_in(kDrawRleScreen2000)) {
    emu_resume_original(0xE9E5, 2);
    return kEmuRts;
  }
  hide_bg_and_sprites();
  if(emu_lag_in(kDrawRleScreen2000)) {
    emu_resume_original(0xE9E8, 2);
    return kEmuRts;
  }
  disable_nmi();
  advance_inline_ptr();
  cpu_bus_read(0x2002);
  cpu_bus_write(0x2006, 0x20);
  cpu_bus_write(0x2006, 0x00);
  g_ram[0x1E] = g_ram[0x1F] = g_ram[0x1C] = 0;
  emu_set_a(0);
  rle_to_ppu_2000();
  enable_nmi();
  show_bg_and_sprites();
  if(emu_lag_in(kDrawRleScreen2000)) {
    emu_resume_original(0xEA0A, 2);
    return kEmuRts;
  }
  request_oam_dma();
  request_palette_upload();
  wait_nmi_work_done();
  emu_pull();
  emu_pull();
  c->y = y;
  c->x = x;
  emu_set_a(x);
  return kEmuRts;
}

// For a routine that is only JSR draw_rle_screen_2000 with the data after it (title, dialogs, logo,
// ending): what the JSR pushes, as the screen's data is read through it.
void rle_screen_from(uint16_t routine) {
  if(emu_lag_in(routine) || emu_lag_in(kDrawRleScreen2000)) {
    emu_call_original(routine);
    return;
  }
  emu_push((uint8_t)((routine + 2) >> 8));
  emu_push((uint8_t)(routine + 2));
  draw_rle_screen_2000();
}
