// The special scenes: a picture shown for a moment over the room, with
// sounds or a message, for script opcodes $46 and $5A. And the waits they use. Which scene is which
// in the game isn't mapped yet; the comments say what the code does.
//
// Sound 4 and the others are play_sound's numbers. Most of this
// waits for the NMI, so after such a call it checks whether to go on as original code; the handlers
// the jump tables reach start with TAIL_ORIGINAL_IF_LAG, as special_scene_extras jumps to some.
#include "funcs.h"
#include "variables.h"

// ---- Waits ---------------------------------------------------------------------------------

// wait_180_frames (7:C7E4)
void wait_180_frames(void) {
  RUN_ORIGINAL_IF_LAG(kWait180Frames);
  wait_60_frames();
  RESUME_ORIGINAL_IF_LAG(kWait180Frames, 0xC7E7, 0);
  wait_60_frames();
  RESUME_ORIGINAL_IF_LAG(kWait180Frames, 0xC7EA, 0);
  wait_60_frames();
  RESUME_ORIGINAL_IF_LAG(kWait180Frames, 0xC7ED, 0);
}

// wait_60_frames (7:C7EE)
void wait_60_frames(void) {
  RUN_ORIGINAL_IF_LAG(kWait60Frames);
  wait_30_frames();
  RESUME_ORIGINAL_IF_LAG(kWait60Frames, 0xC7F1, 0);
  wait_30_frames();
  RESUME_ORIGINAL_IF_LAG(kWait60Frames, 0xC7F4, 0);
}

// wait_30_frames (7:C7F5): returns Y = $1E.
void wait_30_frames(void) {
  RUN_ORIGINAL_IF_LAG(kWait30Frames);
  Cpu6502 *c = &g_nes.cpu;
  c->y = 0;
  do {
    wait_vblank();
    RESUME_ORIGINAL_IF_LAG(kWait30Frames, 0xC7FA, 0);
    c->y++;
    emu_cmp(c->y, 0x1E);
  } while(c->y != 0x1E);
}

// ---- Showing one ---------------------------------------------------------------------------

// run_special_scene (7:C55D): the scene special_scene names, its action, and back; the sound in
// repeat_sound again after. Keeps A, X and Y.
void run_special_scene(void) {
  RUN_ORIGINAL_IF_LAG(kRunSpecialScene);
  Cpu6502 *c = &g_nes.cpu;
  emu_push(c->a);
  emu_set_a(c->x);
  emu_push(c->x);
  emu_set_a(c->y);
  emu_push(c->y);
  emu_set_a(repeat_sound);
  emu_push(c->a);
  enter_special_scene();
  RESUME_ORIGINAL_IF_LAG(kRunSpecialScene, 0xC568, 4);
  special_scene_action();
  RESUME_ORIGINAL_IF_LAG(kRunSpecialScene, 0xC56B, 4);
  leave_special_scene();
  RESUME_ORIGINAL_IF_LAG(kRunSpecialScene, 0xC56E, 4);
  play_sound(emu_pull());
  c->y = emu_pull();
  c->x = emu_pull();
  emu_set_a(emu_pull());
}

// The saved scene, and the picture of the special one, as enter_special_scene and
// show_special_scene both do it. Returns X = special_scene.
static void set_special_picture(void) {
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(scene);
  saved_scene = c->a;
  c->x = special_scene;
  emu_set_a(cpu_bus_read((uint16_t)(kSpecialScenePictures + c->x)));
  scene = c->a;
}

// enter_special_scene (7:C735): no object sprites while it is up.
void enter_special_scene(void) {
  RUN_ORIGINAL_IF_LAG(kEnterSpecialScene);
  set_special_picture();
  change_scene();
  RESUME_ORIGINAL_IF_LAG(kEnterSpecialScene, 0xC746, 0);
  emu_set_a(0xFF);
  object_sprite_list[0] = 0xFF;
}

// show_special_scene (7:C74B): for op_5A, which stays there: with the scene's extras. Keeps X and
// Y; returns A = X.
void show_special_scene(void) {
  RUN_ORIGINAL_IF_LAG(kShowSpecialScene);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(c->x);
  emu_push(c->x);
  emu_set_a(c->y);
  emu_push(c->y);
  set_special_picture();
  change_scene();
  RESUME_ORIGINAL_IF_LAG(kShowSpecialScene, 0xC760, 2);
  emu_set_a(0xFF);
  object_sprite_list[0] = 0xFF;
  special_scene_extras();
  RESUME_ORIGINAL_IF_LAG(kShowSpecialScene, 0xC767, 2);
  c->y = emu_pull();
  c->x = emu_pull();
  emu_set_a(c->x);
}

// leave_special_scene (7:C76C)
void leave_special_scene(void) {
  RUN_ORIGINAL_IF_LAG(kLeaveSpecialScene);
  emu_set_a(saved_scene);
  scene = g_nes.cpu.a;
  change_scene();
  RESUME_ORIGINAL_IF_LAG(kLeaveSpecialScene, 0xC775, 0);
  update_cursor_and_sprites();
  RESUME_ORIGINAL_IF_LAG(kLeaveSpecialScene, 0xC778, 0);
}

// ---- What each does ------------------------------------------------------------------------

// 7:C5E3, 7:C5F4, 7:C605: sound 0, a second, another sound, and a wait.
static void scene_sound_20(void) {
  TAIL_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC5E3, 0);
  play_sound(0x00);
  wait_60_frames();
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC5EB, 0);
  play_sound(0x20);
  wait_60_frames();
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC5F3, 0);
}

static void scene_sound_1A(void) {
  TAIL_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC5F4, 0);
  play_sound(0x00);
  wait_60_frames();
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC5FC, 0);
  play_sound(0x1A);
  wait_60_frames();
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC604, 0);
}

static void scene_sound_1F(void) {
  TAIL_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC605, 0);
  play_sound(0x00);
  wait_60_frames();
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC60D, 0);
  play_sound(0x1F);
  wait_180_frames();
}

// 7:C615: the message depends on event flags $78 and $B2.
static void scene_message_by_flags(void) {
  Cpu6502 *c = &g_nes.cpu;
  TAIL_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC615, 0);
  play_sound(0x0B);
  set_special_scene_flag();
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC61D, 0);
  test_event_flag(0x78);
  emu_set_a(script_flags & 0x04);
  if(c->a) {
    emu_set_a(0xFC);
    print_message_2_or_5();
    RESUME_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC64A, 0);
    clear_special_scene_flag();
    RESUME_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC64D, 0);
    return;
  }
  test_event_flag(0xB2);
  emu_set_a(script_flags & 0x04);
  if(c->a) {
    emu_set_a(0x32);
    print_message_2_or_5();
    RESUME_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC641, 0);
    clear_special_scene_flag();
    RESUME_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC644, 0);
    return;
  }
  print_message_4(0x0A);
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC638, 0);
  clear_special_scene_flag();
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC63B, 0);
}

// 7:C64E, 7:C65D: two copies of special_scene_with_message.
static void scene_with_message_2(void) {
  TAIL_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC64E, 0);
  play_sound(0x0B);
  set_special_scene_flag();
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC656, 0);
  special_scene_message();
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC659, 0);
  clear_special_scene_flag();
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC65C, 0);
}

static void scene_with_message_3(void) {
  TAIL_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC65D, 0);
  play_sound(0x0B);
  set_special_scene_flag();
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC665, 0);
  special_scene_message();
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC668, 0);
  clear_special_scene_flag();
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC66B, 0);
}

// special_scene_action (7:C593): what scene special_scene does while it is up, before going back.
void special_scene_action(void) {
  RUN_ORIGINAL_IF_LAG(kSpecialSceneAction);
  uint8_t which = special_scene;
  emu_set_a(which);
  JUMP_TABLE_OR_ORIGINAL(kSpecialSceneAction, 0xC595, 0, which, 27);
  jump_table(0xC597, which);
  switch(which) {
    case 0:
      far_call_8020(0x6F);
      RESUME_ORIGINAL_IF_LAG(kSpecialSceneAction, 0xC5D3, 0);
      break;
    case 1: case 2: case 3: case 4: case 5: case 24: case 25: case 26:
      special_scene_with_message();
      break;
    case 6: scene_sound_20(); break;
    case 7: scene_sound_1A(); break;
    case 8: scene_sound_1F(); break;
    case 9: scene_message_by_flags(); break;
    case 10: scene_with_message_2(); break;
    case 11: scene_with_message_3(); break;
    case 23: special_scene_final(); break;
    default: break;
  }
}

// special_scene_with_message (7:C5D4): sound $0B, and the scene's message with its flag set.
void special_scene_with_message(void) {
  RUN_ORIGINAL_IF_LAG(kSpecialSceneWithMessage);
  play_sound(0x0B);
  set_special_scene_flag();
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC5DC, 0);
  special_scene_message();
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC5DF, 0);
  clear_special_scene_flag();
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneWithMessage, 0xC5E2, 0);
}

// special_scene_final (7:C66C): the message, then scene $91 with message $DD, then a far call.
void special_scene_final(void) {
  RUN_ORIGINAL_IF_LAG(kSpecialSceneFinal);
  emu_set_a(0x03);
  text_delay = 0x03;
  set_special_scene_flag();
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneFinal, 0xC673, 0);
  special_scene_message();
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneFinal, 0xC676, 0);
  clear_special_scene_flag();
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneFinal, 0xC679, 0);
  emu_set_a(0x91);
  scene = 0x91;
  change_scene();
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneFinal, 0xC681, 0);
  set_special_scene_flag();
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneFinal, 0xC684, 0);
  emu_set_a(0xDD);
  print_message_2_or_5();
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneFinal, 0xC689, 0);
  far_call_8040(0x61);
  RESUME_ORIGINAL_IF_LAG(kSpecialSceneFinal, 0xC68E, 0);
}

// set_special_scene_flag (7:C68F): the scene's event flag (7:C6BF) on, and the object sprites just
// object 0, $6C.
void set_special_scene_flag(void) {
  RUN_ORIGINAL_IF_LAG(kSetSpecialSceneFlag);
  Cpu6502 *c = &g_nes.cpu;
  c->y = special_scene;
  set_event_flag(cpu_bus_read((uint16_t)(kSpecialSceneFlags + c->y)));
  g_ram[0x67] = 0xC5;
  g_ram[0x68] = 0x00;
  g_ram[0xBD] = 0x00;
  g_ram[0xBE] = 0x6C;
  far_call_8020(0x60);
  RESUME_ORIGINAL_IF_LAG(kSetSpecialSceneFlag, 0xC6AC, 0);
  c->y = 0;
  emu_set_a(0xFF);
  emu_write_ind_y(0x67, 0, 0xFF);
}

// clear_special_scene_flag (7:C6B3): the flag off, and the room's sprites again.
void clear_special_scene_flag(void) {
  RUN_ORIGINAL_IF_LAG(kClearSpecialSceneFlag);
  Cpu6502 *c = &g_nes.cpu;
  c->y = special_scene;
  clear_event_flag(cpu_bus_read((uint16_t)(kSpecialSceneFlags + c->y)));
  collect_room_sprites();
  RESUME_ORIGINAL_IF_LAG(kClearSpecialSceneFlag, 0xC6BE, 0);
}

// special_scene_message (7:C6DA): the scene's message: 7:C700 has which printer at 2n, and the
// byte before it the message.
void special_scene_message(void) {
  RUN_ORIGINAL_IF_LAG(kSpecialSceneMessage);
  Cpu6502 *c = &g_nes.cpu;
  c->y = asl_a(special_scene);
  uint8_t printer = cpu_bus_read((uint16_t)(kSpecialSceneMessageKinds + c->y));
  emu_set_a(printer);
  JUMP_TABLE_OR_ORIGINAL(kSpecialSceneMessage, 0xC6E1, 0, printer, 3);
  jump_table(0xC6E3, printer);
  emu_set_a(cpu_bus_read((uint16_t)(kSpecialSceneMessageIds + c->y)));
  switch(printer) {
    case 0:
      print_message_5(c->a);
      RESUME_ORIGINAL_IF_LAG(kSpecialSceneMessage, 0xC6F0, 0);
      break;
    case 1:
      print_message_4(c->a);
      RESUME_ORIGINAL_IF_LAG(kSpecialSceneMessage, 0xC6F7, 0);
      break;
    default:
      print_message_2_or_5();
      RESUME_ORIGINAL_IF_LAG(kSpecialSceneMessage, 0xC6FE, 0);
      break;
  }
}

// special_scene_extras (7:C779): what show_special_scene adds, for a few scenes.
void special_scene_extras(void) {
  RUN_ORIGINAL_IF_LAG(kSpecialSceneExtras);
  uint8_t which = special_scene;
  emu_set_a(which);
  JUMP_TABLE_OR_ORIGINAL(kSpecialSceneExtras, 0xC77B, 0, which, 27);
  jump_table(0xC77D, which);
  switch(which) {
    case 6: scene_sound_20(); break;
    case 7: scene_sound_1A(); break;
    case 8: scene_sound_1F(); break;
    case 20:
      play_sound(0x00);
      play_sound(0x28);
      wait_180_frames();
      RESUME_ORIGINAL_IF_LAG(kSpecialSceneExtras, 0xC7CC, 0);
      play_sound(0x00);
      wait_vblank();
      RESUME_ORIGINAL_IF_LAG(kSpecialSceneExtras, 0xC7D4, 0);
      play_sound(0x04);
      break;
    case 21:
      play_sound(0x04);
      break;
    case 23:
      special_scene_final();
      RESUME_ORIGINAL_IF_LAG(kSpecialSceneExtras, 0xC7E3, 0);
      break;
    default:
      break;
  }
}
