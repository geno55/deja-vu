// A room's hotspots (docs/rooms.md): what a click on the picture picks,
// the default command for it, and the objects shown as sprites. And the item codes.
//
// The records are in bank 2, 6 bytes each: x1 x2 y1 y2 kind id. Bit 7 of kind says the object is
// shown as a sprite. $6B/$6C point to where the pick goes: the target (script_table, object_id), or
// the choice (switch_key) while a script waits for one. $67/$68 point into object_sprite_list.
//
// A glitched room number (TAS #7044) walks these loops through far more than a list, for many
// frames: the NMIs come in the routines they call and in their own code, so they check whether to
// go on as original code on every pass.
#include "funcs.h"
#include "variables.h"

// ---- What a click on the picture picks -----------------------------------------------------

// What find_clicked_hotspot does after a hotspot's check.
enum {
  kHotspotPicked,    // the check stored the id: it returns
  kHotspotNoTarget,  // 7:C318: the kind stored goes back to 0, and on to the next hotspot
  kHotspotPassed,    // 7:C31D: on to the next hotspot
};

// LDA (hotspot_ptr),Y, with Y = 5 where the checks use it.
static uint8_t hotspot_id(void) {
  emu_set_a(emu_read_ind_y(0xA8, g_nes.cpu.y));
  return g_nes.cpu.a;
}

static int scene_is(uint8_t s) {
  emu_set_a(scene);
  emu_cmp(g_nes.cpu.a, s);
  return g_nes.cpu.a == s;
}

static int scene_below(uint8_t s) {
  emu_set_a(scene);
  emu_cmp(g_nes.cpu.a, s);
  return g_nes.cpu.a < s;
}

// 7:C377: the id goes in the pick's second byte.
static int pick_hotspot_id(void) {
  Cpu6502 *c = &g_nes.cpu;
  hotspot_id();
  c->y = 1;
  emu_set_nz(c->y);
  emu_write_ind_y(0x6B, 1, c->a);
  return kHotspotPicked;
}

// Kind 0 (7:C365): four ids only count in certain scenes.
static int check_kind_0(void) {
  uint8_t id = hotspot_id();
  int counts = 1;
  emu_cmp(id, 0x6B);
  if(id == 0x6B) {
    counts = scene_is(0x73);
  } else {
    emu_cmp(id, 0x5F);
    if(id == 0x5F) {
      counts = scene_is(0x7A);
    } else {
      emu_cmp(id, 0x2E);
      if(id == 0x2E) {
        counts = scene_is(0x40) || scene_is(0x46) || scene_is(0x49) || scene_is(0x4C);
      } else {
        emu_cmp(id, 0x13);
        if(id == 0x13) {
          counts = scene_is(0x12);
        }
      }
    }
  }
  return counts ? pick_hotspot_id() : kHotspotPassed;
}

// Kind 3 (7:C401): the id is a story flag, which must be clear; flag $2C only counts in scene $44.
static int check_kind_3(void) {
  Cpu6502 *c = &g_nes.cpu;
  uint8_t id = hotspot_id();
  emu_cmp(id, 0x2C);
  if(id == 0x2C && !scene_is(0x44)) {
    return kHotspotNoTarget;
  }
  id = hotspot_id();
  emu_set_flag(kFlagC, id & 0x04);  // the last LSR
  g_ram[0x69] = id & 0x07;
  c->y = g_ram[0x69];
  emu_set_a(story_flags[id >> 3] & cpu_bus_read((uint16_t)(kBitMasks + c->y)));
  if(c->a) {
    return kHotspotNoTarget;
  }
  c->y = 5;
  hotspot_id();
  c->y = 1;
  emu_set_nz(c->y);
  emu_write_ind_y(0x6B, 1, c->a);
  return kHotspotPicked;
}

// Kind 4 (7:C433): the id is an item, not obtained yet; items 0 and 2 only count in scene 0.
static int check_kind_4(void) {
  Cpu6502 *c = &g_nes.cpu;
  uint8_t id = hotspot_id();
  emu_cmp(id, 0x00);
  if(id != 0x00) {
    emu_cmp(id, 0x02);
  }
  if((id == 0x00 || id == 0x02) && !scene_is(0x00)) {
    return kHotspotNoTarget;
  }
  id = hotspot_id();
  c->y = id;
  emu_set_a(item_status[c->y] & 0x01);
  if(c->a) {
    return kHotspotNoTarget;
  }
  emu_set_a(c->y);
  c->y = 1;
  emu_set_nz(c->y);
  emu_write_ind_y(0x6B, 1, c->a);
  return kHotspotPicked;
}

// Kind 5 (7:C3D8): nine ids only count in certain scenes.
static int check_kind_5(void) {
  uint8_t id = hotspot_id();
  int counts;
  switch(id) {
    case 0x24: counts = scene_below(0x5E); break;
    case 0x25: counts = scene_below(0x68); break;
    case 0x26: counts = scene_below(0x6D); break;
    case 0x27: counts = scene_is(0x77); break;
    case 0x29: counts = !scene_below(0x5E); break;
    case 0x2A: counts = !scene_below(0x68); break;
    case 0x2B: counts = !scene_below(0x6D); break;
    case 0x2C: counts = scene_is(0x78); break;
    case 0x47: counts = scene_is(0x43); break;
    default:
      emu_cmp(id, 0x47);
      counts = 1;
      break;
  }
  return counts ? pick_hotspot_id() : kHotspotNoTarget;
}

// click_picture (7:C2A8): which hotspot was clicked, in bank 2, and the command for it.
void click_picture(void) {
  RUN_ORIGINAL_IF_LAG(kClickPicture);
  push_and_switch_bank(0x02);
  find_clicked_hotspot();
  pop_bank();
  pick_default_command();
}

// find_clicked_hotspot (7:C2B7): the room's hotspots in order. The first the cursor is in whose
// check passes sets the target or the choice to its kind and id; with none, it is 0 and $6F.
void find_clicked_hotspot(void) {
  RUN_ORIGINAL_IF_LAG(kFindClickedHotspot);
  Cpu6502 *c = &g_nes.cpu;
  g_ram[0x6B] = 0xAA;
  g_ram[0x6C] = 0x00;
  test_event_flag(0x80);
  emu_set_a(script_flags & 0x04);
  if(c->a) {
    g_ram[0x6B] = 0xAD;
    g_ram[0x6C] = 0x00;
  }
  c->y = 0;
  emu_set_a(0x00);
  emu_write_ind_y(0x6B, 0, 0x00);
  c->y = 1;
  emu_set_a(0x6F);
  emu_write_ind_y(0x6B, 1, 0x6F);
  c->y = asl_a(room);
  hotspot_ptr[0] = cpu_bus_read((uint16_t)(kRoomHotspots + c->y));
  hotspot_ptr[1] = cpu_bus_read((uint16_t)(kRoomHotspots + 1 + c->y));
  for(;;) {
    c->y = 0;
    emu_set_a(emu_read_ind_y(0xA8, 0));
    emu_cmp(c->a, 0xFF);
    if(c->a == 0xFF) {
      return;
    }
    test_hotspot();
    RESUME_ORIGINAL_IF_LAG(kFindClickedHotspot, 0xC2F7, 0);
    emu_set_a(script_flags & 0x01);
    if(c->a) {
      c->y = 4;
      uint8_t kind = emu_read_ind_y(0xA8, 4) & 0x7F;
      emu_set_a(kind);
      c->y = 0;
      emu_write_ind_y(0x6B, 0, kind);
      c->y = 5;
      JUMP_TABLE_OR_ORIGINAL(kFindClickedHotspot, 0xC309, 0, kind, 6);
      jump_table(0xC30B, kind);
      int next;
      switch(kind) {
        case 0: next = check_kind_0(); break;
        case 3: next = check_kind_3(); break;
        case 4: next = check_kind_4(); break;
        case 5: next = check_kind_5(); break;
        default: next = pick_hotspot_id(); break;
      }
      if(next == kHotspotPicked) {
        return;
      }
      if(next == kHotspotNoTarget) {
        c->y = 0;
        emu_set_a(0x00);
        emu_write_ind_y(0x6B, 0, 0x00);
      }
    }
    next_hotspot();
    RESUME_ORIGINAL_IF_LAG(kFindClickedHotspot, 0xC320, 0);
  }
}

// pick_default_command (7:CB08): with no script waiting and no command chosen, sets event flag $BF
// and the command: $0A or 3 for a door, by its flag, 0 for the rest.
void pick_default_command(void) {
  RUN_ORIGINAL_IF_LAG(kPickDefaultCommand);
  Cpu6502 *c = &g_nes.cpu;
  test_event_flag(0x80);
  emu_set_a(script_flags & 0x04);
  if(c->a) {
    return;
  }
  emu_set_a(command);
  emu_cmp(c->a, 0xFF);
  if(c->a != 0xFF) {
    return;
  }
  set_event_flag(0xBF);
  uint8_t table = script_table;
  emu_set_a(table);
  JUMP_TABLE_OR_ORIGINAL(kPickDefaultCommand, 0xCB22, 0, table, 6);
  jump_table(0xCB24, table);
  if(table == 5) {
    test_door_flag(object_id);
    emu_set_a(script_flags & 0x04);
    emu_set_a(c->a ? 0x0A : 0x03);
  } else {
    emu_set_a(0x00);
  }
  command = c->a;
}

// ---- The objects shown as sprites ----------------------------------------------------------

// collect_room_sprites (7:C462): object_sprite_list from the room's hotspots with bit 7 of the kind
// set ($BD the rest of the kind, $BE the id, added by a far call), ended by $FF.
void collect_room_sprites(void) {
  RUN_ORIGINAL_IF_LAG(kCollectRoomSprites);
  Cpu6502 *c = &g_nes.cpu;
  push_and_switch_bank(0x02);
  c->y = asl_a(room);
  hotspot_ptr[0] = cpu_bus_read((uint16_t)(kRoomHotspots + c->y));
  hotspot_ptr[1] = cpu_bus_read((uint16_t)(kRoomHotspots + 1 + c->y));
  g_ram[0x67] = 0xC5;
  g_ram[0x68] = 0x00;
  for(;;) {
    c->y = 0;
    emu_set_a(emu_read_ind_y(0xA8, 0));
    emu_cmp(c->a, 0xFF);
    if(c->a == 0xFF) {
      emu_write_ind_y(0x67, 0, 0xFF);
      pop_bank();
      return;
    }
    c->y = 4;
    emu_set_a(emu_read_ind_y(0xA8, 4));
    if(c->a & 0x80) {
      g_ram[0xBD] = c->a & 0x7F;
      c->y = 5;
      emu_set_a(emu_read_ind_y(0xA8, 5));
      g_ram[0xBE] = c->a;
      far_call_8020(0x60);
      RESUME_ORIGINAL_IF_LAG(kCollectRoomSprites, 0xC4A8, 0);
      next_hotspot();
      RESUME_ORIGINAL_IF_LAG(kCollectRoomSprites, 0xC4AB, 0);
    } else {
      next_hotspot();
      RESUME_ORIGINAL_IF_LAG(kCollectRoomSprites, 0xC497, 0);
    }
  }
}

// ---- Item codes ----------------------------------------------------------------------------

// item_code_for_object (7:C520): $C1 = the target as one byte: object_id, with a number for its
// script table (7:C551) in the top 3 bits. Keeps Y; returns A = Y.
void item_code_for_object(void) {
  RUN_ORIGINAL_IF_LAG(kItemCodeForObject);
  Cpu6502 *c = &g_nes.cpu;
  emu_push(c->y);
  c->y = script_table;
  uint8_t a = cpu_bus_read((uint16_t)(kItemCodeTableNumbers + c->y));
  a = asl_a(asl_a(asl_a(asl_a(asl_a(a)))));
  g_ram[0xC1] = a | object_id;
  c->y = emu_pull();
  emu_set_a(c->y);
}

// split_item_code (7:C533): the other way, from A: $BD the script table (7:C557), $BE the object.
// With table 3 the top bits stay in $BE. Keeps Y; returns A = Y.
void split_item_code(uint8_t code) {
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(code);
  RUN_ORIGINAL_IF_LAG(kSplitItemCode);
  g_ram[0xBE] = code;
  emu_push(c->y);
  c->y = code >> 5;
  emu_set_a(cpu_bus_read((uint16_t)(kItemCodeTables + c->y)));
  g_ram[0xBD] = c->a;
  emu_cmp(c->a, 0x03);
  if(c->a != 0x03) {
    g_ram[0xBE] = code & 0x1F;
  }
  c->y = emu_pull();
  emu_set_a(c->y);
}
