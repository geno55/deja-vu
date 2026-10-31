// Screen layouts, the numbers they print, and metasprites: a layout is a
// list of commands in bank 6 for menus and title text ($BC00 has a pointer to each; docs/conventions.md,
// "Screen layouts"), most of it plain tiles going through the VRAM queue.
//
// The scratch bytes, as the originals use them: $77-$79 the command and its two operands, $9C/$9D
// the next tile's column and row, $98 bit 6 while a name prints, $9A/$9B a name's address, $9E-$A1
// the block of one tile for the VRAM
// queue, scene_index/$76 a tile's column and row, $0E/$0F the same, $1C/$1D a VRAM address or where a
// number is, $84 how many digits were left. And for draw_metasprite: $1E/$1F its data, $20/$21 x
// and y, $22 the attribute bits, data_ptr the columns left and per row, $57 the rows left, $58/$59
// the next sprite's x and y.
//
// A queue that is full waits for the NMI (queue_vram_block), so what leads to it checks whether to
// go on as original code.
#include "funcs.h"
#include "variables.h"

// PHA, TXA, PHA, TYA, PHA, and the end that undoes it.
static void push_registers(void) {
  Cpu6502 *c = &g_nes.cpu;
  emu_push(c->a);
  emu_set_a(c->x);
  emu_push(c->x);
  emu_set_a(c->y);
  emu_push(c->y);
}

static void pull_registers(void) {
  Cpu6502 *c = &g_nes.cpu;
  c->y = emu_pull();
  c->x = emu_pull();
  emu_set_a(emu_pull());
}

// ---- Running a layout ----------------------------------------------------------------------

// draw_layout_number (7:FBED): layout A, in bank 6. Keeps A, X and Y.
void draw_layout_number(uint8_t number) {
  emu_set_a(number);
  RUN_ORIGINAL_IF_LAG(kDrawLayoutNumber);
  Cpu6502 *c = &g_nes.cpu;
  data_ptr[0] = number;
  push_registers();
  push_and_switch_bank(0x06);
  data_ptr[1] = number >> 7;
  data_ptr[0] = (uint8_t)(number << 1);
  emu_set_flag(kFlagC, 0);
  data_ptr[0] = emu_adc(0x00, data_ptr[0]);
  data_ptr[1] = emu_adc(kLayoutTable >> 8, data_ptr[1]);
  c->y = 0;
  layout_ptr[0] = emu_read_ind_y(0x55, 0);
  c->y = 1;
  layout_ptr[1] = emu_read_ind_y(0x55, 1);
  emu_set_a(layout_ptr[1]);
  layout_run();
  RESUME_ORIGINAL_IF_LAG(kDrawLayoutNumber, 0xFC1C, 3);
  pop_bank();
  pull_registers();
}

// layout_run (7:FC25): its commands, up to $FF.
void layout_run(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutRun);
  Cpu6502 *c = &g_nes.cpu;
  layout_reset_state();
  for(;;) {
    layout_fetch();
    emu_set_a(g_ram[0x77]);
    emu_cmp(c->a, 0xFF);
    if(c->a == 0xFF) {
      return;
    }
    layout_command();
    RESUME_ORIGINAL_IF_LAG(kLayoutRun, 0xFC34, 0);
  }
}

// layout_reset_state (7:FC38)
void layout_reset_state(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutResetState);
  emu_set_a(0x00);
  g_ram[0x98] = g_ram[0x9A] = g_ram[0x9B] = 0x00;
  g_ram[0x9E] = g_ram[0x9F] = g_ram[0xA0] = g_ram[0xA1] = 0x00;
  g_ram[0xA2] = g_ram[0xA3] = g_ram[0xA4] = 0x00;
}

// layout_fetch (7:FC4F): the next three bytes to $77-$79.
void layout_fetch(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutFetch);
  Cpu6502 *c = &g_nes.cpu;
  c->y = 0;
  g_ram[0x77] = emu_read_ind_y(0x95, 0);
  c->y = 1;
  g_ram[0x78] = emu_read_ind_y(0x95, 1);
  c->y = 2;
  g_ram[0x79] = emu_read_ind_y(0x95, 2);
  emu_set_a(g_ram[0x79]);
}

// layout_advance_1 (7:FC60), layout_advance_2 (7:FC6E), layout_advance_3 (7:FC74): layout_ptr on.
void layout_advance_1(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutAdvance1);
  add16(layout_ptr, 0x01);
}

void layout_advance_2(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutAdvance2);
  layout_advance_1();
  layout_advance_1();
}

void layout_advance_3(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutAdvance3);
  layout_advance_1();
  layout_advance_2();
}

// layout_put_tile (7:FC7A): tile $77 at the next place.
void layout_put_tile(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutPutTile);
  scene_index = g_ram[0x9C];
  emu_set_a(g_ram[0x9D]);
  g_ram[0x76] = g_nes.cpu.a;
  layout_draw_tile();
  RESUME_ORIGINAL_IF_LAG(kLayoutPutTile, 0xFC85, 0);
  g_ram[0x9C]++;
  emu_set_nz(g_ram[0x9C]);
}

// layout_command (7:FC88): below $13 what the table says, the rest a tile. $90 and $A0 print nothing.
void layout_command(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutCommand);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(g_ram[0x77]);
  emu_cmp(c->a, 0x13);
  if(c->a < 0x13) {
    uint8_t op = c->a;
    JUMP_TABLE_OR_ORIGINAL(kLayoutCommand, 0xFC8E, 0, op, 19);
    jump_table(0xFC90, op);
    switch(op) {
      case 0x00: layout_name_by_index_command(); return;
      case 0x01: layout_byte_padded_command(); return;
      case 0x02: layout_word_padded_command(); return;
      case 0x03: layout_name_at_address_command(); return;
      case 0x04: layout_byte_command(); return;
      case 0x05: layout_word_command(); return;
      case 0x06: layout_position_command(); return;
      case 0x12: layout_name_in_ram_command(); return;
      default: break;
    }
  }
  emu_set_a(g_ram[0x77]);
  emu_cmp(c->a, 0x90);
  if(c->a != 0x90) {
    emu_cmp(c->a, 0xA0);
    if(c->a != 0xA0) {
      layout_put_tile();
      RESUME_ORIGINAL_IF_LAG(kLayoutCommand, 0xFCC4, 0);
    }
  }
  layout_advance_1();
}

// ---- The commands ---------------------------------------------------------------------------

// layout_name_by_index_command (7:FCCE), command 0: name $78.
void layout_name_by_index_command(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutNameByIndexCommand);
  layout_advance_2();
  layout_name_from_index();
  layout_print_name_buffer();
  RESUME_ORIGINAL_IF_LAG(kLayoutNameByIndexCommand, 0xFCD7, 0);
}

// layout_name_at_address_command (7:FCD8), command 3: the name at $78/$79.
void layout_name_at_address_command(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutNameAtAddressCommand);
  layout_advance_3();
  layout_name_from_address();
  layout_print_name_buffer();
  RESUME_ORIGINAL_IF_LAG(kLayoutNameAtAddressCommand, 0xFCE1, 0);
}

// layout_name_in_ram_command (7:FCE2), command $12: the name whose number is at $78/$79.
void layout_name_in_ram_command(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutNameInRamCommand);
  layout_advance_3();
  layout_name_from_ram();
  layout_print_name_buffer();
  RESUME_ORIGINAL_IF_LAG(kLayoutNameInRamCommand, 0xFCEB, 0);
}

// layout_byte_padded_command (7:FD66), command 1, and the same for the others: the number at $78/$79.
void layout_byte_padded_command(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutBytePaddedCommand);
  layout_advance_3();
  layout_set_vram_dest();
  layout_print_byte_padded();
  RESUME_ORIGINAL_IF_LAG(kLayoutBytePaddedCommand, 0xFD6F, 0);
  layout_skip_3();
}

// layout_word_padded_command (7:FD73), command 2.
void layout_word_padded_command(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutWordPaddedCommand);
  layout_advance_3();
  layout_set_vram_dest();
  layout_print_word_padded();
  RESUME_ORIGINAL_IF_LAG(kLayoutWordPaddedCommand, 0xFD7C, 0);
  layout_skip_5();
}

// layout_byte_command (7:FD80), command 4.
void layout_byte_command(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutByteCommand);
  layout_advance_3();
  layout_set_vram_dest();
  layout_print_byte();
  RESUME_ORIGINAL_IF_LAG(kLayoutByteCommand, 0xFD89, 0);
  layout_advance_x();
}

// layout_word_command (7:FD8D), command 5.
void layout_word_command(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutWordCommand);
  layout_advance_3();
  layout_set_vram_dest();
  layout_print_word();
  RESUME_ORIGINAL_IF_LAG(kLayoutWordCommand, 0xFD96, 0);
  layout_advance_x();
}

// layout_position_command (7:FDBE), command 6: the next tile goes at $78, $79.
void layout_position_command(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutPositionCommand);
  layout_advance_3();
  g_ram[0x9C] = g_ram[0x78];
  emu_set_a(g_ram[0x79]);
  g_ram[0x9D] = g_nes.cpu.a;
}

// ---- Names ---------------------------------------------------------------------------------

// layout_name_from_ram (7:FCEC) and layout_name_from_index (7:FCF3): name_buffer from name A, of 8
// bytes each at $8D00 in bank 2.
static void name_from_number(void) {
  TAIL_ORIGINAL_IF_LAG(kLayoutNameFromIndex, 0xFCF5, 0);
  unsigned offset = g_nes.cpu.a * 8u;
  g_ram[0x9A] = (uint8_t)offset;
  emu_set_flag(kFlagC, 0);
  g_ram[0x9A] = emu_adc(0x00, g_ram[0x9A]);
  g_ram[0x9B] = emu_adc(kNames >> 8, (uint8_t)(offset >> 8));
  push_and_switch_bank(0x02);
  copy_name_to_buffer();
  pop_bank();
}

void layout_name_from_ram(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutNameFromRam);
  g_nes.cpu.y = 0;
  emu_set_a(emu_read_ind_y(0x78, 0));
  name_from_number();
}

void layout_name_from_index(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutNameFromIndex);
  emu_set_a(g_ram[0x78]);
  name_from_number();
}

// layout_name_from_address (7:FD20)
void layout_name_from_address(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutNameFromAddress);
  g_ram[0x9A] = g_ram[0x78];
  emu_set_a(g_ram[0x79]);
  g_ram[0x9B] = g_nes.cpu.a;
  copy_name_to_buffer();
}

// copy_name_to_buffer (7:FD2C): 8 bytes from $9A/$9B.
void copy_name_to_buffer(void) {
  RUN_ORIGINAL_IF_LAG(kCopyNameToBuffer);
  Cpu6502 *c = &g_nes.cpu;
  c->y = 0;
  do {
    emu_set_a(emu_read_ind_y(0x9A, c->y));
    name_buffer[c->y] = c->a;
    c->y++;
    emu_cmp(c->y, 0x08);
  } while(c->y != 0x08);
}

// layout_print_name_buffer (7:FD39): its tiles, up to 8 or $FF. $90 and $A0 print nothing.
void layout_print_name_buffer(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutPrintNameBuffer);
  Cpu6502 *c = &g_nes.cpu;
  g_ram[0x98] |= 0x40;
  c->x = 0;
  do {
    emu_set_a(name_buffer[c->x]);
    g_ram[0x77] = c->a;
    emu_cmp(c->a, 0xFF);
    if(c->a == 0xFF) {
      break;
    }
    emu_cmp(c->a, 0x90);
    if(c->a != 0x90) {
      emu_cmp(c->a, 0xA0);
      if(c->a != 0xA0) {
        emu_set_a(name_buffer[1 + c->x]);
        g_ram[0x78] = c->a;
        layout_put_tile();
        RESUME_ORIGINAL_IF_LAG(kLayoutPrintNameBuffer, 0xFD5A, 0);
      }
    }
    c->x++;
    emu_cmp(c->x, 0x08);
  } while(c->x != 0x08);
  emu_set_a(g_ram[0x98] & 0xBF);
  g_ram[0x98] = c->a;
}

// ---- Places --------------------------------------------------------------------------------

// layout_advance_x (7:FD9A): the column on by $84.
void layout_advance_x(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutAdvanceX);
  emu_set_flag(kFlagC, 0);
  g_ram[0x9C] = emu_adc(g_ram[0x9C], g_ram[0x84]);
  g_nes.cpu.a = g_ram[0x9C];
}

// layout_set_vram_dest (7:FDA2)
void layout_set_vram_dest(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutSetVramDest);
  g_ram[0x1C] = g_ram[0x78];
  g_ram[0x1D] = g_ram[0x79];
  g_ram[0x0E] = g_ram[0x9C];
  emu_set_a(g_ram[0x9D]);
  g_ram[0x0F] = g_nes.cpu.a;
}

// layout_skip_5 (7:FDB3), layout_skip_3 (7:FDB7): the column on.
void layout_skip_5(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutSkip5);
  g_ram[0x9C] += 2;
  layout_skip_3();
}

void layout_skip_3(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutSkip3);
  g_ram[0x9C] += 3;
  emu_set_nz(g_ram[0x9C]);
}

// ---- One tile ------------------------------------------------------------------------------

// layout_draw_tile (7:FDCA): tile $77 at column scene_index, row $76 - 1.
void layout_draw_tile(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutDrawTile);
  layout_tile_position();
  layout_tile_kind();
  tile_position_to_vram();
  queue_tile();
  RESUME_ORIGINAL_IF_LAG(kLayoutDrawTile, 0xFDD6, 0);
}

// layout_tile_position (7:FDD7)
void layout_tile_position(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutTilePosition);
  g_ram[0x0E] = scene_index;
  emu_set_a(g_ram[0x76]);
  g_ram[0x0F] = g_nes.cpu.a;
}

// tile_position_to_vram (7:FDE0): $1C/$1D = $2000 + row * 32 + column.
void tile_position_to_vram(void) {
  RUN_ORIGINAL_IF_LAG(kTilePositionToVram);
  unsigned offset = g_ram[0x0F] * 32u;
  emu_set_flag(kFlagC, 0);
  g_ram[0x1C] = emu_adc((uint8_t)offset, g_ram[0x0E]);
  g_ram[0x1D] = emu_adc((uint8_t)(offset >> 8), 0x20);
  g_nes.cpu.a = g_ram[0x1D];
}

// layout_tile_kind (7:FE0A): the row - 1, and a block of one tile at $9E: width 1, height 2, the
// font's two tiles for $77 by its range.
void layout_tile_kind(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutTileKind);
  Cpu6502 *c = &g_nes.cpu;
  g_ram[0x0F]--;
  uint8_t tile = g_ram[0x77];
  emu_set_a(tile);
  emu_cmp(tile, 0x16);
  int range = 0;  // 0: as it is, 1: + $70, top $A0, 2: + $80, top $90
  if(tile >= 0x16) {
    emu_cmp(tile, 0x50);
    if(tile < 0x50) {
      range = 2;
    } else {
      emu_cmp(tile, 0x60);
      if(tile < 0x60) {
        range = 1;
      } else {
        emu_cmp(tile, 0x70);
        if(tile < 0x70) {
          range = 2;
        } else {
          emu_cmp(tile, 0x80);
          if(tile < 0x80) {
            range = 1;
          }
        }
      }
    }
  }
  if(range == 1) {
    emu_set_flag(kFlagC, 0);
    g_ram[0xA1] = emu_adc(tile, 0x70);
    g_ram[0xA0] = 0xA0;
  } else if(range == 2) {
    emu_set_flag(kFlagC, 0);
    g_ram[0xA1] = emu_adc(tile, 0x80);
    g_ram[0xA0] = 0x90;
  } else {
    emu_cmp(tile, 0xFE);
    g_ram[0xA1] = tile == 0xFE ? 0x7E : tile;
    g_ram[0xA0] = 0xDE;
  }
  g_ram[0x9F] = 0x02;
  emu_set_a(0x01);
  g_ram[0x9E] = c->a;
}

// queue_tile (7:FE51): the block at $9E, to the VRAM queue.
void queue_tile(void) {
  RUN_ORIGINAL_IF_LAG(kQueueTile);
  g_ram[0x1E] = 0x9E;
  emu_set_a(0x00);
  g_ram[0x1F] = 0x00;
  queue_vram_block();
  RESUME_ORIGINAL_IF_LAG(kQueueTile, 0xFE5C, 0);
}

// ---- Numbers -------------------------------------------------------------------------------

// layout_print_byte_padded (7:FEC8), layout_print_byte (7:FEE4), layout_print_word_padded (7:FF00),
// layout_print_word (7:FF1C): the number at $1C/$1D at $0E, $0F, in decimal. Padded: with spaces
// for the leading zeros, in a field of 3 for a byte and 5 for a word. Not padded: $84 = its length.
// All keep A, X and Y.
void layout_print_byte_padded(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutPrintBytePadded);
  push_registers();
  load_byte_value();
  decimal_digits();
  trim_leading_zeros();
  digits_to_tiles_padded();
  g_nes.cpu.x = 0x02;
  print_decimal_digits();
  RESUME_ORIGINAL_IF_LAG(kLayoutPrintBytePadded, 0xFEDE, 3);
  pull_registers();
}

void layout_print_byte(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutPrintByte);
  push_registers();
  load_byte_value();
  decimal_digits();
  trim_leading_zeros();
  digits_to_tiles();
  g_nes.cpu.x = 0x00;
  print_decimal_digits();
  RESUME_ORIGINAL_IF_LAG(kLayoutPrintByte, 0xFEFA, 3);
  pull_registers();
}

void layout_print_word_padded(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutPrintWordPadded);
  push_registers();
  load_word_value();
  decimal_digits();
  trim_leading_zeros();
  digits_to_tiles_padded();
  g_nes.cpu.x = 0x00;
  print_decimal_digits();
  RESUME_ORIGINAL_IF_LAG(kLayoutPrintWordPadded, 0xFF16, 3);
  pull_registers();
}

void layout_print_word(void) {
  RUN_ORIGINAL_IF_LAG(kLayoutPrintWord);
  push_registers();
  load_word_value();
  decimal_digits();
  trim_leading_zeros();
  digits_to_tiles();
  g_nes.cpu.x = 0x00;
  print_decimal_digits();
  RESUME_ORIGINAL_IF_LAG(kLayoutPrintWord, 0xFF32, 3);
  pull_registers();
}

// load_byte_value (7:FF38), load_word_value (7:FF40): data_ptr = the number at $1C/$1D. Y = 0.
void load_byte_value(void) {
  RUN_ORIGINAL_IF_LAG(kLoadByteValue);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(0x00);
  c->y = 0;
  data_ptr[1] = 0x00;
  TAIL_ORIGINAL_IF_LAG(kLoadWordValue, 0xFF47, 0);
  emu_set_a(emu_read_ind_y(0x1C, 0));
  data_ptr[0] = c->a;
}

void load_word_value(void) {
  RUN_ORIGINAL_IF_LAG(kLoadWordValue);
  Cpu6502 *c = &g_nes.cpu;
  c->y = 1;
  data_ptr[1] = emu_read_ind_y(0x1C, 1);
  c->y = 0;
  emu_set_a(emu_read_ind_y(0x1C, 0));
  data_ptr[0] = c->a;
}

// decimal_digits (7:FF4C): data_ptr to its 5 digits at scene_index, most significant first.
void decimal_digits(void) {
  RUN_ORIGINAL_IF_LAG(kDecimalDigits);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(0x00);
  c->x = 0;
  do {
    g_ram[0x75 + c->x] = 0x00;
    c->x++;
    emu_cmp(c->x, 0x05);
  } while(c->x != 0x05);
  c->x = 0;
  c->y = 0;
  do {
    uint8_t lo = cpu_bus_read((uint16_t)(kDecimalPlaces + c->y));
    uint8_t hi = cpu_bus_read((uint16_t)(kDecimalPlaces + 1 + c->y));
    for(;;) {
      emu_set_flag(kFlagC, 1);
      emu_sbc(data_ptr[0], lo);
      emu_sbc(data_ptr[1], hi);
      if(!(c->p & kFlagC)) {
        break;
      }
      emu_set_flag(kFlagC, 1);
      data_ptr[0] = emu_sbc(data_ptr[0], lo);
      data_ptr[1] = emu_sbc(data_ptr[1], hi);
      g_ram[0x75 + c->x]++;
    }
    c->y += 2;
    c->x++;
    emu_cmp(c->x, 0x04);
  } while(c->x != 0x04);
  emu_set_a(data_ptr[0]);
  g_ram[0x75 + c->x] = c->a;
}

// trim_leading_zeros (7:FF90): name_buffer blank, and X = $84 = the first digit that isn't 0, or
// the last.
void trim_leading_zeros(void) {
  RUN_ORIGINAL_IF_LAG(kTrimLeadingZeros);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(0xFF);
  c->x = 0;
  do {
    name_buffer[c->x] = 0xFF;
    c->x++;
    emu_cmp(c->x, 0x05);
  } while(c->x != 0x05);
  c->x = 0;
  for(;;) {
    emu_set_a(g_ram[(uint8_t)(0x75 + c->x)]);
    if(c->a) {
      break;
    }
    c->x++;
    emu_cmp(c->x, 0x04);
    if(c->x == 0x04) {
      break;
    }
  }
  g_ram[0x84] = c->x;
}

// digits_to_tiles_padded (7:FFAA): digits X on to their places in name_buffer, whose start stays
// blank, ended by $FE.
void digits_to_tiles_padded(void) {
  RUN_ORIGINAL_IF_LAG(kDigitsToTilesPadded);
  Cpu6502 *c = &g_nes.cpu;
  do {
    emu_set_flag(kFlagC, 0);
    c->a = emu_adc(g_ram[(uint8_t)(0x75 + c->x)], 0x80);
    name_buffer[c->x] = c->a;
    c->x++;
    emu_cmp(c->x, 0x05);
  } while(c->x != 0x05);
  emu_set_a(0xFE);
  name_buffer[5] = 0xFE;
}

// digits_to_tiles (7:FFBD): digits X on to the start of name_buffer, ended by $FE. $84 = how many.
void digits_to_tiles(void) {
  RUN_ORIGINAL_IF_LAG(kDigitsToTiles);
  Cpu6502 *c = &g_nes.cpu;
  c->y = 0;
  do {
    emu_set_flag(kFlagC, 0);
    c->a = emu_adc(g_ram[(uint8_t)(0x75 + c->x)], 0x80);
    name_buffer[c->y] = c->a;
    c->y++;
    c->x++;
    emu_cmp(c->x, 0x05);
  } while(c->x != 0x05);
  emu_set_a(0xFE);
  name_buffer[c->y] = 0xFE;
  g_ram[0x84] = c->y;
}

// print_decimal_digits (7:FFD5): name_buffer from X, up to $FE, at $0E, $0F.
void print_decimal_digits(void) {
  RUN_ORIGINAL_IF_LAG(kPrintDecimalDigits);
  Cpu6502 *c = &g_nes.cpu;
  scene_index = g_ram[0x0E];
  emu_set_a(g_ram[0x0F]);
  g_ram[0x76] = c->a;
  for(;;) {
    emu_set_a(name_buffer[c->x]);
    emu_cmp(c->a, 0xFE);
    if(c->a == 0xFE) {
      return;
    }
    g_ram[0x77] = c->a;
    layout_draw_tile();
    RESUME_ORIGINAL_IF_LAG(kPrintDecimalDigits, 0xFFE9, 0);
    scene_index++;
    c->x++;
  }
}

// ---- Metasprites ---------------------------------------------------------------------------

// draw_metasprite (7:FE5D): the sprites at $1E/$1F (a byte with columns and rows, then tile and
// attributes per sprite) at x = $20, y = $21 - 1, into oam_buffer after the ones there. Keeps X and
// Y; returns A = X.
void draw_metasprite(void) {
  RUN_ORIGINAL_IF_LAG(kDrawMetasprite);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->x);
  emu_push(c->x);
  emu_set_a(c->y);
  emu_push(c->y);
  g_ram[0x21]--;
  c->x = asl_a(asl_a(sprite0_reserved));
  c->y = 0;
  g_ram[0x57] = emu_read_ind_y(0x1E, 0) & 0x0F;
  data_ptr[0] = data_ptr[1] = emu_read_ind_y(0x1E, 0) >> 4;
  g_ram[0x59] = g_ram[0x21];
  g_ram[0x58] = g_ram[0x20];
  c->y++;
  do {
    do {
      emu_count_work(64);  // about what the original's loop takes, in cycles
      oam_buffer[c->x] = g_ram[0x59];
      oam_buffer[3 + c->x] = g_ram[0x58];
      oam_buffer[1 + c->x] = emu_read_ind_y(0x1E, c->y);
      c->y++;
      oam_buffer[2 + c->x] = emu_read_ind_y(0x1E, c->y) | g_ram[0x22];
      c->y++;
      c->x += 4;
      sprite0_reserved++;
      emu_set_flag(kFlagC, 0);
      g_ram[0x58] = emu_adc(g_ram[0x58], 0x08);
      data_ptr[0]--;
    } while(data_ptr[0]);
    g_ram[0x58] = g_ram[0x20];
    emu_set_flag(kFlagC, 0);
    g_ram[0x59] = emu_adc(g_ram[0x59], 0x08);
    data_ptr[0] = data_ptr[1];
    g_ram[0x57]--;
  } while(g_ram[0x57]);
  g_ram[0x21]++;
  c->y = emu_pull();
  c->x = emu_pull();
  emu_set_a(c->x);
}
