// What a room looks like (bank 6): which scene it shows, the tile patches
// over the picture, and the objects drawn as sprites. Each room has a list at 6:9023 of pairs: a
// place in the picture, and a door flag or a rule for it.
//
// The scratch bytes, as the originals use them: $5E/$5F the room's list, $66 the variant of the
// scene, $67 the list's size here and elsewhere (with $68) where in object_sprite_list the next
// sprite goes, $63/$64 the record in an object table, $BD a kind and $BE an object, bit 5 of
// script_flags that a patch is hidden.
#include "funcs.h"
#include "variables.h"

// LDA script_flags, AND #$04: the script condition, in A and as the result.
static uint8_t condition(void) {
  emu_set_a(script_flags & 0x04);
  return g_nes.cpu.a;
}

// ---- The scene -----------------------------------------------------------------------------

// room_scene_list (6:900C): $5E/$5F = the room's list.
void room_scene_list(void) {
  RUN_ORIGINAL_IF_LAG(kRoomSceneList);
  Cpu6502 *c = &g_nes.cpu;
  c->y = asl_a(room);
  g_ram[0x5E] = cpu_bus_read((uint16_t)(kRoomSceneLists + c->y));
  emu_set_a(cpu_bus_read((uint16_t)(kRoomSceneLists + 1 + c->y)));
  g_ram[0x5F] = c->a;
}

// select_room_scene (6:9345, far call $61): scene = the room's scene for its variant. Keeps X and Y;
// returns A = X.
void select_room_scene(void) {
  RUN_ORIGINAL_IF_LAG(kSelectRoomScene);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->x);
  emu_push(c->x);
  emu_set_a(c->y);
  emu_push(c->y);
  pick_room_variant();
  c->y = asl_a(room);
  g_ram[0x5E] = cpu_bus_read((uint16_t)(kRoomScenes + c->y));
  g_ram[0x5F] = cpu_bus_read((uint16_t)(kRoomScenes + 1 + c->y));
  c->y = g_ram[0x66];
  scene = emu_read_ind_y(0x5E, c->y);
  c->y = emu_pull();
  c->x = emu_pull();
  emu_set_a(c->x);
}

// pick_room_variant (6:9367): $66 = a bit for each door flag set among the first entries of the
// room's list (6:9522 has how many bytes of it).
void pick_room_variant(void) {
  RUN_ORIGINAL_IF_LAG(kPickRoomVariant);
  Cpu6502 *c = &g_nes.cpu;
  room_scene_list();
  c->y = 0;
  g_ram[0x66] = 0x00;
  c->x = room;
  emu_set_a(cpu_bus_read((uint16_t)(kRoomVariantSizes + c->x)));
  g_ram[0x67] = c->a;
  if(!c->a) {
    return;
  }
  do {
    emu_read_ind_y(0x5E, c->y);
    c->y++;
    test_door_flag(emu_read_ind_y(0x5E, c->y) & 0x7F);
    c->y--;
    if(condition()) {
      c->x = c->y >> 1;
      emu_set_flag(kFlagC, 0);
      g_ram[0x66] = emu_adc(g_ram[0x66], cpu_bus_read((uint16_t)(kBitMasks + c->x)));
      c->a = g_ram[0x66];
    }
    c->y += 2;
    emu_cmp(c->y, g_ram[0x67]);
  } while(c->y < g_ram[0x67]);
}

// ---- Tile patches --------------------------------------------------------------------------

// draw_room_tile_patches (6:9FBD, far call $62): the patches in the room's list that aren't hidden.
void draw_room_tile_patches(void) {
  RUN_ORIGINAL_IF_LAG(kDrawRoomTilePatches);
  Cpu6502 *c = &g_nes.cpu;
  room_scene_list();
  for(;;) {
    emu_set_a(0x00);
    g_ram[0x1D] = 0x00;
    c->y = 0;
    emu_set_a(emu_read_ind_y(0x5E, 0));
    emu_cmp(c->a, 0xFF);
    if(c->a == 0xFF) {
      return;
    }
    if(!(c->p & kFlagN)) {
      tile_patch_hidden();
      RESUME_ORIGINAL_IF_LAG(kDrawRoomTilePatches, 0x9FD2, 0);
      emu_set_a(script_flags & 0x20);
      if(!c->a) {
        c->y = 0;
        tile_patch_vram_address(emu_read_ind_y(0x5E, 0));
        g_ram[0x1E] = 0x95;
        emu_set_a(0xA0);
        g_ram[0x1F] = 0xA0;
        queue_vram_block();
        RESUME_ORIGINAL_IF_LAG(kDrawRoomTilePatches, 0x9FEA, 0);
      }
    }
    add16(g_ram + 0x5E, 0x02);
  }
}

// tile_patch_hidden (6:9FFA): bit 5 of script_flags = the patch's door flag says to hide it: with bit
// 7 of its second byte, the rest is that flag.
void tile_patch_hidden(void) {
  RUN_ORIGINAL_IF_LAG(kTilePatchHidden);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(script_flags & 0xDF);
  script_flags = c->a;
  c->y = 1;
  emu_set_a(emu_read_ind_y(0x5E, 1));
  if(c->a & 0x80) {
    TAIL_ORIGINAL_IF_LAG(kTilePatchRoomRules, 0xA03D, 0);
    test_door_flag(c->a & 0x7F);
    if(!condition()) {
      emu_set_a(script_flags | 0x20);
      script_flags = c->a;
    }
    return;
  }
  tile_patch_hidden_alt();
}

// tile_patch_hidden_alt (6:A00A): keeps Y; returns A = Y.
void tile_patch_hidden_alt(void) {
  RUN_ORIGINAL_IF_LAG(kTilePatchHiddenAlt);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->y);
  emu_push(c->y);
  tile_patch_room_rules();
  c->y = emu_pull();
  emu_set_a(c->y);
}

// tile_patch_room_rules (6:A012): bit 5 of script_flags = the patch's second byte is $24-$27 and door
// flag $0D is set, or $29-$2C and it is clear.
void tile_patch_room_rules(void) {
  RUN_ORIGINAL_IF_LAG(kTilePatchRoomRules);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(script_flags & 0xDF);
  script_flags = c->a;
  c->y = 1;
  uint8_t rule = emu_read_ind_y(0x5E, 1);
  emu_set_a(rule);
  int hide_if_set = rule >= 0x24 && rule <= 0x27;
  int hide_if_clear = rule >= 0x29 && rule <= 0x2C;
  if(!hide_if_set && !hide_if_clear) {
    emu_cmp(rule, 0x2C);
    return;
  }
  test_door_flag(0x0D);
  if(condition() ? hide_if_set : hide_if_clear) {
    emu_set_a(script_flags | 0x20);
    script_flags = c->a;
  }
}

// tile_patch_vram_address (6:A068): $1C/$1D from the patch's place: the row in the low nibble, the
// column in the high one, from $22F1.
void tile_patch_vram_address(uint8_t place) {
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(place);
  RUN_ORIGINAL_IF_LAG(kTilePatchVramAddress);
  emu_push(place);
  uint8_t low = asl_a(asl_a(asl_a(asl_a(asl_a(place & 0x0F)))));
  g_ram[0x1D] = (uint8_t)(g_ram[0x1D] << 1 | (c->p & kFlagC));
  g_ram[0x1C] = low;
  emu_set_flag(kFlagC, 0);
  g_ram[0x1C] = emu_adc(emu_pull() >> 4, g_ram[0x1C]);
  g_ram[0x1D] = emu_adc(g_ram[0x1D], 0x00);
  emu_set_flag(kFlagC, 0);
  g_ram[0x1C] = emu_adc(0xF1, g_ram[0x1C]);
  g_ram[0x1D] = emu_adc(0x22, g_ram[0x1D]);
  c->a = g_ram[0x1D];
}

// ---- Object sprites ------------------------------------------------------------------------

// draw_object_sprites (6:A808, far call $65): each of object_sprite_list, up to $FF.
void draw_object_sprites(void) {
  RUN_ORIGINAL_IF_LAG(kDrawObjectSprites);
  Cpu6502 *c = &g_nes.cpu;
  c->y = 0;
  for(;;) {
    emu_set_a(object_sprite_list[c->y]);
    emu_cmp(c->a, 0xFF);
    if(c->a == 0xFF) {
      return;
    }
    g_ram[0x20] = c->a;
    g_ram[0x21] = object_sprite_list[1 + c->y];
    draw_object_sprite(object_sprite_list[2 + c->y]);
    RESUME_ORIGINAL_IF_LAG(kDrawObjectSprites, 0xA81E, 0);
    c->y += 3;
  }
}

// draw_object_sprite (6:A098): sprite A at $20, $21, from its attributes and metasprite in one of two
// tables, by bit 7. $FF draws nothing. Keeps X and Y; returns A = X.
void draw_object_sprite(uint8_t sprite) {
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(sprite);
  RUN_ORIGINAL_IF_LAG(kDrawObjectSprite);
  emu_cmp(sprite, 0xFF);
  if(sprite == 0xFF) {
    return;
  }
  data_ptr[0] = sprite;
  emu_set_a(c->x);
  emu_push(c->x);
  emu_set_a(c->y);
  emu_push(c->y);
  uint16_t table = sprite & 0x80 ? kObjectSpritesHigh : kObjectSpritesLow;
  c->x = asl_a(sprite & 0x7F);
  g_ram[0x22] = cpu_bus_read((uint16_t)(table + 1 + c->x));
  c->x = asl_a(cpu_bus_read((uint16_t)(table + c->x)));
  g_ram[0x1E] = cpu_bus_read((uint16_t)(kMetaspritePointers + c->x));
  emu_set_a(cpu_bus_read((uint16_t)(kMetaspritePointers + 1 + c->x)));
  g_ram[0x1F] = c->a;
  draw_metasprite();
  c->y = emu_pull();
  c->x = emu_pull();
  emu_set_a(c->x);
}

// add_object_sprites (6:A6CC, far call $60): the records of table $BD for object $BE that are
// visible now go on object_sprite_list.
void add_object_sprites(void) {
  RUN_ORIGINAL_IF_LAG(kAddObjectSprites);
  Cpu6502 *c = &g_nes.cpu;
  c->y = asl_a(g_ram[0xBD]);
  g_ram[0x63] = cpu_bus_read((uint16_t)(kObjectSpriteTables + c->y));
  g_ram[0x64] = cpu_bus_read((uint16_t)(kObjectSpriteTables + 1 + c->y));
  c->y = 0;
  for(;;) {
    emu_set_a(emu_read_ind_y(0x63, c->y));
    emu_cmp(c->a, 0xFF);
    if(c->a == 0xFF) {
      TAIL_ORIGINAL_IF_LAG(kNextObjectSpriteRecord, 0xA6FC, 0);
      return;
    }
    emu_cmp(c->a, g_ram[0xBE]);
    if(c->a == g_ram[0xBE]) {
      object_sprite_visible();
      RESUME_ORIGINAL_IF_LAG(kAddObjectSprites, 0xA6E9, 0);
    }
    next_object_sprite_record();
    RESUME_ORIGINAL_IF_LAG(kAddObjectSprites, 0xA6EC, 0);
  }
}

// next_object_sprite_record (6:A6EF)
void next_object_sprite_record(void) {
  RUN_ORIGINAL_IF_LAG(kNextObjectSpriteRecord);
  add16(g_ram + 0x63, 0x04);
}

// append_object_sprite (6:A738): the record's three bytes after Y, to $67/$68, which moves on.
// Returns Y + 3.
void append_object_sprite(void) {
  RUN_ORIGINAL_IF_LAG(kAppendObjectSprite);
  Cpu6502 *c = &g_nes.cpu;
  for(int i = 0; i < 3; i++) {
    c->y++;
    emu_set_a(emu_read_ind_y(0x63, c->y));
    c->y--;
    emu_write_ind_y(0x67, c->y, c->a);
    c->y++;
  }
  add16(g_ram + 0x67, 0x03);
}

// The checks of sprite_value_is_0A_or_16 (6:A75B), sprite_value_is_2C (6:A7A6) and
// sprite_value_is_0_or_2 (6:A7DE): the condition = the record's value is one of these two, and the
// scene isn't the one where it counts.
static void value_outside_scene(uint8_t one, uint8_t other, uint8_t its_scene) {
  Cpu6502 *c = &g_nes.cpu;
  script_condition_false();
  emu_set_a(emu_read_ind_y(0x63, c->y));
  emu_cmp(c->a, one);
  if(c->a != one) {
    emu_cmp(c->a, other);
    if(c->a != other) {
      return;
    }
  }
  emu_set_a(scene);
  emu_cmp(c->a, its_scene);
  if(c->a != its_scene) {
    script_condition_true();
  }
}

void sprite_value_is_0A_or_16(void) {
  RUN_ORIGINAL_IF_LAG(kSpriteValueIs0AOr16);
  value_outside_scene(0x0A, 0x16, 0x73);
}

void sprite_value_is_2C(void) {
  RUN_ORIGINAL_IF_LAG(kSpriteValueIs2C);
  value_outside_scene(0x2C, 0x2C, 0x44);
}

void sprite_value_is_0_or_2(void) {
  RUN_ORIGINAL_IF_LAG(kSpriteValueIs0Or2);
  value_outside_scene(0x00, 0x02, 0x00);
}

// object_sprite_condition (6:A70F): whether the record at $63/$64 is shown now, by its kind $BD, and if
// so it goes on the list.
static void object_sprite_condition(void) {
  Cpu6502 *c = &g_nes.cpu;
  uint8_t kind = g_ram[0xBD];
  emu_set_a(kind);
  JUMP_TABLE_OR_ORIGINAL(kObjectSpriteCondition, 0xA711, 0, kind, 5);
  jump_table(0xA713, kind);
  int show = 1;
  switch(kind) {
    case 0:
      c->y += 3;
      test_event_flag(emu_read_ind_y(0x63, c->y));
      show = condition() != 0;
      if(show) {
        sprite_value_is_0A_or_16();
        show = !condition();
        if(show) {
          c->y -= 3;
        }
      }
      break;
    case 1:
      test_object_open(emu_read_ind_y(0x63, c->y));
      show = condition() != 0;
      break;
    case 3:
      test_story_flag(emu_read_ind_y(0x63, c->y));
      show = !condition();
      if(show) {
        test_flag_045C(emu_read_ind_y(0x63, c->y));
        show = !condition();
      }
      if(show) {
        sprite_value_is_2C();
        show = !condition();
      }
      break;
    default:
      test_item_obtained(emu_read_ind_y(0x63, c->y));
      show = !condition();
      if(show) {
        test_flag_047A(emu_read_ind_y(0x63, c->y));
        show = !condition();
      }
      if(show) {
        sprite_value_is_0_or_2();
        show = !condition();
      }
      break;
  }
  if(show) {
    append_object_sprite();
  }
}

// object_sprite_visible (6:A707): keeps Y; returns A = Y.
//
// Kind 2's check (6:A7F7) ends PLA, TXA, RTS, a byte short of what it should drop, so it would return
// into zero page with Y as the high byte. No room has such an object (the hotspot kinds with bit 7
// are $80, $81, $83 and $84), so that runs as original code.
void object_sprite_visible(void) {
  Cpu6502 *c = &g_nes.cpu;
  if(g_ram[0xBD] == 2 || emu_lag_in(kObjectSpriteVisible) || emu_lag_in(kObjectSpriteCondition)) {
    emu_call_original(kObjectSpriteVisible);
    return;
  }
  emu_set_a(c->y);
  emu_push(c->y);
  object_sprite_condition();
  c->y = emu_pull();
  emu_set_a(c->y);
}
