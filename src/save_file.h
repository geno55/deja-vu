// The save file: the game's 8 KB of save RAM, raw, as emulators keep it, so that saves
// carry over in both directions (docs/saves.md).
#ifndef DV_SAVE_FILE_H
#define DV_SAVE_FILE_H

#include <stddef.h>
#include <stdint.h>

enum { kSaveSize = 0x2000 };

// The file into `sram`, or zeros if there is no file. A file of any other size can't be this game's
// save RAM, and the game would overwrite it, so that is refused: returns -1 with a reason in err, and
// the file is left as it is. Otherwise returns 0.
int save_file_load(const char *path, uint8_t *sram, char *err, size_t err_size);

// Writes `sram` to the file if it has changed since the last load or write, by way of a new file, so
// that a crash leaves either the old save or the new one. The first write after a load keeps the file
// as it was then in PATH.bak. Returns 0, or -1 with a reason in err; the save stays in memory, and
// the next call tries again.
int save_file_update(const char *path, const uint8_t *sram, char *err, size_t err_size);

// Makes a folder for a save, if there isn't one. Returns 0 if it's there now, or -1.
int save_file_make_dir(const char *path);

// Puts the file `from` in the place of `to`, whether or not `to` exists, in one step. Returns 0 or -1.
int save_file_replace(const char *from, const char *to);

#endif  // DV_SAVE_FILE_H
