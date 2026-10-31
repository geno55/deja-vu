// Checks the save file (src/save_file.h) without the ROM. A save is what a player can't get back, so
// this goes through what can go wrong with one: a file that isn't a save, nothing to write, and that
// the file a run started from stays somewhere.
//
// usage: test_save_file DIR    (DIR: a place to put its files)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "save_file.h"

static int failures;

#define CHECK(cond)                                                    \
  do {                                                                 \
    if(!(cond)) {                                                      \
      fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #cond); \
      failures++;                                                      \
    }                                                                  \
  } while(0)

static char save_path[1024], bak_path[1100], new_path[1100];
static uint8_t sram[kSaveSize], image[kSaveSize], other[kSaveSize], got[0x8000 + 1];
static char err[1200];

static void put_file(const char *path, const uint8_t *bytes, size_t size) {
  FILE *f = fopen(path, "wb");
  CHECK(f != NULL);
  if(size) {
    CHECK(fwrite(bytes, 1, size, f) == size);
  }
  fclose(f);
}

// Whether the file is these bytes exactly.
static int file_is(const char *path, const uint8_t *bytes, size_t size) {
  FILE *f = fopen(path, "rb");
  if(!f) {
    return 0;
  }
  size_t n = fread(got, 1, sizeof(got), f);
  fclose(f);
  return n == size && !memcmp(got, bytes, size);
}

static int exists(const char *path) {
  FILE *f = fopen(path, "rb");
  if(f) {
    fclose(f);
  }
  return f != NULL;
}

static void clean(void) {
  remove(save_path);
  remove(bak_path);
  remove(new_path);
}

// No file: zeros, nothing written until there's something to write, and no backup of nothing.
static void test_no_file(void) {
  clean();
  memset(sram, 0xAA, sizeof(sram));
  CHECK(save_file_load(save_path, sram, err, sizeof(err)) == 0);
  CHECK(sram[0] == 0 && sram[kSaveSize - 1] == 0);
  CHECK(save_file_update(save_path, sram, err, sizeof(err)) == 0);
  CHECK(!exists(save_path));
  sram[5] = 0x55;
  CHECK(save_file_update(save_path, sram, err, sizeof(err)) == 0);
  CHECK(file_is(save_path, sram, kSaveSize));
  CHECK(!exists(bak_path));
  CHECK(!exists(new_path));
}

// A file as an emulator keeps it comes in byte for byte. Nothing is written if nothing changes. The
// first write keeps the file in .bak, later ones leave that alone.
static void test_keeps_the_old_file(void) {
  clean();
  put_file(save_path, image, kSaveSize);
  CHECK(save_file_load(save_path, sram, err, sizeof(err)) == 0);
  CHECK(!memcmp(sram, image, kSaveSize));
  CHECK(save_file_update(save_path, sram, err, sizeof(err)) == 0);
  CHECK(!exists(bak_path));
  sram[0] ^= 0xFF;
  CHECK(save_file_update(save_path, sram, err, sizeof(err)) == 0);
  CHECK(file_is(save_path, sram, kSaveSize));
  CHECK(file_is(bak_path, image, kSaveSize));
  sram[1] ^= 0xFF;
  CHECK(save_file_update(save_path, sram, err, sizeof(err)) == 0);
  CHECK(file_is(save_path, sram, kSaveSize));
  CHECK(file_is(bak_path, image, kSaveSize));
  CHECK(!exists(new_path));
}

// The next run's .bak is what the last one left, not the one before it.
static void test_next_run(void) {
  clean();
  put_file(save_path, image, kSaveSize);
  save_file_load(save_path, sram, err, sizeof(err));
  memcpy(sram, other, kSaveSize);
  CHECK(save_file_update(save_path, sram, err, sizeof(err)) == 0);
  CHECK(save_file_load(save_path, sram, err, sizeof(err)) == 0);
  CHECK(!memcmp(sram, other, kSaveSize));
  sram[9] ^= 0xFF;
  CHECK(save_file_update(save_path, sram, err, sizeof(err)) == 0);
  CHECK(file_is(bak_path, other, kSaveSize));
}

// Anything but 8 KB is refused and left alone, even when the game would go on to write.
static void test_wrong_sizes(void) {
  static const size_t kSizes[] = {0, 1, kSaveSize - 1, kSaveSize + 1, 0x4000, 0x8000};
  static uint8_t big[0x8000];
  memset(big, 0x5A, sizeof(big));
  for(size_t i = 0; i < sizeof(kSizes) / sizeof(kSizes[0]); i++) {
    clean();
    put_file(save_path, big, kSizes[i]);
    err[0] = 0;
    CHECK(save_file_load(save_path, sram, err, sizeof(err)) == -1);
    CHECK(strstr(err, "8 KB") != NULL);
    CHECK(file_is(save_path, big, kSizes[i]));
  }
}

// A place that can't be written: it says so, and the save is still there to try again with.
static void test_cant_write(void) {
  char path[1100];
  snprintf(path, sizeof(path), "%s/no/such/folder/x.sav", save_path);
  CHECK(save_file_load(path, sram, err, sizeof(err)) == 0);
  sram[0] = 1;
  err[0] = 0;
  CHECK(save_file_update(path, sram, err, sizeof(err)) == -1);
  CHECK(err[0] != 0);
}

// The saves folder: made if it's missing, fine if it's there, and a save can go in it.
static void test_make_dir(const char *dir) {
  char folder[1024], path[1100], bak[1200];
  snprintf(folder, sizeof(folder), "%s/test_saves", dir);
  snprintf(path, sizeof(path), "%s/x.sav", folder);
  snprintf(bak, sizeof(bak), "%s.bak", path);
  remove(path);
  remove(bak);
  CHECK(save_file_make_dir(folder) == 0);
  CHECK(save_file_make_dir(folder) == 0);
  CHECK(save_file_load(path, sram, err, sizeof(err)) == 0);
  sram[0] ^= 0xFF;
  CHECK(save_file_update(path, sram, err, sizeof(err)) == 0);
  CHECK(exists(path));
  remove(path);
  remove(bak);
  char no_parent[1200];
  snprintf(no_parent, sizeof(no_parent), "%s/no/such/parent", folder);
  CHECK(save_file_make_dir(no_parent) == -1);
}

// Replacing leaves only the new file, whether or not there was an old one.
static void test_replace(void) {
  clean();
  put_file(new_path, image, kSaveSize);
  CHECK(save_file_replace(new_path, save_path) == 0);
  CHECK(file_is(save_path, image, kSaveSize));
  CHECK(!exists(new_path));
  put_file(new_path, other, kSaveSize);
  CHECK(save_file_replace(new_path, save_path) == 0);
  CHECK(file_is(save_path, other, kSaveSize));
  CHECK(!exists(new_path));
}

int main(int argc, char **argv) {
  if(argc != 2) {
    fprintf(stderr, "usage: test_save_file DIR\n");
    return 2;
  }
  snprintf(save_path, sizeof(save_path), "%s/test_save_file.sav", argv[1]);
  snprintf(bak_path, sizeof(bak_path), "%s.bak", save_path);
  snprintf(new_path, sizeof(new_path), "%s.new", save_path);
  for(int i = 0; i < kSaveSize; i++) {
    image[i] = (uint8_t)(i * 7 + 1);
    other[i] = (uint8_t)(i * 13 + 3);
  }
  test_no_file();
  test_keeps_the_old_file();
  test_next_run();
  test_wrong_sizes();
  test_cant_write();
  test_replace();
  test_make_dir(argv[1]);
  clean();
  if(failures) {
    fprintf(stderr, "test_save_file: %d checks failed\n", failures);
    return 1;
  }
  printf("test_save_file: all checks passed\n");
  return 0;
}
