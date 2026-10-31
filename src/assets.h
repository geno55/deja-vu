// The game's data: its PRG and CHR ROM, which only ever come from the player's own
// dump. Once a dump has been checked, what the game needs from it is kept in a pack, so the game runs
// without the dump from then on. Neither is ever in the repo.
#ifndef DV_ASSETS_H
#define DV_ASSETS_H

#include <stddef.h>

#include "nes_hw.h"

#define kAssetsPackName "dejavu_assets.dat"

typedef struct GameData {  // one after the other, as in the ROM file
  uint8_t prg[kPrgSize];
  uint8_t chr[kChrSize];
} GameData;

typedef enum AssetsResult {
  kAssetsOk,
  kAssetsMissing,  // no such file
  kAssetsBad,      // it isn't this game's data; err says how
} AssetsResult;

// Reads a dump (any 16-byte header: only the data is checked) or a pack into `data`. A file that is
// neither, or is another game or a damaged copy, is kAssetsBad.
AssetsResult assets_read(const char *path, GameData *data, char *err, size_t err_size);

// Writes the data, which assets_read has checked, as a pack, by way of a new file so that a crash
// can't leave half of one. Returns 0, or -1 with a reason in err.
int assets_write_pack(const char *path, const GameData *data, char *err, size_t err_size);

#endif  // DV_ASSETS_H
