// The mouse and touch, for the setting MouseAndTouch. Off, as in the harness and the tests, none of this
// does anything, and the game reads the buttons alone.
//
// The game has no pointer of its own, so this drives what it has. The buttons go through the pad: the left
// button or a tap is A, and the right button or a second finger is B. A place goes straight to the
// variables the d-pad would change after many steps: the cursor (get_input), a menu's choice, or the
// address picked. A place only counts when the pointer has moved or clicked, so the d-pad still works
// with the mouse lying still.
//
// A finger moves the cursor while it is down, and clicks where it lifts, so you can see where you aim.
//
// The platform hides the system's cursor, so the pointer is always the game's own: its sprite while the
// game draws it, and otherwise the same picture drawn over the frame (pointer_draw), which is what shows
// it in menus and over text.
#ifndef DV_POINTER_H
#define DV_POINTER_H

#include <stdint.h>

extern int g_pointer_on;

// Places are in the game's picture, and may be outside it: they count as its nearest edge.
void pointer_move(int x, int y);
void pointer_button(int right, int down);
void pointer_finger_down(long long id, int x, int y);
void pointer_finger_move(long long id, int x, int y);
void pointer_finger_up(long long id, int x, int y);

// The buttons to add to the pad for the next frame, in the bits of g_nes.pad. A click lasts a few frames,
// so that the game sees it however short it was.
uint8_t pointer_pad(void);

// While the game waits for input: the cursor to where the pointer went. Returns whether it went anywhere.
int pointer_to_cursor(void);

// Draws the pointer on a finished frame (pixels as ppu_render_frame makes them), unless the game is drawing
// its cursor (it has called pointer_to_cursor since the last frame) or the pointer has never moved.
void pointer_draw(uint16_t *pixels);

// In a menu's loop, after read_joypads: `count` rows, with the menu's cursor at x and at the y's at
// address ys, `step` bytes apart. The row under the pointer becomes *choice, and a click on no row isn't
// a press of A, which would take whatever is chosen now.
void pointer_choose_row(uint8_t *choice, int count, uint8_t x, uint16_t ys, int step);

// The same for the address book's entries (far_8000_66 (6:9947)), after read_input_with_repeat.
void pointer_choose_address(void);

#endif  // DV_POINTER_H
