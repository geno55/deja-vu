// The platform for a build without SDL2: no window, so `dejavu` only runs with --no-window.
#include "platform.h"

#include <stdio.h>

int platform_open(int scale) {
  (void)scale;
  fprintf(stderr, "dejavu: built without SDL2, so there's no window (tools/fetch_sdl2.py, then CMake's "
                  "configure step again)\n");
  return -1;
}

void platform_open_audio(unsigned sample_rate) {
  (void)sample_rate;
}

void platform_queue_audio(const int16_t *samples, size_t count) {
  (void)samples;
  (void)count;
}

int platform_frame(const uint32_t *rgb, uint8_t *pad, const char *screenshot) {
  (void)rgb;
  (void)screenshot;
  *pad = 0;
  return 0;
}

void platform_capture_mouse(void) {
}

void platform_close(void) {
}

void platform_show_error(const char *message) {
  (void)message;
}
