// The script opcodes with more to them than a flag (docs/scripts.md):
// entering rooms, containers and the coat page, coins, the goods pages, and the random checks on
// the way between rooms.
//
// What the bytes do is read from the code; what they mean in the game is mostly not mapped yet.
// The scratch bytes, as the originals use them: $5F/$60 the page and entry find_in_goods found,
// $68/$69 and $6A/$6B the pointers to an item list, $6F whether the page comes back after op_1D,
// $AF/$B0 the container shown (script table, object), $BD/$BE an item (script table, object),
// $C0 the item a message names, $C1 an item code. $04A8 an object op_4C keeps, $04A9, $04AA and
// $04AB three counters, $051C and $051D the counts of items 8 and $2D, $04AE the page to go back
// to, $B2 a room.
//
// Most of these wait for the NMI somewhere, so after such a call they check whether to go on as
// original code (RESUME_ORIGINAL_IF_LAG); the addresses are the original's after that JSR. The
// three that may end the script (op_06, op_32, op_5E) have no guard at the top: run_opcode and the
// hook already take the original when an NMI lands in their code.
#include "funcs.h"
#include "variables.h"

// LDA script_flags, AND #$04: the script condition, in A and as the result.
static uint8_t condition(void) {
  emu_set_a(script_flags & 0x04);
  return g_nes.cpu.a;
}

static void message_2_or_5(uint8_t id) {
  emu_set_a(id);
  print_message_2_or_5();
}

// ---- Waiting and small things --------------------------------------------------------------

// op_68 (7:CCEC): waits as many frames as the operand says.
void op_68(void) {
  RUN_ORIGINAL_IF_LAG(kOp68);
  script_fetch_byte();
  do {
    wait_vblank();
    RESUME_ORIGINAL_IF_LAG(kOp68, 0xCCF2, 0);
    script_args[0]--;
    emu_set_nz(script_args[0]);
  } while(script_args[0]);
}

// wait_frames (7:D40E): A frames.
void wait_frames(uint8_t frames) {
  emu_set_a(frames);
  RUN_ORIGINAL_IF_LAG(kWaitFrames);
  g_ram[0x6A] = frames;
  do {
    wait_vblank();
    RESUME_ORIGINAL_IF_LAG(kWaitFrames, 0xD413, 0);
    g_ram[0x6A]--;
    emu_set_nz(g_ram[0x6A]);
  } while(g_ram[0x6A]);
}

// op_5F (7:CDD9): plays the sound the operand names.
void op_5F(void) {
  RUN_ORIGINAL_IF_LAG(kOp5F);
  script_fetch_byte();
  play_sound(g_nes.cpu.a);
}

// op_60 (7:CDC4): swaps the target and the choice. Returns A = the old object_id.
void op_60(void) {
  RUN_ORIGINAL_IF_LAG(kOp60);
  uint8_t table = script_table, object = object_id;
  script_table = switch_key[0];
  switch_key[0] = table;
  object_id = switch_key[1];
  switch_key[1] = object;
  emu_set_a(object);
}

// op_1E (7:D418), op_1F (7:D421), op_38 (7:D5FD), op_3F (7:D73D): an operand for the next message.
void op_1E(void) {
  RUN_ORIGINAL_IF_LAG(kOp1E);
  script_fetch_byte();
  text_item = g_nes.cpu.a;
}

void op_1F(void) {
  RUN_ORIGINAL_IF_LAG(kOp1F);
  script_fetch_byte();
  text_number = g_nes.cpu.a;
}

void op_38(void) {
  RUN_ORIGINAL_IF_LAG(kOp38);
  script_fetch_byte();
  text_number = g_nes.cpu.a;
}

void op_3F(void) {
  RUN_ORIGINAL_IF_LAG(kOp3F);
  script_fetch_byte();
  text_keep_open = g_nes.cpu.a;
}

// op_35 (7:D5F6): sets the room without going there.
void op_35(void) {
  RUN_ORIGINAL_IF_LAG(kOp35);
  script_fetch_byte();
  room = g_nes.cpu.a;
}

// op_5C (7:DA31): sets the target from two operands. Returns A = the object.
void op_5C(void) {
  RUN_ORIGINAL_IF_LAG(kOp5C);
  script_fetch_2();
  script_table = script_args[0];
  emu_set_a(script_args[1]);
  object_id = script_args[1];
}

// op_27 (7:D5E4): the room's sprites again, on screen by the next frame.
void op_27(void) {
  RUN_ORIGINAL_IF_LAG(kOp27);
  collect_room_sprites();
  RESUME_ORIGINAL_IF_LAG(kOp27, 0xD5E7, 0);
  far_call_8020(0x65);  // draw_object_sprites
  RESUME_ORIGINAL_IF_LAG(kOp27, 0xD5EC, 0);
  wait_vblank_and_hide_sprites();
  RESUME_ORIGINAL_IF_LAG(kOp27, 0xD5EF, 0);
}

// op_2A (7:D5F0)
void op_2A(void) {
  RUN_ORIGINAL_IF_LAG(kOp2A);
  far_call_8020(0x6C);
  RESUME_ORIGINAL_IF_LAG(kOp2A, 0xD5F5, 0);
}

// op_46_special_scene (7:D812), op_5A (7:DA28): the special scene the operand names.
void op_46_special_scene(void) {
  RUN_ORIGINAL_IF_LAG(kOp46SpecialScene);
  script_fetch_byte();
  special_scene = g_nes.cpu.a;
  run_special_scene();
  RESUME_ORIGINAL_IF_LAG(kOp46SpecialScene, 0xD81A, 0);
}

void op_5A(void) {
  RUN_ORIGINAL_IF_LAG(kOp5A);
  script_fetch_byte();
  special_scene = g_nes.cpu.a;
  show_special_scene();
  RESUME_ORIGINAL_IF_LAG(kOp5A, 0xDA30, 0);
}

// ---- Counters ------------------------------------------------------------------------------

// op_4A (7:D8D2): the condition = no coins.
void op_4A(void) {
  RUN_ORIGINAL_IF_LAG(kOp4A);
  script_condition_false();
  emu_set_a(coins);
  if(!coins) {
    script_condition_true();
  }
}

// op_4B (7:D8DE): the condition = $04A8 is the operand.
void op_4B(void) {
  RUN_ORIGINAL_IF_LAG(kOp4B);
  script_condition_false();
  script_fetch_byte();
  emu_cmp(g_nes.cpu.a, g_ram[0x4A8]);
  if(g_nes.cpu.a == g_ram[0x4A8]) {
    script_condition_true();
  }
}

// op_4C (7:D8ED): $04A8 = object_id if it is 0; the condition says whether it was.
void op_4C(void) {
  RUN_ORIGINAL_IF_LAG(kOp4C);
  script_condition_false();
  emu_set_a(g_ram[0x4A8]);
  if(!g_ram[0x4A8]) {
    emu_set_a(object_id);
    g_ram[0x4A8] = object_id;
    script_condition_true();
  }
}

// op_4D (7:D8FE): $04A8 = 0; the condition says whether it wasn't already.
void op_4D(void) {
  RUN_ORIGINAL_IF_LAG(kOp4D);
  script_condition_false();
  emu_set_a(g_ram[0x4A8]);
  if(g_ram[0x4A8]) {
    emu_set_a(0x00);
    g_ram[0x4A8] = 0x00;
    script_condition_true();
  }
}

// The counters at $04A9 and $04AA, up to 6 (op_4E, op_58) and down to 0 (op_4F, op_59, op_51).
static void counter_up(uint16_t addr) {
  emu_set_a(g_ram[addr]);
  emu_cmp(g_nes.cpu.a, 0x06);
  if(g_nes.cpu.a >= 0x06) {
    clear_event_flag(0x80);
    script_condition_false();
    return;
  }
  g_ram[addr]++;
  script_condition_true();
}

static void counter_down(uint16_t addr) {
  script_condition_false();
  emu_set_a(g_ram[addr]);
  if(g_ram[addr]) {
    g_ram[addr]--;
    script_condition_true();
  }
}

// op_4E (7:D90F), op_4F (7:D926)
void op_4E(void) {
  RUN_ORIGINAL_IF_LAG(kOp4E);
  counter_up(0x4A9);
}

void op_4F(void) {
  RUN_ORIGINAL_IF_LAG(kOp4F);
  counter_down(0x4A9);
}

// op_50 (7:D935): like op_4E for $04AB, the first time clearing story flag $4B.
void op_50(void) {
  RUN_ORIGINAL_IF_LAG(kOp50);
  emu_set_a(g_ram[0x4AB]);
  emu_cmp(g_nes.cpu.a, 0x06);
  if(g_nes.cpu.a >= 0x06) {
    clear_event_flag(0x80);
    script_condition_false();
    return;
  }
  g_ram[0x4AB]++;
  test_event_flag(0xB1);
  if(condition()) {
    set_event_flag(0xB1);
  } else {
    clear_story_flag(0x4B);
  }
  script_condition_true();
}

// op_51 (7:D965), op_58 (7:D974), op_59 (7:D98B)
void op_51(void) {
  RUN_ORIGINAL_IF_LAG(kOp51);
  counter_down(0x4AB);
}

void op_58(void) {
  RUN_ORIGINAL_IF_LAG(kOp58);
  counter_up(0x4AA);
}

void op_59(void) {
  RUN_ORIGINAL_IF_LAG(kOp59);
  counter_down(0x4AA);
}

// ---- Walking between rooms -----------------------------------------------------------------

// enter_room (7:D0D2): goes to room A: its scene, and with bit 3 of script_flags clear its objects
// too and a dissolve. Clears that bit.
void enter_room(uint8_t new_room) {
  emu_set_a(new_room);
  RUN_ORIGINAL_IF_LAG(kEnterRoom);
  room = new_room;
  far_call_8020(0x61);  // select_room_scene
  RESUME_ORIGINAL_IF_LAG(kEnterRoom, 0xD0DA, 0);
  emu_set_a(script_flags & 0x08);
  if(!g_nes.cpu.a) {
    far_call_8000(0x67);
    RESUME_ORIGINAL_IF_LAG(kEnterRoom, 0xD0E5, 0);
    far_call_8020(0x62);  // draw_room_tile_patches
    RESUME_ORIGINAL_IF_LAG(kEnterRoom, 0xD0EA, 0);
  }
  emu_set_a(script_flags & 0x08);
  if(!g_nes.cpu.a) {
    change_scene();
    RESUME_ORIGINAL_IF_LAG(kEnterRoom, 0xD0F3, 0);
  } else {
    change_scene_instant();
    RESUME_ORIGINAL_IF_LAG(kEnterRoom, 0xD0F9, 0);
  }
  collect_room_sprites();
  RESUME_ORIGINAL_IF_LAG(kEnterRoom, 0xD0FC, 0);
  emu_set_a(script_flags & 0xF7);
  script_flags = g_nes.cpu.a;
}

// clear_listed_story_flags (7:D047): on leaving a room, the 6 story flags at 7:D06C if $051C is
// 0, and the next 6 if $051D is.
void clear_listed_story_flags(void) {
  RUN_ORIGINAL_IF_LAG(kClearListedStoryFlags);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(g_ram[0x51C]);
  if(!c->a) {
    c->y = 0;
    do {
      clear_story_flag(cpu_bus_read((uint16_t)(kListedStoryFlags + c->y)));
      c->y++;
      emu_cmp(c->y, 0x06);
    } while(c->y < 0x06);
  }
  c->y = 0x06;
  emu_set_a(g_ram[0x51D]);
  if(!c->a) {
    do {
      clear_story_flag(cpu_bus_read((uint16_t)(kListedStoryFlags + c->y)));
      c->y++;
      emu_cmp(c->y, 0x0C);
    } while(c->y < 0x0C);
  }
}

// op_06 (7:D078): walks through door object_id, if it is open; if not, says so and ends the script.
void op_06(void) {
  Cpu6502 *c = &g_nes.cpu;
  test_door_flag(object_id);
  if(!condition()) {
    message_2_or_5(0x63);
    RESUME_ORIGINAL_IF_LAG(kOp06, 0xD088, 0);
    end_script();
    return;
  }
  clear_listed_story_flags();
  test_event_flag(0xA9);
  if(condition()) {
    emu_set_a(room);
    emu_cmp(c->a, 0x26);
    if(c->a == 0x26) {
      set_event_flag(0x81);
    } else {
      emu_cmp(c->a, 0x27);
      if(c->a == 0x27) {
        set_event_flag(0xA0);
      }
    }
  }
  clear_event_flag(0xA9);
  backup_state_to_save_ram();
  RESUME_ORIGINAL_IF_LAG(kOp06, 0xD0BE, 0);
  close_all_containers();
  emu_set_a(inventory_page);
  emu_cmp(c->a, 0x01);
  if(c->a == 0x01) {
    far_call_8020(0x67);
    RESUME_ORIGINAL_IF_LAG(kOp06, 0xD0CD, 0);
  }
  script_fetch_byte();
  enter_room(script_args[0]);
}

// The end of op_1A, which op_33 and op_36 jump to (7:D329), and of op_1B, for op_34 and op_37
// (7:D347): sets or clears a door flag, and enters the room again.
static void door_flag_again(uint8_t bit) {
  TAIL_ORIGINAL_IF_LAG(kOp1A, 0xD329, 0);
  set_door_flag(bit);
  enter_room(room);
  RESUME_ORIGINAL_IF_LAG(kOp1A, 0xD332, 0);
}

static void clear_door_flag_again(uint8_t bit) {
  TAIL_ORIGINAL_IF_LAG(kOp1B, 0xD347, 0);
  clear_door_flag(bit);
  enter_room(room);
  RESUME_ORIGINAL_IF_LAG(kOp1B, 0xD350, 0);
}

static void instant_room_change(void) {
  emu_set_a(script_flags | 0x08);
  script_flags = g_nes.cpu.a;
}

// op_33 (7:D315), op_36 (7:D31B), op_1A (7:D321): sets a door flag, by the operand or object_id,
// and enters the room again. All but op_36 set bit 3 of script_flags, which makes the scene change
// at once.
void op_33(void) {
  RUN_ORIGINAL_IF_LAG(kOp33);
  instant_room_change();
  op_36();
}

void op_36(void) {
  RUN_ORIGINAL_IF_LAG(kOp36);
  script_fetch_byte();
  door_flag_again(g_nes.cpu.a);
}

void op_1A(void) {
  RUN_ORIGINAL_IF_LAG(kOp1A);
  instant_room_change();
  emu_set_a(object_id);
  door_flag_again(object_id);
}

// op_34 (7:D333), op_37 (7:D339), op_1B (7:D33F): the same clearing the flag.
void op_34(void) {
  RUN_ORIGINAL_IF_LAG(kOp34);
  instant_room_change();
  op_37();
}

void op_37(void) {
  RUN_ORIGINAL_IF_LAG(kOp37);
  script_fetch_byte();
  clear_door_flag_again(g_nes.cpu.a);
}

void op_1B(void) {
  RUN_ORIGINAL_IF_LAG(kOp1B);
  instant_room_change();
  emu_set_a(object_id);
  clear_door_flag_again(object_id);
}

// ---- Containers and the coat page ----------------------------------------------------------

// close_container_entry (7:CFED): undoes the opening of container_stack entry Y, of script table A.
void close_container_entry(uint8_t table) {
  emu_set_a(table);
  RUN_ORIGINAL_IF_LAG(kCloseContainerEntry);
  Cpu6502 *c = &g_nes.cpu;
  JUMP_TABLE_OR_ORIGINAL(kCloseContainerEntry, 0xCFED, 0, table, 6);
  jump_table(0xCFEF, table);
  switch(table) {
    case 1:
      close_object(container_stack[1 + c->y]);
      clear_door_flag(0x47);
      clear_door_flag(0x48);
      clear_door_flag(0x4A);
      clear_door_flag(0x4B);
      break;
    case 2:
      emu_set_flag(kFlagC, 0);
      c->a = emu_adc(container_stack[1 + c->y], 0x20);
      clear_event_flag(c->a);
      break;
    case 4:
      clear_item_open(container_stack[1 + c->y]);
      break;
    default:
      break;
  }
}

// close_all_containers (7:D028): undoes every open container and empties the list.
void close_all_containers(void) {
  RUN_ORIGINAL_IF_LAG(kCloseAllContainers);
  Cpu6502 *c = &g_nes.cpu;
  c->y = 0;
  do {
    emu_set_a(container_stack[c->y]);
    emu_cmp(c->a, 0xFF);
    if(c->a == 0xFF) {
      break;
    }
    close_container_entry(c->a);
    c->y += 2;
    emu_cmp(c->y, 0x0E);
  } while(c->y < 0x0E);
  emu_set_a(0xFF);
  c->y = 0;
  do {
    (&open_containers)[c->y] = 0xFF;
    c->y++;
    emu_cmp(c->y, 0x10);
  } while(c->y < 0x10);
}

// op_26 (7:D4A6): opens container object_id. Keeps X and Y; returns A = Y.
void op_26(void) {
  RUN_ORIGINAL_IF_LAG(kOp26);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  emu_set_a(c->x);
  emu_push(c->x);
  open_container();
  RESUME_ORIGINAL_IF_LAG(kOp26, 0xD4AD, 2);
  c->x = emu_pull();
  c->y = emu_pull();
  emu_set_a(c->y);
}

// open_container (7:D4B8): the target goes on container_stack, unless 6 are open already, and its
// contents to the coat page.
void open_container(void) {
  RUN_ORIGINAL_IF_LAG(kOpenContainer);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(open_containers);
  emu_cmp(c->a, 0x06);
  if(c->a == 0x06) {
    TAIL_ORIGINAL_IF_LAG(kOp26, 0xD4B2, 0);
    message_2_or_5(0x71);
    RESUME_ORIGINAL_IF_LAG(kOp26, 0xD4B7, 0);
    return;
  }
  open_containers++;
  emu_set_a(open_containers);
  current_container = c->a;
  c->y = asl_a(c->a);
  emu_set_a(script_table);
  g_ram[0xAF] = c->a;
  container_stack[c->y] = c->a;
  emu_set_a(object_id);
  g_ram[0xB0] = c->a;
  container_stack[1 + c->y] = c->a;
  load_container_view();
  RESUME_ORIGINAL_IF_LAG(kOpenContainer, 0xD4DB, 0);
  refresh_coat_page();
  RESUME_ORIGINAL_IF_LAG(kOpenContainer, 0xD4DE, 0);
}

// load_container_view (7:D4DF): coat_slots from the container's items, or from the item list for
// a place that isn't a container proper.
void load_container_view(void) {
  RUN_ORIGINAL_IF_LAG(kLoadContainerView);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(g_ram[0xAF]);
  emu_cmp(c->a, 0x02);
  if(c->a == 0x02) {
    copy_container_to_coat();
    RESUME_ORIGINAL_IF_LAG(kLoadContainerView, 0xD4E8, 0);
  } else {
    fill_coat_from_item_list();
    RESUME_ORIGINAL_IF_LAG(kLoadContainerView, 0xD4EC, 0);
  }
}

// select_item_list (7:D4ED): $AF/$B0 = entry current_container of container_stack. Y = its offset;
// returns A = the object.
void select_item_list(void) {
  RUN_ORIGINAL_IF_LAG(kSelectItemList);
  Cpu6502 *c = &g_nes.cpu;
  c->y = asl_a(current_container);
  g_ram[0xAF] = container_stack[c->y];
  emu_set_a(container_stack[1 + c->y]);
  g_ram[0xB0] = c->a;
}

// fill_coat_from_item_list (7:D4FD): the item list in bank 2 ($AF/$B0 pick it), its visible items
// to coat_slots.
void fill_coat_from_item_list(void) {
  RUN_ORIGINAL_IF_LAG(kFillCoatFromItemList);
  Cpu6502 *c = &g_nes.cpu;
  push_and_switch_bank(0x02);
  clear_coat_slots();
  emu_set_a(0x00);
  c->y = g_ram[0xAF];
  emu_cmp(c->y, 0x01);
  uint8_t list = c->y == 0x01 ? 0 : 1;
  c->y = asl_a(list);
  g_ram[0x68] = cpu_bus_read((uint16_t)(kItemLists + c->y));
  g_ram[0x69] = cpu_bus_read((uint16_t)(kItemLists + 1 + c->y));
  uint8_t a = asl_a(g_ram[0xB0]);
  emu_set_flag(kFlagC, 0);
  g_ram[0x68] = emu_adc(a, g_ram[0x68]);
  g_ram[0x69] = emu_adc(g_ram[0x69], 0x00);
  c->y = 0;
  g_ram[0x6A] = emu_read_ind_y(0x68, 0);
  c->y = 1;
  g_ram[0x6B] = emu_read_ind_y(0x68, 1);
  c->y = 0;
  c->x = 0;
  for(;;) {
    emu_set_a(emu_read_ind_y(0x6A, c->y));
    emu_cmp(c->a, 0xFF);
    if(c->a == 0xFF) {
      break;
    }
    split_item_code(c->a);
    item_list_entry_visible();
    if(!condition()) {
      emu_set_a(emu_read_ind_y(0x6A, c->y));
      coat_slots[c->x] = c->a;
      c->x++;
    }
    c->y++;
  }
  pop_bank();
}

// refresh_coat_page (7:D558): shows the page for what is now in the coat.
void refresh_coat_page(void) {
  RUN_ORIGINAL_IF_LAG(kRefreshCoatPage);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(inventory_page);
  emu_cmp(c->a, 0x01);
  if(c->a > 0x01) {
    emu_set_a(0x02);
    inventory_page = 0x02;
    far_call_8020(0x6D);
    RESUME_ORIGINAL_IF_LAG(kRefreshCoatPage, 0xD576, 0);
    return;
  }
  emu_set_a(0x00);
  inventory_page = 0x00;
  far_call_8020(0x67);
  RESUME_ORIGINAL_IF_LAG(kRefreshCoatPage, 0xD56B, 0);
}

// item_list_entry_visible (7:D577): keeps X and Y; returns A = Y.
void item_list_entry_visible(void) {
  RUN_ORIGINAL_IF_LAG(kItemListEntryVisible);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  emu_set_a(c->x);
  emu_push(c->x);
  item_list_entry_condition();
  c->x = emu_pull();
  c->y = emu_pull();
  emu_set_a(c->y);
}

// item_list_entry_condition (7:D583): the condition = the item ($BD table, $BE object) isn't in
// the list any more: taken, or put away.
void item_list_entry_condition(void) {
  RUN_ORIGINAL_IF_LAG(kItemListEntryCondition);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_flag(kFlagC, 1);
  c->a = emu_sbc(g_ram[0xBD], 0x03);
  uint8_t kind = c->a;
  JUMP_TABLE_OR_ORIGINAL(kItemListEntryCondition, 0xD588, 0, kind, 2);
  jump_table(0xD58A, kind);
  if(kind == 0) {
    test_story_flag(g_ram[0xBE]);
    if(!condition()) {
      test_flag_045C(g_ram[0xBE]);
    }
  } else {
    test_item_obtained(g_ram[0xBE]);
    if(!condition()) {
      test_flag_047A(g_ram[0xBE]);
    }
  }
}

// clear_coat_slots (7:D5B3)
void clear_coat_slots(void) {
  RUN_ORIGINAL_IF_LAG(kClearCoatSlots);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(0xFF);
  c->y = 0;
  do {
    coat_slots[c->y] = 0xFF;
    c->y++;
    emu_cmp(c->y, 0x07);
  } while(c->y < 0x07);
}

// copy_container_to_coat (7:D5C0): container $B0's items to coat_slots. Keeps X and Y; returns
// A = Y.
void copy_container_to_coat(void) {
  RUN_ORIGINAL_IF_LAG(kCopyContainerToCoat);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  emu_set_a(c->x);
  emu_push(c->x);
  clear_coat_slots();
  uint8_t a = asl_a(asl_a(asl_a(g_ram[0xB0])));
  c->x = a;
  c->y = 0;
  do {
    emu_set_a(container_items[c->x]);
    emu_cmp(c->a, 0xFF);
    if(c->a == 0xFF) {
      break;
    }
    coat_slots[c->y] = c->a;
    c->y++;
    c->x++;
    emu_cmp(c->y, 0x07);
  } while(c->y < 0x07);
  c->x = emu_pull();
  c->y = emu_pull();
  emu_set_a(c->y);
}

// op_47 (7:D81B): closes container object_id. Keeps Y; returns A = Y.
void op_47(void) {
  RUN_ORIGINAL_IF_LAG(kOp47);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  close_container();
  RESUME_ORIGINAL_IF_LAG(kOp47, 0xD820, 1);
  c->y = emu_pull();
  emu_set_a(c->y);
}

// close_container (7:D823): takes the target off container_stack, and shows what is left.
void close_container(void) {
  RUN_ORIGINAL_IF_LAG(kCloseContainer);
  Cpu6502 *c = &g_nes.cpu;
  open_containers--;
  c->y = 0;
  for(;;) {
    emu_count_work(1);
    emu_set_a(container_stack[c->y]);
    emu_cmp(c->a, script_table);
    if(c->a == script_table) {
      emu_set_a(container_stack[1 + c->y]);
      emu_cmp(c->a, object_id);
      if(c->a == object_id) {
        break;
      }
    }
    c->y += 2;
  }
  do {
    container_stack[c->y] = container_stack[2 + c->y];
    emu_set_a(container_stack[3 + c->y]);
    container_stack[1 + c->y] = c->a;
    c->y += 2;
    emu_cmp(c->y, 0x0C);
  } while(c->y < 0x0C);
  emu_set_a(0xFF);
  container_stack[c->y] = 0xFF;
  container_stack[1 + c->y] = 0xFF;
  emu_set_a(open_containers);
  emu_cmp(c->a, 0xFF);
  if(c->a == 0xFF) {
    emu_set_a(inventory_page);
    emu_cmp(c->a, 0x02);
    if(c->a == 0x02) {
      return;
    }
    emu_set_a(0x01);
    inventory_page = 0x01;
    far_call_8020(0x67);
    RESUME_ORIGINAL_IF_LAG(kCloseContainer, 0xD89F, 0);
    return;
  }
  c->y = asl_a(current_container);
  emu_set_a(container_stack[c->y]);
  emu_cmp(c->a, 0xFF);
  if(c->a == 0xFF) {
    current_container--;
    select_item_list();
    load_container_view();
    RESUME_ORIGINAL_IF_LAG(kCloseContainer, 0xD871, 0);
    emu_set_a(0x02);
    inventory_page = 0x02;
    far_call_8020(0x6D);
    RESUME_ORIGINAL_IF_LAG(kCloseContainer, 0xD87B, 0);
    return;
  }
  select_item_list();
  load_container_view();
  RESUME_ORIGINAL_IF_LAG(kCloseContainer, 0xD882, 0);
  emu_set_a(0x00);
  inventory_page = 0x00;
  far_call_8020(0x67);
  RESUME_ORIGINAL_IF_LAG(kCloseContainer, 0xD88C, 0);
}

// ---- Taking things -------------------------------------------------------------------------

// op_1D (7:D351): stores the target in the goods. Keeps X and Y; returns A = Y.
void op_1D(void) {
  RUN_ORIGINAL_IF_LAG(kOp1D);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  emu_set_a(c->x);
  emu_push(c->x);
  op_1D_body();
  RESUME_ORIGINAL_IF_LAG(kOp1D, 0xD358, 2);
  c->x = emu_pull();
  c->y = emu_pull();
  emu_set_a(c->y);
}

// op_1D_body (7:D35D): stores the item in the next free slot of the goods, showing its page; taken
// from the coat page, also out of the container it was in, and the coat page comes back.
void op_1D_body(void) {
  RUN_ORIGINAL_IF_LAG(kOp1DBody);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(0x00);
  g_ram[0x6F] = 0x00;
  far_call_8020(0x66);  // store_in_inventory_slot
  RESUME_ORIGINAL_IF_LAG(kOp1DBody, 0xD366, 0);
  emu_set_a(screen_area);
  emu_cmp(c->a, 0x02);
  if(c->a == 0x02) {
    emu_set_a(inventory_page);
    emu_cmp(c->a, 0x01);
    if(c->a == 0x01) {
      far_call_8020(0x6C);
      RESUME_ORIGINAL_IF_LAG(kOp1DBody, 0xD378, 0);
      emu_set_a(g_ram[0xAF]);
      emu_cmp(c->a, 0x02);
      if(c->a == 0x02) {
        find_item_in_container();
        take_item_from_container();
        RESUME_ORIGINAL_IF_LAG(kOp1DBody, 0xD384, 0);
      }
      emu_set_a(0xFF);
      g_ram[0x6F] = 0xFF;
    }
  }
  emu_set_a(inventory_page);
  emu_cmp(c->a, store_page);
  if(c->a < store_page) {
    select_store_slot();
    inventory_page--;
    far_call_8020(0x67);
    RESUME_ORIGINAL_IF_LAG(kOp1DBody, 0xD39B, 0);
  } else {
    select_store_slot();
    far_call_8020(0x68);  // draw_inventory_entry
    RESUME_ORIGINAL_IF_LAG(kOp1DBody, 0xD3A6, 0);
  }
  store_entry++;
  emu_set_a(store_entry);
  emu_cmp(c->a, 0x07);
  if(c->a >= 0x07) {
    emu_set_a(0x00);
    store_entry = 0x00;
    store_page++;
  }
  emu_set_a(g_ram[0x6F]);
  if(!c->a) {
    return;
  }
  wait_frames(0x1E);
  RESUME_ORIGINAL_IF_LAG(kOp1DBody, 0xD3C2, 0);
  emu_set_a(0x02);
  inventory_page = 0x02;
  far_call_8020(0x6D);
  RESUME_ORIGINAL_IF_LAG(kOp1DBody, 0xD3CC, 0);
}

// take_item_from_container (7:D3CD): clears the flag saying the item is in a container.
void take_item_from_container(void) {
  RUN_ORIGINAL_IF_LAG(kTakeItemFromContainer);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_flag(kFlagC, 1);
  c->a = emu_sbc(script_table, 0x03);
  uint8_t kind = c->a;
  JUMP_TABLE_OR_ORIGINAL(kTakeItemFromContainer, 0xD3D2, 0, kind, 2);
  jump_table(0xD3D4, kind);
  if(kind == 0) {
    op_22_clear_flag_045C_of_object();
  } else {
    op_25_clear_flag_047A_of_object();
  }
}

// find_item_in_container (7:D3D9): removes the target's item code from container $B0's 7 items,
// moving the later ones up.
void find_item_in_container(void) {
  RUN_ORIGINAL_IF_LAG(kFindItemInContainer);
  Cpu6502 *c = &g_nes.cpu;
  c->x = 0;
  c->y = asl_a(asl_a(asl_a(g_ram[0xB0])));
  for(;;) {
    item_code_for_object();
    emu_set_a(container_items[c->y]);
    emu_cmp(c->a, g_ram[0xC1]);
    if(c->a == g_ram[0xC1]) {
      break;
    }
    c->y++;
    c->x++;
    emu_cmp(c->x, 0x07);
    if(c->x >= 0x07) {
      return;
    }
  }
  do {
    c->y++;
    emu_set_a(container_items[c->y]);
    c->y--;
    container_items[c->y] = c->a;
    c->y++;
    c->x++;
    emu_cmp(c->x, 0x07);
  } while(c->x < 0x07);
}

// select_store_slot (7:D401): the page and entry to the slot the next item goes in. Returns
// A = the entry.
void select_store_slot(void) {
  RUN_ORIGINAL_IF_LAG(kSelectStoreSlot);
  inventory_page = store_page;
  emu_set_a(store_entry);
  inventory_entry = store_entry;
}

// previous_store_entry (7:D8BE): the slot before.
void previous_store_entry(void) {
  RUN_ORIGINAL_IF_LAG(kPreviousStoreEntry);
  Cpu6502 *c = &g_nes.cpu;
  store_entry--;
  emu_set_a(store_entry);
  emu_cmp(c->a, 0xFF);
  if(c->a == 0xFF) {
    emu_set_a(0x06);
    store_entry = 0x06;
    store_page--;
    emu_set_nz(store_page);
  }
}

// op_49 (7:D8AB): removes the entry, redraws the page and frees its slot.
void op_49(void) {
  RUN_ORIGINAL_IF_LAG(kOp49);
  far_call_8020(0x6B);  // delete_inventory_entry
  RESUME_ORIGINAL_IF_LAG(kOp49, 0xD8B0, 0);
  far_call_8000(0x6E);
  RESUME_ORIGINAL_IF_LAG(kOp49, 0xD8B5, 0);
  far_call_8020(0x6A);  // draw_inventory_page
  RESUME_ORIGINAL_IF_LAG(kOp49, 0xD8BA, 0);
  previous_store_entry();
}

// op_5B (7:CF20): the target's flags change (story flag or item_status off, $045C or $047A on), its
// entry goes, and a message names it.
void op_5B(void) {
  RUN_ORIGINAL_IF_LAG(kOp5B);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(script_table);
  emu_cmp(c->a, 0x03);
  if(c->a == 0x03) {
    clear_story_flag(object_id);
    set_flag_045C(object_id);
  } else {
    clear_item_obtained(object_id);
    set_flag_047A(object_id);
  }
  op_49();
  RESUME_ORIGINAL_IF_LAG(kOp5B, 0xCF40, 0);
  g_ram[0xBD] = script_table;
  emu_set_a(object_id);
  g_ram[0xBE] = object_id;
  far_call_8040(0x62);  // item_number_for_object
  RESUME_ORIGINAL_IF_LAG(kOp5B, 0xCF4D, 0);
  emu_set_a(g_ram[0xC0]);
  text_item = c->a;
  message_2_or_5(0x05);
  RESUME_ORIGINAL_IF_LAG(kOp5B, 0xCF57, 0);
  script_condition_false();
}

// ---- Money and the goods pages -------------------------------------------------------------

// find_in_goods (7:D9BD): $5F/$60 = the page and entry of item script_args[0] in the goods, from
// page 2. X = the entry; Y = where it stopped.
void find_in_goods(void) {
  RUN_ORIGINAL_IF_LAG(kFindInGoods);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(0x02);
  g_ram[0x5F] = 0x02;
  c->x = 0;
  c->y = 0x0E;
  do {
    emu_set_a(address_book[c->y]);
    emu_cmp(c->a, script_args[0]);
    if(c->a == script_args[0]) {
      break;
    }
    c->y++;
    c->x++;
    emu_cmp(c->x, 0x07);
    if(c->x >= 0x07) {
      c->x = 0;
      g_ram[0x5F]++;
    }
    emu_cmp(c->y, 0x54);
  } while(c->y < 0x54);
  g_ram[0x60] = c->x;
}

// show_found_page (7:D711): the page find_in_goods found, or just the entry if it is up.
void show_found_page(void) {
  RUN_ORIGINAL_IF_LAG(kShowFoundPage);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(g_ram[0x60]);
  inventory_entry = c->a;
  emu_set_a(g_ram[0x5F]);
  emu_cmp(c->a, inventory_page);
  if(c->a == inventory_page) {
    far_call_8020(0x68);  // draw_inventory_entry
    RESUME_ORIGINAL_IF_LAG(kShowFoundPage, 0xD730, 0);
  } else if(c->a > inventory_page) {
    inventory_page = c->a;
    inventory_page--;
    far_call_8020(0x67);
    RESUME_ORIGINAL_IF_LAG(kShowFoundPage, 0xD72A, 0);
  } else {
    inventory_page = c->a;
    inventory_page++;
    far_call_8020(0x6D);
    RESUME_ORIGINAL_IF_LAG(kShowFoundPage, 0xD73C, 0);
  }
}

// show_found_page_next (7:CD76): the same in the other direction, redrawing the page if it is up.
void show_found_page_next(void) {
  RUN_ORIGINAL_IF_LAG(kShowFoundPageNext);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(g_ram[0x5F]);
  emu_cmp(c->a, inventory_page);
  if(c->a == inventory_page) {
    far_call_8020(0x6A);  // draw_inventory_page
    RESUME_ORIGINAL_IF_LAG(kShowFoundPageNext, 0xCD90, 0);
  } else if(c->a > inventory_page) {
    inventory_page = c->a;
    inventory_page++;
    far_call_8020(0x6D);
    RESUME_ORIGINAL_IF_LAG(kShowFoundPageNext, 0xCD8A, 0);
  } else {
    inventory_page = c->a;
    inventory_page--;
    far_call_8020(0x67);
    RESUME_ORIGINAL_IF_LAG(kShowFoundPageNext, 0xCD9C, 0);
  }
}

// op_39 (7:D606): adds coins, by the operand.
void op_39(void) {
  RUN_ORIGINAL_IF_LAG(kOp39);
  script_fetch_byte();
  add_coins();
}

// add_coins (7:D609): coins += script_args[0]; item $00 for the message. Returns A = 0.
void add_coins(void) {
  RUN_ORIGINAL_IF_LAG(kAddCoins);
  emu_set_flag(kFlagC, 0);
  coins = emu_adc(coins, script_args[0]);
  emu_set_a(0x00);
  g_ram[0xC0] = 0x00;
}

// op_62 (7:D617): adds coins and shows them.
void op_62(void) {
  RUN_ORIGINAL_IF_LAG(kOp62);
  script_fetch_byte();
  add_coins_and_show();
}

// The middle of the money opcodes: finds item A in the goods and shows its page.
static void find_and_show_page(uint8_t item) {
  script_args[0] = item;
  find_in_goods();
  show_found_page();
}

// add_coins_and_show (7:D61A)
void add_coins_and_show(void) {
  RUN_ORIGINAL_IF_LAG(kAddCoinsAndShow);
  add_coins();
  find_and_show_page(g_nes.cpu.a);
  RESUME_ORIGINAL_IF_LAG(kAddCoinsAndShow, 0xD625, 0);
}

// op_67 (7:CCF7): the condition = whether 2 coins can be paid, and pays them.
void op_67(void) {
  RUN_ORIGINAL_IF_LAG(kOp67);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(coins);
  emu_cmp(c->a, 0x02);
  if(c->a < 0x02) {
    script_condition_false();
    return;
  }
  emu_set_a(0xFE);
  script_args[0] = 0xFE;
  add_coins_and_show();
  RESUME_ORIGINAL_IF_LAG(kOp67, 0xCD09, 0);
  script_condition_true();
}

// op_56 (7:D626): $051C += the operand; item 8 for the message. Returns A = 8.
void op_56(void) {
  RUN_ORIGINAL_IF_LAG(kOp56);
  script_fetch_byte();
  emu_set_flag(kFlagC, 0);
  g_ram[0x51C] = emu_adc(g_ram[0x51C], script_args[0]);
  emu_set_a(0x08);
  g_ram[0xC0] = 0x08;
}

// op_3A (7:D637)
void op_3A(void) {
  RUN_ORIGINAL_IF_LAG(kOp3A);
  op_56();
  find_and_show_page(g_nes.cpu.a);
  RESUME_ORIGINAL_IF_LAG(kOp3A, 0xD642, 0);
  script_condition_true();
}

// op_57 (7:D646): $051D += 1; item $2D for the message. Returns A = $2D.
void op_57(void) {
  RUN_ORIGINAL_IF_LAG(kOp57);
  g_ram[0x51D]++;
  emu_set_a(0x2D);
  g_ram[0xC0] = 0x2D;
}

// op_3B (7:D651)
void op_3B(void) {
  RUN_ORIGINAL_IF_LAG(kOp3B);
  op_57();
  find_and_show_page(g_nes.cpu.a);
  RESUME_ORIGINAL_IF_LAG(kOp3B, 0xD65C, 0);
  script_condition_true();
}

// op_3C (7:D660): takes the operand off the coins, if there are enough, setting event flag $82; if
// not, in rooms $26 and $27 another event flag goes on.
void op_3C(void) {
  RUN_ORIGINAL_IF_LAG(kOp3C);
  Cpu6502 *c = &g_nes.cpu;
  script_fetch_byte();
  emu_set_flag(kFlagC, 1);
  c->a = emu_sbc(coins, script_args[0]);
  if(c->p & kFlagC) {
    coins = c->a;
    set_event_flag(0x82);
    emu_set_a(0x00);
    g_ram[0xC0] = 0x00;
    find_and_show_page(0x00);
    RESUME_ORIGINAL_IF_LAG(kOp3C, 0xD67F, 0);
    script_condition_true();
    return;
  }
  emu_set_a(room);
  emu_cmp(c->a, 0x26);
  if(c->a == 0x26) {
    set_event_flag(0x81);
  } else {
    emu_cmp(c->a, 0x27);
    if(c->a == 0x27) {
      set_event_flag(0xA0);
    }
  }
  script_condition_false();
}

// op_3D (7:D6A4), op_3E (7:D6CA): one less of the item counted at $051C or $051D; with none left,
// a message and event flag $80 cleared.
void op_3D(void) {
  RUN_ORIGINAL_IF_LAG(kOp3D);
  emu_set_a(g_ram[0x51C]);
  if(!g_ram[0x51C]) {
    clear_event_flag(0x80);
    message_2_or_5(0x77);
    RESUME_ORIGINAL_IF_LAG(kOp3D, 0xD6C6, 0);
    script_condition_false();
    return;
  }
  g_ram[0x51C]--;
  emu_set_a(0x08);
  g_ram[0xC0] = 0x08;
  find_and_show_page(0x08);
  RESUME_ORIGINAL_IF_LAG(kOp3D, 0xD6B8, 0);
  script_condition_true();
}

void op_3E(void) {
  RUN_ORIGINAL_IF_LAG(kOp3E);
  emu_set_a(g_ram[0x51D]);
  if(!g_ram[0x51D]) {
    clear_event_flag(0x80);
    message_2_or_5(0x78);
    RESUME_ORIGINAL_IF_LAG(kOp3E, 0xD6EC, 0);
    script_condition_false();
    return;
  }
  g_ram[0x51D]--;
  emu_set_a(0x2D);
  g_ram[0xC0] = 0x2D;
  find_and_show_page(0x2D);
  RESUME_ORIGINAL_IF_LAG(kOp3E, 0xD6DE, 0);
  script_condition_true();
}

// op_40 (7:D6F0): one coin less; with none left, a message.
void op_40(void) {
  RUN_ORIGINAL_IF_LAG(kOp40);
  script_condition_false();
  emu_set_a(coins);
  if(!coins) {
    message_2_or_5(0x72);
    RESUME_ORIGINAL_IF_LAG(kOp40, 0xD710, 0);
    return;
  }
  coins--;
  emu_set_a(0x00);
  g_ram[0xC0] = 0x00;
  find_and_show_page(0x00);
  RESUME_ORIGINAL_IF_LAG(kOp40, 0xD707, 0);
  script_condition_true();
}

// op_55 (7:D99A): shows the operand's page, leaving the cursor as it was. Keeps X and Y; returns
// A = Y.
void op_55(void) {
  RUN_ORIGINAL_IF_LAG(kOp55);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  emu_set_a(c->x);
  emu_push(c->x);
  script_fetch_byte();
  find_in_goods();
  show_item_page_keep_cursor();
  RESUME_ORIGINAL_IF_LAG(kOp55, 0xD9A7, 2);
  c->x = emu_pull();
  c->y = emu_pull();
  emu_set_a(c->y);
}

// find_and_show_item (7:D9AC): the same for item A.
void find_and_show_item(uint8_t item) {
  emu_set_a(item);
  RUN_ORIGINAL_IF_LAG(kFindAndShowItem);
  Cpu6502 *c = &g_nes.cpu;
  script_args[0] = item;
  emu_set_a(c->y);
  emu_push(c->y);
  emu_set_a(c->x);
  emu_push(c->x);
  find_in_goods();
  show_item_page_keep_cursor();
  RESUME_ORIGINAL_IF_LAG(kFindAndShowItem, 0xD9B8, 2);
  c->x = emu_pull();
  c->y = emu_pull();
  emu_set_a(c->y);
}

// show_item_page_keep_cursor (7:D9E0): removes the found entry, on its page, and the page shown
// stays or comes back.
void show_item_page_keep_cursor(void) {
  RUN_ORIGINAL_IF_LAG(kShowItemPageKeepCursor);
  Cpu6502 *c = &g_nes.cpu;
  saved_inventory_entry = inventory_entry;
  saved_inventory_page = inventory_page;
  inventory_entry = g_ram[0x60];
  emu_set_a(g_ram[0x5F]);
  emu_cmp(c->a, inventory_page);
  if(c->a == inventory_page) {
    show_page_then_op_49(c->a);
    return;
  }
  if(c->a > inventory_page) {
    inventory_page = c->a;
    far_call_8020(0x6B);  // delete_inventory_entry
    RESUME_ORIGINAL_IF_LAG(kShowItemPageKeepCursor, 0xDA00, 0);
    previous_store_entry();
    restore_inventory_cursor();
    return;
  }
  show_page_then_op_49(c->a);
  RESUME_ORIGINAL_IF_LAG(kShowItemPageKeepCursor, 0xDA0A, 0);
  restore_inventory_cursor();
  inventory_page++;
  far_call_8020(0x6D);
  RESUME_ORIGINAL_IF_LAG(kShowItemPageKeepCursor, 0xDA15, 0);
}

// restore_inventory_cursor (7:DA16): returns A = the page.
void restore_inventory_cursor(void) {
  RUN_ORIGINAL_IF_LAG(kRestoreInventoryCursor);
  inventory_entry = saved_inventory_entry;
  emu_set_a(saved_inventory_page);
  inventory_page = saved_inventory_page;
}

// show_page_then_op_49 (7:DA21)
void show_page_then_op_49(uint8_t page) {
  emu_set_a(page);
  RUN_ORIGINAL_IF_LAG(kShowPageThenOp49);
  inventory_page = page;
  op_49();
  RESUME_ORIGINAL_IF_LAG(kShowPageThenOp49, 0xDA27, 0);
}

// op_64 (7:CD0D): the condition = store_page has reached $0C, with a message.
void op_64(void) {
  RUN_ORIGINAL_IF_LAG(kOp64);
  emu_set_a(store_page);
  emu_cmp(store_page, 0x0C);
  if(store_page < 0x0C) {
    script_condition_false();
    return;
  }
  message_2_or_5(0x73);
  RESUME_ORIGINAL_IF_LAG(kOp64, 0xCD19, 0);
  script_condition_true();
}

// op_61 (7:CD9D): if $04A8 is set, that object goes into the goods again, and $04A8 = 0.
void op_61(void) {
  RUN_ORIGINAL_IF_LAG(kOp61);
  script_condition_false();
  emu_set_a(g_ram[0x4A8]);
  if(!g_ram[0x4A8]) {
    return;
  }
  emu_set_a(0x03);
  script_table = 0x03;
  emu_set_a(g_ram[0x4A8]);
  object_id = g_ram[0x4A8];
  clear_flag_045C(object_id);
  op_1D();
  RESUME_ORIGINAL_IF_LAG(kOp61, 0xCDB5, 0);
  inventory_entry = g_ram[0x4B1];
  emu_set_a(0x00);
  g_ram[0x4A8] = 0x00;
  script_condition_true();
}

// op_63 (7:CD21): items 7 and $54 go from the goods if their story flags are on, the coins go to 0,
// and the game enters room $16.
void op_63(void) {
  RUN_ORIGINAL_IF_LAG(kOp63);
  script_condition_false();
  test_story_flag(0x07);
  if(condition()) {
    find_and_show_item(0x07);
    RESUME_ORIGINAL_IF_LAG(kOp63, 0xCD34, 0);
    clear_story_flag(0x07);
    set_flag_045C(0x07);
  }
  script_condition_false();
  test_story_flag(0x54);
  if(condition()) {
    find_and_show_item(0x54);
    RESUME_ORIGINAL_IF_LAG(kOp63, 0xCD51, 0);
    clear_story_flag(0x54);
    set_flag_045C(0x07);
  }
  emu_set_a(0x00);
  coins = 0x00;
  script_args[0] = 0x00;
  find_in_goods();
  show_found_page_next();
  RESUME_ORIGINAL_IF_LAG(kOp63, 0xCD6A, 0);
  script_table = 0x05;
  emu_set_a(0x16);
  object_id = 0x16;
  enter_room(0x16);
  RESUME_ORIGINAL_IF_LAG(kOp63, 0xCD75, 0);
}

// ---- Choices -------------------------------------------------------------------------------

// op_31 (7:CE0B): keeps Y; returns A = Y.
void op_31(void) {
  RUN_ORIGINAL_IF_LAG(kOp31);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  choice_is_table_2();
  c->y = emu_pull();
  emu_set_a(c->y);
}

// choice_is_table_2 (7:CE13): the condition = switch_key is on table 2, or a hotspot of the room
// for the target is under the cursor, which then becomes switch_key. Without one, event flag $80
// is cleared.
void choice_is_table_2(void) {
  RUN_ORIGINAL_IF_LAG(kChoiceIsTable2);
  Cpu6502 *c = &g_nes.cpu;
  script_condition_false();
  emu_set_a(switch_key[0]);
  emu_cmp(c->a, 0x02);
  if(c->a == 0x02) {
    script_condition_true();
    return;
  }
  push_and_switch_bank(0x02);
  c->y = asl_a(room);
  hotspot_ptr[0] = cpu_bus_read((uint16_t)(kRoomHotspots + c->y));
  hotspot_ptr[1] = cpu_bus_read((uint16_t)(kRoomHotspots + 1 + c->y));
  for(;;) {
    c->y = 0;
    emu_set_a(emu_read_ind_y(0xA8, 0));
    emu_cmp(c->a, 0xFF);
    if(c->a == 0xFF) {
      clear_event_flag(0x80);
      pop_bank();
      script_condition_false();
      return;
    }
    c->y = 4;
    emu_set_a(emu_read_ind_y(0xA8, 4) & 0x7F);
    emu_cmp(c->a, script_table);
    if(c->a == script_table) {
      c->y = 5;
      emu_set_a(emu_read_ind_y(0xA8, 5));
      emu_cmp(c->a, object_id);
      if(c->a == object_id) {
        test_hotspot();
        emu_set_a(script_flags & 0x01);
        if(c->a) {
          c->y = 4;
          switch_key[0] = emu_read_ind_y(0xA8, 4);
          c->y = 5;
          emu_set_a(emu_read_ind_y(0xA8, 5));
          switch_key[1] = c->a;
          pop_bank();
          script_condition_true();
          return;
        }
      }
    }
    next_hotspot();
  }
}

// next_hotspot (7:C323): hotspot_ptr to the next 6-byte record.
void next_hotspot(void) {
  RUN_ORIGINAL_IF_LAG(kNextHotspot);
  add16(hotspot_ptr, 0x06);
}

// op_32 (7:CE7A): acts on the choice, as op_32_body says. Objects 0, 8 and $2D of table 3 only give
// a message, by whether their count (coins, $051C, $051D) is 0, and that ends the script, as does
// bit 6 of script_flags from op_32_body.
void op_32(void) {
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(script_table);
  emu_cmp(c->a, 0x03);
  if(c->a == 0x03) {
    emu_set_a(object_id);
    emu_cmp(c->a, 0x00);
    int checked = 1;
    uint8_t count = 0;
    if(c->a == 0x00) {
      count = coins;
    } else {
      emu_cmp(c->a, 0x08);
      if(c->a == 0x08) {
        count = g_ram[0x51C];
      } else {
        emu_cmp(c->a, 0x2D);
        if(c->a == 0x2D) {
          count = g_ram[0x51D];
        } else {
          checked = 0;
        }
      }
    }
    if(checked) {
      emu_set_a(count);
      if(!count) {
        message_2_or_5(0x29);
        RESUME_ORIGINAL_IF_LAG(kOp32, 0xCE90, 0);
        clear_event_flag(0x80);
      } else {
        clear_event_flag(0x80);
        message_2_or_5(0x7D);
        RESUME_ORIGINAL_IF_LAG(kOp32, 0xCEB7, 0);
      }
      emu_set_a(script_flags & 0xBF);
      script_flags = c->a;
      end_script();
      return;
    }
  }
  emu_set_a(c->y);
  emu_push(c->y);
  emu_set_a(c->x);
  emu_push(c->x);
  emu_set_a(script_flags & 0xBF);
  script_flags = c->a;
  op_32_body();
  RESUME_ORIGINAL_IF_LAG(kOp32, 0xCEC7, 2);
  clear_event_flag(0x80);
  c->x = emu_pull();
  c->y = emu_pull();
  emu_set_a(script_flags & 0x40);
  if(c->a) {
    emu_set_a(script_flags & 0xBF);
    script_flags = c->a;
    end_script();
  }
}

// op_32_body (7:CEE2): by the choice's table. Table 2: the target's item code goes in container
// switch_key[1] and op_5B runs, unless that is $04 (only the condition set) or the 7 places are
// full (a message, and bit 6 of script_flags). Tables 3 and 4: the entry goes, with a message.
void op_32_body(void) {
  RUN_ORIGINAL_IF_LAG(kOp32Body);
  Cpu6502 *c = &g_nes.cpu;
  script_condition_false();
  emu_set_flag(kFlagC, 1);
  c->a = emu_sbc(switch_key[0], 0x02);
  uint8_t kind = c->a;
  JUMP_TABLE_OR_ORIGINAL(kOp32Body, 0xCEEA, 0, kind, 3);
  jump_table(0xCEEC, kind);
  if(kind == 0) {
    emu_set_a(switch_key[1]);
    emu_cmp(c->a, 0x04);
    if(c->a == 0x04) {
      TAIL_ORIGINAL_IF_LAG(kOp32BodyEnd, 0xCF5B, 0);
      script_condition_true();
      return;
    }
    c->y = asl_a(asl_a(asl_a(c->a)));
    c->x = 0;
    for(;;) {
      emu_set_a(container_items[c->y]);
      emu_cmp(c->a, 0xFF);
      if(c->a == 0xFF) {
        break;
      }
      c->y++;
      c->x++;
      emu_cmp(c->x, 0x07);
      if(c->x >= 0x07) {
        message_2_or_5(0x76);
        RESUME_ORIGINAL_IF_LAG(kOp32Body, 0xCF11, 0);
        emu_set_a(script_flags | 0x40);
        script_flags = c->a;
        return;
      }
    }
    item_code_for_object();
    emu_set_a(g_ram[0xC1]);
    container_items[c->y] = c->a;
    op_5B();
    return;
  }
  if(kind == 1) {
    TAIL_ORIGINAL_IF_LAG(kOp32BodyEnd, 0xCF5F, 0);
    op_49();
    RESUME_ORIGINAL_IF_LAG(kOp32BodyEnd, 0xCF62, 0);
    clear_story_flag(object_id);
    op_27();
    RESUME_ORIGINAL_IF_LAG(kOp32BodyEnd, 0xCF6A, 0);
    message_2_or_5(0x58);
    RESUME_ORIGINAL_IF_LAG(kOp32BodyEnd, 0xCF6F, 0);
  } else {
    TAIL_ORIGINAL_IF_LAG(kOp32BodyEnd, 0xCF73, 0);
    op_49();
    RESUME_ORIGINAL_IF_LAG(kOp32BodyEnd, 0xCF76, 0);
    clear_item_obtained(object_id);
    op_27();
    RESUME_ORIGINAL_IF_LAG(kOp32BodyEnd, 0xCF7E, 0);
    message_2_or_5(0x58);
    RESUME_ORIGINAL_IF_LAG(kOp32BodyEnd, 0xCF83, 0);
  }
  script_condition_false();
}

// ---- The address book, and travel ----------------------------------------------------------

// op_41 (7:D746): far call $66 leaves an address_book index in $84. $B2 = the room for that entry
// (7:D79A), plus 1 in an odd room. With $FF there, the page comes back and the condition is set.
void op_41(void) {
  RUN_ORIGINAL_IF_LAG(kOp41);
  Cpu6502 *c = &g_nes.cpu;
  script_condition_false();
  emu_set_a(0x00);
  g_ram[0x4AE] = 0x00;
  emu_set_a(inventory_page);
  if(c->a) {
    g_ram[0x4AE] = c->a;
    emu_set_a(0x01);
    inventory_page = 0x01;
    far_call_8000(0x64);
    RESUME_ORIGINAL_IF_LAG(kOp41, 0xD763, 0);
  }
  far_call_8000(0x66);
  RESUME_ORIGINAL_IF_LAG(kOp41, 0xD768, 0);
  c->y = g_ram[0x84];
  emu_cmp(c->y, 0xFF);
  if(c->y == 0xFF) {
    op_44();
    RESUME_ORIGINAL_IF_LAG(kOp41, 0xD7A2, 0);
    script_condition_true();
    return;
  }
  static const uint8_t kAddressItems[5] = {0xA3, 0xA0, 0xA1, 0xA2, 0xA4};
  emu_set_a(address_book[c->y]);
  c->y = 0;
  for(int i = 0; i < 5; i++) {
    emu_cmp(c->a, kAddressItems[i]);
    if(c->a == kAddressItems[i]) {
      break;
    }
    if(i < 4) {
      c->y++;
    }
  }
  emu_set_a(cpu_bus_read((uint16_t)(kAddressRooms + c->y)));
  g_ram[0xB2] = c->a;
  emu_set_a(room & 0x01);
  if(c->a) {
    g_ram[0xB2]++;
    emu_set_nz(g_ram[0xB2]);
  }
}

// op_44 (7:D7A6): the page from before op_41 comes back.
void op_44(void) {
  RUN_ORIGINAL_IF_LAG(kOp44);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(g_ram[0x4AE]);
  if(c->a) {
    inventory_page = c->a;
    inventory_page--;
    far_call_8020(0x67);
  }
  op_45();
}

// op_45 (7:D7B6): returns A = 0.
void op_45(void) {
  RUN_ORIGINAL_IF_LAG(kOp45);
  emu_set_a(0x00);
  text_keep_open = 0x00;
  split_nametable = 0x00;
}

// op_43 (7:D7C0): the condition = already in room $B2; if not, it becomes the room, with event flag
// $83 set.
void op_43(void) {
  RUN_ORIGINAL_IF_LAG(kOp43);
  Cpu6502 *c = &g_nes.cpu;
  script_condition_false();
  emu_set_a(room);
  emu_cmp(c->a, g_ram[0xB2]);
  if(c->a == g_ram[0xB2]) {
    script_condition_true();
    return;
  }
  emu_set_a(g_ram[0xB2]);
  room = c->a;
  set_event_flag(0x83);
}

// op_42 (7:D7D9): the operand goes in the address book, which comes up for 40 frames if another page
// is showing.
void op_42(void) {
  RUN_ORIGINAL_IF_LAG(kOp42);
  Cpu6502 *c = &g_nes.cpu;
  script_fetch_byte();
  g_ram[0xC0] = c->a;
  c->y = addresses_found;
  address_book[c->y] = c->a;
  addresses_found++;
  emu_set_a(inventory_page);
  if(!c->a) {
    far_call_8020(0x68);  // draw_inventory_entry
    RESUME_ORIGINAL_IF_LAG(kOp42, 0xD7F1, 0);
    return;
  }
  emu_set_a(inventory_page);
  emu_push(c->a);
  emu_set_a(0x01);
  inventory_page = 0x01;
  far_call_8000(0x64);
  RESUME_ORIGINAL_IF_LAG(kOp42, 0xD800, 1);
  wait_frames(0x28);
  RESUME_ORIGINAL_IF_LAG(kOp42, 0xD805, 1);
  emu_set_a(emu_pull());
  inventory_page = c->a;
  inventory_page--;
  far_call_8020(0x67);
  RESUME_ORIGINAL_IF_LAG(kOp42, 0xD811, 0);
}

// ---- Encounters on the way -----------------------------------------------------------------

// op_5E (7:DA3D): like op_06 for door object_id, but the operand goes to op_5E_special_check, and
// the script ends if that sets the condition.
void op_5E(void) {
  Cpu6502 *c = &g_nes.cpu;
  test_door_flag(object_id);
  if(!condition()) {
    message_2_or_5(0x63);
    RESUME_ORIGINAL_IF_LAG(kOp5E, 0xDA4D, 0);
    end_script();
    return;
  }
  backup_state_to_save_ram();
  RESUME_ORIGINAL_IF_LAG(kOp5E, 0xDA55, 0);
  script_fetch_byte();
  emu_set_a(script_args[0]);
  script_args[3] = c->a;
  op_5E_special_check();
  RESUME_ORIGINAL_IF_LAG(kOp5E, 0xDA5F, 0);
  if(condition()) {
    end_script();
  }
}

// op_5E_special_check (7:DA6B): keeps Y; returns A = Y.
void op_5E_special_check(void) {
  RUN_ORIGINAL_IF_LAG(kOp5ESpecialCheck);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  op_5E_special_check_body();
  RESUME_ORIGINAL_IF_LAG(kOp5ESpecialCheck, 0xDA70, 1);
  c->y = emu_pull();
  emu_set_a(c->y);
}

static int story_flag(uint8_t bit) {
  test_story_flag(bit);
  return condition();
}

static int event_flag(uint8_t bit) {
  test_event_flag(bit);
  return condition();
}

// next_random, then LDA random_value, AND mask.
static int random_bits(uint8_t mask) {
  next_random();
  emu_set_a(random_value & mask);
  return g_nes.cpu.a;
}

// op_5E_special_check_body (7:DA73): for the 8 rooms at 7:DC92 (script_args[3]). Each has its own
// test of flags, most with a roll of random; if it passes, that room is entered with a message and
// the condition is set.
void op_5E_special_check_body(void) {
  RUN_ORIGINAL_IF_LAG(kOp5ESpecialCheckBody);
  Cpu6502 *c = &g_nes.cpu;
  script_condition_false();
  c->y = 0;
  for(;;) {
    emu_count_work(1);
    emu_set_a(cpu_bus_read((uint16_t)(kEncounterRooms + c->y)));
    emu_cmp(c->a, 0xFF);
    if(c->a == 0xFF) {
      return;
    }
    emu_cmp(c->a, script_args[3]);
    if(c->a == script_args[3]) {
      break;
    }
    c->y++;
  }
  uint8_t which = c->y;
  emu_set_a(which);
  JUMP_TABLE_OR_ORIGINAL(kOp5ESpecialCheckBody, 0xDA84, 0, which, 8);
  jump_table(0xDA86, which);
  switch(which) {
    case 0:
      script_condition_false();
      if(!story_flag(0x07) && !story_flag(0x54)) {
        return;
      }
      script_condition_false();
      emu_set_a(coins);
      if(!coins) {
        return;
      }
      if(event_flag(0x93)) {
        script_condition_false();
        return;
      }
      script_condition_false();
      if(random_bits(0x03)) {
        return;
      }
      enter_room(script_args[3]);
      RESUME_ORIGINAL_IF_LAG(kOp5ESpecialCheckBody, 0xDADF, 0);
      print_message_4(0xE0);
      RESUME_ORIGINAL_IF_LAG(kOp5ESpecialCheckBody, 0xDAE4, 0);
      script_condition_true();
      return;
    case 1:
      if(event_flag(0x92)) {
        script_condition_false();
        return;
      }
      if(!random_bits(0x01)) {
        script_condition_false();
        return;
      }
      enter_room(script_args[3]);
      RESUME_ORIGINAL_IF_LAG(kOp5ESpecialCheckBody, 0xDB09, 0);
      message_2_or_5(0xE5);
      RESUME_ORIGINAL_IF_LAG(kOp5ESpecialCheckBody, 0xDB0E, 0);
      script_condition_true();
      return;
    case 2:
      test_item_obtained(0x02);
      if(!condition()) {
        test_item_obtained(0x07);
        if(!condition()) {
          return;
        }
      }
      script_condition_false();
      emu_set_a(g_ram[0x4B7]);
      emu_cmp(c->a, 0x03);
      if(c->a >= 0x03) {
        return;
      }
      if(!random_bits(0x01)) {
        return;
      }
      g_ram[0x4B7]++;
      enter_room(script_args[3]);
      RESUME_ORIGINAL_IF_LAG(kOp5ESpecialCheckBody, 0xDB46, 0);
      print_message_4(0x33);
      RESUME_ORIGINAL_IF_LAG(kOp5ESpecialCheckBody, 0xDB4B, 0);
      script_condition_true();
      return;
    case 3:
      if(random_bits(0x01)) {
        script_condition_false();
        return;
      }
      set_event_flag(0xA8);
      enter_room(script_args[3]);
      RESUME_ORIGINAL_IF_LAG(kOp5ESpecialCheckBody, 0xDB66, 0);
      if(event_flag(0x91)) {
        message_2_or_5(0xDE);
        RESUME_ORIGINAL_IF_LAG(kOp5ESpecialCheckBody, 0xDB7F, 0);
      } else {
        message_2_or_5(0xDB);
        RESUME_ORIGINAL_IF_LAG(kOp5ESpecialCheckBody, 0xDB76, 0);
      }
      script_condition_true();
      return;
    case 4:
      if(event_flag(0x05) || event_flag(0x2A)) {
        script_condition_false();
        return;
      }
      script_condition_false();
      if(random_bits(0x18)) {
        return;
      }
      enter_room(script_args[3]);
      RESUME_ORIGINAL_IF_LAG(kOp5ESpecialCheckBody, 0xDBB3, 0);
      print_message_4(0xEB);
      RESUME_ORIGINAL_IF_LAG(kOp5ESpecialCheckBody, 0xDBB8, 0);
      script_condition_true();
      return;
    case 5:
      if(event_flag(0x06) || event_flag(0x2A) || !event_flag(0x05)) {
        script_condition_false();
        return;
      }
      script_condition_false();
      if(random_bits(0x18)) {
        return;
      }
      enter_room(script_args[3]);
      RESUME_ORIGINAL_IF_LAG(kOp5ESpecialCheckBody, 0xDBFB, 0);
      print_message_4(0xF1);
      RESUME_ORIGINAL_IF_LAG(kOp5ESpecialCheckBody, 0xDC00, 0);
      script_condition_true();
      return;
    case 6:
      if(event_flag(0x07) || event_flag(0x2A) || !event_flag(0x06)) {
        script_condition_false();
        return;
      }
      script_condition_false();
      if(random_bits(0x18)) {
        return;
      }
      enter_room(script_args[3]);
      RESUME_ORIGINAL_IF_LAG(kOp5ESpecialCheckBody, 0xDC43, 0);
      print_message_4(0xF4);
      RESUME_ORIGINAL_IF_LAG(kOp5ESpecialCheckBody, 0xDC48, 0);
      script_condition_true();
      return;
    default:
      if(event_flag(0x2A) || !event_flag(0x07) || (!story_flag(0x07) && !story_flag(0x54))) {
        script_condition_false();
        return;
      }
      enter_room(script_args[3]);
      RESUME_ORIGINAL_IF_LAG(kOp5ESpecialCheckBody, 0xDC89, 0);
      print_message_4(0xF6);
      RESUME_ORIGINAL_IF_LAG(kOp5ESpecialCheckBody, 0xDC8E, 0);
      script_condition_true();
      return;
  }
}
