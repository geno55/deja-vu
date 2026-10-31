// Checks the colors of the palette (ppu_make_palette) without the ROM or an emulator. The harness compares
// them with Mesen2's (dv_harness --check-palette); this says what each color is meant to be, so CI catches
// a palette that has gone wrong: the hues rotated by one step, say, which every comparison of the picture
// passed, as those compare palette entries and not what they look like.
#include <math.h>
#include <stdio.h>

#include "ppu_render.h"

static int failures;

#define CHECK(cond, color)                                                              \
  do {                                                                                  \
    if(!(cond)) {                                                                       \
      fprintf(stderr, "%s:%d: color $%02X (%3u, %3u, %3u): %s\n", __FILE__, __LINE__, color, \
              red(color), green(color), blue(color), #cond);                            \
      failures++;                                                                       \
    }                                                                                   \
  } while(0)

static uint32_t palette[kPpuColors];

static unsigned red(int color) { return palette[color] >> 16 & 255; }
static unsigned green(int color) { return palette[color] >> 8 & 255; }
static unsigned blue(int color) { return palette[color] & 255; }

// The color's hue, in degrees: 0 red, 60 yellow, 120 green, 180 cyan, 240 blue, 300 magenta.
static double hue_of(int color) {
  double r = red(color), g = green(color), b = blue(color);
  double hue = atan2(sqrt(3.0) * (g - b), 2 * r - g - b) * 180.0 / 3.14159265358979323846;
  return hue < 0 ? hue + 360 : hue;
}

static double hue_distance(double a, double b) {
  double d = fabs(a - b);
  return d > 180 ? 360 - d : d;
}

static void test_grays(void) {
  static const int kGrays[] = {0x00, 0x10, 0x20, 0x30, 0x2D, 0x3D, 0x0F, 0x1F, 0x2E, 0x3E};
  for(size_t i = 0; i < sizeof(kGrays) / sizeof(kGrays[0]); i++) {
    int c = kGrays[i];
    CHECK(red(c) == green(c) && green(c) == blue(c), c);
  }
  CHECK(red(0x00) < red(0x10) && red(0x10) < red(0x20), 0x10);
  CHECK(red(0x0F) == 0, 0x0F);    // the game's black
  CHECK(red(0x0D) == 0, 0x0D);    // blacker than black
  CHECK(red(0x20) >= 250, 0x20);  // white
  CHECK(red(0x30) >= 250, 0x30);
}

// Each of the 12 hues, a step of 30 degrees apart, where it should be: blue at 2, magenta about 4 or 5,
// red about 6, yellow at 8, green about A, cyan about C.
static void test_hues(void) {
  static const struct {
    int index;
    double hue;
  } kHues[] = {
    {0x2, 240}, {0x4, 290}, {0x6, 0}, {0x8, 60}, {0xA, 110}, {0xC, 185},
  };
  for(int row = 0; row < 3; row++) {
    for(size_t i = 0; i < sizeof(kHues) / sizeof(kHues[0]); i++) {
      int c = row << 4 | kHues[i].index;
      double hue = hue_of(c);
      if(hue_distance(hue, kHues[i].hue) > 15) {
        CHECK(hue_distance(hue, kHues[i].hue) <= 15, c);
        fprintf(stderr, "  its hue is %.0f, and should be about %.0f\n", hue, kHues[i].hue);
      }
    }
  }
}

// How strong they are: the title screen's blue sky and yellow letters, by name. A palette that is too
// pale shows here.
static void test_strength(void) {
  CHECK(blue(0x12) >= 240 && red(0x12) <= 90 && green(0x12) <= 90, 0x12);    // blue
  CHECK(red(0x28) >= 170 && green(0x28) >= 170 && blue(0x28) <= 30, 0x28);   // yellow
  CHECK(green(0x1A) >= 130 && red(0x1A) <= 40 && blue(0x1A) <= 20, 0x1A);    // green
  CHECK(red(0x16) >= 170 && green(0x16) <= 70 && blue(0x16) <= 40, 0x16);    // red
}

// Emphasis: bit 6 of the pixel is PPUMASK's red, which dims the other two.
static void test_emphasis(void) {
  int white = 0x20;
  CHECK(red(white | 0x40) > green(white | 0x40) && red(white | 0x40) > blue(white | 0x40), white | 0x40);
  CHECK(green(white | 0x80) > red(white | 0x80) && green(white | 0x80) > blue(white | 0x80), white | 0x80);
  CHECK(blue(white | 0x100) > red(white | 0x100) && blue(white | 0x100) > green(white | 0x100), white | 0x100);
}

int main(void) {
  ppu_make_palette(palette);
  test_grays();
  test_hues();
  test_strength();
  test_emphasis();
  if(failures) {
    fprintf(stderr, "test_palette: %d checks failed\n", failures);
    return 1;
  }
  printf("test_palette: all checks passed\n");
  return 0;
}
