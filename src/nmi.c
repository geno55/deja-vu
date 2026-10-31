// The NMI handler, which runs at every vblank: it does the work the
// game asked for in nmi_requests, sets the scroll, ends wait_vblank and steps the sound driver
// (in sound_update), and splits the screen at sprite 0.
//
// Only the handler calls these routines. No NMI lands in them, because the handler turns the
// NMI off first, so their guards never run the original. They still count the calls.
#include "funcs.h"
#include "variables.h"

// nmi_ppu_updates (7:ED24): one upload per NMI, the highest bit of nmi_requests that asks for
// one. With none, A is left with the seven shifts that looked for it.
void nmi_ppu_updates(void) {
  RUN_ORIGINAL_IF_LAG(kNmiPpuUpdates);
  uint8_t requests = nmi_requests;
  emu_set_flag(kFlagC, 1);  // from the ASL that found the bit
  if(requests & 0x40) {
    nmi_upload_palette();
  } else if(requests & 0x20) {
    nmi_upload_attrs_2000();
  } else if(requests & 0x10) {
    nmi_upload_attrs_2nd();
  } else if(requests & 0x08) {
    nmi_flush_vram_queue();
  } else if(requests & 0x04) {
    nmi_black_palette();
  } else if(requests & 0x02) {
    nmi_flush_vram_queue_down();
  } else {
    emu_set_flag(kFlagC, 0);
    emu_set_a((uint8_t)(requests << 7));
  }
}

// A whole attribute table, to VRAM $hhC0. Leaves X = $40 and the flags of the last CPX.
static void upload_attrs(uint8_t hi, const uint8_t *attrs) {
  cpu_bus_write(0x2006, hi);
  cpu_bus_write(0x2006, 0xC0);
  for(int i = 0; i < 0x40; i++) {
    cpu_bus_write(0x2007, attrs[i]);
  }
  g_nes.cpu.x = 0x40;
  emu_cmp(0x40, 0x40);
}

// nmi_upload_attrs_2000 (7:ED4C)
void nmi_upload_attrs_2000(void) {
  RUN_ORIGINAL_IF_LAG(kNmiUploadAttrs2000);
  cpu_bus_read(0x2002);
  upload_attrs(0x23, attr_buffer_2000);
  nmi_requests &= 0xDF;
  emu_set_a(nmi_requests);
}

// nmi_upload_attrs_2nd (7:ED6D): to the second nametable, $2400 or (horizontal mirroring) $2800.
void nmi_upload_attrs_2nd(void) {
  RUN_ORIGINAL_IF_LAG(kNmiUploadAttrs2nd);
  cpu_bus_read(0x2002);
  upload_attrs(mirroring_mode & 0x08 ? 0x2B : 0x27, attr_buffer_2400);
  nmi_requests &= 0xEF;
  emu_set_a(nmi_requests);
}

// nmi_flush_vram_queue (7:ED97)
void nmi_flush_vram_queue(void) {
  RUN_ORIGINAL_IF_LAG(kNmiFlushVramQueue);
  g_nes.cpu.x = 0;
  flush_vram_queue();
  nmi_requests &= 0xF7;
  emu_set_a(nmi_requests);
}

// nmi_flush_vram_queue_down (7:EDA3): the same with PPUDATA stepping down the columns, for this
// upload only (ppuctrl_shadow keeps it off).
void nmi_flush_vram_queue_down(void) {
  RUN_ORIGINAL_IF_LAG(kNmiFlushVramQueueDown);
  cpu_bus_write(0x2000, ppuctrl_shadow | 0x04);
  g_nes.cpu.x = 0;
  flush_vram_queue();
  nmi_requests &= 0xFD;
  cpu_bus_write(0x2000, ppuctrl_shadow);
  emu_set_a(ppuctrl_shadow);
}

// flush_vram_queue (7:EDE3, with its loop at flush_vram_entry, 7:EDBB): writes out the entries
// of vram_queue from X on (address high, low, length, then the bytes; length 0 is 256) until
// vram_queue_len, counting down, is 0 at the end of one. Returns A = Y = 0.
void flush_vram_queue(void) {
  RUN_ORIGINAL_IF_LAG(kFlushVramQueue);
  Cpu6502 *c = &g_nes.cpu;
  while(vram_queue_len) {
    cpu_bus_read(0x2002);
    cpu_bus_write(0x2006, vram_queue[c->x++]);
    cpu_bus_write(0x2006, vram_queue[c->x++]);
    vram_queue_len = (uint8_t)(vram_queue_len - 3);
    uint8_t length = vram_queue[c->x++];
    // A made-up queue can loop forever: the fuzz needs to see that.
    emu_count_work(length ? 3u + length : 259u);
    do {
      cpu_bus_write(0x2007, vram_queue[c->x++]);
      vram_queue_len--;
    } while(--length);
  }
  c->y = 0;
  emu_set_a(0);
  cpu_bus_write(0x2006, 0x00);
  cpu_bus_write(0x2006, 0x00);
}

// nmi_black_palette (7:EDF1)
void nmi_black_palette(void) {
  RUN_ORIGINAL_IF_LAG(kNmiBlackPalette);
  cpu_bus_read(0x2002);
  ppu_addr_palette();
  for(int i = 0; i < 0x20; i++) {
    cpu_bus_write(0x2007, 0x0F);
  }
  nmi_requests &= 0xFB;
  g_nes.cpu.y = 0x20;
  emu_cmp(0x20, 0x20);
  ppu_addr_leave_palette();
}

// nmi_upload_palette (7:EE0B), which runs into ppu_addr_leave_palette.
void nmi_upload_palette(void) {
  RUN_ORIGINAL_IF_LAG(kNmiUploadPalette);
  cpu_bus_read(0x2002);
  ppu_addr_palette();
  for(int i = 0; i < 0x20; i++) {
    cpu_bus_write(0x2007, palette_buffer[i]);
  }
  nmi_requests &= 0xBF;
  g_nes.cpu.y = 0x20;
  emu_cmp(0x20, 0x20);
  ppu_addr_leave_palette();
}

// ppu_addr_leave_palette (7:EE23): the VRAM address back on $0000, off the palette.
void ppu_addr_leave_palette(void) {
  RUN_ORIGINAL_IF_LAG(kPpuAddrLeavePalette);
  ppu_addr_palette();
  cpu_bus_write(0x2006, 0x00);
  cpu_bus_write(0x2006, 0x00);
}

// ppu_addr_palette (7:EE2D): the VRAM address on $3F00. Returns A = 0.
void ppu_addr_palette(void) {
  RUN_ORIGINAL_IF_LAG(kPpuAddrPalette);
  cpu_bus_write(0x2006, 0x3F);
  cpu_bus_write(0x2006, 0x00);
  emu_set_a(0);
}

// nmi_set_scroll (7:EE38): scroll_x and scroll_y to PPUSCROLL, and their nametable bits to the
// low bits of PPUCTRL. The original shifts those bits out of copies in $08 and $09, used nowhere
// else, which keep the rest. Keeps X and Y; returns A = ppuscroll_y, carry clear (the last ROL's
// bit 7 came from the first two LSRs).
void nmi_set_scroll(void) {
  RUN_ORIGINAL_IF_LAG(kNmiSetScroll);
  g_ram[0x08] = scroll_x[1] >> 1;
  g_ram[0x09] = scroll_y[1] >> 1;
  ppuscroll_x = scroll_x[0];
  ppuscroll_y = scroll_y[0];
  ppuctrl_shadow = (uint8_t)((ppuctrl_shadow & 0xFC) | (scroll_y[1] & 1) << 1 | (scroll_x[1] & 1));
  cpu_bus_write(0x2000, ppuctrl_shadow);
  cpu_bus_read(0x2002);
  cpu_bus_write(0x2005, ppuscroll_x);
  cpu_bus_write(0x2005, ppuscroll_y);
  emu_set_a(ppuscroll_y);
  emu_set_flag(kFlagC, 0);
}

// nmi (7:EC9A), with the 6502's return address and flags already on the stack; the hook does the
// RTI. It keeps A, X and Y on the stack, and turns the NMI off until it is done.
void nmi(void) {
  Cpu6502 *c = &g_nes.cpu;
  g_nes.split_on = 0;
  emu_push(c->a);
  emu_push(c->x);
  emu_push(c->y);
  cpu_bus_read(0x2002);
  // The upper screen: background tiles from $0000, sprites from $1000.
  ppuctrl_shadow = (ppuctrl_shadow & 0x67) | 0x08;
  cpu_bus_write(0x2000, ppuctrl_shadow);
  if(nmi_requests & 0x80) {
    cpu_bus_write(0x2003, 0x00);
    cpu_bus_write(0x4014, 0x02);  // oam_buffer
    nmi_requests &= 0x7F;
  }
  nmi_ppu_updates();
  if(nmi_requests & 0x01) {
    cpu_bus_write(0x2001, ppumask_shadow);
    nmi_requests &= 0xFE;
  }
  nmi_set_scroll();
  frame_counter++;
  split_nametable &= 0x01;
  sound_update();
  // The sprite-0 split, when it's on and the PPU is drawing both layers. The original spins on
  // PPUSTATUS here (7:ECF7) until the sprite-0 flag from the last frame is gone and this
  // frame's is up, or the next vblank comes. That only times the write below against the
  // picture, which is the renderer's business, so C doesn't spin.
  if((system_flags & 0x01) && (ppumask_shadow & 0x18) == 0x18) {
    // The lower screen: background tiles from $1000, sprites from $0000.
    g_nes.split_on = 1;
    g_nes.split_upper_ctrl = g_nes.ppu_ctrl;
    ppuctrl_shadow = (uint8_t)((ppuctrl_shadow & 0xE7) | 0x10 | split_nametable);
    cpu_bus_write(0x2000, ppuctrl_shadow);
  }
  cpu_bus_read(0x2002);
  ppuctrl_shadow |= 0x80;
  cpu_bus_write(0x2000, ppuctrl_shadow);
  c->y = emu_pull();
  c->x = emu_pull();
  c->a = emu_pull();
}
