// The menus (bank 2): the title, the file select screen and the yes/no
// dialogs. Far calls $21-$24 at $8400. $84 is the menu's choice or which dialog it is, scene_index
// the dialog's answer and which files are good, $76 the files left to mark.
//
// These wait for the NMI most of the way, and the title spins on the controller without a wait, so
// they check whether to go on as original code on every pass (RESUME_ORIGINAL_IF_LAG).
#include "funcs.h"
#include "pointer.h"
#include "variables.h"

// The two run-length-coded screens: their data follows the JSR.
void draw_title_picture(void) {
  rle_screen_from(kDrawTitlePicture);
}

void draw_dialog_screen(void) {
  rle_screen_from(kDrawDialogScreen);
}

// title_screen (2:840C, far call $21): shown until A, B or Start is pressed and let go.
void title_screen(void) {
  RUN_ORIGINAL_IF_LAG(kTitleScreen);
  Cpu6502 *c = &g_nes.cpu;
  draw_title_picture();
  RESUME_ORIGINAL_IF_LAG(kTitleScreen, 0x840F, 0);
  SET_CHR_BANKS_INLINE(0x840F, 0);
  clear_and_upload_sprites();
  RESUME_ORIGINAL_IF_LAG(kTitleScreen, 0x841D, 0);
  g_ram[0x1E] = 0x3D;
  emu_set_a(0x84);
  g_ram[0x1F] = 0x84;
  fade_in_palette();
  RESUME_ORIGINAL_IF_LAG(kTitleScreen, 0x8428, 0);
  do {
    read_joypads();
    RESUME_ORIGINAL_IF_LAG(kTitleScreen, 0x842B, 0);
    emu_set_a(joy_pressed[0] & 0xD0);
    if(!c->a) {
      emu_spin(10);
    }
  } while(!c->a);
  do {
    read_joypads();
    RESUME_ORIGINAL_IF_LAG(kTitleScreen, 0x8434, 0);
    emu_set_a(joy_held[0] & 0xD0);
    if(c->a) {
      emu_spin(10);
    }
  } while(c->a);
  fade_out_palette();
}

// file_select_menu (2:86D0, far call $22): chooses a file, and for a good one what to do with it: load
// it, go back, or erase it after a yes. An empty file starts a new game. game_started says which.
void file_select_menu(void) {
  RUN_ORIGINAL_IF_LAG(kFileSelectMenu);
  Cpu6502 *c = &g_nes.cpu;
  file_select_screen();
  RESUME_ORIGINAL_IF_LAG(kFileSelectMenu, 0x86D3, 0);
  for(;;) {
    draw_file_cursor();
    wait_vblank_and_hide_sprites();
    RESUME_ORIGINAL_IF_LAG(kFileSelectMenu, 0x86D9, 0);
    read_joypads();
    pointer_choose_row(&save_file, 3, 0x80, kFileCursorYs, 1);
    emu_set_a(joy_pressed[0]);
    if(!(c->a & 0x80)) {
      emu_set_a(c->a & 0x0C);
      if(c->a) {
        emu_cmp(c->a, 0x04);
        if(c->a == 0x04) {
          save_file++;
          emu_set_a(save_file);
          emu_cmp(c->a, 0x03);
          if(c->a == 0x03) {
            emu_set_a(0x00);
            save_file = 0x00;
          }
        } else {
          save_file--;
          emu_set_a(save_file);
          if(c->a & 0x80) {
            emu_set_a(0x02);
            save_file = 0x02;
          }
        }
      }
      continue;
    }
    c->x = save_file;
    emu_set_a(scene_index & cpu_bus_read((uint16_t)(kFileBits + c->x)));
    if(!c->a) {
      emu_set_a(0x00);
      break;
    }
    emu_set_a(0x00);
    g_ram[0x84] = 0x00;
    draw_file_options();
    RESUME_ORIGINAL_IF_LAG(kFileSelectMenu, 0x8712, 0);
    for(;;) {
      draw_option_cursor();
      wait_vblank_and_hide_sprites();
      RESUME_ORIGINAL_IF_LAG(kFileSelectMenu, 0x8718, 0);
      read_joypads();
      pointer_choose_row(&g_ram[0x84], 3, 0x70, kOptionCursorYs, 1);
      emu_set_a(joy_pressed[0]);
      if(c->a & 0x80) {
        break;
      }
      emu_set_a(c->a & 0x0C);
      if(!c->a) {
        continue;
      }
      emu_set_a(c->a & 0x04);
      if(c->a) {
        g_ram[0x84]++;
        emu_set_a(g_ram[0x84]);
        emu_cmp(c->a, 0x03);
        if(c->a == 0x03) {
          emu_set_a(0x00);
          g_ram[0x84] = 0x00;
        }
      } else {
        g_ram[0x84]--;
        emu_set_a(g_ram[0x84]);
        if(c->a & 0x80) {
          emu_set_a(0x02);
          g_ram[0x84] = 0x02;
        }
      }
    }
    uint8_t choice = g_ram[0x84];
    emu_set_a(choice);
    JUMP_TABLE_OR_ORIGINAL(kFileSelectMenu, 0x8746, 0, choice, 3);
    jump_table(0x8748, choice);
    if(choice == 0) {
      load_game();
      RESUME_ORIGINAL_IF_LAG(kFileSelectMenu, 0x8756, 0);
      emu_set_a(0xFF);
      break;
    }
    if(choice == 2) {
      show_file_options();
      RESUME_ORIGINAL_IF_LAG(kFileSelectMenu, 0x8767, 0);
      emu_set_a(scene_index);
      if(!c->a) {
        erase_save_file();
        RESUME_ORIGINAL_IF_LAG(kFileSelectMenu, 0x876E, 0);
      }
      check_all_save_files();
      RESUME_ORIGINAL_IF_LAG(kFileSelectMenu, 0x8771, 0);
    }
    draw_file_list();
    RESUME_ORIGINAL_IF_LAG(kFileSelectMenu, 0x8761, 0);
  }
  game_started = c->a;
  fade_out_palette();
  RESUME_ORIGINAL_IF_LAG(kFileSelectMenu, 0x875D, 0);
}

// The end of file_select_screen (2:879B), which open_confirm_dialog jumps to: the palette at 2:87A6
// faded in.
static void fade_in_menu_palette(void) {
  TAIL_ORIGINAL_IF_LAG(kFileSelectScreen, 0x879B, 0);
  g_ram[0x1E] = 0xA6;
  emu_set_a(0x87);
  g_ram[0x1F] = 0x87;
  fade_in_palette();
}

// file_select_screen (2:8774): which files are good, and the screen listing them.
void file_select_screen(void) {
  RUN_ORIGINAL_IF_LAG(kFileSelectScreen);
  check_all_save_files();
  RESUME_ORIGINAL_IF_LAG(kFileSelectScreen, 0x8777, 0);
  fade_out_palette();
  RESUME_ORIGINAL_IF_LAG(kFileSelectScreen, 0x877A, 0);
  clear_and_upload_sprites();
  RESUME_ORIGINAL_IF_LAG(kFileSelectScreen, 0x877D, 0);
  SET_CHR_BANKS_INLINE(0x877D, 0);
  draw_dialog_screen();
  RESUME_ORIGINAL_IF_LAG(kFileSelectScreen, 0x878B, 0);
  draw_file_list();
  RESUME_ORIGINAL_IF_LAG(kFileSelectScreen, 0x878E, 0);
  emu_set_a(0x00);
  save_file = 0x00;
  draw_file_cursor();
  wait_vblank_and_hide_sprites();
  RESUME_ORIGINAL_IF_LAG(kFileSelectScreen, 0x8798, 0);
  fade_in_menu_palette();
}

// draw_file_list (2:87C6): the three files, and the mark on each that is empty.
void draw_file_list(void) {
  RUN_ORIGINAL_IF_LAG(kDrawFileList);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(scene_index);
  g_ram[0x76] = c->a;
  clear_file_area();
  RESUME_ORIGINAL_IF_LAG(kDrawFileList, 0x87CD, 0);
  c->x = 0;
  do {
    draw_file_entry();
    RESUME_ORIGINAL_IF_LAG(kDrawFileList, 0x87D2, 0);
    draw_file_status();
    RESUME_ORIGINAL_IF_LAG(kDrawFileList, 0x87D5, 0);
    c->x++;
    emu_cmp(c->x, 0x03);
  } while(c->x != 0x03);
  request_vram_queue();
}

// draw_file_entry (2:87DE): file X's name.
void draw_file_entry(void) {
  RUN_ORIGINAL_IF_LAG(kDrawFileEntry);
  Cpu6502 *c = &g_nes.cpu;
  file_entry_setup();
  g_ram[0x1C] = cpu_bus_read((uint16_t)(kFileEntryVram + c->x));
  emu_set_a(0x22);
  g_ram[0x1D] = 0x22;
  queue_vram_block();
  RESUME_ORIGINAL_IF_LAG(kDrawFileEntry, 0x87ED, 0);
}

// file_entry_setup (2:87EE): a block at data_ptr of 6 tiles by 1: 5 from 2:882A and file X's letter.
void file_entry_setup(void) {
  RUN_ORIGINAL_IF_LAG(kFileEntrySetup);
  Cpu6502 *c = &g_nes.cpu;
  c->y = 0;
  do {
    emu_set_a(cpu_bus_read((uint16_t)(kFileEntryTiles + c->y)));
    g_ram[0x57 + c->y] = c->a;
    c->y++;
    emu_cmp(c->y, 0x05);
  } while(c->y != 0x05);
  g_ram[0x5C] = cpu_bus_read((uint16_t)(kFileLetters + c->x));
  data_ptr[0] = 0x06;
  data_ptr[1] = 0x01;
  g_ram[0x1E] = 0x55;
  emu_set_a(0x00);
  g_ram[0x1F] = 0x00;
}

// draw_file_status (2:8811): shifts the next file's bit out of $76; a good file has nothing more, and
// the others the mark at 2:8838.
void draw_file_status(void) {
  RUN_ORIGINAL_IF_LAG(kDrawFileStatus);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_flag(kFlagC, g_ram[0x76] & 0x01);
  g_ram[0x76] >>= 1;
  emu_set_nz(g_ram[0x76]);
  if(c->p & kFlagC) {
    return;
  }
  g_ram[0x1E] = 0x38;
  g_ram[0x1F] = 0x88;
  g_ram[0x1D] = 0x22;
  emu_set_a(cpu_bus_read((uint16_t)(kFileStatusVram + c->x)));
  g_ram[0x1C] = c->a;
  queue_vram_block();
  RESUME_ORIGINAL_IF_LAG(kDrawFileStatus, 0x8829, 0);
}

// draw_file_options (2:883D): the chosen file's name and the three options.
void draw_file_options(void) {
  RUN_ORIGINAL_IF_LAG(kDrawFileOptions);
  Cpu6502 *c = &g_nes.cpu;
  clear_file_area();
  RESUME_ORIGINAL_IF_LAG(kDrawFileOptions, 0x8840, 0);
  c->x = save_file;
  file_entry_setup();
  g_ram[0x1C] = 0x70;
  emu_set_a(0x22);
  g_ram[0x1D] = 0x22;
  queue_vram_block();
  RESUME_ORIGINAL_IF_LAG(kDrawFileOptions, 0x8850, 0);
  c->x = 0;
  do {
    g_ram[0x1E] = cpu_bus_read((uint16_t)(kFileOptionBlocks + c->x));
    g_ram[0x1F] = cpu_bus_read((uint16_t)(kFileOptionBlocks + 1 + c->x));
    g_ram[0x1C] = cpu_bus_read((uint16_t)(kFileOptionVram + c->x));
    emu_set_a(cpu_bus_read((uint16_t)(kFileOptionVram + 1 + c->x)));
    g_ram[0x1D] = c->a;
    queue_vram_block();
    RESUME_ORIGINAL_IF_LAG(kDrawFileOptions, 0x8869, 0);
    c->x += 2;
    emu_cmp(c->x, 0x06);
  } while(c->x != 0x06);
  request_vram_queue();
}

// clear_file_area (2:88BF), fill_file_area (2:88CB): the block at $1E/$1F filled in at $22xx, written out
// by the next frame.
void clear_file_area(void) {
  RUN_ORIGINAL_IF_LAG(kClearFileArea);
  g_ram[0x1E] = 0xD9;
  g_ram[0x1F] = 0x88;
  emu_set_a(0x70);
  g_ram[0x1C] = 0x70;
  fill_file_area();
}

void fill_file_area(void) {
  RUN_ORIGINAL_IF_LAG(kFillFileArea);
  emu_set_a(0x22);
  g_ram[0x1D] = 0x22;
  queue_vram_fill();
  RESUME_ORIGINAL_IF_LAG(kFillFileArea, 0x88D2, 0);
  request_vram_queue();
  wait_vblank();
  RESUME_ORIGINAL_IF_LAG(kFillFileArea, 0x88D8, 0);
}

// The end of draw_file_cursor (2:88E7), which the other cursors jump to: the sprite at 2:88FA.
static void draw_menu_cursor(void) {
  TAIL_ORIGINAL_IF_LAG(kDrawFileCursor, 0x88E7, 0);
  g_ram[0x1E] = 0xFA;
  g_ram[0x1F] = 0x88;
  emu_set_a(0x02);
  g_ram[0x22] = 0x02;
  draw_metasprite();
}

// draw_file_cursor (2:88DC): beside file save_file.
void draw_file_cursor(void) {
  RUN_ORIGINAL_IF_LAG(kDrawFileCursor);
  Cpu6502 *c = &g_nes.cpu;
  c->x = save_file;
  g_ram[0x21] = cpu_bus_read((uint16_t)(kFileCursorYs + c->x));
  emu_set_a(0x80);
  g_ram[0x20] = 0x80;
  draw_menu_cursor();
}

// draw_option_cursor (2:88FD): beside option $84.
void draw_option_cursor(void) {
  RUN_ORIGINAL_IF_LAG(kDrawOptionCursor);
  Cpu6502 *c = &g_nes.cpu;
  c->x = g_ram[0x84];
  g_ram[0x21] = cpu_bus_read((uint16_t)(kOptionCursorYs + c->x));
  emu_set_a(0x70);
  g_ram[0x20] = 0x70;
  draw_menu_cursor();
}

// show_file_options (2:890D): the erase question, on the file screen, answered in scene_index.
void show_file_options(void) {
  RUN_ORIGINAL_IF_LAG(kShowFileOptions);
  Cpu6502 *c = &g_nes.cpu;
  g_ram[0x84] = 0x02;
  scene_index = 0x00;
  g_ram[0x1E] = 0x4B;
  g_ram[0x1F] = 0x89;
  emu_set_a(0x90);
  g_ram[0x1C] = 0x90;
  fill_file_area();
  RESUME_ORIGINAL_IF_LAG(kShowFileOptions, 0x8924, 0);
  c->x = 0x04;
  c->y = 0x00;
  do {
    g_ram[0x1E] = cpu_bus_read((uint16_t)(kDialogBlocks + c->x));
    g_ram[0x1F] = cpu_bus_read((uint16_t)(kDialogBlocks + 1 + c->x));
    g_ram[0x1C] = cpu_bus_read((uint16_t)(kYesNoVram + c->y));
    emu_set_a(0x22);
    g_ram[0x1D] = 0x22;
    queue_vram_block();
    RESUME_ORIGINAL_IF_LAG(kShowFileOptions, 0x893E, 0);
    c->x += 2;
    c->y++;
    emu_cmp(c->y, 0x02);
  } while(c->y != 0x02);
  request_vram_queue();
  confirm_dialog_loop();
}

// quit_dialog (2:8950, far call $23): on a yes, the backup comes back.
void quit_dialog(void) {
  RUN_ORIGINAL_IF_LAG(kQuitDialog);
  emu_set_a(0x01);
  g_ram[0x84] = 0x01;
  open_confirm_dialog();
  RESUME_ORIGINAL_IF_LAG(kQuitDialog, 0x8957, 0);
  confirm_dialog_loop();
  RESUME_ORIGINAL_IF_LAG(kQuitDialog, 0x895A, 0);
  emu_set_a(scene_index);
  if(!g_nes.cpu.a) {
    restore_state_backup();
    RESUME_ORIGINAL_IF_LAG(kQuitDialog, 0x8961, 0);
  }
  fade_out_palette();
  RESUME_ORIGINAL_IF_LAG(kQuitDialog, 0x8964, 0);
}

// save_dialog (2:8965, far call $24): on a yes, saves; then the game screen and its sound again.
void save_dialog(void) {
  RUN_ORIGINAL_IF_LAG(kSaveDialog);
  emu_set_a(repeat_sound);
  emu_push(repeat_sound);
  emu_set_a(0x00);
  g_ram[0x84] = 0x00;
  open_confirm_dialog();
  RESUME_ORIGINAL_IF_LAG(kSaveDialog, 0x896F, 1);
  confirm_dialog_loop();
  RESUME_ORIGINAL_IF_LAG(kSaveDialog, 0x8972, 1);
  emu_set_a(scene_index);
  if(!g_nes.cpu.a) {
    save_game();
    RESUME_ORIGINAL_IF_LAG(kSaveDialog, 0x8979, 1);
  }
  fade_out_palette();
  RESUME_ORIGINAL_IF_LAG(kSaveDialog, 0x897C, 1);
  draw_game_screen();
  RESUME_ORIGINAL_IF_LAG(kSaveDialog, 0x897F, 1);
  play_sound(emu_pull());
}

// open_confirm_dialog (2:8984): the dialog's screen, faded in.
void open_confirm_dialog(void) {
  RUN_ORIGINAL_IF_LAG(kOpenConfirmDialog);
  fade_out_palette();
  RESUME_ORIGINAL_IF_LAG(kOpenConfirmDialog, 0x8987, 0);
  play_sound(0x01);
  system_flags &= 0xFE;  // LSR, ASL
  emu_set_flag(kFlagC, 0);
  emu_set_nz(system_flags);
  clear_and_upload_sprites();
  RESUME_ORIGINAL_IF_LAG(kOpenConfirmDialog, 0x8993, 0);
  SET_CHR_BANKS_INLINE(0x8993, 0);
  draw_dialog_screen();
  RESUME_ORIGINAL_IF_LAG(kOpenConfirmDialog, 0x89A1, 0);
  draw_dialog();
  RESUME_ORIGINAL_IF_LAG(kOpenConfirmDialog, 0x89A4, 0);
  fade_in_menu_palette();
}

// draw_dialog (2:89A7)
void draw_dialog(void) {
  RUN_ORIGINAL_IF_LAG(kDrawDialog);
  draw_dialog_text();
  RESUME_ORIGINAL_IF_LAG(kDrawDialog, 0x89AA, 0);
  request_vram_queue();
  reset_dialog_answer();
  RESUME_ORIGINAL_IF_LAG(kDrawDialog, 0x89B0, 0);
  wait_vblank_and_hide_sprites();
  RESUME_ORIGINAL_IF_LAG(kDrawDialog, 0x89B3, 0);
}

// draw_dialog_text (2:89B4): 4 blocks for the save dialog, 2 for the quit one.
void draw_dialog_text(void) {
  RUN_ORIGINAL_IF_LAG(kDrawDialogText);
  Cpu6502 *c = &g_nes.cpu;
  emu_set_a(g_ram[0x84]);
  c->x = 0;
  if(!c->a) {
    do {
      g_ram[0x1E] = cpu_bus_read((uint16_t)(kDialogBlocks + c->x));
      g_ram[0x1F] = cpu_bus_read((uint16_t)(kDialogBlocks + 1 + c->x));
      g_ram[0x1C] = cpu_bus_read((uint16_t)(kDialogVram + c->x));
      emu_set_a(cpu_bus_read((uint16_t)(kDialogVram + 1 + c->x)));
      g_ram[0x1D] = c->a;
      queue_vram_block();
      RESUME_ORIGINAL_IF_LAG(kDrawDialogText, 0x89D1, 0);
      c->x += 2;
      emu_cmp(c->x, 0x08);
    } while(c->x != 0x08);
    return;
  }
  do {
    g_ram[0x1E] = cpu_bus_read((uint16_t)(kQuitDialogBlocks + c->x));
    g_ram[0x1F] = cpu_bus_read((uint16_t)(kQuitDialogBlocks + 1 + c->x));
    g_ram[0x1C] = cpu_bus_read((uint16_t)(kQuitDialogVram + c->x));
    emu_set_a(cpu_bus_read((uint16_t)(kQuitDialogVram + 1 + c->x)));
    g_ram[0x1D] = c->a;
    queue_vram_block();
    RESUME_ORIGINAL_IF_LAG(kDrawDialogText, 0x8A17, 0);
    c->x += 2;
    emu_cmp(c->x, 0x04);
  } while(c->x != 0x04);
}

// reset_dialog_answer (2:8A35): yes.
void reset_dialog_answer(void) {
  RUN_ORIGINAL_IF_LAG(kResetDialogAnswer);
  emu_set_a(0x00);
  scene_index = 0x00;
  draw_dialog_cursor();
}

// draw_dialog_cursor (2:8A3D): by the dialog and the answer (2:8A57).
void draw_dialog_cursor(void) {
  RUN_ORIGINAL_IF_LAG(kDrawDialogCursor);
  Cpu6502 *c = &g_nes.cpu;
  data_ptr[0] = asl_a(asl_a(g_ram[0x84]));
  uint8_t a = asl_a(scene_index);
  emu_set_flag(kFlagC, 0);
  c->y = emu_adc(a, data_ptr[0]);
  g_ram[0x20] = cpu_bus_read((uint16_t)(kDialogCursors + c->y));
  emu_set_a(cpu_bus_read((uint16_t)(kDialogCursors + 1 + c->y)));
  g_ram[0x21] = c->a;
  draw_menu_cursor();
}

// confirm_dialog_loop (2:8A63): up and down change the answer until A.
void confirm_dialog_loop(void) {
  RUN_ORIGINAL_IF_LAG(kConfirmDialogLoop);
  Cpu6502 *c = &g_nes.cpu;
  for(;;) {
    draw_dialog_cursor();
    wait_vblank_and_hide_sprites();
    RESUME_ORIGINAL_IF_LAG(kConfirmDialogLoop, 0x8A69, 0);
    read_joypads();
    uint16_t answers = (uint16_t)(kDialogCursors + g_ram[0x84] * 4);
    pointer_choose_row(&scene_index, 2, cpu_bus_read(answers), (uint16_t)(answers + 1), 2);
    emu_set_a(joy_pressed[0]);
    if(c->a & 0x80) {
      TAIL_ORIGINAL_IF_LAG(kResetDialogAnswer, 0x8A3C, 0);
      return;
    }
    emu_set_a(c->a & 0x0C);
    if(!c->a) {
      continue;
    }
    emu_cmp(c->a, 0x04);
    if(c->a == 0x04) {
      scene_index++;
    } else {
      scene_index--;
    }
    emu_set_a(scene_index & 0x01);
    scene_index = c->a;
  }
}
