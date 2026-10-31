// The inventory area (bank 6): its pages, entries and page labels, and
// storing and deleting items. A page is 7 entries of address_book, by inventory_page * 7 +
// inventory_entry.
//
// The scratch bytes, as the originals use them: $BD/$BE a script table and object, $C0 the item a
// layout names, $C1 an item code, $9C/$9D where a layout starts, $AF/$B0 the container shown. For the
// labels: $5A/$5B a string, $5C/$5D a list of strings, $77/$78 the VRAM address, $79 which way a
// label runs.
#include "funcs.h"
#include "variables.h"

// LDA script_flags, AND #$04: the script condition, in A and as the result.
static uint8_t condition(void) {
  emu_set_a(script_flags & 0x04);
  return g_nes.cpu.a;
}

// Which group an item code is in, of those named and stored as one item (the coin amounts, say): -1
// for none. Groups 0 and 1 are the ones store_in_inventory_slot also folds.
static int item_group(uint8_t code) {
  if((code >= 0x3D && code <= 0x43) || code == 0x49) return 0;
  if(code == 0x4A || (code >= 0x4F && code <= 0x53)) return 1;
  if(code == 0x54) return 2;
  if(code >= 0x55 && code <= 0x57) return 3;
  if(code == 0x58 || code == 0x59) return 4;
  if(code == 0x5A) return 5;
  if(code == 0x5B || code == 0x5C) return 6;
  return -1;
}

// ---- Storing an item -------------------------------------------------------------------------

// store_in_inventory_slot (6:AC29, far call $66): the target's code goes in the slot store_page and
// store_entry say; $C0 names it. Keeps Y; returns A = Y.
void store_in_inventory_slot(void) {
  RUN_ORIGINAL_IF_LAG(kStoreInInventorySlot);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  item_code_for_object();
  item_code_in_list_AC53();
  uint8_t a = asl_a(asl_a(asl_a(store_page)));
  emu_set_flag(kFlagC, 1);
  a = emu_sbc(a, store_page);
  emu_set_flag(kFlagC, 0);
  c->y = emu_adc(a, store_entry);
  emu_set_a(g_ram[0xC1]);
  address_book[c->y] = c->a;
  g_ram[0xBD] = script_table;
  emu_set_a(object_id);
  g_ram[0xBE] = c->a;
  item_number_for_object();
  c->y = emu_pull();
  emu_set_a(c->y);
}

// item_code_in_list_AC53 (6:AC53): codes in group 0 become item 8 (and the target too), and those in
// group 1 item $2D.
void item_code_in_list_AC53(void) {
  RUN_ORIGINAL_IF_LAG(kItemCodeInListAC53);
  uint8_t code = g_ram[0xC1];
  int group = item_group(code);
  emu_set_a(code);
  if(group != 0 && group != 1) {
    emu_cmp(code, 0x53);
    return;
  }
  emu_set_flag(kFlagC, 1);  // the CMP that matched
  emu_set_a(group ? 0x2D : 0x08);
  object_id = g_nes.cpu.a;
  g_ram[0xC1] = g_nes.cpu.a;
}

// item_number_for_object (6:AC9C, far call $62): $C0 = the item number of $BD and $BE, from the base of
// each table at 6:AD70. Keeps Y; returns A = Y.
void item_number_for_object(void) {
  RUN_ORIGINAL_IF_LAG(kItemNumberForObject);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  c->y = g_ram[0xBD];
  emu_set_flag(kFlagC, 0);
  g_ram[0xC0] = emu_adc(cpu_bus_read((uint16_t)(kItemNumberBases + c->y)), g_ram[0xBE]);
  c->a = g_ram[0xC0];
  kind_is_3();
  c->y = emu_pull();
  emu_set_a(c->y);
}

// kind_is_3 (6:ACAE): in table 3, $C0 of a group of codes becomes the one item they count as
// (6:AD22). Y = the group, or 6.
void kind_is_3(void) {
  RUN_ORIGINAL_IF_LAG(kKindIs3);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(g_ram[0xBD]);
  emu_cmp(c->a, 0x03);
  if(c->a != 0x03) {
    return;
  }
  uint8_t code = g_ram[0xC0];
  int group = item_group(code);
  emu_set_a(code);
  if(group < 0) {
    c->y = 0x06;
    emu_cmp(code, 0x5C);
    return;
  }
  c->y = (uint8_t)group;
  emu_set_flag(kFlagC, 1);  // the CMP that matched
  emu_set_a(cpu_bus_read((uint16_t)(kGroupItems + c->y)));
  g_ram[0xC0] = c->a;
}

// check_inventory_slot_empty (6:AD29): bit 4 of script_flags = the slot under the page and entry is
// empty; otherwise its item is the target, or the choice if a script waits for one.
void check_inventory_slot_empty(void) {
  RUN_ORIGINAL_IF_LAG(kCheckInventorySlotEmpty);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(script_flags & 0xEF);
  script_flags = c->a;
  inventory_slot_index();
  emu_set_a(address_book[c->y]);
  emu_cmp(c->a, 0xFF);
  if(c->a == 0xFF) {
    emu_set_a(script_flags | 0x10);
    script_flags = c->a;
    return;
  }
  split_item_code(c->a);
  test_event_flag(0x80);
  uint8_t *pick = condition() ? switch_key : &script_table;
  pick[0] = g_ram[0xBD];
  emu_set_a(g_ram[0xBE]);
  pick[1] = c->a;
}

// inventory_slot_index (6:AD60): Y = inventory_page * 7 + inventory_entry.
void inventory_slot_index(void) {
  RUN_ORIGINAL_IF_LAG(kInventorySlotIndex);
  uint8_t a = asl_a(asl_a(asl_a(inventory_page)));
  emu_set_flag(kFlagC, 1);
  a = emu_sbc(a, inventory_page);
  emu_set_flag(kFlagC, 0);
  a = emu_adc(a, inventory_entry);
  g_nes.cpu.a = a;
  g_nes.cpu.y = a;
}

// ---- Drawing a page -------------------------------------------------------------------------

// draw_inventory_page (6:AD76, far call $6A): the page's entries up to the first empty one. Keeps the
// target, which check_inventory_slot_empty sets.
void draw_inventory_page(void) {
  RUN_ORIGINAL_IF_LAG(kDrawInventoryPage);
  Cpu6502 *c = &g_nes.cpu;
  emu_push(script_table);
  emu_push(object_id);
  inventory_entry = 0x00;
  do {
    check_inventory_slot_empty();
    item_number_for_object();
    emu_set_a(script_flags & 0x10);
    if(c->a) {
      break;
    }
    draw_inventory_entry();
    RESUME_ORIGINAL_IF_LAG(kDrawInventoryPage, 0xAD90, 2);
    inventory_entry++;
    emu_set_a(inventory_entry);
    emu_cmp(c->a, 0x07);
  } while(c->a < 0x07);
  object_id = emu_pull();
  emu_set_a(emu_pull());
  script_table = c->a;
}

// draw_address_book (6:ADA1, far call $63): the same for page 0, whose entries are items already.
void draw_address_book(void) {
  RUN_ORIGINAL_IF_LAG(kDrawAddressBook);
  Cpu6502 *c = &g_nes.cpu;
  emu_push(script_table);
  emu_push(object_id);
  inventory_entry = 0x00;
  do {
    c->y = inventory_entry;
    emu_set_a(address_book[c->y]);
    emu_cmp(c->a, 0xFF);
    if(c->a == 0xFF) {
      break;
    }
    g_ram[0xC0] = c->a;
    draw_inventory_entry();
    RESUME_ORIGINAL_IF_LAG(kDrawAddressBook, 0xADBB, 2);
    inventory_entry++;
    emu_set_a(inventory_entry);
    emu_cmp(c->a, 0x07);
  } while(c->a < 0x07);
  object_id = emu_pull();
  emu_set_a(emu_pull());
  script_table = c->a;
}

// draw_inventory_entry (6:ADCC, far call $68): entry inventory_entry through a layout: 0 names item $C0,
// and items 0, 8 and $2D have their own, by the page. Keeps Y; returns A = Y.
void draw_inventory_entry(void) {
  RUN_ORIGINAL_IF_LAG(kDrawInventoryEntry);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  g_ram[0x9C] = 0x14;
  c->y = inventory_entry;
  emu_set_a(cpu_bus_read((uint16_t)(kEntryRows + c->y)));
  g_ram[0x9D] = c->a;
  uint8_t layout = 0x00;
  c->y = g_ram[0xC0];
  emu_cmp(c->y, 0x00);
  if(c->y == 0x00) {
    layout = 0x01;
    c->y = inventory_page;
    emu_cmp(c->y, 0x01);
    if(c->y == 0x01) {
      layout = 0x02;
    }
  } else {
    emu_cmp(c->y, 0x08);
    if(c->y == 0x08) {
      layout = 0x03;
      c->y = inventory_page;
      emu_cmp(c->y, 0x01);
      if(c->y == 0x01) {
        layout = 0x04;
        c->y = g_ram[0xB0];
        emu_cmp(c->y, 0x02);
        if(c->y != 0x02) {
          layout = 0x09;  // 6:AE12 loads 8, then a BEQ on a nonzero Y doesn't branch
          c->y = 0x07;
        }
      }
    } else {
      emu_cmp(c->y, 0x2D);
      if(c->y == 0x2D) {
        layout = 0x05;
        c->y = inventory_page;
        emu_cmp(c->y, 0x01);
        if(c->y == 0x01) {
          layout = 0x0A;
        }
      }
    }
  }
  draw_layout_number(layout);
  RESUME_ORIGINAL_IF_LAG(kDrawInventoryEntry, 0xADED, 1);
  request_vram_queue();
  c->y = emu_pull();
  emu_set_a(c->y);
}

// far_8020_6E (6:AE2B, far call $6E): the page's header, through layout 0. Keeps Y; returns A = Y.
void far_8020_6E(void) {
  RUN_ORIGINAL_IF_LAG(kFar8020_6E);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  c->y = asl_a(current_container);
  g_ram[0xAF] = container_stack[c->y];
  g_ram[0xB0] = container_stack[1 + c->y];
  g_ram[0x9C] = 0x13;
  g_ram[0x9D] = 0x03;
  inventory_page_header();
  draw_layout_number(0x00);
  RESUME_ORIGINAL_IF_LAG(kFar8020_6E, 0xAE4C, 1);
  request_vram_queue();
  c->y = emu_pull();
  emu_set_a(c->y);
}

// inventory_page_header (6:AE52): $C0 = what the header names: page 0 $BE, page 1 the container
// or $BF with none, and the rest $BD.
void inventory_page_header(void) {
  RUN_ORIGINAL_IF_LAG(kInventoryPageHeader);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(inventory_page);
  emu_cmp(c->a, 0x02);
  uint8_t page = c->a < 0x02 ? c->a : 0x02;
  emu_set_a(page);
  JUMP_TABLE_OR_ORIGINAL(kInventoryPageHeader, 0xAE5B, 0, page, 3);
  jump_table(0xAE5D, page);
  if(page == 1) {
    emu_set_a(c->y);
    emu_push(c->y);
    c->y = g_ram[0xAF];
    emu_cmp(c->y, 0xFF);
    if(c->y == 0xFF) {
      emu_set_a(0xBF);
    } else {
      emu_set_flag(kFlagC, 0);
      c->a = emu_adc(cpu_bus_read((uint16_t)(kItemNumberBases + c->y)), g_ram[0xB0]);
    }
    g_ram[0xC0] = c->a;
    c->y = emu_pull();
    emu_set_a(c->y);
    return;
  }
  emu_set_a(page == 0 ? 0xBE : 0xBD);
  g_ram[0xC0] = c->a;
}

// ---- Turning pages, and their labels ----------------------------------------------------------

// far_8000_6E (6:9CD4, far call $6E): the label of the first page and the header.
void far_8000_6E(void) {
  RUN_ORIGINAL_IF_LAG(kFar8000_6E);
  emu_set_a(0x00);
  g_ram[0x79] = 0x00;
  g_ram[0x532] = 0x00;
  g_ram[0x77] = 0x51;
  emu_set_a(0x20);
  g_ram[0x78] = 0x20;
  draw_label_string();
  RESUME_ORIGINAL_IF_LAG(kFar8000_6E, 0x9CE6, 0);
  far_8020_6E();
  RESUME_ORIGINAL_IF_LAG(kFar8000_6E, 0x9CE9, 0);
}

// far_8020_6D (6:9CEA, far call $6D): a page back, unless on page 0.
void far_8020_6D(void) {
  RUN_ORIGINAL_IF_LAG(kFar8020_6D);
  emu_set_a(inventory_page);
  if(!g_nes.cpu.a) {
    return;
  }
  draw_object_sprites();
  RESUME_ORIGINAL_IF_LAG(kFar8020_6D, 0x9CF3, 0);
  wait_vblank_and_hide_sprites();
  RESUME_ORIGINAL_IF_LAG(kFar8020_6D, 0x9CF6, 0);
  previous_page();
  RESUME_ORIGINAL_IF_LAG(kFar8020_6D, 0x9CF9, 0);
  far_8020_6E();
  RESUME_ORIGINAL_IF_LAG(kFar8020_6D, 0x9CFC, 0);
  draw_inventory_page();
  RESUME_ORIGINAL_IF_LAG(kFar8020_6D, 0x9CFF, 0);
}

// far_8000_64 (6:9D00, far call $64): the same to the address book.
void far_8000_64(void) {
  RUN_ORIGINAL_IF_LAG(kFar8000_64);
  emu_set_a(inventory_page);
  if(!g_nes.cpu.a) {
    return;
  }
  previous_page();
  RESUME_ORIGINAL_IF_LAG(kFar8000_64, 0x9D09, 0);
  far_8020_6E();
  RESUME_ORIGINAL_IF_LAG(kFar8000_64, 0x9D0C, 0);
  draw_address_book();
  RESUME_ORIGINAL_IF_LAG(kFar8000_64, 0x9D0F, 0);
}

// previous_page (6:9D10): the page number back, and the labels that go with it (6:9D38).
void previous_page(void) {
  RUN_ORIGINAL_IF_LAG(kPreviousPage);
  inventory_page--;
  g_ram[0x5C] = 0x38;
  g_ram[0x5D] = 0x9D;
  emu_set_a(0x00);
  g_ram[0x79] = 0x00;
  draw_page_strings();
}

// draw_page_strings (6:9D1F): 14 labels from the list at $5C/$5D: a low VRAM address and a number each.
void draw_page_strings(void) {
  RUN_ORIGINAL_IF_LAG(kDrawPageStrings);
  Cpu6502 *c = &g_nes.cpu;
  c->y = 0;
  do {
    g_ram[0x78] = 0x20;
    g_ram[0x77] = emu_read_ind_y(0x5C, c->y);
    c->y++;
    emu_set_a(emu_read_ind_y(0x5C, c->y));
    g_ram[0x532] = c->a;
    c->y++;
    draw_label_string();
    RESUME_ORIGINAL_IF_LAG(kDrawPageStrings, 0x9D33, 0);
    emu_cmp(c->y, 0x1C);
  } while(c->y < 0x1C);
}

// far_8020_67 (6:9D54, far call $67): a page on, unless the last has been reached.
void far_8020_67(void) {
  RUN_ORIGINAL_IF_LAG(kFar8020_67);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(inventory_page);
  emu_cmp(c->a, store_page);
  if(c->a >= store_page) {
    return;
  }
  draw_object_sprites();
  RESUME_ORIGINAL_IF_LAG(kFar8020_67, 0x9D60, 0);
  wait_vblank_and_hide_sprites();
  RESUME_ORIGINAL_IF_LAG(kFar8020_67, 0x9D63, 0);
  next_page();
  RESUME_ORIGINAL_IF_LAG(kFar8020_67, 0x9D66, 0);
  far_8020_6E();
  RESUME_ORIGINAL_IF_LAG(kFar8020_67, 0x9D69, 0);
  draw_inventory_page();
  RESUME_ORIGINAL_IF_LAG(kFar8020_67, 0x9D6C, 0);
}

// next_page (6:9D6D): the page number on, and its labels (6:9D7E).
void next_page(void) {
  RUN_ORIGINAL_IF_LAG(kNextPage);
  inventory_page++;
  g_ram[0x5C] = 0x7E;
  emu_set_a(0x9D);
  g_ram[0x5D] = 0x9D;
  g_ram[0x79] = 0x9D;
  draw_page_strings();
  RESUME_ORIGINAL_IF_LAG(kNextPage, 0x9D7D, 0);
}

// draw_label_string (6:9D9A): label $0532 (6:9EED has them) as columns of tiles of 18 bytes each
// (6:9F2D), written at once, each further left or right by $79. Keeps Y; returns A = Y.
void draw_label_string(void) {
  RUN_ORIGINAL_IF_LAG(kDrawLabelString);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  c->y = asl_a(g_ram[0x532]);
  g_ram[0x5A] = cpu_bus_read((uint16_t)(kLabelStrings + c->y));
  g_ram[0x5B] = cpu_bus_read((uint16_t)(kLabelStrings + 1 + c->y));
  c->y = 0;
  for(;;) {
    emu_set_a(emu_read_ind_y(0x5A, c->y));
    emu_cmp(c->a, 0xFF);
    if(c->a == 0xFF) {
      break;
    }
    uint8_t a = asl_a(c->a);
    g_ram[0x1E] = a;
    a = asl_a(asl_a(asl_a(a)));
    emu_set_flag(kFlagC, 0);
    g_ram[0x1E] = emu_adc(a, g_ram[0x1E]);
    emu_set_flag(kFlagC, 0);
    g_ram[0x1E] = emu_adc(kLabelColumns & 0xFF, g_ram[0x1E]);
    g_ram[0x1F] = emu_adc(kLabelColumns >> 8, 0x00);
    emu_cmp(c->y, 0x00);
    if(c->y) {
      if(g_ram[0x79]) {
        g_ram[0x77]--;
      } else {
        g_ram[0x77]++;
      }
    }
    g_ram[0x1C] = g_ram[0x77];
    emu_set_a(g_ram[0x78]);
    g_ram[0x1D] = c->a;
    queue_vram_column();
    flush_vram_queue_down();
    RESUME_ORIGINAL_IF_LAG(kDrawLabelString, 0x9DE8, 1);
    c->y++;
  }
  c->y = emu_pull();
  emu_set_a(c->y);
}

// ---- The fixed labels, and small things -------------------------------------------------------

// far_8000_6F (6:9C92), far_8000_6C (6:9CA9), far_8000_67 (6:9CBD): blocks of the game screen.
void far_8000_6F(void) {
  RUN_ORIGINAL_IF_LAG(kFar8000_6F);
  g_ram[0x1E] = 0xA6;
  g_ram[0x1F] = 0x9C;
  g_ram[0x1C] = 0x61;
  emu_set_a(0x20);
  g_ram[0x1D] = 0x20;
  queue_vram_fill();
  RESUME_ORIGINAL_IF_LAG(kFar8000_6F, 0x9CA5, 0);
}

void far_8000_6C(void) {
  RUN_ORIGINAL_IF_LAG(kFar8000_6C);
  g_ram[0x1E] = 0xEF;
  g_ram[0x1F] = 0x9D;
  g_ram[0x1C] = 0x82;
  emu_set_a(0x22);
  g_ram[0x1D] = 0x22;
  queue_vram_block();
  RESUME_ORIGINAL_IF_LAG(kFar8000_6C, 0x9CBC, 0);
}

void far_8000_67(void) {
  RUN_ORIGINAL_IF_LAG(kFar8000_67);
  g_ram[0x1E] = 0xD1;
  g_ram[0x1F] = 0x9C;
  g_ram[0x1C] = 0xF1;
  emu_set_a(0x22);
  g_ram[0x1D] = 0x22;
  queue_vram_fill();
  RESUME_ORIGINAL_IF_LAG(kFar8000_67, 0x9CD0, 0);
}

// far_8020_69 (6:AE8D, far call $69): the door flags listed at 6:AE9F all set. Keeps X; returns A = X.
void far_8020_69(void) {
  RUN_ORIGINAL_IF_LAG(kFar8020_69);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->x);
  emu_push(c->x);
  c->x = 0;
  do {
    set_door_flag(cpu_bus_read((uint16_t)(kDoorFlagsToSet + c->x)));
    c->x++;
    emu_cmp(c->x, 0x23);
  } while(c->x < 0x23);
  c->x = emu_pull();
  emu_set_a(c->x);
}

// The end of delete_inventory_entry, which far_8020_6C jumps to (6:AEC6): removes the slot under the
// page and entry, moving the later ones down, up to slot $84. Its end pulls X and Y, but doesn't keep
// them: PLA, TXA, PLA, TYA, so X is the last index used and A = Y.
static void delete_entry_up_to_84(void) {
  Cpu6502 *c = &g_nes.cpu;
  TAIL_ORIGINAL_IF_LAG(kDeleteInventoryEntry, 0xAEC6, 0);
  emu_set_a(c->y);
  emu_push(c->y);
  emu_set_a(c->x);
  emu_push(c->x);
  inventory_slot_index();
  c->x = c->y;
  c->x++;
  for(;;) {
    emu_cmp(c->x, g_ram[0x84]);
    if(c->x >= g_ram[0x84]) {
      break;
    }
    emu_count_work(1);
    emu_set_a(address_book[c->x]);
    address_book[c->y] = c->a;
    c->y++;
    c->x++;
  }
  emu_set_a(0xFF);
  address_book[c->y] = 0xFF;
  emu_pull();
  emu_pull();
  emu_set_a(c->y);
}

// delete_inventory_entry (6:AEC2, far call $6B): up to the end of the goods.
void delete_inventory_entry(void) {
  RUN_ORIGINAL_IF_LAG(kDeleteInventoryEntry);
  emu_set_a(0x54);
  g_ram[0x84] = 0x54;
  delete_entry_up_to_84();
}

// far_8020_6C (6:AEE9, far call $6C): up to the end of the coat page.
void far_8020_6C(void) {
  RUN_ORIGINAL_IF_LAG(kFar8020_6C);
  emu_set_a(0x0E);
  g_ram[0x84] = 0x0E;
  delete_entry_up_to_84();
}

// far_8000_6D (6:804C, far call $6D): sprite 0 for the split, at 8, $98, and the split on.
void far_8000_6D(void) {
  RUN_ORIGINAL_IF_LAG(kFar8000_6D);
  g_ram[0x20] = 0x08;
  g_ram[0x21] = 0x98;
  g_ram[0x1E] = kSplitSprite & 0xFF;
  g_ram[0x1F] = kSplitSprite >> 8;
  emu_set_a(0x03);
  g_ram[0x22] = 0x03;
  draw_metasprite();
  request_oam_dma();
  emu_set_a(system_flags | 0x01);
  system_flags = g_nes.cpu.a;
}

// far_8000_63 (6:81F7): palette scene_palette_id, faded in.
void far_8000_63(void) {
  RUN_ORIGINAL_IF_LAG(kFar8000_63);
  palette_pointer();
  fade_in_palette();
  RESUME_ORIGINAL_IF_LAG(kFar8000_63, 0x81FD, 0);
}
