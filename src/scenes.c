// Scene loading and drawing: the location picture, a 14x14 tile map
// and 16 attribute bytes at nametable $2082 (docs/scenes.md), with its CHR banks and palette.
// It is drawn all at once (draw_scene) or tile by tile in a dissolve (draw_scene_dissolve), which
// also blanks the old picture first. The routines in bank 7 are at its start; the five in bank 6
// are far calls $60, $61, $62 and $6B, and a helper of theirs.
//
// The scratch bytes, as the originals use them: $1C/$1D a tile's VRAM address, $1E/$1F a block
// for the VRAM queue or the palette's address, $5B-$5D a one-tile block (width, height, tile),
// $5D/$5E the CHR bank entry, $57/$58 the dissolve's row masks, $59/$5A their offsets, $77 the
// dissolve's row and $78 its pass, $83/$84 the row's mask of columns to draw.
//
// Several of these wait for the NMI somewhere, so after such a call they check whether to go on
// as original code (RESUME_ORIGINAL_IF_LAG); the addresses are the original's after that JSR.
#include "funcs.h"
#include "variables.h"

enum {
  kSceneBlockHeader = 0x552,  // the block draw_scene queues: width and height, then scene_tiles
  kSceneTilesBlockLo = 0x52,
  kSceneTilesBlockHi = 0x05,
  kScenePictures = 0x8000,    // [n] a pointer to each picture, in banks 0, 1 and 2
};

// ---- Loading a scene -----------------------------------------------------------------------

// The end of change_scene, change_scene_instant and load_scene: pop the bank select_scene_bank
// pushed, and X and Y as they were. Returns A = X.
static void scene_done(void) {
  Cpu6502 *c = &g_nes.cpu;
  pop_bank();
  c->y = emu_pull();
  c->x = emu_pull();
  emu_set_a(c->x);
}

// change_scene (7:C000): blanks the picture in a dissolve, loads the new scene's CHR banks,
// attributes and palette, and dissolves the new picture in. Keeps X and Y; returns A = X.
void change_scene(void) {
  RUN_ORIGINAL_IF_LAG(kChangeScene);
  Cpu6502 *c = &g_nes.cpu;
  emu_push(c->x);
  emu_push(c->y);
  emu_set_a(c->y);
  load_dissolve_pattern();
  clear_and_upload_sprites();
  emu_set_a(0x01);
  scene_blank = 0x01;
  draw_scene_dissolve();
  RESUME_ORIGINAL_IF_LAG(kChangeScene, 0xC011, 2);
  select_scene_bank();
  far_call_8000(0x61);  // load_scene_chr_banks
  load_scene_attrs();
  request_attr_upload();
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kChangeScene, 0xC022, 2);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kChangeScene, 0xC025, 2);
  far_call_8000(0x60);  // load_scene_palette
  request_palette_upload();
  emu_set_a(0x00);
  scene_blank = 0x00;
  draw_scene_dissolve();
  RESUME_ORIGINAL_IF_LAG(kChangeScene, 0xC034, 2);
  apply_room_settings();
  scene_done();
}

// change_scene_instant (7:C03F): the same without the dissolves: the old picture stays, with its
// colours black, until the new one is drawn at once. Keeps X and Y; returns A = X.
void change_scene_instant(void) {
  RUN_ORIGINAL_IF_LAG(kChangeSceneInstant);
  Cpu6502 *c = &g_nes.cpu;
  emu_push(c->x);
  emu_push(c->y);
  emu_set_a(c->y);
  clear_and_upload_sprites();
  blacken_palette_colors();
  select_scene_bank();
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kChangeSceneInstant, 0xC04F, 2);
  far_call_8000(0x61);  // load_scene_chr_banks
  load_scene_attrs();
  request_attr_upload();
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kChangeSceneInstant, 0xC05D, 2);
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kChangeSceneInstant, 0xC060, 2);
  apply_room_settings();
  draw_scene();
  RESUME_ORIGINAL_IF_LAG(kChangeSceneInstant, 0xC066, 2);
  far_call_8000(0x60);  // load_scene_palette
  request_palette_upload();
  scene_done();
}

// load_scene (7:C076): draws the picture for scene at once, with its CHR banks and attributes but
// not its palette. Keeps X and Y; returns A = X.
void load_scene(void) {
  RUN_ORIGINAL_IF_LAG(kLoadScene);
  Cpu6502 *c = &g_nes.cpu;
  emu_push(c->x);
  emu_push(c->y);
  emu_set_a(c->y);
  clear_and_upload_sprites();
  select_scene_bank();
  far_call_8000(0x61);  // load_scene_chr_banks
  load_scene_attrs();
  request_attr_upload();
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kLoadScene, 0xC08E, 2);
  apply_room_settings();
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kLoadScene, 0xC094, 2);
  draw_scene();
  RESUME_ORIGINAL_IF_LAG(kLoadScene, 0xC097, 2);
  scene_done();
}

// select_scene_bank (7:C09F): pushes and switches to the bank holding scene's picture (0, 1 or
// 2, 76 pictures each), and sets scene_index to its number there. Ends with A = scene_index
// and the flags of the SBC, or for bank 0 as push_and_switch_bank leaves them.
void select_scene_bank(void) {
  RUN_ORIGINAL_IF_LAG(kSelectSceneBank);
  Cpu6502 *c = &g_nes.cpu;
  scene_index = scene;
  if(scene < 0x4C) {
    push_and_switch_bank(0x00);
    return;
  }
  uint8_t first = scene < 0x98 ? 0x4C : 0x98;
  push_and_switch_bank(first == 0x4C ? 0x01 : 0x02);
  emu_set_flag(kFlagC, 1);
  c->a = emu_sbc(scene, first);
  scene_index = c->a;
}

// scene_pointer (7:C0CE): data_ptr = picture scene_index of the bank mapped at $8000. Y is the
// table offset; returns A = the pointer's high byte, the carry from the ASL.
void scene_pointer(void) {
  RUN_ORIGINAL_IF_LAG(kScenePointer);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_flag(kFlagC, scene_index >> 7);
  c->y = (uint8_t)(scene_index << 1);
  data_ptr[0] = cpu_bus_read((uint16_t)(kScenePictures + c->y));
  data_ptr[1] = cpu_bus_read((uint16_t)(kScenePictures + 1 + c->y));
  emu_set_a(data_ptr[1]);
}

// load_scene_attrs (7:C186): the picture's 16 attribute bytes, after its tiles, into
// attr_buffer_2000 at the places scene_attr_positions gives. Ends with X the last place, Y = $10.
void load_scene_attrs(void) {
  RUN_ORIGINAL_IF_LAG(kLoadSceneAttrs);
  Cpu6502 *c = &g_nes.cpu;
  scene_pointer();
  add16(data_ptr, 0xC4);
  c->y = 0;
  do {
    c->x = cpu_bus_read((uint16_t)(kSceneAttrPositions + c->y));
    c->a = emu_read_ind_y(0x55, c->y);
    attr_buffer_2000[c->x] = c->a;
    c->y++;
    emu_cmp(c->y, 0x10);
  } while(c->y < 0x10);
}

// blacken_palette_colors (7:C1B6): colours 1-3 of the first three background palettes to black
// ($0F), and asks the NMI to upload the palette.
void blacken_palette_colors(void) {
  RUN_ORIGINAL_IF_LAG(kBlackenPaletteColors);
  Cpu6502 *c = &g_nes.cpu;
  for(c->y = 0; c->y < 3; c->y++) {
    palette_buffer[1 + c->y] = 0x0F;
    palette_buffer[5 + c->y] = 0x0F;
    palette_buffer[9 + c->y] = 0x0F;
  }
  emu_set_a(0x0F);
  emu_cmp(c->y, 0x03);
  request_palette_upload();
}

// apply_room_settings (7:C201): plays the sound in the low nibble of room's room_settings entry,
// except in room $33. Y = room.
void apply_room_settings(void) {
  RUN_ORIGINAL_IF_LAG(kApplyRoomSettings);
  Cpu6502 *c = &g_nes.cpu;
  c->y = room;
  emu_cmp(c->y, 0x33);
  if(c->y == 0x33) {
    return;
  }
  play_sound(cpu_bus_read((uint16_t)(kRoomSettings + c->y)) & 0x0F);
}

// load_dissolve_pattern (7:C211): dissolve_pattern = the high nibble of room's room_settings
// entry. Y = room; returns it in A, with the carry from the last LSR.
void load_dissolve_pattern(void) {
  RUN_ORIGINAL_IF_LAG(kLoadDissolvePattern);
  Cpu6502 *c = &g_nes.cpu;
  c->y = room;
  uint8_t settings = cpu_bus_read((uint16_t)(kRoomSettings + c->y));
  emu_set_a(settings >> 4);
  emu_set_flag(kFlagC, (settings >> 3) & 1);
  dissolve_pattern = c->a;
}

// ---- Drawing the picture -------------------------------------------------------------------

// draw_scene (7:C1CC): copies the picture's tiles to scene_tiles, queues them as one 14x14 block
// at $2082, and waits until the NMI has written them.
void draw_scene(void) {
  RUN_ORIGINAL_IF_LAG(kDrawScene);
  scene_pointer();
  copy_scene_tiles();
  emu_set_a(0x0E);
  g_ram[kSceneBlockHeader] = 0x0E;
  g_ram[kSceneBlockHeader + 1] = 0x0E;
  g_ram[0x1E] = kSceneTilesBlockLo;
  g_ram[0x1F] = kSceneTilesBlockHi;
  g_ram[0x1C] = 0x82;
  emu_set_a(0x20);
  g_ram[0x1D] = 0x20;
  queue_vram_block();
  RESUME_ORIGINAL_IF_LAG(kDrawScene, 0xC1ED, 0);
  request_vram_queue();
  wait_nmi_work_done();
  RESUME_ORIGINAL_IF_LAG(kDrawScene, 0xC1F3, 0);
}

// copy_scene_tiles (7:C1F4): the picture's 196 tiles from data_ptr to scene_tiles. Y = $C4.
void copy_scene_tiles(void) {
  RUN_ORIGINAL_IF_LAG(kCopySceneTiles);
  Cpu6502 *c = &g_nes.cpu;
  c->y = 0;
  do {
    c->a = emu_read_ind_y(0x55, c->y);
    scene_tiles[c->y] = c->a;
    c->y++;
    emu_cmp(c->y, 0xC4);
  } while(c->y != 0xC4);
}

// draw_scene_dissolve (7:C0DD): draws the picture (or $FF everywhere, with scene_blank set) a
// tile at a time over 6 passes, a frame each: in each pass, each of the 14 rows draws the
// columns dissolve_row_mask picks. Ends with A = 6, X = $0E.
void draw_scene_dissolve(void) {
  RUN_ORIGINAL_IF_LAG(kDrawSceneDissolve);
  Cpu6502 *c = &g_nes.cpu;
  play_sound(0x27);
  scene_pointer();
  emu_set_a(0x00);
  g_ram[0x78] = 0x00;
  do {
    emu_set_a(0x00);
    g_ram[0x77] = 0x00;
    do {
      far_call_8000(0x6B);  // dissolve_row_mask
      c->x = 0;
      do {
        // lsr $84, ror $83: the next column's bit into the carry
        uint8_t hi = g_ram[0x84], lo = g_ram[0x83];
        g_ram[0x84] = hi >> 1;
        g_ram[0x83] = (uint8_t)(lo >> 1 | hi << 7);
        if(lo & 1) {
          dissolve_draw_tile();
          RESUME_ORIGINAL_IF_LAG(kDrawSceneDissolve, 0xC0FD, 0);
        }
        c->x++;
        emu_cmp(c->x, 0x0E);
      } while(c->x != 0x0E);
      g_ram[0x77]++;
      emu_set_a(g_ram[0x77]);
      emu_cmp(c->a, 0x0E);
    } while(c->a != 0x0E);
    request_vram_queue();
    wait_vblank();
    RESUME_ORIGINAL_IF_LAG(kDrawSceneDissolve, 0xC110, 0);
    g_ram[0x78]++;
    emu_set_a(g_ram[0x78]);
    emu_cmp(c->a, 0x06);
  } while(c->a != 0x06);
}

// dissolve_draw_tile (7:C119): queues the tile at column X, row $77 as a one-tile block.
void dissolve_draw_tile(void) {
  RUN_ORIGINAL_IF_LAG(kDissolveDrawTile);
  dissolve_tile_value();
  dissolve_tile_address();
  emu_set_a(0x01);
  g_ram[0x5B] = 0x01;
  g_ram[0x5C] = 0x01;
  g_ram[0x1E] = 0x5B;
  emu_set_a(0x00);
  g_ram[0x1F] = 0x00;
  queue_vram_block();
  RESUME_ORIGINAL_IF_LAG(kDissolveDrawTile, 0xC130, 0);
}

// dissolve_tile_value (7:C131): $5D = the picture's tile at column X, row $77, or $FF with
// scene_blank set. Y = its offset (14 * row + column); A = scene_blank, or $FF.
void dissolve_tile_value(void) {
  RUN_ORIGINAL_IF_LAG(kDissolveTileValue);
  Cpu6502 *c = &g_nes.cpu;
  uint8_t a = (uint8_t)(g_ram[0x77] << 1);
  g_ram[0x5B] = a;
  emu_set_flag(kFlagC, 1);
  g_ram[0x5B] = emu_sbc((uint8_t)(a << 3), g_ram[0x5B]);
  emu_set_flag(kFlagC, 0);
  c->y = emu_adc(c->x, g_ram[0x5B]);
  g_ram[0x5D] = emu_read_ind_y(0x55, c->y);
  emu_set_a(scene_blank);
  if(scene_blank) {
    emu_set_a(0xFF);
    g_ram[0x5D] = 0xFF;
  }
}

// dissolve_tile_address (7:C150): $1C/$1D = the VRAM address of column X, row $77 of the picture
// ($2082 + 32 * row + column). Returns A = the high byte, with the flags of its ADC.
void dissolve_tile_address(void) {
  RUN_ORIGINAL_IF_LAG(kDissolveTileAddress);
  Cpu6502 *c = &g_nes.cpu;
  uint16_t offset = (uint16_t)(g_ram[0x77] << 5);
  emu_set_flag(kFlagC, 0);
  g_ram[0x1C] = emu_adc(c->x, (uint8_t)offset);
  g_ram[0x1D] = emu_adc(0x00, (uint8_t)(offset >> 8));
  emu_set_flag(kFlagC, 0);
  g_ram[0x1C] = emu_adc(g_ram[0x1C], 0x82);
  c->a = emu_adc(g_ram[0x1D], 0x20);
  g_ram[0x1D] = c->a;
}

// ---- Bank 6 --------------------------------------------------------------------------------

// load_scene_chr_banks (6:8070, far call $61): the CHR banks for scene, R0 and R4 from its
// 2-byte entry at 6:80AC (and R2 = R6 = 0, R5 = $C0, R7 = 1 in chr_bank_shadows), then sets them.
void load_scene_chr_banks(void) {
  RUN_ORIGINAL_IF_LAG(kLoadSceneChrBanks);
  Cpu6502 *c = &g_nes.cpu;
  g_ram[0x5E] = scene >> 7;
  g_ram[0x5D] = (uint8_t)(scene << 1);
  emu_set_flag(kFlagC, 0);
  g_ram[0x5D] = emu_adc(g_ram[0x5D], kSceneChrBankTable & 0xFF);
  g_ram[0x5E] = emu_adc(g_ram[0x5E], kSceneChrBankTable >> 8);
  c->y = 0;
  chr_bank_shadows[0] = emu_read_ind_y(0x5D, 0);
  c->y = 1;
  chr_bank_shadows[4] = emu_read_ind_y(0x5D, 1);
  c->y = 0;
  chr_bank_shadows[2] = 0;
  chr_bank_shadows[6] = 0;
  c->y = 1;
  chr_bank_shadows[7] = 1;
  emu_set_a(0xC0);
  chr_bank_shadows[5] = 0xC0;
  apply_chr_banks();
}

// load_scene_palette (6:81DE, far call $60): scene_palette_id = scene's entry of
// scene_palette_ids, then that palette into palette_buffer (it runs into load_palette).
void load_scene_palette(void) {
  RUN_ORIGINAL_IF_LAG(kLoadScenePalette);
  Cpu6502 *c = &g_nes.cpu;
  c->y = scene;
  emu_set_a(cpu_bus_read((uint16_t)(kScenePaletteIds + c->y)));
  scene_palette_id = c->a;
  load_palette();
}

// load_palette (6:81E7, far call $62): palette scene_palette_id into palette_buffer. Y = $20.
void load_palette(void) {
  RUN_ORIGINAL_IF_LAG(kLoadPalette);
  Cpu6502 *c = &g_nes.cpu;
  palette_pointer();
  c->y = 0;
  do {
    c->a = emu_read_ind_y(0x1E, c->y);
    palette_buffer[c->y] = c->a;
    c->y++;
    emu_cmp(c->y, 0x20);
  } while(c->y < 0x20);
}

// palette_pointer (6:8213): $1E/$1F = palette scene_palette_id's entry of palette_table. Y is
// the table offset; returns A = the high byte, the carry from the ASL.
void palette_pointer(void) {
  RUN_ORIGINAL_IF_LAG(kPalettePointer);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_flag(kFlagC, scene_palette_id >> 7);
  c->y = (uint8_t)(scene_palette_id << 1);
  g_ram[0x1E] = cpu_bus_read((uint16_t)(kPaletteTable + c->y));
  g_ram[0x1F] = cpu_bus_read((uint16_t)(kPaletteTable + 1 + c->y));
  emu_set_a(g_ram[0x1F]);
}

// dissolve_row_mask (6:9A50, far call $6B): $83/$84 = which columns of row $77 pass $78 draws,
// one bit each from bit 0 of $83: entry 14 * pass + row of the order dissolve_pattern picks. Y = 1;
// returns A = $84.
void dissolve_row_mask(void) {
  RUN_ORIGINAL_IF_LAG(kDissolveRowMask);
  Cpu6502 *c = &g_nes.cpu;
  c->y = (uint8_t)(dissolve_pattern << 1);
  g_ram[0x57] = cpu_bus_read((uint16_t)(kDissolvePatterns + c->y));
  g_ram[0x58] = cpu_bus_read((uint16_t)(kDissolvePatterns + 1 + c->y));
  // 28 * pass, in 8 bits: 4 * pass + 8 * pass + 16 * pass
  uint8_t a = (uint8_t)(g_ram[0x78] << 2);
  g_ram[0x59] = a;
  a = (uint8_t)(a << 1);
  g_ram[0x5A] = a;
  a = (uint8_t)(a << 1);
  emu_set_flag(kFlagC, 0);
  a = emu_adc(a, g_ram[0x59]);
  emu_set_flag(kFlagC, 0);
  a = emu_adc(a, g_ram[0x5A]);
  emu_set_flag(kFlagC, 0);
  g_ram[0x57] = emu_adc(a, g_ram[0x57]);
  g_ram[0x58] = emu_adc(0x00, g_ram[0x58]);
  emu_set_flag(kFlagC, 0);
  g_ram[0x57] = emu_adc((uint8_t)(g_ram[0x77] << 1), g_ram[0x57]);
  g_ram[0x58] = emu_adc(0x00, g_ram[0x58]);
  c->y = 0;
  g_ram[0x83] = emu_read_ind_y(0x57, 0);
  c->y = 1;
  emu_set_a(emu_read_ind_y(0x57, 1));
  g_ram[0x84] = c->a;
}
