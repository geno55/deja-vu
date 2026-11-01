// Plays the game with the pointer (src/pointer.h) from the title into the game: the menus, the cursor, the
// clicks and the fingers. The mouse and fingers come in as the platform hands them over, so this is
// everything but SDL's part.
//
// usage: test_pointer ROM
#include <stdio.h>
#include <string.h>

#include "funcs.h"
#include "nes_hw.h"
#include "pointer.h"
#include "ppu_render.h"
#include "variables.h"

#define CHECK(cond)                                                    \
  do {                                                                 \
    if(!(cond)) {                                                      \
      fprintf(stderr, "%s:%d: check failed: %s (frame %ld, cursor %02X,%02X mode %02X)\n", __FILE__, __LINE__, \
              #cond, frames_run, cursor_x, cursor_y, cursor_mode);     \
      failures++;                                                      \
    }                                                                  \
  } while(0)

enum {
  kLogoFrames = 900,
  kSettle = 120,  // long enough for a menu to take a click and draw what follows
};

static int failures;
static long frames_run;
static uint8_t keys;  // the buttons held on the pad as well
static uint16_t picture[kScreenWidth * kScreenHeight];

static void render(void) {
  ppu_render_frame(picture);
}

static void run(int frames) {
  char err[256];
  for(int i = 0; i < frames; i++, frames_run++) {
    g_nes.pad[0] = keys | pointer_pad();
    if(emu_run_frame(err, sizeof(err))) {
      fprintf(stderr, "test_pointer: frame %ld: %s\n", frames_run, err);
      failures++;
      return;
    }
    g_nes.apu_count = 0;
  }
}

static void click_at(int x, int y) {
  pointer_move(x, y);
  pointer_button(0, 1);
  pointer_button(0, 0);
}

static void right_click(void) {
  pointer_button(1, 1);
  pointer_button(1, 0);
}

static void check_pad_timing(void) {
  // A click shorter than a frame still shows for a few frames, then goes up.
  CHECK(pointer_pad() == 0);
  pointer_button(0, 1);
  pointer_button(0, 0);
  CHECK(pointer_pad() == 0x01);
  CHECK(pointer_pad() == 0x01);
  CHECK(pointer_pad() == 0x01);
  CHECK(pointer_pad() == 0);
  // Held, it stays down.
  pointer_button(1, 1);
  for(int i = 0; i < 10; i++) {
    CHECK(pointer_pad() == 0x02);
  }
  pointer_button(1, 0);
  CHECK(pointer_pad() == 0);
  // A second click during the first is a second press: the pad goes up between them.
  pointer_button(0, 1);
  pointer_button(0, 0);
  CHECK(pointer_pad() == 0x01);
  pointer_button(0, 1);
  pointer_button(0, 0);
  CHECK(pointer_pad() == 0x01);
  CHECK(pointer_pad() == 0x01);
  CHECK(pointer_pad() == 0);
  CHECK(pointer_pad() == 0x01);
  while(pointer_pad()) {
  }
}

// How many pixels of the frame just run the pointer changes.
static int pixels_pointer_changes(void) {
  static uint16_t drawn[kScreenWidth * kScreenHeight];
  memcpy(drawn, picture, sizeof(drawn));
  pointer_draw(drawn);
  int changed = 0;
  for(int i = 0; i < kScreenWidth * kScreenHeight; i++) {
    changed += drawn[i] != picture[i];
  }
  return changed;
}

static void check_off(void) {
  g_pointer_on = 0;
  pointer_move(0x40, 0x50);
  pointer_button(0, 1);
  pointer_button(1, 1);
  pointer_finger_down(1, 0x40, 0x50);
  pointer_finger_up(1, 0x40, 0x50);
  CHECK(pointer_pad() == 0);
  CHECK(!pointer_to_cursor());
  CHECK(pixels_pointer_changes() == 0);
  pointer_button(0, 0);
  pointer_button(1, 0);
  g_pointer_on = 1;
}

static void check_pointer_is_the_games(int x, int y) {
  pointer_move(x, y);
  run(5);
  CHECK(cursor_x == x && cursor_y == y);
  CHECK(pixels_pointer_changes() == 0);  // while the game draws its cursor, this doesn't draw
  CHECK(pixels_pointer_changes() == 0);  // and if it did, it would look the same
}

static void check_address_book(void) {
  // Reaching the address book takes most of the game, so this sets up what the routine reads. It doesn't
  // change anything that matters to the rest of the run, as the entries are put back.
  uint8_t book[7], cursor = command_cursor, pad0 = data_ptr[0];
  memcpy(book, address_book, 7);
  memset(address_book, 0xFF, 7);
  address_book[0] = 0;
  address_book[3] = 1;
  command_cursor = 0;
  data_ptr[0] = 0;
  pointer_move(0xA0, 0x54);  // the row of entry 3
  pointer_choose_address();
  CHECK(command_cursor == 3);
  pointer_move(0xA0, 0x44);  // an empty entry: it stays
  pointer_choose_address();
  CHECK(command_cursor == 3);
  pointer_move(0x20, 0x54);  // the row's height, but not over the list
  pointer_choose_address();
  CHECK(command_cursor == 3);
  pointer_move(0xA0, 0x10);  // above the list
  pointer_choose_address();
  CHECK(command_cursor == 3);
  pointer_move(0xA0, 0xA0);  // below the list
  pointer_choose_address();
  CHECK(command_cursor == 3);
  // A click on nothing isn't an A; on an entry, it is.
  click_at(0x20, 0x54);
  CHECK(pointer_pad() == 0x01);
  data_ptr[0] = 0x80;
  pointer_choose_address();
  CHECK(data_ptr[0] == 0 && command_cursor == 3);
  click_at(0xA0, 0x24);
  while(pointer_pad()) {
  }
  CHECK(pointer_pad() == 0x01);
  data_ptr[0] = 0x80;
  pointer_choose_address();
  CHECK(data_ptr[0] == 0x80 && command_cursor == 0);
  while(pointer_pad()) {
  }
  memcpy(address_book, book, 7);
  command_cursor = cursor;
  data_ptr[0] = pad0;
}

int main(int argc, char **argv) {
  if(argc != 2) {
    fprintf(stderr, "usage: test_pointer ROM\n");
    return 2;
  }
  FILE *f = fopen(argv[1], "rb");
  if(!f) {
    fprintf(stderr, "test_pointer: can't read %s\n", argv[1]);
    return 2;
  }
  static uint8_t rom[16 + kPrgSize + kChrSize];
  size_t got = fread(rom, 1, sizeof(rom), f);
  fclose(f);
  if(got != sizeof(rom)) {
    fprintf(stderr, "test_pointer: %s isn't the size of the game's ROM\n", argv[1]);
    return 2;
  }
  emu_set_frame_hook(render);
  if(dv_boot(rom + 16, rom + 16 + kPrgSize)) {
    fprintf(stderr, "test_pointer: can't start the game's coroutine\n");
    return 1;
  }

  check_off();
  g_pointer_on = 1;
  check_pad_timing();

  // The title takes a click anywhere. Until the pointer has moved, it isn't drawn.
  run(kLogoFrames);
  CHECK(pixels_pointer_changes() == 0);
  click_at(0x20, 0x20);
  run(300);

  // The file list: the pointer is drawn, hovering chooses a row, and the keys still work while the mouse
  // lies still.
  pointer_move(0xA0, 0xAC);
  run(5);
  CHECK(pixels_pointer_changes() > 0);
  CHECK(save_file == 1);
  pointer_move(0xA0, 0xBC);
  run(5);
  CHECK(save_file == 2);
  keys = 0x10;  // up
  run(3);
  keys = 0;
  run(3);
  CHECK(save_file == 1);
  // A click on no row isn't a press of A: all the files are empty here, so one would start the game.
  click_at(0x10, 0x10);
  run(kSettle);
  CHECK(game_started == 0);
  CHECK(save_file == 1);
  // A click on a row chooses it and starts a new game there. Clicks go through the opening text, until
  // the cursor comes to the pointer.
  click_at(0xA0, 0x9C);
  run(6);
  CHECK(save_file == 0);
  for(int i = 0; i < 200; i++) {
    pointer_move(0x08, 0x70);  // where a click hits nothing, once the cursor can get there
    run(3);
    if(cursor_x == 0x08 && cursor_y == 0x70) {
      break;
    }
    click_at(0x08, 0x70);
    run(60);
  }
  CHECK(cursor_x == 0x08 && cursor_y == 0x70);

  // Where the cursor goes sets how the d-pad goes on from there.
  pointer_move(0x41, 0x52);
  run(3);
  CHECK(cursor_x == 0x41 && cursor_y == 0x52 && (cursor_mode & 0x0F) == 0);
  pointer_move(0xA0, 0xC8);  // the exits
  run(3);
  CHECK(cursor_x == 0xA0 && cursor_y == 0xC8 && (cursor_mode & 0x0F) == 2);
  pointer_move(0x20, 0xA8);  // the first command
  run(3);
  CHECK(cursor_x == 0x20 && cursor_y == 0xA8 && (cursor_mode & 0x0F) == 1 && command_cursor == 7);
  pointer_move(-30, 500);  // outside the picture: its nearest edge
  run(3);
  CHECK(cursor_x == 0x05 && cursor_y == 0xE7);

  // A click takes the command under the pointer, wherever the cursor was; B lets it go.
  click_at(0x20, 0xA8);
  run(kSettle);
  CHECK(command == 0);
  right_click();
  run(kSettle);
  CHECK(command == 0xFF);
  // A click where there's nothing does nothing.
  click_at(0x20, 0xB8);
  run(kSettle);
  CHECK(command == 1);
  click_at(0x08, 0x70);
  run(kSettle);
  CHECK(command == 1);

  // The d-pad from where the mouse left it: down the first column, and the mouse doesn't pull it back.
  pointer_move(0x20, 0xA8);
  run(3);
  keys = 0x20;
  run(3);
  keys = 0;
  run(30);
  CHECK(command_cursor == 8 && cursor_x == 0x1A && cursor_y == 0xBA);
  // The keys' A clicks where the d-pad went.
  keys = 0x01;
  run(3);
  keys = 0;
  run(kSettle);
  CHECK(command == 1);
  right_click();
  run(kSettle);

  // A finger: the cursor follows it and nothing clicks until it lifts, at the place it lifts.
  pointer_finger_down(7, 0x20, 0xA8);
  run(3);
  CHECK(cursor_x == 0x20 && cursor_y == 0xA8);
  pointer_finger_move(7, 0x20, 0xC8);
  run(kSettle);
  CHECK(cursor_y == 0xC8 && command == 0xFF);
  pointer_finger_up(7, 0x20, 0xC8);
  run(kSettle);
  CHECK(command == 2);
  // A second finger is B, and the first then doesn't click.
  pointer_finger_down(8, 0x20, 0xA8);
  run(3);
  pointer_finger_down(9, 0x60, 0x60);
  run(kSettle);
  CHECK(command == 0xFF);
  pointer_finger_up(9, 0x60, 0x60);
  pointer_finger_up(8, 0x20, 0xA8);
  run(kSettle);
  CHECK(command == 0xFF);

  // The quit dialog (the command at the bottom right): the answers, and a click on neither.
  click_at(0xD0, 0xDA);
  run(kSettle);
  int dialog = g_ram[0x84];
  uint8_t x = cpu_bus_read((uint16_t)(kDialogCursors + dialog * 4));
  int yes_y = cpu_bus_read((uint16_t)(kDialogCursors + dialog * 4 + 1));
  int no_y = cpu_bus_read((uint16_t)(kDialogCursors + dialog * 4 + 3));
  CHECK(scene_index == 0);
  pointer_move(x + 8, no_y);
  run(5);
  CHECK(scene_index == 1);
  pointer_move(x + 8, yes_y);
  run(5);
  CHECK(scene_index == 0);
  click_at(0x08, 0x08);  // on neither, or it would quit
  run(kSettle);
  CHECK(scene_index == 0);
  pointer_move(0x40, 0x50);
  run(3);
  CHECK(cursor_x != 0x40);  // still the dialog: the game's cursor doesn't move
  click_at(x + 8, no_y);
  run(kSettle * 3);
  pointer_move(0x44, 0x54);
  run(3);
  CHECK(cursor_x == 0x44 && cursor_y == 0x54);  // back in the game

  // The drawn pointer is the game's own, to the pixel: drawn over the game's cursor, it changes nothing.
  // In the picture, and below the split, where the PPU's registers differ.
  check_pointer_is_the_games(0x44, 0x54);
  check_pointer_is_the_games(0x20, 0xB8);

  check_address_book();

  if(failures) {
    fprintf(stderr, "test_pointer: %d checks failed\n", failures);
    return 1;
  }
  printf("test_pointer: all checks passed\n");
  return 0;
}
