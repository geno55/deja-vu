// See pointer.h.
#include "pointer.h"

#include "funcs.h"
#include "nes_hw.h"
#include "ppu_render.h"
#include "variables.h"

enum {
  kPadA = 0x01, kPadB = 0x02,
  kClickFrames = 3,  // about the shortest press of a hand on a pad
  // How far the cursor may go: draw_cursor (6:9922) draws it 5 pixels left of cursor_x, and these are all
  // just outside the rectangles the game tests, so that a click there hits nothing.
  kCursorMinX = 0x05, kCursorMaxX = 0xF0, kCursorMinY = 0x08, kCursorMaxY = 0xE7,
  // Where the d-pad moves the cursor freely (move_cursor_in_scene (6:964A), move_cursor_in_inventory
  // (6:9692)) that is also inside what a click there tests. Anywhere else it goes between commands.
  kPictureX1 = 0x10, kPictureX2 = 0x7F, kPictureY1 = 0x20, kPictureY2 = 0x8F,
  kExitsX1 = 0x86, kExitsX2 = 0xB8, kExitsY1 = 0xB6, kExitsY2 = 0xE5,
  kCommandCursors = 20,  // the places in the tables at kCommandCursorXs
  kModePicture = 0, kModeCommands = 1, kModeExits = 2,
  // A menu's row, around its cursor: its height is the closest two rows get, and its width takes in the
  // longest text.
  kRowLeft = 8, kRowWidth = 0x60, kRowAbove = 4, kRowBelow = 12,
  // The entries of the inventory's list (the rectangles at kEntryRects).
  kEntriesX1 = 0x90, kEntriesX2 = 0xF0, kEntriesY1 = 0x20, kEntryHeight = 0x10, kEntries = 7,
  // The cursor's four tiles, 2 by 2 (the metasprite kCursorSprite), and where CHR ROM has them: menus map
  // other banks. draw_cursor draws it this far left of its x.
  kPointerTiles = 4, kPointerChrAddress = 3 * 0x400 + 0x3A0, kTileBytes = 16, kTilePixels = 8, kPointerLeft = 5,
};

// The colors of the sprite palette the game draws it with, for 1, 2 and 3, as NES colors: a menu's palette
// has others.
static const uint8_t kPointerColors[4] = {0, 0x0F, 0x26, 0x36};

typedef struct Click {
  int held;    // the button is down
  int queued;  // it has been pressed, and the pad hasn't shown it yet
  int left;    // frames still to show it for
  int sent;    // the pad shows it now
} Click;

int g_pointer_on;

static int place_x, place_y;
static int moved;
static int seen;         // the pointer has moved at least once
static int cursor_live;  // pointer_to_cursor has run since the last pointer_draw
static Click click_a, click_b;
static int finger_down;
static long long finger_id;
static int finger_cancelled;  // a second finger has made this a B, so the lift isn't a click

void pointer_move(int x, int y) {
  if(!g_pointer_on) {
    return;
  }
  place_x = x;
  place_y = y;
  moved = 1;
  seen = 1;
}

void pointer_button(int right, int down) {
  if(!g_pointer_on) {
    return;
  }
  Click *k = right ? &click_b : &click_a;
  k->held = down;
  if(down) {
    k->queued = 1;
  }
}

void pointer_finger_down(long long id, int x, int y) {
  if(!g_pointer_on) {
    return;
  }
  if(finger_down) {
    click_b.queued = 1;
    finger_cancelled = 1;
    return;
  }
  finger_down = 1;
  finger_id = id;
  finger_cancelled = 0;
  pointer_move(x, y);
}

void pointer_finger_move(long long id, int x, int y) {
  if(finger_down && id == finger_id) {
    pointer_move(x, y);
  }
}

void pointer_finger_up(long long id, int x, int y) {
  if(!g_pointer_on || !finger_down || id != finger_id) {
    return;
  }
  finger_down = 0;
  if(!finger_cancelled) {
    pointer_move(x, y);
    click_a.queued = 1;
  }
}

static int next_click_state(Click *k) {
  if(k->left) {
    k->left--;
    k->sent = 1;
  } else if(k->queued) {
    // A frame up first if it is showing the last press, or the game would see one long press.
    k->sent = !k->sent;
    if(k->sent) {
      k->queued = 0;
      k->left = kClickFrames - 1;
    }
  } else {
    k->sent = k->held;
  }
  return k->sent;
}

uint8_t pointer_pad(void) {
  if(!g_pointer_on) {
    return 0;
  }
  uint8_t pad = 0;
  if(next_click_state(&click_a)) pad |= kPadA;
  if(next_click_state(&click_b)) pad |= kPadB;
  return pad;
}

static int clamp(int v, int low, int high) {
  return v < low ? low : v > high ? high : v;
}

// The place in the command tables nearest the cursor, which is where the d-pad goes on from.
static uint8_t nearest_command_cursor(void) {
  int best = 0, best_distance = 0x7FFFFFFF;
  for(int i = 0; i < kCommandCursors; i++) {
    int dx = cpu_bus_read((uint16_t)(kCommandCursorXs + i)) - cursor_x;
    int dy = cpu_bus_read((uint16_t)(kCommandCursorYs + i)) - cursor_y;
    int distance = dx * dx + dy * dy;
    if(distance < best_distance) {
      best_distance = distance;
      best = i;
    }
  }
  return (uint8_t)best;
}

void pointer_draw(uint16_t *pixels) {
  int live = cursor_live;
  cursor_live = 0;
  if(!g_pointer_on || !seen || live) {
    return;
  }
  const uint8_t *tiles = g_nes.chr + kPointerChrAddress;
  int left = clamp(place_x, kCursorMinX, kCursorMaxX) - kPointerLeft;
  int top = clamp(place_y, kCursorMinY, kCursorMaxY);
  for(int tile = 0; tile < kPointerTiles; tile++) {
    const uint8_t *pattern = tiles + tile * kTileBytes;
    for(int row = 0; row < kTilePixels; row++) {
      int y = top + (tile >> 1) * kTilePixels + row;
      for(int col = 0; col < kTilePixels; col++) {
        int x = left + (tile & 1) * kTilePixels + col;
        int bit = kTilePixels - 1 - col;
        int color = (pattern[row] >> bit & 1) | (pattern[row + kTilePixels] >> bit & 1) << 1;
        if(color && y < kScreenHeight && x < kScreenWidth) {
          pixels[y * kScreenWidth + x] = kPointerColors[color];
        }
      }
    }
  }
}

int pointer_to_cursor(void) {
  cursor_live = 1;
  if(!moved) {
    return 0;
  }
  moved = 0;
  cursor_x = (uint8_t)clamp(place_x, kCursorMinX, kCursorMaxX);
  cursor_y = (uint8_t)clamp(place_y, kCursorMinY, kCursorMaxY);
  key_repeat = 0;
  if(cursor_x >= kPictureX1 && cursor_x < kPictureX2 && cursor_y >= kPictureY1 && cursor_y < kPictureY2) {
    cursor_mode = kModePicture;
  } else if(cursor_x >= kExitsX1 && cursor_x < kExitsX2 && cursor_y >= kExitsY1 && cursor_y < kExitsY2) {
    cursor_mode = kModeExits;
  } else {
    cursor_mode = kModeCommands;
    command_cursor = nearest_command_cursor();
  }
  return 1;
}

void pointer_choose_row(uint8_t *choice, int count, uint8_t x, uint16_t ys, int step) {
  int pressed = (joy_pressed[0] & 0x80) && click_a.sent;
  if(!g_pointer_on || (!moved && !pressed)) {
    return;
  }
  moved = 0;
  if(place_x >= x - kRowLeft && place_x < x + kRowWidth) {
    for(int i = 0; i < count; i++) {
      int y = cpu_bus_read((uint16_t)(ys + i * step));
      if(place_y >= y - kRowAbove && place_y < y + kRowBelow) {
        *choice = (uint8_t)i;
        return;
      }
    }
  }
  if(pressed) {
    joy_pressed[0] &= 0x7F;
  }
}

void pointer_choose_address(void) {
  int pressed = (data_ptr[0] & 0x80) && click_a.sent;
  if(!g_pointer_on || (!moved && !pressed)) {
    return;
  }
  moved = 0;
  int row = (place_y - kEntriesY1) / kEntryHeight;
  if(place_x >= kEntriesX1 && place_x < kEntriesX2 && place_y >= kEntriesY1 && row < kEntries &&
     address_book[row] != 0xFF) {
    command_cursor = (uint8_t)row;
  } else if(pressed) {
    data_ptr[0] &= 0x7F;
  }
}
