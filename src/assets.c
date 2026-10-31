// See assets.h. A pack is a 16-byte header of its own, then the PRG and CHR ROM. It's checked as a dump
// is, by the CRC of those; a pack from another version of this format is refused, as if damaged, and
// the dump can make a new one.
#include "assets.h"

#include <stdio.h>
#include <string.h>

#include "save_file.h"

enum {
  kHeaderSize = 16,
  kDataSize = kPrgSize + kChrSize,
  kGameCrc32 = 0x43D01C10,  // of the PRG and CHR ROM, whatever the header says
  kPackVersion = 1,
};

static const char kPackMagic[4] = {'D', 'V', 'A', 'S'};

static uint32_t crc32_of(const uint8_t *data, size_t size) {
  uint32_t crc = 0xFFFFFFFFu;
  for(size_t i = 0; i < size; i++) {
    crc ^= data[i];
    for(int bit = 0; bit < 8; bit++) {
      crc = crc >> 1 ^ (crc & 1 ? 0xEDB88320u : 0);
    }
  }
  return ~crc;
}

AssetsResult assets_read(const char *path, GameData *data, char *err, size_t err_size) {
  FILE *f = fopen(path, "rb");
  if(!f) {
    snprintf(err, err_size, "can't read %s", path);
    return kAssetsMissing;
  }
  uint8_t header[kHeaderSize];
  size_t got = fread(header, 1, sizeof(header), f);
  got += fread(data->prg, 1, kPrgSize, f);
  got += fread(data->chr, 1, kChrSize, f);
  int more = fgetc(f) != EOF;
  fclose(f);
  if(got != kHeaderSize + kDataSize || more) {
    snprintf(err, err_size, "%s isn't the size of Deja Vu (USA)'s ROM", path);
    return kAssetsBad;
  }
  if(!memcmp(header, kPackMagic, sizeof(kPackMagic)) && header[4] != kPackVersion) {
    snprintf(err, err_size, "%s is from another version of this program", path);
    return kAssetsBad;
  }
  if(crc32_of(data->prg, sizeof(*data)) != kGameCrc32) {
    snprintf(err, err_size, "%s isn't Deja Vu (USA)", path);
    return kAssetsBad;
  }
  return kAssetsOk;
}

int assets_write_pack(const char *path, const GameData *data, char *err, size_t err_size) {
  char temp[1024];
  snprintf(temp, sizeof(temp), "%s.new", path);
  FILE *f = fopen(temp, "wb");
  if(!f) {
    snprintf(err, err_size, "can't write %s", temp);
    return -1;
  }
  uint8_t header[kHeaderSize] = {0};
  memcpy(header, kPackMagic, sizeof(kPackMagic));
  header[4] = kPackVersion;
  size_t wrote = fwrite(header, 1, sizeof(header), f);
  wrote += fwrite(data->prg, 1, kPrgSize, f);
  wrote += fwrite(data->chr, 1, kChrSize, f);
  if(fclose(f) || wrote != kHeaderSize + kDataSize || save_file_replace(temp, path)) {
    remove(temp);
    snprintf(err, err_size, "can't write %s", path);
    return -1;
  }
  return 0;
}
