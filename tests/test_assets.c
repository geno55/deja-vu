// Checks the game's data (src/assets.h): without the ROM, that what isn't the game is refused; with it,
// that a dump and a pack made from it give the same data, and that a damaged pack is refused.
//
// usage: test_assets DIR [ROM]    (DIR: a place to put its files)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assets.h"

static int failures;

#define CHECK(cond)                                                    \
  do {                                                                 \
    if(!(cond)) {                                                      \
      fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #cond); \
      failures++;                                                      \
    }                                                                  \
  } while(0)

enum { kFileSize = 16 + kPrgSize + kChrSize };

static GameData dump, from_pack;
static uint8_t file[kFileSize + 1];
static char err[1200];

static void put_file(const char *path, const uint8_t *bytes, size_t size) {
  FILE *f = fopen(path, "wb");
  CHECK(f != NULL);
  if(size) {
    CHECK(fwrite(bytes, 1, size, f) == size);
  }
  fclose(f);
}

static size_t get_file(const char *path, uint8_t *bytes, size_t size) {
  FILE *f = fopen(path, "rb");
  if(!f) {
    return 0;
  }
  size_t n = fread(bytes, 1, size, f);
  fclose(f);
  return n;
}

static void test_not_the_game(const char *path) {
  remove(path);
  CHECK(assets_read(path, &dump, err, sizeof(err)) == kAssetsMissing);

  put_file(path, file, 100);
  CHECK(assets_read(path, &dump, err, sizeof(err)) == kAssetsBad);
  CHECK(strstr(err, "size") != NULL);

  put_file(path, file, kFileSize + 1);
  CHECK(assets_read(path, &dump, err, sizeof(err)) == kAssetsBad);

  memset(file, 0x41, sizeof(file));
  put_file(path, file, kFileSize);
  CHECK(assets_read(path, &dump, err, sizeof(err)) == kAssetsBad);
  CHECK(strstr(err, "isn't Deja Vu") != NULL);
  remove(path);
}

static void test_with_rom(const char *rom, const char *pack) {
  char temp[1100];
  snprintf(temp, sizeof(temp), "%s.new", pack);
  CHECK(assets_read(rom, &dump, err, sizeof(err)) == kAssetsOk);
  CHECK(assets_write_pack(pack, &dump, err, sizeof(err)) == 0);
  FILE *leftover = fopen(temp, "rb");
  CHECK(leftover == NULL);
  if(leftover) {
    fclose(leftover);
  }
  CHECK(assets_read(pack, &from_pack, err, sizeof(err)) == kAssetsOk);
  CHECK(!memcmp(&dump, &from_pack, sizeof(dump)));

  // Writing over a pack works too.
  CHECK(assets_write_pack(pack, &dump, err, sizeof(err)) == 0);

  // The data of a pack, or of a dump with any header, is what's checked.
  CHECK(get_file(pack, file, sizeof(file)) == kFileSize);
  file[0] = 'N';
  put_file(pack, file, kFileSize);
  CHECK(assets_read(pack, &from_pack, err, sizeof(err)) == kAssetsOk);

  // One byte wrong anywhere in the data.
  CHECK(assets_write_pack(pack, &dump, err, sizeof(err)) == 0);
  CHECK(get_file(pack, file, sizeof(file)) == kFileSize);
  file[16 + kPrgSize + 1234] ^= 0x01;
  put_file(pack, file, kFileSize);
  CHECK(assets_read(pack, &from_pack, err, sizeof(err)) == kAssetsBad);

  // A pack of another version.
  CHECK(assets_write_pack(pack, &dump, err, sizeof(err)) == 0);
  CHECK(get_file(pack, file, sizeof(file)) == kFileSize);
  file[4] = 99;
  put_file(pack, file, kFileSize);
  CHECK(assets_read(pack, &from_pack, err, sizeof(err)) == kAssetsBad);
  CHECK(strstr(err, "another version") != NULL);
  remove(pack);
}

int main(int argc, char **argv) {
  if(argc < 2) {
    fprintf(stderr, "usage: test_assets DIR [ROM]\n");
    return 2;
  }
  char pack[1100];
  snprintf(pack, sizeof(pack), "%s/test_assets.dat", argv[1]);
  test_not_the_game(pack);
  if(argc > 2) {
    test_with_rom(argv[2], pack);
  }
  if(failures) {
    fprintf(stderr, "test_assets: %d checks failed\n", failures);
    return 1;
  }
  printf("test_assets: all checks passed%s\n", argc > 2 ? "" : " (the ones that need no ROM)");
  return 0;
}
