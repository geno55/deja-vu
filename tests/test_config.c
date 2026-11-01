// Checks the settings file (src/config.h) without the ROM.
//
// usage: test_config DIR    (DIR: a place to put its files)
#include <stdio.h>
#include <string.h>

#include "config.h"

static int failures;

#define CHECK(cond)                                                    \
  do {                                                                 \
    if(!(cond)) {                                                      \
      fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #cond); \
      failures++;                                                      \
    }                                                                  \
  } while(0)

static char path[1024], err[1200];

// Reads this text as a file, starting from `start`; returns the setting, or -1 if the read failed.
static int read_text(const char *text, int start) {
  FILE *f = fopen(path, "wb");
  CHECK(f != NULL);
  fputs(text, f);
  fclose(f);
  DvConfig c = {start};
  return config_read(path, &c, err, sizeof(err)) ? -1 : c.fix_bugs;
}

int main(int argc, char **argv) {
  if(argc != 2) {
    fprintf(stderr, "usage: test_config DIR\n");
    return 2;
  }
  snprintf(path, sizeof(path), "%s/test_config.ini", argv[1]);

  CHECK(read_text("[Features]\nFixBugs = 0\n", 1) == 0);
  CHECK(read_text("[Features]\nFixBugs = 1\n", 0) == 1);
  // As an editor might leave it: other line ends, case, spaces, comments, no last line end.
  CHECK(read_text("# a comment\r\n\r\n; another\r\n  [features]  \r\n  fixbugs=OFF\r\n", 1) == 0);
  CHECK(read_text("[Features]\nFixBugs = yes", 0) == 1);
  // What it doesn't name keeps its default.
  CHECK(read_text("", 1) == 1);
  CHECK(read_text("", 0) == 0);
  // A line it can't use is skipped, and the rest still count.
  CHECK(read_text("[Features]\nFixBugs = maybe\n", 1) == 1);
  CHECK(read_text("[Features]\nNoSuchSetting = 1\nFixBugs = 0\n", 1) == 0);
  CHECK(read_text("nonsense\n[Features\nFixBugs = 0\n", 1) == 1);
  // The right key in another section isn't the setting.
  CHECK(read_text("[General]\nFixBugs = 0\n", 1) == 1);

  // The other setting, on its own.
  {
    FILE *f = fopen(path, "wb");
    CHECK(f != NULL);
    fputs("[Features]\nFixText = 1\n", f);
    fclose(f);
    DvConfig other = {0};
    CHECK(config_read(path, &other, err, sizeof(err)) == 0);
    CHECK(other.fix_text == 1 && other.fix_bugs == 0);
    f = fopen(path, "wb");
    CHECK(f != NULL);
    fputs("[Features]\nMouseAndTouch = 1\n", f);
    fclose(f);
    DvConfig mouse = {0};
    CHECK(config_read(path, &mouse, err, sizeof(err)) == 0);
    CHECK(mouse.mouse_and_touch == 1 && mouse.fix_text == 0 && mouse.fix_bugs == 0);
  }

  // No file: it says so, and the settings stay.
  remove(path);
  DvConfig c = {1};
  err[0] = 0;
  CHECK(config_read(path, &c, err, sizeof(err)) == -1);
  CHECK(err[0] != 0 && c.fix_bugs == 1);

  if(failures) {
    fprintf(stderr, "test_config: %d checks failed\n", failures);
    return 1;
  }
  printf("test_config: all checks passed\n");
  return 0;
}
