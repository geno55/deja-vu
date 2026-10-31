// The text decoder and printer. A message is a bit stream of
// Golomb-Rice codes (docs/text.md), printed a tile at a time into the window at the bottom of
// the screen, which lives in the second nametable. text_window keeps what the window shows, as
// 8 rows of 26 tiles (blank rows between the text), so the text can scroll.
//
// The scratch bytes, as the originals use them: $00/$01 the next byte of the message, $0E/$0F
// a tile's column and row on screen, $1C/$1D the VRAM address, $1E/$1F a block for the VRAM
// queue, $20/$21/$22 a sprite's x, y and attributes, $70/$71 the message table, $72/$73 the
// entry in it, $74 its carry, $84 the table's bank. $85/$86 the item or number being printed,
// $87/$88 a name's address or the number's, $89-$8D the digits (only $8B-$8D are shown).
//
// Most of these wait for the NMI somewhere, so after such a call they check whether to go on as
// original code (RESUME_ORIGINAL_IF_LAG); the addresses are the original's after that JSR.
#include "funcs.h"
#include "variables.h"

enum {
  kTextCodeTable = 0xE4FC,   // [64] bit-stream index -> character code
  kDigitPlaces = 0xE14D,     // [5] 16-bit powers of ten, from 10000 down
  kTextRowOffsets = 0xE2C5,  // [5] text_window offset of each text row
  kRuleRowBlock = 0xC9,      // $E3C9: a block one row of $DE (fill)
  kTextRowsBlock = 0xCC,     // $E3CC: a block two rows of $7E (fill)
  kWindowBlocksHi = 0xE3,
  kArrowSpriteLo = 0xF3,     // $E4F3: the arrow's metasprite
  kArrowSpriteHi = 0xE4,
};

// ---- Messages ------------------------------------------------------------------------------

// print_message_from (7:C4BC): message_id's entry of the table at $70/$71 in bank $84. Keeps X
// and Y; returns A = X.
void print_message_from(void) {
  RUN_ORIGINAL_IF_LAG(kPrintMessageFrom);
  Cpu6502 *c = &g_nes.cpu;
  emu_push(c->x);
  emu_push(c->y);
  push_and_switch_bank(g_ram[0x84]);
  g_ram[0x74] = message_id >> 7;
  emu_set_flag(kFlagC, 0);
  g_ram[0x72] = emu_adc((uint8_t)(message_id << 1), g_ram[0x70]);
  g_ram[0x73] = emu_adc(g_ram[0x71], g_ram[0x74]);
  c->y = 0;
  g_ram[0x00] = emu_read_ind_y(0x72, 0);
  c->y = 1;
  g_ram[0x01] = emu_read_ind_y(0x72, 1);
  emu_set_a(g_ram[0x01]);
  print_message();
  RESUME_ORIGINAL_IF_LAG(kPrintMessageFrom, 0xC4E7, 2);
  pop_bank();
  c->y = emu_pull();
  c->x = emu_pull();
  emu_set_a(c->x);
}

// The end of print_message_5, which print_message_2_or_5 jumps to (7:C4B0).
static void print_from_table_5(void) {
  g_ram[0x84] = 0x05;
  g_ram[0x70] = 0x00;
  g_ram[0x71] = 0x9A;
  emu_set_a(0x9A);
  print_message_from();
}

// print_message_5 (7:C4AE): message `id` of the table at 5:9A00.
void print_message_5(uint8_t id) {
  emu_set_a(id);
  RUN_ORIGINAL_IF_LAG(kPrintMessage5);
  message_id = id;
  print_from_table_5();
}

// print_message_4 (7:C4EF): message `id` of the table at 4:8000.
void print_message_4(uint8_t id) {
  emu_set_a(id);
  RUN_ORIGINAL_IF_LAG(kPrintMessage4);
  message_id = id;
  g_ram[0x84] = 0x04;
  g_ram[0x70] = 0x00;
  g_ram[0x71] = 0x80;
  emu_set_a(0x80);
  print_message_from();
  RESUME_ORIGINAL_IF_LAG(kPrintMessage4, 0xC500, 0);
}

// print_message_2_or_5 (7:C501): A is the id, and N the flag of the instruction that loaded it,
// which is what the original tests: entry id & $7F of the table at 2:A500 with N set, otherwise
// entry id | $80 of the one at 5:9A00.
void print_message_2_or_5(void) {
  if(emu_lag_in(kPrintMessage2Or5) || emu_lag_in(kPrintMessage5)) {
    emu_call_original(kPrintMessage2Or5);
    return;
  }
  uint8_t id = g_nes.cpu.a;
  if(!(g_nes.cpu.p & kFlagN)) {
    message_id = id | 0x80;
    print_from_table_5();
    return;
  }
  message_id = id & 0x7F;
  g_ram[0x84] = 0x02;
  g_ram[0x70] = 0x00;
  g_ram[0x71] = 0xA5;
  emu_set_a(0xA5);
  print_message_from();
  RESUME_ORIGINAL_IF_LAG(kPrintMessage2Or5, 0xC518, 0);
}

// print_message (7:DF74): prints the message at $00/$01, a code at a time.
void print_message(void) {
  RUN_ORIGINAL_IF_LAG(kPrintMessage);
  Cpu6502 *c = &g_nes.cpu;
  far_call_8020(0x65);  // draw_object_sprites
  wait_vblank_and_hide_sprites();
  RESUME_ORIGINAL_IF_LAG(kPrintMessage, 0xDF7C, 0);
  clear_text_window();
  RESUME_ORIGINAL_IF_LAG(kPrintMessage, 0xDF7F, 0);
  text_bits_used = 0;
  text_lines = 0;
  emu_set_a(0);
  for(;;) {
    text_read_code();
    uint8_t code = c->a;
    text_char = code;
    emu_cmp(code, 0x1F);
    if(code == 0x1F) {  // wait, then the end
      text_wait_page();
      RESUME_ORIGINAL_IF_LAG(kPrintMessage, 0xDFC2, 0);
      break;
    }
    emu_cmp(code, 0x1E);
    if(code == 0x1E) {  // the end
      break;
    }
    emu_cmp(code, 0x20);
    if(!(c->p & kFlagN)) {
      print_control(code & 0x1F);
      RESUME_ORIGINAL_IF_LAG(kPrintMessage, 0xDFDB, 0);
    } else if(code == 0x1B) {  // wait
      text_wait_page();
      RESUME_ORIGINAL_IF_LAG(kPrintMessage, 0xDFAD, 0);
    } else if(code == 0x1C) {  // wait, then clear the window
      text_wait_page();
      RESUME_ORIGINAL_IF_LAG(kPrintMessage, 0xDFB3, 0);
      print_clear();
      RESUME_ORIGINAL_IF_LAG(kPrintMessage, 0xDFB6, 0);
    } else if(code == 0x1D) {  // new line
      text_newline();
      RESUME_ORIGINAL_IF_LAG(kPrintMessage, 0xDFBC, 0);
    } else {
      print_char();
      RESUME_ORIGINAL_IF_LAG(kPrintMessage, 0xDFA7, 0);
    }
  }
  text_window_drawn = 0;
  text_keep_open--;
  emu_set_a(text_keep_open);
  if(text_keep_open & 0x80) {
    split_nametable = 0;
    text_keep_open = 0;
    emu_set_a(0);
  }
}

// print_control (7:DFDE): code = the character code & $1F, for codes from $20.
void print_control(uint8_t code) {
  emu_set_a(code);
  RUN_ORIGINAL_IF_LAG(kPrintControl);
  emu_cmp(code, 0x1C);
  if(code == 0x1C) {
    print_item_name_arg();
    return;
  }
  emu_cmp(code, 0x1D);
  if(code == 0x1D) {
    print_word_arg();
    return;
  }
  emu_cmp(code, 0x1F);
  if(code == 0x1F) {
    print_item_name_at();
    return;
  }
  emu_cmp(code, 0x1E);
  if(code == 0x1E) {
    print_number_at();
    return;
  }
  emu_cmp(code, 0x1B);
  if(code == 0x1B) {
    print_clear();
    return;
  }
  print_char();
  RESUME_ORIGINAL_IF_LAG(kPrintControl, 0xE004, 0);
}

// ---- The bit stream ------------------------------------------------------------------------

// text_read_code (7:DF00): the next character code, in A: 0 bits counted up to a 1 bit, 3 more
// bits, and the two together index text_code_table. Keeps Y. X comes back as the A it came in
// with: the original starts TAX, where it must have meant TXA.
void text_read_code(void) {
  RUN_ORIGINAL_IF_LAG(kTextReadCode);
  Cpu6502 *c = &g_nes.cpu;
  uint8_t a = c->a, y = c->y;
  c->x = 0;
  text_value = 0;
  emu_set_a(0);
  for(;;) {
    text_read_bit();
    emu_set_a(text_value);
    if(text_value) {
      break;
    }
    c->x++;
  }
  emu_set_a(0);
  text_value = 0;
  for(int i = 0; i < 3; i++) {
    text_read_bit();
  }
  uint8_t index = (uint8_t)(c->x << 3 | text_value);
  emu_set_flag(kFlagC, c->x & 0x20);  // from the last ASL
  c->x = index;
  text_value = cpu_bus_read((uint16_t)(kTextCodeTable + index));
  c->y = y;
  c->x = a;
  emu_set_a(text_value);
}

// text_read_byte (7:DF34): the next 8 bits, in A. Keeps Y; X comes back as text_read_code's does.
void text_read_byte(void) {
  RUN_ORIGINAL_IF_LAG(kTextReadByte);
  Cpu6502 *c = &g_nes.cpu;
  uint8_t a = c->a, y = c->y;
  c->x = 0;
  text_value = 0;
  emu_set_a(0);
  do {
    text_read_bit();
    c->x++;
    emu_cmp(c->x, 0x08);
  } while(c->x != 0x08);
  c->y = y;
  c->x = a;
  emu_set_a(text_value);
}

// text_read_bit (7:DF4C): the next bit, into the bottom of text_value. Y = 0 if it read a byte.
void text_read_bit(void) {
  RUN_ORIGINAL_IF_LAG(kTextReadBit);
  Cpu6502 *c = &g_nes.cpu;
  if(!text_bits_used) {
    c->y = 0;
    text_bit_buffer = emu_read_ind_y(0x00, 0);
    add16(g_ram + 0x00, 0x01);
  }
  uint8_t bit = text_bit_buffer >> 7;
  text_bit_buffer = (uint8_t)(text_bit_buffer << 1);
  text_value = (uint8_t)(text_value << 1 | bit);
  text_bits_used++;
  c->a = text_bits_used;
  emu_cmp(text_bits_used, 0x08);
  if(text_bits_used == 0x08) {
    text_bits_used = 0;
    emu_set_a(0);
  }
}

// ---- Names and numbers ---------------------------------------------------------------------

// The end of print_item_name_arg (7:E02F) and print_word_arg, whose code is the same: name $85
// of the table at $hh00 in bank 2, eight bytes each.
static void print_table_name(uint8_t table_hi, uint16_t routine, uint16_t resume) {
  push_and_switch_bank(0x02);
  uint8_t index = g_ram[0x85];
  g_ram[0x86] = index >> 5;
  g_ram[0x87] = (uint8_t)(index << 3);
  emu_set_flag(kFlagC, 0);
  g_ram[0x88] = emu_adc(g_ram[0x86], table_hi);
  g_nes.cpu.a = g_ram[0x88];
  print_name();
  RESUME_ORIGINAL_IF_LAG(routine, resume, 0);
  pop_bank();
}

// print_item_name_at (7:E005): the name of the item whose number is at the address that follows
// in the message, byte-aligned.
void print_item_name_at(void) {
  if(emu_lag_in(kPrintItemNameAt) || emu_lag_in(kPrintItemNameArg)) {
    emu_call_original(kPrintItemNameAt);
    return;
  }
  Cpu6502 *c = &g_nes.cpu;
  c->y = 0;
  g_ram[0x85] = emu_read_ind_y(0x00, 0);
  c->y = 1;
  g_ram[0x86] = emu_read_ind_y(0x00, 1);
  text_bits_used = 0;
  add16(g_ram + 0x00, 0x02);
  c->y = 0;
  g_ram[0x85] = emu_read_ind_y(0x85, 0);
  print_table_name(0x8D, kPrintItemNameArg, 0xE051);
}

// print_item_name_arg (7:E02A): the name of the item numbered by the next 8 bits.
void print_item_name_arg(void) {
  RUN_ORIGINAL_IF_LAG(kPrintItemNameArg);
  text_read_byte();
  g_ram[0x85] = g_nes.cpu.a;
  print_table_name(0x8D, kPrintItemNameArg, 0xE051);
}

// print_word_arg (7:E055): the word numbered by the next 8 bits.
void print_word_arg(void) {
  RUN_ORIGINAL_IF_LAG(kPrintWordArg);
  text_read_byte();
  g_ram[0x85] = g_nes.cpu.a;
  print_table_name(0x95, kPrintWordArg, 0xE07C);
}

// print_name (7:E080): up to 8 tiles from $87/$88; $FF ends it early. Y = 0.
void print_name(void) {
  RUN_ORIGINAL_IF_LAG(kPrintName);
  Cpu6502 *c = &g_nes.cpu;
  c->x = 0;
  do {
    c->y = 0;
    text_tile = emu_read_ind_y(0x87, 0);
    add16(g_ram + 0x87, 0x01);
    c->a = text_tile;
    emu_cmp(text_tile, 0xFF);
    if(text_tile == 0xFF) {
      return;
    }
    print_tile();
    RESUME_ORIGINAL_IF_LAG(kPrintName, 0xE0A0, 0);
    c->x++;
    emu_cmp(c->x, 0x08);
  } while(c->x != 0x08);
}

// print_number_at (7:E0A6): the number at the address that follows in the message, byte-aligned.
// Up to three digits, no leading zeros.
void print_number_at(void) {
  RUN_ORIGINAL_IF_LAG(kPrintNumberAt);
  read_number_arg();
  number_to_digits();
  emu_set_a(g_ram[0x8B]);
  print_digit_skip_zero();
  RESUME_ORIGINAL_IF_LAG(kPrintNumberAt, 0xE0B1, 0);
  emu_set_a(g_ram[0x8C]);
  print_digit_skip_zero();
  RESUME_ORIGINAL_IF_LAG(kPrintNumberAt, 0xE0B6, 0);
  print_digit(g_ram[0x8D]);
  RESUME_ORIGINAL_IF_LAG(kPrintNumberAt, 0xE0BB, 0);
}

// print_digit_skip_zero (7:E0F0), which runs into print_digit: nothing for a leading zero. A is
// the digit, and Z the flag of the instruction that loaded it, which is what the original tests.
void print_digit_skip_zero(void) {
  RUN_ORIGINAL_IF_LAG(kPrintDigitSkipZero);
  text_char = g_nes.cpu.a;
  if(g_nes.cpu.p & kFlagZ) {
    emu_set_a(printed_digit);
    if(!printed_digit) {
      return;
    }
  }
  print_digit(text_char);
}

// print_digit (7:E0FD)
void print_digit(uint8_t digit) {
  emu_set_a(digit);
  RUN_ORIGINAL_IF_LAG(kPrintDigit);
  text_char = digit | 0x30;
  text_digit_mark = 0xFF;
  printed_digit = 0xFF;
  emu_set_a(0xFF);
  print_char();
  RESUME_ORIGINAL_IF_LAG(kPrintDigit, 0xE10C, 0);
}

// number_to_digits (7:E10D): the number's hundreds, tens and ones into $8B, $8C and $8D. It
// starts by clearing the high byte, so a number is only ever its low byte. Leaves in $85-$88
// what the last failed subtraction left.
void number_to_digits(void) {
  RUN_ORIGINAL_IF_LAG(kNumberToDigits);
  Cpu6502 *c = &g_nes.cpu;
  g_ram[0x86] = 0;
  for(c->y = 2; c->y != 4; c->y++) {
    c->x = (uint8_t)(c->y << 1);
    uint8_t place_lo = cpu_bus_read((uint16_t)(kDigitPlaces + c->x));
    uint8_t place_hi = cpu_bus_read((uint16_t)(kDigitPlaces + 1 + c->x));
    for(;;) {
      g_ram[0x87] = g_ram[0x85];
      emu_set_flag(kFlagC, 1);
      g_ram[0x85] = emu_sbc(g_ram[0x85], place_lo);
      g_ram[0x88] = g_ram[0x86];
      g_ram[0x86] = emu_sbc(g_ram[0x86], place_hi);
      if(!(c->p & kFlagC)) {
        break;
      }
      g_ram[0x89 + c->y]++;
    }
    g_ram[0x85] = g_ram[0x87];
    g_ram[0x86] = g_ram[0x88];
  }
  g_ram[0x8D] = g_ram[0x85];
  printed_digit = 0;
  emu_set_a(0);
  emu_set_flag(kFlagC, 1);  // from the last CPY
}

// read_number_arg (7:E157): the number at the address that follows in the message into $85/$86,
// which then go past it, byte-aligned; the digits cleared.
void read_number_arg(void) {
  RUN_ORIGINAL_IF_LAG(kReadNumberArg);
  Cpu6502 *c = &g_nes.cpu;
  c->y = 0;
  g_ram[0x87] = emu_read_ind_y(0x00, 0);
  c->y = 1;
  g_ram[0x88] = emu_read_ind_y(0x00, 1);
  c->a = g_ram[0x88];
  skip_inline_word();
  c->y = 0;
  g_ram[0x85] = emu_read_ind_y(0x87, 0);
  c->y = 1;
  g_ram[0x86] = emu_read_ind_y(0x87, 1);
  text_bits_used = 0;
  for(int i = 0; i < 5; i++) {
    g_ram[0x89 + i] = 0;
  }
  c->y = 5;
  c->a = 0;
  emu_cmp(5, 5);
}

// skip_inline_word (7:E17F): $00/$01 past two bytes. It jumps into the end of
// unreferenced_E187, a routine nothing calls that does the same for one byte.
void skip_inline_word(void) {
  RUN_ORIGINAL_IF_LAG(kSkipInlineWord);
  add16(g_ram + 0x00, 0x02);
}

// ---- Pages and lines -----------------------------------------------------------------------

// text_wait_page (7:E0BC): blank lines down to the end of the page, then waits for the player.
void text_wait_page(void) {
  RUN_ORIGINAL_IF_LAG(kTextWaitPage);
  for(;;) {
    g_nes.cpu.a = text_lines;
    emu_cmp(text_lines, 0x03);
    if(text_lines == 0x03) {
      break;
    }
    text_newline();
    RESUME_ORIGINAL_IF_LAG(kTextWaitPage, 0xE0C6, 0);
  }
  emu_set_a(text_fast);
  if(text_fast) {
    request_vram_queue();
  }
  text_fast = 0;
  emu_set_a(0);
  wait_page_input();
  RESUME_ORIGINAL_IF_LAG(kTextWaitPage, 0xE0D7, 0);
}

// text_newline (7:E0D8): counts the line, to 3 at most, and moves down.
void text_newline(void) {
  RUN_ORIGINAL_IF_LAG(kTextNewline);
  Cpu6502 *c = &g_nes.cpu;
  c->a = text_lines;
  emu_cmp(text_lines, 0x03);
  if(text_lines != 0x03) {
    emu_set_flag(kFlagC, 0);
    text_lines = emu_adc(text_lines, 0x01);
    c->a = text_lines;
  }
  text_line_feed();
  RESUME_ORIGINAL_IF_LAG(kTextNewline, 0xE0EB, 0);
}

// print_clear (7:E0EC)
void print_clear(void) {
  RUN_ORIGINAL_IF_LAG(kPrintClear);
  clear_text_window();
  RESUME_ORIGINAL_IF_LAG(kPrintClear, 0xE0EF, 0);
}

// clear_text_window (7:E195): the window blanked at its fixed place, and the split turned to it.
void clear_text_window(void) {
  RUN_ORIGINAL_IF_LAG(kClearTextWindow);
  text_window_x = 0x02;
  text_window_y = 0x14;
  emu_set_a(0x14);
  blank_text_window();
  RESUME_ORIGINAL_IF_LAG(kClearTextWindow, 0xE1A2, 0);
  text_lines = 0;
  text_wait_enabled = 0;
  text_fast = 0;
  text_no_wrap = 0;
  split_nametable = 0x01;
  emu_set_a(0x01);
}

// blank_text_window (7:E1B2): draws the sides the first time, then blanks the rows; the cursor
// goes to the last text row.
void blank_text_window(void) {
  RUN_ORIGINAL_IF_LAG(kBlankTextWindow);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(text_window_drawn);
  if(!text_window_drawn) {
    draw_text_window_sides();
    RESUME_ORIGINAL_IF_LAG(kBlankTextWindow, 0xE1B9, 0);
    text_window_drawn = 0xFF;
  }
  text_column = 0;
  text_row = 0x03;
  for(int i = 0; i < 0x1A; i++) {
    text_window[i] = text_window[i + 52] = text_window[i + 104] = text_window[i + 156] = 0xDE;
    text_window[i + 26] = text_window[i + 78] = text_window[i + 130] = text_window[i + 182] = 0x7E;
  }
  c->x = 0x1A;
  c->a = 0x7E;
  emu_cmp(0x1A, 0x1A);
  upload_text_window();
  RESUME_ORIGINAL_IF_LAG(kBlankTextWindow, 0xE1EB, 0);
  blank_window_text_rows();
  RESUME_ORIGINAL_IF_LAG(kBlankTextWindow, 0xE1EE, 0);
  draw_window_rule_row();
  RESUME_ORIGINAL_IF_LAG(kBlankTextWindow, 0xE1F1, 0);
}

// ---- One tile ------------------------------------------------------------------------------

// print_tile (7:E1F2): put_tile, keeping X and Y. Returns A = Y.
void print_tile(void) {
  RUN_ORIGINAL_IF_LAG(kPrintTile);
  Cpu6502 *c = &g_nes.cpu;
  emu_push(c->y);
  emu_push(c->x);
  emu_set_a(c->x);
  put_tile();
  RESUME_ORIGINAL_IF_LAG(kPrintTile, 0xE1F9, 2);
  c->x = emu_pull();
  c->y = emu_pull();
  emu_set_a(c->y);
}

// print_char (7:E1FE): the tile for character code text_char, keeping X and Y. Returns A = Y.
void print_char(void) {
  RUN_ORIGINAL_IF_LAG(kPrintChar);
  Cpu6502 *c = &g_nes.cpu;
  emu_push(c->y);
  emu_push(c->x);
  c->y = text_char;
  text_tile = cpu_bus_read((uint16_t)(kTextTileTable + c->y));
  emu_set_a(text_tile);
  put_tile();
  RESUME_ORIGINAL_IF_LAG(kPrintChar, 0xE20E, 2);
  c->x = emu_pull();
  c->y = emu_pull();
  emu_set_a(c->y);
}

// put_tile (7:E253): text_tile at the cursor. A pressed while printing finishes the page at
// once (text_fast); otherwise each tile goes up with the arrow beside it, a click, and a pause
// of text_delay frames.
void put_tile(void) {
  RUN_ORIGINAL_IF_LAG(kPutTile);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(text_fast);
  if(!text_fast) {
    read_joypads();
    emu_set_a(joy_pressed[0] & 0x80);
    if(joy_pressed[0] & 0x80) {
      text_fast = 0xFF;
      far_call_8020(0x65);  // draw_object_sprites
      wait_vblank_and_hide_sprites();
      RESUME_ORIGINAL_IF_LAG(kPutTile, 0xE26C, 0);
    }
  }
  // Its place on screen: inside the window's left side, on the lower of the text row's two
  // rows.
  g_ram[0x0E] = text_window_x;
  g_ram[0x0F] = text_window_y;
  g_ram[0x0E]++;
  emu_set_flag(kFlagC, 0);
  g_ram[0x0E] = emu_adc(text_column, g_ram[0x0E]);
  emu_set_flag(kFlagC, 0);
  c->a = emu_adc((uint8_t)(text_row << 1), g_ram[0x0F]);
  g_ram[0x0F] = (uint8_t)(c->a + 1);
  text_cursor_vram_address();
  text_tile_block[0] = 0x01;
  text_tile_block[1] = 0x01;
  g_ram[0x1E] = 0x1E;  // text_tile_block
  g_ram[0x1F] = 0x06;
  emu_set_a(0x06);
  queue_vram_block();
  RESUME_ORIGINAL_IF_LAG(kPutTile, 0xE2A1, 0);
  emu_set_a(text_fast);
  if(!text_fast) {
    request_vram_queue();
    show_more_arrow();
    far_call_8020(0x65);  // draw_object_sprites
    wait_vblank_and_hide_sprites();
    RESUME_ORIGINAL_IF_LAG(kPutTile, 0xE2B3, 0);
    play_sound(0x31);
    c->x = text_delay;
    for(;;) {
      emu_cmp(c->x, 0x00);
      if(!c->x) {
        break;
      }
      wait_vblank();
      RESUME_ORIGINAL_IF_LAG(kPutTile, 0xE2C1, 0);
      c->x--;
    }
  }
  // text_store_tile (7:E2CA): also in text_window, for scrolling; the cursor moves on.
  c->x = text_row;
  uint8_t offset = cpu_bus_read((uint16_t)(kTextRowOffsets + c->x));
  emu_set_flag(kFlagC, 0);
  c->x = emu_adc(offset, text_column);
  text_window[c->x] = 0xDE;
  text_window[0x1A + c->x] = text_tile;
  text_column++;
  c->a = text_column;
  emu_cmp(text_column, 0x1A);
  if(text_column != 0x1A) {
    return;
  }
  emu_set_a(text_no_wrap);
  if(text_no_wrap) {
    text_column = 0x19;
    emu_set_a(0x19);
    return;
  }
  text_newline();
  RESUME_ORIGINAL_IF_LAG(kPutTile, 0xE2F2, 0);
}

// text_line_feed (7:E2F9): the cursor to the next row, scrolling at the bottom. Without
// text_fast, the arrow runs along the line first.
void text_line_feed(void) {
  RUN_ORIGINAL_IF_LAG(kTextLineFeed);
  text_row++;
  g_nes.cpu.a = text_row;
  emu_cmp(text_row, 0x04);
  if(text_row == 0x04) {
    text_scroll();
    RESUME_ORIGINAL_IF_LAG(kTextLineFeed, 0xE306, 0);
    text_row--;
  }
  emu_set_a(text_fast);
  if(!text_fast) {
    wait_for_more_button();
    RESUME_ORIGINAL_IF_LAG(kTextLineFeed, 0xE310, 0);
  }
  text_column = 0;
  emu_set_a(0);
}

// text_scroll (7:E316): text_window up two rows, one at a time; the rows under the text are
// redrawn after each.
void text_scroll(void) {
  RUN_ORIGINAL_IF_LAG(kTextScroll);
  text_scroll_rows(0xDE);
  RESUME_ORIGINAL_IF_LAG(kTextScroll, 0xE31B, 0);
  blank_window_text_rows();
  RESUME_ORIGINAL_IF_LAG(kTextScroll, 0xE31E, 0);
  text_scroll_rows(0x7E);
  RESUME_ORIGINAL_IF_LAG(kTextScroll, 0xE323, 0);
  draw_window_rule_row();
  RESUME_ORIGINAL_IF_LAG(kTextScroll, 0xE326, 0);
}

// text_scroll_rows (7:E327): text_window up one row, with a new last row of `fill`.
void text_scroll_rows(uint8_t fill) {
  emu_set_a(fill);
  RUN_ORIGINAL_IF_LAG(kTextScrollRows);
  Cpu6502 *c = &g_nes.cpu;
  for(int i = 0; i < 0xB6; i++) {
    text_window[i] = text_window[i + 0x1A];
  }
  for(int i = 0xB6; i < 0xD0; i++) {
    text_window[i] = fill;
  }
  c->x = 0xD0;
  c->a = fill;
  emu_cmp(0xD0, 0xD0);
  upload_text_window();
  RESUME_ORIGINAL_IF_LAG(kTextScrollRows, 0xE341, 0);
  request_vram_queue();
}

// text_cursor_vram_address (7:E345): $1C/$1D = the address of column $0E, row $0F in the second
// nametable. The carry is from the last ROR, of a byte that started at 0.
void text_cursor_vram_address(void) {
  RUN_ORIGINAL_IF_LAG(kTextCursorVramAddress);
  uint8_t row = g_ram[0x0F];
  g_ram[0x1D] = row >> 3 | 0x24;
  g_ram[0x1C] = (uint8_t)(row << 5) | g_ram[0x0E];
  emu_set_a(g_ram[0x1C]);
  emu_set_flag(kFlagC, 0);
}

// ---- The window on screen ------------------------------------------------------------------

// upload_text_window (7:E35F): all of text_window into the VRAM queue.
void upload_text_window(void) {
  RUN_ORIGINAL_IF_LAG(kUploadTextWindow);
  g_ram[0x0E] = text_window_x;
  g_ram[0x0F] = text_window_y;
  g_nes.cpu.a = text_window_y;
  g_ram[0x0E]++;
  text_cursor_vram_address();
  text_window_block[0] = 0x1A;
  text_window_block[1] = 0x08;
  g_ram[0x1E] = 0x26;  // text_window_block
  g_ram[0x1F] = 0x06;
  emu_set_a(0x06);
  queue_vram_block();
  RESUME_ORIGINAL_IF_LAG(kUploadTextWindow, 0xE383, 0);
  request_vram_queue();
}

// The start of draw_window_rule_row and blank_window_text_rows: a fill of the block at $E3xx
// below the window's text, at the window's left side.
static void fill_below_text(uint8_t block_lo) {
  g_ram[0x0E] = text_window_x;
  g_ram[0x0E]++;
  emu_set_flag(kFlagC, 0);
  g_ram[0x0F] = emu_adc(text_window_y, 0x08);
  g_nes.cpu.a = g_ram[0x0F];
  text_cursor_vram_address();
  g_ram[0x1E] = block_lo;
  g_ram[0x1F] = kWindowBlocksHi;
  emu_set_a(kWindowBlocksHi);
  queue_vram_fill();
}

// draw_window_rule_row (7:E387): one row of $DE under the text.
void draw_window_rule_row(void) {
  RUN_ORIGINAL_IF_LAG(kDrawWindowRuleRow);
  fill_below_text(kRuleRowBlock);
  RESUME_ORIGINAL_IF_LAG(kDrawWindowRuleRow, 0xE3A4, 0);
  request_vram_queue();
}

// blank_window_text_rows (7:E3A8): two rows of $7E under the text.
void blank_window_text_rows(void) {
  RUN_ORIGINAL_IF_LAG(kBlankWindowTextRows);
  fill_below_text(kTextRowsBlock);
  RESUME_ORIGINAL_IF_LAG(kBlankWindowTextRows, 0xE3C5, 0);
  request_vram_queue();
}

// draw_text_window_sides (7:E3CF)
void draw_text_window_sides(void) {
  RUN_ORIGINAL_IF_LAG(kDrawTextWindowSides);
  request_vram_queue();
  g_ram[0x0E] = text_window_x;
  g_ram[0x0F] = text_window_y;
  emu_set_a(text_window_y);
  draw_window_side_column();
  RESUME_ORIGINAL_IF_LAG(kDrawTextWindowSides, 0xE3DF, 0);
  emu_set_flag(kFlagC, 0);
  g_ram[0x0E] = emu_adc(text_window_x, 0x1B);
  g_ram[0x0F] = text_window_y;
  emu_set_a(text_window_y);
  draw_window_side_column();
  RESUME_ORIGINAL_IF_LAG(kDrawTextWindowSides, 0xE3EF, 0);
}

// draw_window_side_column (7:E3F0): the column of 10 tiles at $E402, at column $0E, row $0F.
void draw_window_side_column(void) {
  RUN_ORIGINAL_IF_LAG(kDrawWindowSideColumn);
  text_cursor_vram_address();
  g_ram[0x1E] = 0x02;
  g_ram[0x1F] = 0xE4;
  emu_set_a(0xE4);
  queue_vram_column();
  RESUME_ORIGINAL_IF_LAG(kDrawWindowSideColumn, 0xE3FE, 0);
  flush_vram_queue_down();
  RESUME_ORIGINAL_IF_LAG(kDrawWindowSideColumn, 0xE401, 0);
}

// ---- Waiting for the player ----------------------------------------------------------------

// text_pause (7:E40D): 20 frames, then up to 30 more unless A or B is pressed. Keeps A, X and Y,
// and joy_pressed.
void text_pause(void) {
  RUN_ORIGINAL_IF_LAG(kTextPause);
  Cpu6502 *c = &g_nes.cpu;
  emu_push(c->a);
  emu_push(c->x);
  emu_push(c->y);
  emu_set_a(c->y);
  request_vram_queue();
  c->y = 0;
  do {
    wait_vblank();
    RESUME_ORIGINAL_IF_LAG(kTextPause, 0xE41A, 3);
    c->y++;
    emu_cmp(c->y, 0x14);
  } while(c->y != 0x14);
  emu_set_a(joy_pressed[0]);
  emu_push(joy_pressed[0]);
  for(;;) {
    read_joypads();
    emu_set_a(joy_pressed[0] & 0xC0);
    if(joy_pressed[0] & 0xC0) {
      break;
    }
    wait_vblank();
    RESUME_ORIGINAL_IF_LAG(kTextPause, 0xE42E, 4);
    c->y++;
    emu_cmp(c->y, 0x32);
    if(c->y == 0x32) {
      break;
    }
  }
  joy_pressed[0] = emu_pull();
  c->y = emu_pull();
  c->x = emu_pull();
  emu_set_a(emu_pull());
}

// wait_page_input (7:E43C): the end of a page. With text_wait_enabled, text_delay + 1 pauses;
// otherwise the arrow blinks until A is pressed and let go.
void wait_page_input(void) {
  RUN_ORIGINAL_IF_LAG(kWaitPageInput);
  Cpu6502 *c = &g_nes.cpu;
  text_lines = 0;
  emu_set_a(text_fast);
  if(!text_fast) {
    wait_for_more_button();
    RESUME_ORIGINAL_IF_LAG(kWaitPageInput, 0xE448, 0);
  }
  request_vram_queue();
  emu_set_a(text_wait_enabled);
  if(text_wait_enabled) {
    c->x = text_delay;
    for(;;) {
      text_pause();
      RESUME_ORIGINAL_IF_LAG(kWaitPageInput, 0xE454, 0);
      emu_cmp(c->x, 0x00);
      if(!c->x) {
        return;
      }
      c->x--;
    }
  }
  do {
    blink_more_arrow();
    RESUME_ORIGINAL_IF_LAG(kWaitPageInput, 0xE460, 0);
    read_joypads();
    emu_set_a(joy_pressed[0] & 0x80);
  } while(!(joy_pressed[0] & 0x80));
  do {
    blink_more_arrow();
    RESUME_ORIGINAL_IF_LAG(kWaitPageInput, 0xE46C, 0);
    read_joypads();
    emu_set_a(joy_held[0] & 0x80);
  } while(joy_held[0] & 0x80);
}

// blink_more_arrow (7:E476): a frame, with the arrow on for 16 frames in every 32.
void blink_more_arrow(void) {
  RUN_ORIGINAL_IF_LAG(kBlinkMoreArrow);
  far_call_8020(0x65);  // draw_object_sprites
  emu_set_a(frame_counter & 0x10);
  if(frame_counter & 0x10) {
    g_ram[0x20] = 0x0A;
    emu_set_a(0x0A);
    draw_more_arrow();
  }
  wait_vblank_and_hide_sprites();
  RESUME_ORIGINAL_IF_LAG(kBlinkMoreArrow, 0xE48B, 0);
}

// show_more_arrow (7:E48C): the arrow at the cursor.
void show_more_arrow(void) {
  RUN_ORIGINAL_IF_LAG(kShowMoreArrow);
  text_cursor_x_pixels();
  draw_more_arrow();
}

// wait_for_more_button (7:E493): the arrow slides left from the cursor to x = $10, 3 pixels a
// frame; holding A ends the slide at once. text_column holds the arrow's x meanwhile.
void wait_for_more_button(void) {
  RUN_ORIGINAL_IF_LAG(kWaitForMoreButton);
  text_cursor_x_pixels();
  emu_set_a(g_ram[0x20]);
  text_column = g_ram[0x20];
  for(;;) {
    draw_more_arrow_frame();
    RESUME_ORIGINAL_IF_LAG(kWaitForMoreButton, 0xE49E, 0);
    g_nes.cpu.a = text_column;
    emu_cmp(text_column, 0x11);
    if(text_column < 0x11) {
      break;
    }
    text_column = (uint8_t)(text_column - 3);
    read_joypads();
    emu_set_a(joy_held[0]);
    if(joy_held[0] & 0x80) {
      break;
    }
  }
  text_column = 0x10;
  emu_set_a(0x10);
  draw_more_arrow_frame();
  RESUME_ORIGINAL_IF_LAG(kWaitForMoreButton, 0xE4C0, 0);
}

// draw_more_arrow_frame (7:E4C1): a frame with the arrow at x = text_column.
void draw_more_arrow_frame(void) {
  RUN_ORIGINAL_IF_LAG(kDrawMoreArrowFrame);
  far_call_8020(0x65);  // draw_object_sprites
  g_ram[0x20] = text_column;
  emu_set_a(text_column);
  draw_more_arrow();
  wait_vblank_and_hide_sprites();
  RESUME_ORIGINAL_IF_LAG(kDrawMoreArrowFrame, 0xE4D1, 0);
}

// text_cursor_x_pixels (7:E4D2): $20 = the cursor's x on screen.
void text_cursor_x_pixels(void) {
  RUN_ORIGINAL_IF_LAG(kTextCursorXPixels);
  emu_set_flag(kFlagC, 0);
  uint8_t column = emu_adc(text_window_x, text_column);
  g_ram[0x20] = (uint8_t)(column << 3);
  emu_set_a(g_ram[0x20]);
  emu_set_flag(kFlagC, column & 0x20);  // from the last ASL
}

// draw_more_arrow (7:E4DF): the arrow's sprites at x = $20, in the window's last row.
void draw_more_arrow(void) {
  RUN_ORIGINAL_IF_LAG(kDrawMoreArrow);
  g_ram[0x21] = 0xD8;
  g_ram[0x22] = 0x00;
  g_ram[0x1E] = kArrowSpriteLo;
  g_ram[0x1F] = kArrowSpriteHi;
  emu_set_a(kArrowSpriteHi);
  draw_metasprite();
}
