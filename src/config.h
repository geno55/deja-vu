// The settings file, as zelda3 has it: dejavu.ini next to the program, or dejavu.user.ini in its place, so
// that your settings survive a new version's file. A setting the file doesn't name keeps its default.
#ifndef DV_CONFIG_H
#define DV_CONFIG_H

#include <stddef.h>

typedef struct DvConfig {
  int fix_bugs;  // [Features] FixBugs
  int fix_text;  // [Features] FixText
  int mouse_and_touch;  // [Features] MouseAndTouch
} DvConfig;

// Reads the file at `path` over what's in `config`. A line it can't use is reported on stderr and skipped,
// so a typo doesn't stop the game. Returns 0, or -1 if there's no such file, with a reason in err.
int config_read(const char *path, DvConfig *config, char *err, size_t err_size);

#endif  // DV_CONFIG_H
