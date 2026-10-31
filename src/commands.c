// The cursor and the commands: reading the controller, moving the
// cursor in the picture, the command area and the inventory, drawing it, and what A and B do
// with it: which area and which command or entry was clicked, and the highlight on the chosen
// command and entry. All in bank 6 (far calls $68 reset_cursor, $69 handle_input, $6A
// draw_cursor), but for test_hotspot and cursor_in_hotspot in bank 7.
//
// What a click leads to is elsewhere: the scripts (scripts.c), the picture's hotspots (rooms.c), and
// the room-specific commands (room_commands.c).
//
// The scratch bytes, as the originals use them: data_ptr ($55) the buttons read this frame (high
// nibble A, B, Select, Start; low nibble the d-pad) and data_ptr+1/$57 the cursor's step in x and
// y, $59/$5A a place in the command grid and $58 the command there, $64 the d-pad held (in the
// high nibble), $65-$68 a rectangle (x1, x2, y1, y2), $1C/$1D a VRAM address and $1E/$1F a block.
//
// get_input spins on read_input until a button is down, with no wait for the NMI, so the
// reference's NMIs can land anywhere in that loop: every routine it runs checks its guard on
// every pass, and jump_table, which C calls without one, gets JUMP_TABLE_OR_ORIGINAL.
#include "funcs.h"
#include "variables.h"

// ---- Reading the controller ----------------------------------------------------------------

// handle_input (6:9585, far call $69): waits for the controller in get_input. Keeps Y; returns
// A = Y.
void handle_input(void) {
  RUN_ORIGINAL_IF_LAG(kHandleInput);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  get_input();
  RESUME_ORIGINAL_IF_LAG(kHandleInput, 0x958A, 1);
  c->y = emu_pull();
  emu_set_a(c->y);
}

// get_input (6:958D): reads the controller until something is down, then moves the cursor
// (d-pad) or handles the buttons.
void get_input(void) {
  Cpu6502 *c = &g_nes.cpu;
  do {
    RUN_ORIGINAL_IF_LAG(kGetInput);  // the loop starts at the routine's start
    read_input();
    emu_set_a(data_ptr[0]);
    // The original's key_repeat counts 9.7 a frame on average while a direction is held (measured on
    // Mesen2), which gives a step every 14 frames there and here.
    if(!c->a) {
      emu_spin(97);
    }
  } while(!c->a);
  emu_set_a(c->a & 0xF0);
  if(!c->a) {
    move_cursor();
    return;
  }
  handle_buttons();
}

// move_cursor (6:959B): moves the cursor in the area cursor_mode says.
void move_cursor(void) {
  RUN_ORIGINAL_IF_LAG(kMoveCursor);
  uint8_t mode = cursor_mode & 0x0F;
  emu_set_a(mode);
  JUMP_TABLE_OR_ORIGINAL(kMoveCursor, 0x959F, 0, mode, 3);
  jump_table(0x95A1, mode);
  switch(mode) {
    case 0: move_cursor_in_scene(); break;
    case 1: move_cursor_in_commands(); break;
    default: move_cursor_in_inventory(); break;
  }
}

// read_input (6:95A8): reads the controllers and sets data_ptr and the cursor's step, with key
// repeat in the command area.
void read_input(void) {
  RUN_ORIGINAL_IF_LAG(kReadInput);
  emu_set_a(0x00);
  data_ptr[0] = 0x00;
  read_joypads();
  uint8_t mode = cursor_mode & 0x0F;
  emu_set_a(mode);
  JUMP_TABLE_OR_ORIGINAL(kReadInput, 0x95B3, 0, mode, 3);
  jump_table(0x95B5, mode);
  if(mode == 1) {
    read_input_with_repeat();
  } else {
    read_input_free();
  }
}

// read_input_free (6:95BC): the d-pad held and the buttons just pressed; the cursor moves a pixel
// a frame.
void read_input_free(void) {
  RUN_ORIGINAL_IF_LAG(kReadInputFree);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(joy_held[0] & 0x0F);
  data_ptr[0] = c->a;
  emu_set_a((joy_pressed[0] & 0xF0) | data_ptr[0]);
  data_ptr[0] = c->a;
  dpad_to_delta();
  reset_key_repeat();
}

// read_input_with_repeat (6:95D1): the buttons just pressed, and the d-pad only when it changes
// or has been held for 128 passes of get_input's loop, counted in key_repeat, about 14 frames; the
// direction held goes to the high nibble of cursor_mode.
void read_input_with_repeat(void) {
  RUN_ORIGINAL_IF_LAG(kReadInputWithRepeat);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(0x00);
  data_ptr[1] = 0x00;
  g_ram[0x57] = 0x00;
  emu_set_a(joy_pressed[0] & 0xF0);
  data_ptr[0] = c->a;
  emu_set_flag(kFlagC, 0);  // the last of four ASLs of a 4-bit value
  emu_set_a((uint8_t)((joy_held[0] & 0x0F) << 4));
  g_ram[0x64] = c->a;
  if(!c->a) {
    reset_key_repeat();
    return;
  }
  emu_set_a(cursor_mode & 0xF0);
  if(c->a) {
    emu_cmp(c->a, g_ram[0x64]);
    if(c->a == g_ram[0x64]) {
      key_repeat++;
      emu_set_a(key_repeat & 0x7F);
      if(c->a) {
        return;
      }
    }
  }
  emu_set_a((joy_held[0] & 0x0F) | data_ptr[0]);
  data_ptr[0] = c->a;
  emu_set_a((cursor_mode & 0x0F) | g_ram[0x64]);
  cursor_mode = c->a;
  dpad_to_delta();
}

// reset_key_repeat (6:960F): no direction held. Returns A = the cursor's area.
void reset_key_repeat(void) {
  RUN_ORIGINAL_IF_LAG(kResetKeyRepeat);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(0x00);
  key_repeat = 0x00;
  emu_set_a(cursor_mode & 0x0F);
  cursor_mode = c->a;
}

// dpad_to_delta (6:961A): the cursor's step in x and y for the d-pad in data_ptr. Y = the d-pad;
// returns A = the step in y.
void dpad_to_delta(void) {
  RUN_ORIGINAL_IF_LAG(kDpadToDelta);
  Cpu6502 *c = &g_nes.cpu;
  c->y = data_ptr[0] & 0x0F;
  data_ptr[1] = cpu_bus_read((uint16_t)(kDpadDx + c->y));
  emu_set_a(cpu_bus_read((uint16_t)(kDpadDy + c->y)));
  g_ram[0x57] = c->a;
}

// ---- Moving the cursor ---------------------------------------------------------------------

// reset_cursor (6:9570, far call $68): the cursor on the command area's first command.
void reset_cursor(void) {
  RUN_ORIGINAL_IF_LAG(kResetCursor);
  cursor_x = 0x1A;
  cursor_y = 0xAA;
  cursor_mode = 0x01;
  command_cursor = 0x07;
  emu_set_a(0x00);
  key_repeat = 0x00;
}

// move_cursor_x (6:971C), move_cursor_y (6:9724): one step. Return the new position in A, with
// the flags of the ADC.
void move_cursor_x(void) {
  RUN_ORIGINAL_IF_LAG(kMoveCursorX);
  emu_set_flag(kFlagC, 0);
  g_nes.cpu.a = emu_adc(cursor_x, data_ptr[1]);
  cursor_x = g_nes.cpu.a;
}

void move_cursor_y(void) {
  RUN_ORIGINAL_IF_LAG(kMoveCursorY);
  emu_set_flag(kFlagC, 0);
  g_nes.cpu.a = emu_adc(cursor_y, g_ram[0x57]);
  cursor_y = g_nes.cpu.a;
}

// The end of move_cursor_in_inventory (6:96D3), which move_cursor_in_scene jumps to too: the
// cursor goes to command A of the command area, which reads the d-pad with key repeat from now
// on, starting with the direction held.
static void enter_command_area(void) {
  TAIL_ORIGINAL_IF_LAG(kMoveCursorInInventory, 0x96D3, 0);
  Cpu6502 *c = &g_nes.cpu;
  command_cursor = c->a;
  emu_set_a(0x01);
  cursor_mode = 0x01;
  emu_set_flag(kFlagC, (joy_held[0] >> 4) & 1);  // from the last of four ASLs
  emu_set_a((uint8_t)(joy_held[0] << 4) | cursor_mode);
  cursor_mode = c->a;
  emu_set_a(0x00);
  key_repeat = 0x00;
  snap_cursor_to_command();
}

// move_cursor_in_scene (6:964A): one step in the picture; past its left, top or bottom edge the
// cursor goes to the nearest command.
void move_cursor_in_scene(void) {
  RUN_ORIGINAL_IF_LAG(kMoveCursorInScene);
  Cpu6502 *c = &g_nes.cpu;
  move_cursor_x();
  emu_set_a(cursor_x);
  if(c->a & 0x80) {  // off the left: the command beside the cursor's row
    emu_set_flag(kFlagC, 1);
    c->a = emu_sbc(cursor_y >> 4, 0x02);
    enter_command_area();
    return;
  }
  emu_cmp(c->a, 0x10);
  if(c->a < 0x10) {
    emu_set_a(0x10);
    cursor_x = 0x10;
  }
  move_cursor_y();
  emu_set_a(cursor_y);
  emu_cmp(c->a, 0x90);
  if(c->a >= 0x90) {  // off the bottom
    emu_set_a(cursor_x);
    emu_cmp(c->a, 0x50);
    emu_set_a(c->a >= 0x50 ? 0x0B : 0x07);
    enter_command_area();
    return;
  }
  emu_cmp(c->a, 0x20);
  if(c->a < 0x20) {  // off the top
    emu_set_a(cursor_x);
    emu_cmp(c->a, 0x50);
    emu_set_a(c->a >= 0x50 ? 0x0E : 0x0A);
    enter_command_area();
  }
}

// The command beside the cursor's row in the inventory, from `base`: (cursor_y - $B0) / 16 + base,
// with the flags of the ADC.
static void inventory_row_command(uint8_t base) {
  Cpu6502 *c = &g_nes.cpu;
  emu_set_flag(kFlagC, 1);
  uint8_t a = (uint8_t)(emu_sbc(cursor_y, 0xB0) >> 4);
  emu_set_flag(kFlagC, 0);
  c->a = emu_adc(a, base);
}

// move_cursor_in_inventory (6:9692): one step in the inventory; past its edges the cursor goes
// to the nearest command.
void move_cursor_in_inventory(void) {
  RUN_ORIGINAL_IF_LAG(kMoveCursorInInventory);
  Cpu6502 *c = &g_nes.cpu;
  move_cursor_x();
  emu_set_a(cursor_x);
  emu_cmp(c->a, 0x80);
  if(c->a < 0x80) {
    inventory_row_command(0x0C);
    enter_command_area();
    return;
  }
  emu_cmp(c->a, 0xB8);
  if(c->a >= 0xB8) {
    inventory_row_command(0x10);
    command_cursor = c->a;
    enter_command_area();
    return;
  }
  move_cursor_y();
  emu_set_a(cursor_y);
  emu_cmp(c->a, 0xB6);
  if(c->a < 0xB6) {
    emu_set_a(0x0F);
    enter_command_area();
    return;
  }
  emu_cmp(c->a, 0xE8);
  if(c->a >= 0xE8) {
    emu_set_a(0x00);
    enter_command_area();
  }
}

// snap_cursor_to_command (6:96E7): the cursor onto command_cursor. Y = the command; returns
// A = cursor_y.
void snap_cursor_to_command(void) {
  RUN_ORIGINAL_IF_LAG(kSnapCursorToCommand);
  Cpu6502 *c = &g_nes.cpu;
  c->y = command_cursor;
  cursor_x = cpu_bus_read((uint16_t)(kCommandCursorXs + c->y));
  emu_set_a(cpu_bus_read((uint16_t)(kCommandCursorYs + c->y)));
  cursor_y = c->a;
}

// move_cursor_in_commands (6:972C): one step in the command grid. A place marked $20 leaves for
// the picture, at a place that depends on the command left; $30 for the inventory.
void move_cursor_in_commands(void) {
  RUN_ORIGINAL_IF_LAG(kMoveCursorInCommands);  // its range takes in the code at 6:9778 too
  Cpu6502 *c = &g_nes.cpu;
  command_grid_position();
  move_in_command_grid();
  command_at_grid();
  emu_set_a(g_ram[0x58]);
  if(!(c->a & 0x80)) {
    emu_cmp(c->a, 0x20);
    if(c->a == 0x20) {
      emu_set_a(cursor_mode & 0xF0);
      cursor_mode = c->a;
      c->y = command_cursor;
      cursor_x = cpu_bus_read((uint16_t)(kPictureCursorXs + c->y));
      emu_set_a(cpu_bus_read((uint16_t)(kPictureCursorYs + c->y)));
      cursor_y = c->a;
      return;
    }
    emu_cmp(c->a, 0x30);
    if(c->a == 0x30) {
      emu_set_a((cursor_mode & 0xF0) | 0x02);
      cursor_mode = c->a;
      cursor_x = 0x98;
      emu_set_a(0xC8);
      cursor_y = 0xC8;
      return;
    }
    command_cursor = c->a;
  }
  snap_cursor_to_command();
}

// command_grid_position (6:9789): $59/$5A = command_cursor's column and row. Y = the command;
// returns A = the row.
void command_grid_position(void) {
  RUN_ORIGINAL_IF_LAG(kCommandGridPosition);
  Cpu6502 *c = &g_nes.cpu;
  c->y = command_cursor;
  g_ram[0x59] = cpu_bus_read((uint16_t)(kCommandGridXs + c->y));
  emu_set_a(cpu_bus_read((uint16_t)(kCommandGridYs + c->y)));
  g_ram[0x5A] = c->a;
}

// move_in_command_grid (6:97BE): one step in the grid. Returns A = the row, with the flags of the
// ADC.
void move_in_command_grid(void) {
  RUN_ORIGINAL_IF_LAG(kMoveInCommandGrid);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_flag(kFlagC, 0);
  g_ram[0x59] = emu_adc(g_ram[0x59], data_ptr[1]);
  emu_set_flag(kFlagC, 0);
  c->a = emu_adc(g_ram[0x5A], g_ram[0x57]);
  g_ram[0x5A] = c->a;
}

// command_at_grid (6:97CD): $58 = the command at column $59, row $5A (7 to a row). Y = the
// place; returns A = the command.
void command_at_grid(void) {
  RUN_ORIGINAL_IF_LAG(kCommandAtGrid);
  Cpu6502 *c = &g_nes.cpu;
  uint8_t row = g_ram[0x5A];
  g_ram[0x5B] = (uint8_t)(row << 1);
  emu_set_flag(kFlagC, 0);
  uint8_t a = emu_adc((uint8_t)(row << 2), g_ram[0x5A]);
  emu_set_flag(kFlagC, 0);
  a = emu_adc(a, g_ram[0x5B]);
  emu_set_flag(kFlagC, 0);
  c->y = emu_adc(a, g_ram[0x59]);
  emu_set_a(cpu_bus_read((uint16_t)(kCommandGrid + c->y)));
  g_ram[0x58] = c->a;
}

// draw_cursor (6:9922, far call $6A): the cursor's metasprite (6:993D), 5 pixels left of
// cursor_x.
void draw_cursor(void) {
  RUN_ORIGINAL_IF_LAG(kDrawCursor);
  emu_set_flag(kFlagC, 1);
  g_ram[0x20] = emu_sbc(cursor_x, 0x05);
  g_ram[0x21] = cursor_y;
  g_ram[0x22] = 0x03;
  g_ram[0x1E] = kCursorSprite & 0xFF;
  g_ram[0x1F] = kCursorSprite >> 8;
  emu_set_a(kCursorSprite >> 8);
  draw_metasprite();
}

// ---- The buttons ---------------------------------------------------------------------------

// handle_buttons (6:983E): A or B, as data_ptr has them.
void handle_buttons(void) {
  RUN_ORIGINAL_IF_LAG(kHandleButtons);
  Cpu6502 *c = &g_nes.cpu;
  uint8_t a = asl_a(data_ptr[0]);
  if(c->p & kFlagC) {
    press_a();
    return;
  }
  asl_a(a);
  if(c->p & kFlagC) {
    press_b();
  }
}

// press_b's end (6:98B0), which press_a jumps to as well: no choice made in a script's wait.
static void clear_switch_key(void) {
  emu_set_a(0xFF);
  switch_key[0] = 0xFF;
  switch_key[1] = 0xFF;
}

// press_b's end from 6:98AA: no target either.
static void clear_target(void) {
  emu_set_a(0xFF);
  script_table = 0xFF;
  object_id = 0xFF;
  clear_switch_key();
}

// press_b's end from 6:98A7: the inventory entry unlit, then no target.
static void clear_entry_and_target(void) {
  clear_entry_highlight();
  RESUME_ORIGINAL_IF_LAG(kPressB, 0x98AA, 0);
  clear_target();
}

// press_b (6:9895): cancels: the choice a script waits for (event flag $80), the command and its
// highlight, the entry's highlight and the target.
void press_b(void) {
  RUN_ORIGINAL_IF_LAG(kPressB);
  Cpu6502 *c = &g_nes.cpu;
  clear_event_flag(0x80);
  emu_set_a(command);
  emu_cmp(c->a, 0x0D);
  if(c->a < 0x0D) {
    unhighlight_command();
    RESUME_ORIGINAL_IF_LAG(kPressB, 0x98A3, 0);
  }
  emu_set_a(0xFF);
  command = 0xFF;
  clear_entry_and_target();
}

// press_a (6:98B7): clicks where the cursor is. If a script waits for a choice (event flag $80)
// and one was made, the script goes on; otherwise, with a command and an object, the command
// runs. Then the selection is cleared, unless a script waits for a choice again.
void press_a(void) {
  RUN_ORIGINAL_IF_LAG(kPressA);
  Cpu6502 *c = &g_nes.cpu;
  play_sound(0x19);
  next_random();
  click_at_cursor();
  RESUME_ORIGINAL_IF_LAG(kPressA, 0x98C2, 0);
  test_event_flag(0x80);
  emu_set_a(script_flags & 0x04);
  if(c->a) {
    emu_set_a(switch_key[0]);
    emu_cmp(c->a, 0xFF);
    if(c->a == 0xFF) {
      return;
    }
    script_resume();
    RESUME_ORIGINAL_IF_LAG(kPressA, 0x98D6, 0);
    emu_set_a(g_ram[0x4B1]);
    inventory_entry = c->a;
    clear_entry_highlight();
    RESUME_ORIGINAL_IF_LAG(kPressA, 0x98DF, 0);
    emu_set_a(g_ram[0x4B3]);
    inventory_entry = c->a;
    clear_entry_highlight();
    RESUME_ORIGINAL_IF_LAG(kPressA, 0x98E8, 0);
    TAIL_ORIGINAL_IF_LAG(kPressB, 0x98AA, 0);
    clear_target();
    return;
  }
  emu_set_a(command);
  emu_cmp(c->a, 0xFF);
  if(c->a == 0xFF) {
    return;
  }
  emu_set_a(object_id);
  emu_cmp(c->a, 0xFF);
  if(c->a == 0xFF) {
    return;
  }
  run_command();
  RESUME_ORIGINAL_IF_LAG(kPressA, 0x98FA, 0);
  test_event_flag(0xBF);
  emu_set_a(script_flags & 0x04);
  if(c->a) {
    emu_set_a(0xFF);
    command = 0xFF;
    clear_event_flag(0xBF);
  }
  test_event_flag(0x80);
  emu_set_a(script_flags & 0x04);
  if(!c->a) {
    TAIL_ORIGINAL_IF_LAG(kPressB, 0x98A7, 0);
    clear_entry_and_target();
    return;
  }
  emu_set_a(inventory_entry);
  g_ram[0xB2] = c->a;
  TAIL_ORIGINAL_IF_LAG(kPressB, 0x98B0, 0);
  clear_switch_key();
}

// ---- Where the cursor clicked --------------------------------------------------------------

// click_at_cursor (6:8D3A): handles a click in the picture, the command area or the inventory.
void click_at_cursor(void) {
  RUN_ORIGINAL_IF_LAG(kClickAtCursor);
  Cpu6502 *c = &g_nes.cpu;
  find_screen_area();
  emu_set_a(screen_area);
  emu_cmp(c->a, 0x03);
  if(c->a >= 0x03) {
    return;
  }
  uint8_t area = c->a;
  JUMP_TABLE_OR_ORIGINAL(kClickAtCursor, 0x8D44, 0, area, 3);
  jump_table(0x8D46, area);
  switch(area) {
    case 0: click_picture(); break;
    case 1: click_command_area(); break;
    default: click_inventory_area(); break;
  }
}

// find_screen_area (6:8D4D): screen_area = the first of the three areas (6:8D79) the cursor is
// in, or 3. Y = it too.
void find_screen_area(void) {
  RUN_ORIGINAL_IF_LAG(kFindScreenArea);
  Cpu6502 *c = &g_nes.cpu;
  c->y = 0;
  hotspot_ptr[0] = kScreenAreaRects & 0xFF;
  emu_set_a(kScreenAreaRects >> 8);
  hotspot_ptr[1] = c->a;
  do {
    test_hotspot();
    emu_set_a(script_flags & 0x01);
    if(c->a) {
      break;
    }
    next_screen_rect();
    c->y++;
    emu_cmp(c->y, 0x03);
  } while(c->y < 0x03);
  screen_area = c->y;
}

// next_screen_rect (6:8D6B): hotspot_ptr to the next 4-byte rectangle.
void next_screen_rect(void) {
  RUN_ORIGINAL_IF_LAG(kNextScreenRect);
  add16(hotspot_ptr, 0x04);
}

// test_hotspot (7:C26F): whether the cursor is in the rectangle at hotspot_ptr (bit 0 of
// script_flags), which it copies to $65-$68. Keeps Y; returns A = Y.
void test_hotspot(void) {
  RUN_ORIGINAL_IF_LAG(kTestHotspot);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  c->y = 0;
  do {
    c->a = emu_read_ind_y(0xA8, c->y);
    g_ram[0x65 + c->y] = c->a;
    c->y++;
    emu_cmp(c->y, 0x04);
  } while(c->y < 0x04);
  cursor_in_hotspot();
  c->y = emu_pull();
  emu_set_a(c->y);
}

// cursor_in_hotspot (7:C283): bit 0 of script_flags = x1 <= cursor_x < x2 and y1 <= cursor_y < y2
// for the rectangle at $65-$68.
void cursor_in_hotspot(void) {
  RUN_ORIGINAL_IF_LAG(kCursorInHotspot);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(script_flags & 0xFE);
  script_flags = c->a;
  emu_set_a(cursor_x);
  emu_cmp(c->a, g_ram[0x65]);
  if(c->a < g_ram[0x65]) {
    return;
  }
  emu_cmp(c->a, g_ram[0x66]);
  if(c->a >= g_ram[0x66]) {
    return;
  }
  emu_set_a(cursor_y);
  emu_cmp(c->a, g_ram[0x67]);
  if(c->a < g_ram[0x67]) {
    return;
  }
  emu_cmp(c->a, g_ram[0x68]);
  if(c->a >= g_ram[0x68]) {
    return;
  }
  emu_set_a(script_flags | 0x01);
  script_flags = c->a;
}

// click_self, in click_command_area (6:8E0A): the player as the target (table 0, object $73), or
// as the choice a script waits for.
static void click_self(void) {
  Cpu6502 *c = &g_nes.cpu;
  test_event_flag(0x80);
  emu_set_a(script_flags & 0x04);
  if(c->a) {
    emu_set_a(0x00);
    switch_key[0] = 0x00;
    emu_set_a(0x73);
    switch_key[1] = 0x73;
    return;
  }
  emu_set_a(0x00);
  script_table = 0x00;
  emu_set_a(0x73);
  object_id = 0x73;
}

// click_command_area (6:8D85): Y = the command clicked (of the 14 rectangles at 6:8E2D), if any.
// While a script waits for a choice, only self ($08), $0B, $0C and $0D count. $08 targets the
// player; $0D runs special_command_by_room with the command chosen; $09 is far call $8400/$24;
// $0B and $0C act at once, leaving the command chosen before them. Others become the command,
// highlighted.
void click_command_area(void) {
  RUN_ORIGINAL_IF_LAG(kClickCommandArea);
  Cpu6502 *c = &g_nes.cpu;
  c->y = 0;
  hotspot_ptr[0] = kCommandRects & 0xFF;
  emu_set_a(kCommandRects >> 8);
  hotspot_ptr[1] = c->a;
  for(;;) {
    test_hotspot();
    emu_set_a(script_flags & 0x01);
    if(c->a) {
      break;
    }
    next_screen_rect();
    c->y++;
    emu_cmp(c->y, 0x0E);
    if(c->y >= 0x0E) {
      return;
    }
  }
  test_event_flag(0x80);
  emu_set_a(script_flags & 0x04);
  if(c->a) {
    static const uint8_t kWhileWaiting[4] = {0x08, 0x0B, 0x0C, 0x0D};
    int counts = 0;
    for(int i = 0; i < 4 && !counts; i++) {
      emu_cmp(c->y, kWhileWaiting[i]);
      counts = c->y == kWhileWaiting[i];
    }
    if(!counts) {
      return;
    }
  }
  emu_cmp(c->y, 0x08);
  if(c->y == 0x08) {
    click_self();
    return;
  }
  emu_cmp(c->y, 0x0D);
  if(c->y == 0x0D) {
    emu_set_a(command);
    emu_cmp(c->a, 0xFF);
    if(c->a != 0xFF) {
      special_command_by_room();
    }
    return;
  }
  emu_set_a(command);
  g_ram[0xB1] = c->a;
  emu_cmp(c->a, 0xFF);
  if(c->a != 0xFF) {
    emu_cmp(c->a, 0x0D);
    if(c->a < 0x0D) {
      unhighlight_command();
      RESUME_ORIGINAL_IF_LAG(kClickCommandArea, 0x8DDE, 0);
    }
  }
  command = c->y;
  emu_cmp(c->y, 0x0D);
  if(c->y < 0x0D) {
    highlight_command();
    RESUME_ORIGINAL_IF_LAG(kClickCommandArea, 0x8DE7, 0);
  }
  emu_cmp(c->y, 0x08);
  if(c->y == 0x08) {
    click_self();
    return;
  }
  emu_cmp(c->y, 0x09);
  if(c->y == 0x09) {
    far_call_8400(0x24);
    RESUME_ORIGINAL_IF_LAG(kClickCommandArea, 0x8E2C, 0);
    return;
  }
  emu_cmp(c->y, 0x0B);
  if(c->y != 0x0B) {
    emu_cmp(c->y, 0x0C);
    if(c->y != 0x0C) {
      return;
    }
  }
  clear_selection_unless_waiting();
  RESUME_ORIGINAL_IF_LAG(kClickCommandArea, 0x8DFB, 0);
  unhighlight_command();
  RESUME_ORIGINAL_IF_LAG(kClickCommandArea, 0x8DFE, 0);
  emu_set_a(g_ram[0xB1]);
  command = c->a;
  emu_cmp(c->a, 0xFF);
  if(c->a != 0xFF) {
    highlight_command();
    RESUME_ORIGINAL_IF_LAG(kClickCommandArea, 0x8E09, 0);
  }
}

// click_inventory_area (6:8E65): with a command chosen and a page other than the address book
// up, the entry clicked (of the 7 rectangles at 6:8EC4) becomes the target, or the choice a
// script waits for (kept at $04B2/$04B3, else $04B0/$04B1), and is highlighted; an empty one
// does nothing.
void click_inventory_area(void) {
  RUN_ORIGINAL_IF_LAG(kClickInventoryArea);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(command);
  emu_cmp(c->a, 0xFF);
  if(c->a == 0xFF) {
    return;
  }
  emu_set_a(inventory_page);
  if(!c->a) {
    return;
  }
  c->y = 0;
  hotspot_ptr[0] = kEntryRects & 0xFF;
  emu_set_a(kEntryRects >> 8);
  hotspot_ptr[1] = c->a;
  for(;;) {
    test_hotspot();
    emu_set_a(script_flags & 0x01);
    if(c->a) {
      break;
    }
    next_screen_rect();
    c->y++;
    emu_cmp(c->y, 0x07);
    if(c->y >= 0x07) {
      return;
    }
  }
  inventory_entry = c->y;
  check_inventory_slot_empty();
  RESUME_ORIGINAL_IF_LAG(kClickInventoryArea, 0x8E94, 0);
  emu_set_a(script_flags & 0x10);
  if(c->a) {
    return;
  }
  test_event_flag(0x80);
  emu_set_a(script_flags & 0x04);
  uint16_t keep = c->a ? 0x4B2 : 0x4B0;
  emu_set_a(inventory_page);
  g_ram[keep] = c->a;
  emu_set_a(inventory_entry);
  g_ram[keep + 1] = c->a;
  highlight_entry();
  RESUME_ORIGINAL_IF_LAG(kClickInventoryArea, 0x8EC3, 0);
}

// ---- Highlights ----------------------------------------------------------------------------

// highlight_command (6:AB7F): lights the marker beside `command`. Keeps Y; returns A = Y.
void highlight_command(void) {
  RUN_ORIGINAL_IF_LAG(kHighlightCommand);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  command_vram_address();
  draw_command_highlight();
  RESUME_ORIGINAL_IF_LAG(kHighlightCommand, 0xAB87, 1);
  c->y = emu_pull();
  emu_set_a(c->y);
}

// draw_command_highlight (6:AB8A): the lit marker tile (the block at 6:ABF8) at $1C/$1D, and
// waits until the NMI has written it.
void draw_command_highlight(void) {
  RUN_ORIGINAL_IF_LAG(kDrawCommandHighlight);
  g_ram[0x1E] = kHighlightBlocks & 0xFF;
  emu_set_a(kHighlightBlocks >> 8);
  g_ram[0x1F] = kHighlightBlocks >> 8;
  queue_vram_block();
  RESUME_ORIGINAL_IF_LAG(kDrawCommandHighlight, 0xAB95, 0);
  request_vram_queue();
  wait_nmi_work_done();
  RESUME_ORIGINAL_IF_LAG(kDrawCommandHighlight, 0xAB9B, 0);
}

// unhighlight_command (6:AB9C): the unlit marker (6:ABFB) beside `command`. Keeps Y; returns
// A = Y.
void unhighlight_command(void) {
  RUN_ORIGINAL_IF_LAG(kUnhighlightCommand);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  command_vram_address();
  g_ram[0x1E] = (kHighlightBlocks + 3) & 0xFF;
  emu_set_a(kHighlightBlocks >> 8);
  g_ram[0x1F] = kHighlightBlocks >> 8;
  queue_vram_block();
  RESUME_ORIGINAL_IF_LAG(kUnhighlightCommand, 0xABAC, 1);
  request_vram_queue();
  wait_nmi_work_done();
  RESUME_ORIGINAL_IF_LAG(kUnhighlightCommand, 0xABB2, 1);
  c->y = emu_pull();
  emu_set_a(c->y);
}

// command_vram_address (6:ABB5): $1C/$1D = where `command`'s marker is. Y is the table offset;
// returns A = the high byte, the carry from the ASL.
void command_vram_address(void) {
  RUN_ORIGINAL_IF_LAG(kCommandVramAddress);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_flag(kFlagC, command >> 7);
  c->y = (uint8_t)(command << 1);
  g_ram[0x1C] = cpu_bus_read((uint16_t)(kCommandVramAddresses + c->y));
  emu_set_a(cpu_bus_read((uint16_t)(kCommandVramAddresses + 1 + c->y)));
  g_ram[0x1D] = c->a;
}

// clear_entry_highlight (6:ABC4): the unlit marker (6:ABFE) beside inventory_entry. Keeps Y;
// returns A = Y.
void clear_entry_highlight(void) {
  RUN_ORIGINAL_IF_LAG(kClearEntryHighlight);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  inventory_entry_vram_address();
  g_ram[0x1E] = (kHighlightBlocks + 6) & 0xFF;
  emu_set_a(kHighlightBlocks >> 8);
  g_ram[0x1F] = kHighlightBlocks >> 8;
  queue_vram_block();
  RESUME_ORIGINAL_IF_LAG(kClearEntryHighlight, 0xABD4, 1);
  request_vram_queue();
  wait_nmi_work_done();
  RESUME_ORIGINAL_IF_LAG(kClearEntryHighlight, 0xABDA, 1);
  c->y = emu_pull();
  emu_set_a(c->y);
}

// highlight_entry (6:ABDD, far call $8040/$65): lights the marker beside inventory_entry. Keeps
// Y; returns A = Y.
void highlight_entry(void) {
  RUN_ORIGINAL_IF_LAG(kHighlightEntry);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  inventory_entry_vram_address();
  draw_command_highlight();
  RESUME_ORIGINAL_IF_LAG(kHighlightEntry, 0xABE5, 1);
  c->y = emu_pull();
  emu_set_a(c->y);
}

// inventory_entry_vram_address (6:ABE8): $1C/$1D = where inventory_entry's marker is. Y is the
// table offset; returns A = the high byte, the carry from the ASL.
void inventory_entry_vram_address(void) {
  RUN_ORIGINAL_IF_LAG(kInventoryEntryVramAddress);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_flag(kFlagC, inventory_entry >> 7);
  c->y = (uint8_t)(inventory_entry << 1);
  g_ram[0x1C] = cpu_bus_read((uint16_t)(kEntryVramAddresses + c->y));
  emu_set_a(cpu_bus_read((uint16_t)(kEntryVramAddresses + 1 + c->y)));
  g_ram[0x1D] = c->a;
}
