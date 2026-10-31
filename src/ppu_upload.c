// Sprites, and the work the game asks of the NMI.
#include "funcs.h"
#include "variables.h"

// disable_nmi (7:EB29)
void disable_nmi(void) {
  RUN_ORIGINAL_IF_LAG(kDisableNmi);
  ppuctrl_shadow &= 0x7F;
  cpu_bus_write(0x2000, ppuctrl_shadow);
  emu_set_a(ppuctrl_shadow);
}

// enable_nmi (7:EB33): reads $2002 first, so a vblank already under way doesn't start an NMI.
void enable_nmi(void) {
  RUN_ORIGINAL_IF_LAG(kEnableNmi);
  cpu_bus_read(0x2002);
  ppuctrl_shadow |= 0x80;
  cpu_bus_write(0x2000, ppuctrl_shadow);
  emu_set_a(ppuctrl_shadow);
}

// request_nmi_work (7:EBA9): asks the NMI for the work in `bits` (see nmi_requests).
void request_nmi_work(uint8_t bits) {
  emu_set_a(bits);
  RUN_ORIGINAL_IF_LAG(kRequestNmiWork);
  nmi_requests |= bits;
  emu_set_a(nmi_requests);
}

// wait_vblank_and_hide_sprites (7:EBB9): asks the NMI to copy the sprites to the PPU, waits
// for it, then clears the buffer for the next frame.
void wait_vblank_and_hide_sprites(void) {
  RUN_ORIGINAL_IF_LAG(kWaitVblankAndHideSprites);
  request_oam_dma();
  wait_vblank();
  clear_oam_buffer();
  emu_lag_behind(kWaitVblankAndHideSprites);
}

// clear_oam_buffer (7:EBBF): moves every sprite off screen (Y = $F8). While the sprite-0 split
// is on (system_flags bit 0) sprite 0 stays, and sprite0_reserved says so. It saves X with
// TXA/PHA and ends with PLA/TAX, so it returns A = X; the carry is from LSR of system_flags.
void clear_oam_buffer(void) {
  RUN_ORIGINAL_IF_LAG(kClearOamBuffer);
  sprite0_reserved = 0;
  int i = 0;
  if(system_flags & 0x01) {
    sprite0_reserved++;
    i = 4;
  }
  for(; i < 0x100; i += 4) {
    oam_buffer[i] = 0xF8;
  }
  emu_set_a(g_nes.cpu.x);
  emu_set_flag(kFlagC, system_flags & 0x01);
}

// clear_and_upload_sprites (7:EBDE), which falls into request_oam_dma.
void clear_and_upload_sprites(void) {
  RUN_ORIGINAL_IF_LAG(kClearAndUploadSprites);
  clear_oam_buffer();
  request_oam_dma();
}

// request_oam_dma (7:EBE1) and its siblings: the bits each asks for.
void request_oam_dma(void) {
  RUN_ORIGINAL_IF_LAG(kRequestOamDma);
  request_nmi_work(0x80);
}

void request_vram_queue(void) {
  RUN_ORIGINAL_IF_LAG(kRequestVramQueue);
  request_nmi_work(0x08);
}

void request_attrs_2000(void) {
  RUN_ORIGINAL_IF_LAG(kRequestAttrs2000);
  request_nmi_work(0x20);
}

void request_attr_upload(void) {
  RUN_ORIGINAL_IF_LAG(kRequestAttrUpload);
  request_nmi_work(0x30);
}

void request_black_palette(void) {
  RUN_ORIGINAL_IF_LAG(kRequestBlackPalette);
  request_nmi_work(0x04);
}

void request_palette_upload(void) {
  RUN_ORIGINAL_IF_LAG(kRequestPaletteUpload);
  request_nmi_work(0x40);
}

// The common end of show_bg_and_sprites and its siblings (7:F28E): a new ppumask_shadow, which
// the NMI writes to PPUMASK, and a wait for that NMI.
static void set_ppumask(uint8_t mask) {
  ppumask_shadow = mask;
  request_nmi_work(0x01);
  wait_vblank();
}

// show_bg_and_sprites (7:F274) and hide_bg_and_sprites (7:F282): turn rendering bits of PPUMASK on
// or off. Their shared end (with its wait) is in hide_sprites' code, so a lag NMI there counts for
// both. Nothing calls show_sprites (7:F278) and hide_sprites (7:F286), so they aren't ported, but
// their code holds what these two share.
static int mask_routine_lag(uint16_t addr) {
  if(emu_lag_in(addr) || emu_lag_in(kHideSprites)) {
    emu_call_original(addr);
    return 1;
  }
  return 0;
}

void show_bg_and_sprites(void) {
  if(!mask_routine_lag(kShowBgAndSprites)) {
    set_ppumask(ppumask_shadow | 0x18);
    emu_lag_behind(kHideSprites);
  }
}

void hide_bg_and_sprites(void) {
  if(!mask_routine_lag(kHideBgAndSprites)) {
    set_ppumask(ppumask_shadow & 0xE7);
    emu_lag_behind(kHideSprites);
  }
}
