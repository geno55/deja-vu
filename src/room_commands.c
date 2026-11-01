// What commands do outside the scripts (bank 6): choosing a script for a
// command, the default responses, and the rooms where the command at $0D or any command ends in a
// special scene. Which room is which in the game isn't mapped; the comments say what the code does.
//
// $60 is the place in the picture the cursor is at, in the form the room lists use.
#include "funcs.h"
#include "pointer.h"
#include "variables.h"

// LDA script_flags, AND #$04: the script condition, in A and as the result.
static uint8_t condition(void) {
  emu_set_a(script_flags & 0x04);
  return g_nes.cpu.a;
}

// LDA #id, JSR print_message_2_or_5, whose branch reads the LDA's flags.
static void message_2_or_5(uint8_t id) {
  emu_set_a(id);
  print_message_2_or_5();
}

static void show_scene(uint8_t which) {
  emu_set_a(which);
  special_scene = which;
  show_special_scene();
}

// ---- Choosing what runs --------------------------------------------------------------------

// script_slot_for_command (6:A96E, far call $63): script_slot from the table at 6:A985 of 11 commands
// per script table. $FF: no script.
void script_slot_for_command(void) {
  RUN_ORIGINAL_IF_LAG(kScriptSlotForCommand);
  Cpu6502 *c = &g_nes.cpu;
  uint8_t a = asl_a(script_table & 0x7F);
  script_slot = a;
  a = asl_a(asl_a(a));
  emu_set_flag(kFlagC, 0);
  a = emu_adc(a, script_table);
  a = emu_adc(a, script_slot);
  a = emu_adc(a, command);
  c->y = a;
  emu_set_a(cpu_bus_read((uint16_t)(kScriptSlots + c->y)));
  script_slot = c->a;
}

// far_8020_64 (6:AA41, far call $64): the response when no script does: a special room, two particular
// objects, or a message from the table at 6:AAFC, 2 bytes per command per script table.
void far_8020_64(void) {
  RUN_ORIGINAL_IF_LAG(kFar8020_64);
  Cpu6502 *c = &g_nes.cpu;
  special_room_response();
  RESUME_ORIGINAL_IF_LAG(kFar8020_64, 0xAA44, 0);
  if(condition()) {
    return;
  }
  script_table_is_0();
  RESUME_ORIGINAL_IF_LAG(kFar8020_64, 0xAA4E, 0);
  if(condition()) {
    return;
  }
  script_table_is_0_b();
  RESUME_ORIGINAL_IF_LAG(kFar8020_64, 0xAA58, 0);
  if(condition()) {
    return;
  }
  uint8_t a = asl_a(asl_a(script_table));
  g_ram[0x5E] = a;
  a = asl_a(a);
  emu_set_flag(kFlagC, 0);
  a = emu_adc(a, g_ram[0x5E]);
  emu_set_flag(kFlagC, 1);
  a = emu_sbc(a, script_table);
  a = asl_a(a);
  emu_set_flag(kFlagC, 0);
  a = emu_adc(a, command);
  a = emu_adc(a, command);
  c->y = a;
  uint8_t printer = cpu_bus_read((uint16_t)(kDefaultResponses + c->y));
  emu_set_a(printer);
  emu_cmp(printer, 0xFF);
  if(printer == 0xFF) {
    return;
  }
  JUMP_TABLE_OR_ORIGINAL(kFar8020_64, 0xAA7A, 0, printer, 3);
  jump_table(0xAA7C, printer);
  emu_set_a(cpu_bus_read((uint16_t)(kDefaultResponses - 1 + c->y)));
  switch(printer) {
    case 0:
      print_message_5(c->a);
      RESUME_ORIGINAL_IF_LAG(kFar8020_64, 0xAA89, 0);
      break;
    case 1:
      print_message_4(c->a);
      RESUME_ORIGINAL_IF_LAG(kFar8020_64, 0xAA90, 0);
      break;
    default:
      print_message_2_or_5();
      RESUME_ORIGINAL_IF_LAG(kFar8020_64, 0xAA97, 0);
      break;
  }
}

// script_table_is_0 (6:A9C7): for object $12 of table 0, commands 3 and 4 swap event flags 3 and 4,
// with a message for each way. The condition says it did.
void script_table_is_0(void) {
  RUN_ORIGINAL_IF_LAG(kScriptTableIs0);
  Cpu6502 *c = &g_nes.cpu;
  script_condition_false();
  emu_set_a(script_table);
  emu_cmp(c->a, 0x00);
  if(c->a) {
    return;
  }
  emu_set_a(object_id);
  emu_cmp(c->a, 0x12);
  if(c->a != 0x12) {
    return;
  }
  emu_set_a(command);
  emu_cmp(c->a, 0x03);
  if(c->a == 0x03) {
    test_event_flag(0x04);
    text_item = 0x6A;
    if(condition()) {
      message_2_or_5(0x01);
      RESUME_ORIGINAL_IF_LAG(kScriptTableIs0, 0xA9F8, 0);
    } else {
      message_2_or_5(0x00);
      RESUME_ORIGINAL_IF_LAG(kScriptTableIs0, 0xAA01, 0);
      clear_event_flag(0x03);
      set_event_flag(0x04);
      op_27();
      RESUME_ORIGINAL_IF_LAG(kScriptTableIs0, 0xAA0E, 0);
    }
    script_condition_true();
    return;
  }
  emu_cmp(c->a, 0x04);
  if(c->a != 0x04) {
    return;
  }
  test_event_flag(0x03);
  text_item = 0x6A;
  if(condition()) {
    message_2_or_5(0x03);
    RESUME_ORIGINAL_IF_LAG(kScriptTableIs0, 0xAA27, 0);
  } else {
    message_2_or_5(0x02);
    RESUME_ORIGINAL_IF_LAG(kScriptTableIs0, 0xAA30, 0);
    clear_event_flag(0x04);
    set_event_flag(0x03);
    op_27();
    RESUME_ORIGINAL_IF_LAG(kScriptTableIs0, 0xAA3D, 0);
  }
  script_condition_true();
}

// script_table_is_0_b (6:AAC7): command 4 on object $3F of table 0, in scene $44, becomes op_1B on
// object $47 of table 5, and clears event flag $5D. The condition says it did.
void script_table_is_0_b(void) {
  RUN_ORIGINAL_IF_LAG(kScriptTableIs0B);
  Cpu6502 *c = &g_nes.cpu;
  script_condition_false();
  emu_set_a(script_table);
  emu_cmp(c->a, 0x00);
  if(c->a) {
    return;
  }
  emu_set_a(object_id);
  emu_cmp(c->a, 0x3F);
  if(c->a != 0x3F) {
    return;
  }
  emu_set_a(scene);
  emu_cmp(c->a, 0x44);
  if(c->a != 0x44) {
    return;
  }
  emu_set_a(command);
  emu_cmp(c->a, 0x04);
  if(c->a != 0x04) {
    return;
  }
  script_table = 0x05;
  object_id = 0x47;
  emu_set_a(0x47);
  op_1B();
  RESUME_ORIGINAL_IF_LAG(kScriptTableIs0B, 0xAAF2, 0);
  clear_event_flag(0x5D);
  script_condition_true();
}

// ---- Rooms with their own ending -----------------------------------------------------------

// The four rooms below have their response to everything: one ends on a message and op_63, the rest
// in scene $15 and message $62 with bit 7 of script_flags, which leaves the input loop.
static void leave_with_scene_15(void) {
  emu_set_a(script_flags | 0x80);
  script_flags = g_nes.cpu.a;
}

// special_command_room_1E (6:8EE0)
void special_command_room_1E(void) {
  RUN_ORIGINAL_IF_LAG(kSpecialCommandRoom1E);
  show_scene(0x06);
  RESUME_ORIGINAL_IF_LAG(kSpecialCommandRoom1E, 0x8EE7, 0);
  text_keep_open = 0x01;
  print_message_4(0xE4);
  RESUME_ORIGINAL_IF_LAG(kSpecialCommandRoom1E, 0x8EF1, 0);
  op_63();
  RESUME_ORIGINAL_IF_LAG(kSpecialCommandRoom1E, 0x8EF4, 0);
  print_message_4(0xE5);
  RESUME_ORIGINAL_IF_LAG(kSpecialCommandRoom1E, 0x8EF9, 0);
  print_message_4(0xE7);
  RESUME_ORIGINAL_IF_LAG(kSpecialCommandRoom1E, 0x8EFE, 0);
}

// special_command_room_20 (6:8F21)
void special_command_room_20(void) {
  RUN_ORIGINAL_IF_LAG(kSpecialCommandRoom20);
  show_scene(0x07);
  RESUME_ORIGINAL_IF_LAG(kSpecialCommandRoom20, 0x8F28, 0);
  show_scene(0x16);
  RESUME_ORIGINAL_IF_LAG(kSpecialCommandRoom20, 0x8F2F, 0);
  message_2_or_5(0xE6);
  RESUME_ORIGINAL_IF_LAG(kSpecialCommandRoom20, 0x8F34, 0);
  show_scene(0x15);
  RESUME_ORIGINAL_IF_LAG(kSpecialCommandRoom20, 0x8F3B, 0);
  message_2_or_5(0x62);
  RESUME_ORIGINAL_IF_LAG(kSpecialCommandRoom20, 0x8F40, 0);
  leave_with_scene_15();
  script_condition_true();
}

// special_command_room_24 (6:8F4A)
void special_command_room_24(void) {
  RUN_ORIGINAL_IF_LAG(kSpecialCommandRoom24);
  print_message_4(0x37);
  RESUME_ORIGINAL_IF_LAG(kSpecialCommandRoom24, 0x8F4F, 0);
  show_scene(0x15);
  RESUME_ORIGINAL_IF_LAG(kSpecialCommandRoom24, 0x8F56, 0);
  message_2_or_5(0x62);
  RESUME_ORIGINAL_IF_LAG(kSpecialCommandRoom24, 0x8F5B, 0);
  leave_with_scene_15();
  script_condition_true();
}

// special_command_rooms_4A (6:8F65), rooms $4A-$4D: also sets event flag $95 if flag 7 is.
void special_command_rooms_4A(void) {
  RUN_ORIGINAL_IF_LAG(kSpecialCommandRooms4A);
  show_scene(0x07);
  RESUME_ORIGINAL_IF_LAG(kSpecialCommandRooms4A, 0x8F6C, 0);
  print_message_4(0xF0);
  RESUME_ORIGINAL_IF_LAG(kSpecialCommandRooms4A, 0x8F71, 0);
  show_scene(0x15);
  RESUME_ORIGINAL_IF_LAG(kSpecialCommandRooms4A, 0x8F78, 0);
  message_2_or_5(0x62);
  RESUME_ORIGINAL_IF_LAG(kSpecialCommandRooms4A, 0x8F7D, 0);
  leave_with_scene_15();
  test_event_flag(0x07);
  if(condition()) {
    set_event_flag(0x95);
  }
  script_condition_true();
}

// special_command_by_room (6:8EFF): command $0D, by room.
void special_command_by_room(void) {
  RUN_ORIGINAL_IF_LAG(kSpecialCommandByRoom);
  uint8_t r = room;
  emu_set_a(r);
  emu_cmp(r, 0x1E);
  if(r == 0x1E) {
    special_command_room_1E();
    return;
  }
  emu_cmp(r, 0x20);
  if(r == 0x20) {
    special_command_room_20();
    return;
  }
  emu_cmp(r, 0x24);
  if(r == 0x24) {
    special_command_room_24();
    return;
  }
  emu_cmp(r, 0x4D);
  if(r >= 0x4A && r <= 0x4D) {
    emu_cmp(r, r);  // the CMP that matched
    special_command_rooms_4A();
    return;
  }
  special_command_other_rooms();
}

// special_command_other_rooms (6:8F97): with the cursor in the rectangle at 6:901C, the door in the
// room's list at that place (not a hidden one) becomes the target or the choice, in table 5.
void special_command_other_rooms(void) {
  RUN_ORIGINAL_IF_LAG(kSpecialCommandOtherRooms);
  Cpu6502 *c = &g_nes.cpu;
  hotspot_ptr[0] = 0x1C;
  emu_set_a(0x90);
  hotspot_ptr[1] = 0x90;
  test_hotspot();
  emu_set_a(script_flags & 0x01);
  if(!c->a) {
    return;
  }
  room_scene_list();
  emu_set_flag(kFlagC, 1);
  g_ram[0x60] = emu_sbc(cursor_y, 0xB8) >> 3;
  emu_set_flag(kFlagC, 1);
  uint8_t a = asl_a(emu_sbc(cursor_x, 0x88));
  emu_set_a((a & 0xF0) | g_ram[0x60]);
  g_ram[0x60] = c->a;
  for(;;) {
    c->y = 0;
    emu_set_a(emu_read_ind_y(0x5E, 0));
    emu_cmp(c->a, 0xFF);
    if(c->a == 0xFF) {
      return;
    }
    tile_patch_hidden_alt();
    RESUME_ORIGINAL_IF_LAG(kSpecialCommandOtherRooms, 0x8FCD, 0);
    emu_set_a(script_flags & 0x20);
    if(!c->a) {
      emu_set_a(emu_read_ind_y(0x5E, c->y));
      emu_cmp(c->a, g_ram[0x60]);
      if(c->a == g_ram[0x60]) {
        test_event_flag(0x80);
        int waiting = condition() != 0;
        c->y++;
        emu_set_a(emu_read_ind_y(0x5E, c->y) & 0x7F);
        if(waiting) {
          switch_key[1] = c->a;
          emu_set_a(0x05);
          switch_key[0] = 0x05;
        } else {
          object_id = c->a;
          emu_set_a(0x05);
          script_table = 0x05;
        }
        return;
      }
    }
    add16(g_ram + 0x5E, 0x02);
  }
}

// special_room_response (6:AA98): for the same four rooms, any command. Otherwise the condition is
// false.
void special_room_response(void) {
  RUN_ORIGINAL_IF_LAG(kSpecialRoomResponse);
  uint8_t r = room;
  emu_set_a(r);
  emu_cmp(r, 0x1E);
  if(r == 0x1E) {
    special_command_room_1E();
    return;
  }
  emu_cmp(r, 0x20);
  if(r == 0x20) {
    special_command_room_20();
    return;
  }
  emu_cmp(r, 0x24);
  if(r == 0x24) {
    special_command_room_24();
    return;
  }
  emu_cmp(r, 0x4D);
  if(r >= 0x4A && r <= 0x4D) {
    emu_cmp(r, r);  // the CMP that matched
    special_command_rooms_4A();
    return;
  }
  script_condition_false();
}

// ---- Clearing a selection, and paging through the inventory -----------------------------------

// clear_selection_unless_waiting (6:9257): after the page arrows: the cursor on one turns the page; the
// target goes unless a script waits for a choice, and the choice goes anyway. Keeps Y; returns A = Y.
void clear_selection_unless_waiting(void) {
  RUN_ORIGINAL_IF_LAG(kClearSelectionUnlessWaiting);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  cursor_in_rects_933D();
  RESUME_ORIGINAL_IF_LAG(kClearSelectionUnlessWaiting, 0x925C, 1);
  test_event_flag(0x80);
  if(!condition()) {
    script_table = object_id = 0xFF;
  }
  switch_key[0] = switch_key[1] = 0xFF;
  c->y = emu_pull();
  emu_set_a(c->y);
}

// on_page_04B0 (6:92B2): on the page of the entry kept at $04B0/$04B1, with a script waiting for a
// choice, marks it again.
void on_page_04B0(void) {
  RUN_ORIGINAL_IF_LAG(kOnPage04B0);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(g_ram[0x4B0]);
  emu_cmp(c->a, inventory_page);
  if(c->a != inventory_page) {
    return;
  }
  test_event_flag(0x80);
  if(!condition()) {
    return;
  }
  emu_set_a(g_ram[0x4B1]);
  inventory_entry = c->a;
  far_call_8040(0x65);  // highlight_entry
  RESUME_ORIGINAL_IF_LAG(kOnPage04B0, 0x92D2, 0);
}

// 6:929A, the second arrow: on through the pages, or with containers open through the coat pages of
// the containers (6:92DD), and from the last of those to the goods (6:92D3).
static void next_inventory_page(void) {
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(inventory_page);
  emu_cmp(c->a, 0x0B);
  if(c->a >= 0x0B) {
    return;
  }
  emu_cmp(c->a, 0x02);
  if(c->a < 0x02) {
    emu_set_a(open_containers);
    emu_cmp(c->a, 0xFF);
    if(c->a != 0xFF) {
      TAIL_ORIGINAL_IF_LAG(kOnPage04B0, 0x92DD, 0);
      current_container++;
      emu_set_a(open_containers);
      emu_cmp(c->a, current_container);
      if(c->a < current_container) {
        TAIL_ORIGINAL_IF_LAG(kOnPage04B0, 0x92D3, 0);
        current_container--;
        far_8020_67();
        RESUME_ORIGINAL_IF_LAG(kOnPage04B0, 0x92D9, 0);
        on_page_04B0();
        RESUME_ORIGINAL_IF_LAG(kOnPage04B0, 0x92DC, 0);
        return;
      }
      select_item_list();
      load_container_view();
      RESUME_ORIGINAL_IF_LAG(kOnPage04B0, 0x92EE, 0);
      emu_set_a(0x00);
      inventory_page = 0x00;
      far_call_8020(0x67);
      RESUME_ORIGINAL_IF_LAG(kOnPage04B0, 0x92F8, 0);
      on_page_04B0();
      RESUME_ORIGINAL_IF_LAG(kOnPage04B0, 0x92FB, 0);
      return;
    }
    inventory_page++;
  }
  far_8020_67();
  on_page_04B0();
}

// cursor_in_rects_933D (6:9276): the cursor on one of the two arrows at 6:933D turns the page.
void cursor_in_rects_933D(void) {
  RUN_ORIGINAL_IF_LAG(kCursorInRects933D);
  Cpu6502 *c = &g_nes.cpu;
  c->y = 0;
  hotspot_ptr[0] = 0x3D;
  emu_set_a(0x93);
  hotspot_ptr[1] = 0x93;
  for(;;) {
    test_hotspot();
    next_screen_rect();
    emu_set_a(script_flags & 0x01);
    if(c->a) {
      break;
    }
    c->y++;
    emu_cmp(c->y, 0x02);
    if(c->y >= 0x02) {
      return;
    }
  }
  uint8_t arrow = c->y;
  emu_set_a(arrow);
  JUMP_TABLE_OR_ORIGINAL(kCursorInRects933D, 0x9293, 0, arrow, 2);
  jump_table(0x9295, arrow);
  if(arrow == 0) {
    far_8040_64();
  } else {
    next_inventory_page();
  }
}

// far_8040_64 (6:92FC), the first arrow: back through the pages, the other way.
void far_8040_64(void) {
  RUN_ORIGINAL_IF_LAG(kFar8040_64);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(inventory_page);
  if(!c->a) {
    TAIL_ORIGINAL_IF_LAG(kOnPage04B0, 0x92BA, 0);
    return;
  }
  emu_cmp(c->a, 0x03);
  if(c->a >= 0x03) {
    far_8020_6D();
    RESUME_ORIGINAL_IF_LAG(kFar8040_64, 0x9319, 0);
    on_page_04B0();
    RESUME_ORIGINAL_IF_LAG(kFar8040_64, 0x931C, 0);
    return;
  }
  emu_set_a(open_containers);
  emu_cmp(c->a, 0xFF);
  int to_address_book = 0;
  if(c->a == 0xFF) {
    inventory_page--;
    emu_set_nz(inventory_page);
    to_address_book = 1;
  } else {
    emu_set_a(inventory_page);
    emu_cmp(c->a, 0x02);
    if(c->a != 0x02) {
      current_container--;
      emu_set_nz(current_container);
      to_address_book = current_container >= 0x80;
    }
  }
  if(to_address_book) {
    far_8000_64();
    RESUME_ORIGINAL_IF_LAG(kFar8040_64, 0x9312, 0);
    on_page_04B0();
    RESUME_ORIGINAL_IF_LAG(kFar8040_64, 0x9315, 0);
    return;
  }
  select_item_list();
  load_container_view();
  RESUME_ORIGINAL_IF_LAG(kFar8040_64, 0x932F, 0);
  emu_set_a(0x02);
  inventory_page = 0x02;
  far_call_8020(0x6D);
  RESUME_ORIGINAL_IF_LAG(kFar8040_64, 0x9339, 0);
  on_page_04B0();
  RESUME_ORIGINAL_IF_LAG(kFar8040_64, 0x933C, 0);
}

// ---- Choosing from the address book ---------------------------------------------------------

// far_8000_66 (6:9947, far call $66): the player picks an address with up, down and A, or B to give up.
// $84 = the entry, or $FF with none. Keeps Y.
void far_8000_66(void) {
  RUN_ORIGINAL_IF_LAG(kFar8000_66);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  emu_set_a(address_book[0]);
  emu_cmp(c->a, 0xFF);
  uint8_t result = 0xFF;
  if(c->a == 0xFF) {
    message_2_or_5(0x69);
    RESUME_ORIGINAL_IF_LAG(kFar8000_66, 0x9979, 1);
  } else {
    emu_set_a(0x00);
    command_cursor = 0x00;
    for(;;) {
      data_ptr[0] = 0x00;
      read_joypads();
      read_input_with_repeat();
      pointer_choose_address();
      emu_set_a(data_ptr[0]);
      g_ram[0x83] = c->a;
      step_address_choice();
      redraw_cursor_and_sprites();
      wait_vblank_and_hide_sprites();
      RESUME_ORIGINAL_IF_LAG(kFar8000_66, 0x996B, 1);
      emu_set_a(g_ram[0x83]);
      if(c->a & 0x80) {
        play_sound(0x19);
        emu_set_a(command_cursor);
        inventory_entry = c->a;
        highlight_entry();
        RESUME_ORIGINAL_IF_LAG(kFar8000_66, 0x998A, 1);
        result = command_cursor;
        break;
      }
      if(asl_a(c->a) & 0x80) {
        break;
      }
    }
  }
  emu_set_a(result);
  g_ram[0x84] = result;
  c->y = emu_pull();
  emu_set_a(c->y);
  reset_cursor();
}

// step_address_choice (6:9994): command_cursor moves by $57, wrapping around 0-6, to an entry in use.
void step_address_choice(void) {
  RUN_ORIGINAL_IF_LAG(kStepAddressChoice);
  Cpu6502 *c = &g_nes.cpu;
  for(;;) {
    emu_count_work(1);
    emu_set_flag(kFlagC, 0);
    command_cursor = emu_adc(command_cursor, g_ram[0x57]);
    c->a = command_cursor;
    if(c->a & 0x80) {
      emu_set_a(0x07);
      command_cursor = 0x07;
      continue;
    }
    emu_cmp(c->a, 0x07);
    if(c->a == 0x07) {
      emu_set_a(0xFF);
      command_cursor = 0xFF;
      continue;
    }
    c->y = c->a;
    emu_set_a(address_book[c->y]);
    emu_cmp(c->a, 0xFF);
    if(c->a != 0xFF) {
      return;
    }
  }
}

// redraw_cursor_and_sprites (6:99B8)
void redraw_cursor_and_sprites(void) {
  RUN_ORIGINAL_IF_LAG(kRedrawCursorAndSprites);
  snap_cursor_to_command();
  draw_cursor();
  draw_object_sprites();
}
