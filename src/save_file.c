// See save_file.h.
#include "save_file.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#include <sys/stat.h>
#endif

static uint8_t on_disk[kSaveSize];  // what the file holds, as far as this program has seen
static int backup_due;              // there was a file at the load, not yet kept in PATH.bak

int save_file_make_dir(const char *path) {
#ifdef _WIN32
  return CreateDirectoryA(path, NULL) || GetLastError() == ERROR_ALREADY_EXISTS ? 0 : -1;
#else
  return !mkdir(path, 0777) || errno == EEXIST ? 0 : -1;
#endif
}

int save_file_replace(const char *from, const char *to) {
#ifdef _WIN32
  // rename() won't replace a file on Windows.
  return MoveFileExA(from, to, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) ? 0 : -1;
#else
  return rename(from, to);
#endif
}

int save_file_load(const char *path, uint8_t *sram, char *err, size_t err_size) {
  memset(on_disk, 0, sizeof(on_disk));
  backup_due = 0;
  FILE *f = fopen(path, "rb");
  if(f) {
    size_t got = fread(on_disk, 1, sizeof(on_disk), f);
    int more = fgetc(f) != EOF;
    fclose(f);
    if(got != sizeof(on_disk) || more) {
      snprintf(err, err_size, "%s isn't 8 KB, so it isn't a save of this game. It hasn't been touched: "
                              "move it, or name another file with --save", path);
      return -1;
    }
    backup_due = 1;
  }
  memcpy(sram, on_disk, sizeof(on_disk));
  return 0;
}

static int write_whole(const char *path, const uint8_t *bytes) {
  FILE *f = fopen(path, "wb");
  if(!f) {
    return -1;
  }
  size_t wrote = fwrite(bytes, 1, kSaveSize, f);
  return fclose(f) || wrote != kSaveSize ? -1 : 0;
}

int save_file_update(const char *path, const uint8_t *sram, char *err, size_t err_size) {
  if(!memcmp(on_disk, sram, sizeof(on_disk))) {
    return 0;
  }
  char temp[1024];
  snprintf(temp, sizeof(temp), "%s.new", path);
  if(write_whole(temp, sram)) {
    remove(temp);
    snprintf(err, err_size, "can't write %s", temp);
    return -1;
  }
  if(backup_due) {
    char backup[1024];
    snprintf(backup, sizeof(backup), "%s.bak", path);
    if(write_whole(backup, on_disk)) {
      remove(temp);
      snprintf(err, err_size, "can't write %s, so %s is left as it was", backup, path);
      return -1;
    }
    backup_due = 0;
  }
  if(save_file_replace(temp, path)) {
    remove(temp);
    snprintf(err, err_size, "can't write %s", path);
    return -1;
  }
  memcpy(on_disk, sram, sizeof(on_disk));
  return 0;
}
