// What the game needs from the machine it runs on: a window, the buttons, and the pace of a frame.
#ifndef DV_PLATFORM_H
#define DV_PLATFORM_H

#include <stddef.h>
#include <stdint.h>

// Opens the window, `scale` times the game's picture. Returns 0, or -1 after saying why on stderr.
int platform_open(int scale);

// Starts the sound, mono, 16-bit samples at `sample_rate`. No sound card isn't an error: it says so on
// stderr, and platform_queue_audio does nothing.
void platform_open_audio(unsigned sample_rate);

// Sound to play after what's already waiting.
void platform_queue_audio(const int16_t *samples, size_t count);

// Shows a frame (`rgb`: 256 x 240 pixels by rows, 0x00RRGGBB), waits until it is time for the next, and
// returns 0 if the player has closed the window. *pad gets the buttons held, in the bits of g_nes.pad.
// screenshot, if not NULL, names a BMP file to write what the window shows.
int platform_frame(const uint32_t *rgb, uint8_t *pad, const char *screenshot);

void platform_close(void);

// Shows an error that stops the game where the player will see it: a message box in a release build on
// Windows (it works before platform_open, and needs no console), as zelda3's Die. Elsewhere, nothing.
void platform_show_error(const char *message);

#endif  // DV_PLATFORM_H
