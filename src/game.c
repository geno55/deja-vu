// The game's main loop, and starting it: the power-on code, a new game, drawing
// the game screen.
#include "funcs.h"
#include "variables.h"

// reset (7:E800), at power-on and when main ends: the PPU, the sound and all of RAM from the start.
// Returns where it carries on, main. Only the 6502 jumps here, so there is no guard: the bridge
// doesn't call the hook when the next NMI is a lag frame inside. After the waits below this code
// doesn't take long enough for one to land in it, and if one did it would go late.
uint16_t game_reset(void) {
  Cpu6502 *c = &g_nes.cpu;
  emu_set_flag(kFlagI, 1);
  emu_set_flag(kFlagD, 0);
  c->x = 0x00;
  cpu_bus_write(0x2000, 0x00);
  // Two spins on PPUSTATUS, to the start of vblank. C code keeps no cycle count, so each pass adds
  // what the original's loop takes.
  for(int i = 0; i < 2; i++) {
    do {
      emu_set_a(cpu_bus_read(0x2002));
      c->cycles += 7;
    } while(!(c->a & 0x80));
  }
  c->x = 0xFE;
  emu_set_stack(0xFE);
  emu_set_a(0x00);
  cpu_bus_write(0x4015, 0x00);
  cpu_bus_write(0x4017, 0x00);
  cpu_bus_write(0x4010, 0x00);
  c->y = 0;
  for(int a = 0; a < 0x800; a++) {
    g_ram[a] = 0x00;
  }
  emu_set_nz(c->y);
  set_current_bank(0x06);
  emu_set_a(0xFF);
  stack_index = 0xFF;
  emu_set_a(cpu_bus_read(0x2002));
  set_mirroring_vertical();
  emu_set_a(0x00);
  cpu_bus_write(0x2005, 0x00);
  cpu_bus_write(0x2005, 0x00);
  sound_table_ptr[0] = 0x00;
  emu_set_a(0x80);
  sound_table_ptr[1] = 0x80;
  sound_reset();
  emu_set_a(0x06);
  ppumask_shadow = 0x06;
  cpu_bus_write(0x2001, 0x06);
  emu_set_a(0x88);
  ppuctrl_shadow = 0x88;
  cpu_bus_write(0x2000, 0x88);
  disable_save_ram();
  wait_vblank();
  set_mirroring_vertical();
  wait_vblank();
  return kMain;
}

// main (7:E600), which reset jumps to: starts a new game, then draws the game screen and
// runs the input loop (the scripts' work happens in the far calls) until a script ends it.
// Returns where the game carries on: reset, after the redraw guard below or by the jump table.
uint16_t game_main(void) {
  same_room_redraws = 0;
  last_room = 0xFF;
  text_delay = 2;
  play_sound(0x00);
  new_game();
  far_call_8040(0x60);
  play_sound(0x01);
  far_call_8400(0x21);
  far_call_8400(0x22);
  backup_state_to_save_ram();
  for(;;) {
    // game_screen_loop
    draw_game_screen();
    script_table = object_id = command = 0xFF;
    emu_set_a(0xFF);
    show_intro_once();
    do {  // input_loop
      far_call_8000(0x6A);
      far_call_8020(0x65);
      wait_vblank_and_hide_sprites();
      far_call_8000(0x69);
    } while(!(script_flags & 0x80));
    if(room == last_room) {
      // Redrawn three times in the same room: the save file is erased and the game restarts.
      if(++same_room_redraws == 3) {
        scene_index = 0;
        g_ram[0x84] = 0x02;
        emu_set_a(0x02);
        erase_save_file();
        check_all_save_files();
        request_black_palette();
        wait_nmi_work_done();
        return kReset;
      }
    } else {
      last_room = room;
      same_room_redraws = 1;
    }
    far_call_8400(0x23);
    script_flags &= 0x7F;
    emu_set_a(scene_index);
    uint16_t next = jump_table(kMainJumpTable - 1, scene_index);
    if(next != kGameScreenLoop) {
      return next;
    }
  }
}

// draw_game_screen (7:E68D): black, clears the nametables, sprites and attributes, and draws it all
// again through far calls.
void draw_game_screen(void) {
  RUN_ORIGINAL_IF_LAG(kDrawGameScreen);
  Cpu6502 *c = &g_nes.cpu;
  request_black_palette();
  clear_nametables(0xA9);
  RESUME_ORIGINAL_IF_LAG(kDrawGameScreen, 0xE695, 0);
  clear_oam_buffer();
  emu_set_a(0xFF);
  c->y = 0;
  do {
    attr_buffer_2000[c->y] = 0xFF;
    c->y++;
    emu_cmp(c->y, 0x80);
  } while(c->y < 0x80);
  far_call_8000(0x6F);
  RESUME_ORIGINAL_IF_LAG(kDrawGameScreen, 0xE6A9, 0);
  far_call_8000(0x6C);
  RESUME_ORIGINAL_IF_LAG(kDrawGameScreen, 0xE6AE, 0);
  far_call_8020(0x62);
  RESUME_ORIGINAL_IF_LAG(kDrawGameScreen, 0xE6B3, 0);
  far_call_8000(0x6E);
  RESUME_ORIGINAL_IF_LAG(kDrawGameScreen, 0xE6B8, 0);
  emu_set_a(inventory_page);
  if(!c->a) {
    far_call_8040(0x63);
    RESUME_ORIGINAL_IF_LAG(kDrawGameScreen, 0xE6C2, 0);
  } else {
    far_call_8020(0x6A);
    RESUME_ORIGINAL_IF_LAG(kDrawGameScreen, 0xE6CA, 0);
  }
  far_call_8000(0x68);
  RESUME_ORIGINAL_IF_LAG(kDrawGameScreen, 0xE6CF, 0);
  far_call_8020(0x69);
  RESUME_ORIGINAL_IF_LAG(kDrawGameScreen, 0xE6D4, 0);
  load_current_scene();
  RESUME_ORIGINAL_IF_LAG(kDrawGameScreen, 0xE6D7, 0);
  far_call_8000(0x6D);
  RESUME_ORIGINAL_IF_LAG(kDrawGameScreen, 0xE6DC, 0);
  update_cursor_and_sprites();
  RESUME_ORIGINAL_IF_LAG(kDrawGameScreen, 0xE6DF, 0);
  far_call_8000(0x60);
  RESUME_ORIGINAL_IF_LAG(kDrawGameScreen, 0xE6E4, 0);
  far_call_8000(0x63);
  RESUME_ORIGINAL_IF_LAG(kDrawGameScreen, 0xE6E9, 0);
}

// update_cursor_and_sprites (7:E6EA): once the game has started, the sprites for this frame.
void update_cursor_and_sprites(void) {
  RUN_ORIGINAL_IF_LAG(kUpdateCursorAndSprites);
  emu_set_a(game_started);
  if(g_nes.cpu.a) {
    far_call_8000(0x6A);
    RESUME_ORIGINAL_IF_LAG(kUpdateCursorAndSprites, 0xE6F3, 0);
    collect_room_sprites();
    RESUME_ORIGINAL_IF_LAG(kUpdateCursorAndSprites, 0xE6F6, 0);
    far_call_8020(0x65);
    RESUME_ORIGINAL_IF_LAG(kUpdateCursorAndSprites, 0xE6FB, 0);
  }
  wait_vblank_and_hide_sprites();
  RESUME_ORIGINAL_IF_LAG(kUpdateCursorAndSprites, 0xE6FE, 0);
}

// load_current_scene (7:E6FF): before the game has started, scene $8F.
void load_current_scene(void) {
  RUN_ORIGINAL_IF_LAG(kLoadCurrentScene);
  emu_set_a(game_started);
  if(!g_nes.cpu.a) {
    emu_set_a(0x8F);
    scene = 0x8F;
  }
  load_scene();
  RESUME_ORIGINAL_IF_LAG(kLoadCurrentScene, 0xE70B, 0);
}

// show_intro_once (7:E70C): the first time, after a wait, scene 0 and message 0.
void show_intro_once(void) {
  RUN_ORIGINAL_IF_LAG(kShowIntroOnce);
  emu_set_a(game_started);
  if(g_nes.cpu.a) {
    return;
  }
  wait_60_frames();
  RESUME_ORIGINAL_IF_LAG(kShowIntroOnce, 0xE713, 0);
  wait_30_frames();
  RESUME_ORIGINAL_IF_LAG(kShowIntroOnce, 0xE716, 0);
  clear_event_flag(0x11);
  emu_set_a(0x00);
  scene = 0x00;
  change_scene();
  RESUME_ORIGINAL_IF_LAG(kShowIntroOnce, 0xE723, 0);
  update_cursor_and_sprites();
  RESUME_ORIGINAL_IF_LAG(kShowIntroOnce, 0xE726, 0);
  game_started--;
  print_message_5(0x00);
  RESUME_ORIGINAL_IF_LAG(kShowIntroOnce, 0xE72D, 0);
}

// new_game (7:E72E): the state a game starts with.
void new_game(void) {
  RUN_ORIGINAL_IF_LAG(kNewGame);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(0x00);
  c->y = 0;
  do {
    object_state[c->y] = 0x00;
    c->y++;
    emu_set_nz(c->y);
  } while(c->y);
  do {
    container_stack[1 + c->y] = 0x00;
    c->y++;
    emu_cmp(c->y, 0x10);
  } while(c->y != 0x10);
  inventory_page = store_page = 0x02;
  inventory_entry = store_entry = 0x00;
  g_ram[0x4A9] = 0x03;
  g_ram[0x4AA] = 0x06;
  g_ram[0x4AB] = 0x00;
  g_ram[0x4AC] = 0x07;
  g_ram[0x4AD] = 0x06;
  set_story_flag_refresh(0x4B);
  RESUME_ORIGINAL_IF_LAG(kNewGame, 0xE76D, 0);
  emu_set_a(0xFF);
  script_table = object_id = command = 0xFF;
  switch_key[0] = switch_key[1] = 0xFF;
  g_ram[0xAF] = g_ram[0xB0] = 0xFF;
  c->y = 0;
  do {
    address_book[c->y] = 0xFF;
    c->y++;
    emu_cmp(c->y, 0x54);
  } while(c->y < 0x54);
  c->y = 0;
  do {
    (&open_containers)[c->y] = 0xFF;
    c->y++;
    emu_cmp(c->y, 0x10);
  } while(c->y < 0x10);
  c->y = 0;
  do {
    container_items[c->y] = 0xFF;
    c->y++;
    emu_cmp(c->y, 0x20);
  } while(c->y < 0x20);
  container_items[0] = 0x0A;
  g_ram[0x4A8] = 0x00;
  scene = 0x00;
  room = 0x00;
  emu_set_a(0x00);
  dissolve_pattern = 0x00;
}

// next_random (7:E7B1): rng_state = (rng_state + 1) * 5, and A = random_value = that with its
// nibbles swapped.
void next_random(void) {
  RUN_ORIGINAL_IF_LAG(kNextRandom);
  uint8_t state = (uint8_t)((rng_state + 1) * 5);
  rng_state = state;
  uint8_t a = asl_a(asl_a(asl_a(asl_a(state))));
  random_value = (uint8_t)(a | state >> 4);
  emu_set_a(random_value);
}
