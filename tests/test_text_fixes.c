// Checks the text corrections (src/text_fixes.h) on the ROM. Every correction must apply, nothing else may
// change, and every changed message must still lay out properly: the window wraps by itself after 26
// characters, so a change of length could split or join words, which this finds as a word the game doesn't
// have and the corrections don't add.
//
// usage: test_text_fixes ROM
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assets.h"
#include "text_fixes.h"

static uint8_t rom[16 + kPrgSize + kChrSize], fixed[kPrgSize];
static int failures;

#define CHECK(cond)                                                    \
  do {                                                                 \
    if(!(cond)) {                                                      \
      fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #cond); \
      failures++;                                                      \
    }                                                                  \
  } while(0)

enum { kMaxWords = 8000, kMaxText = 2048, kWidth = 26 };
static const struct { uint8_t bank; uint16_t table, entries; } kTables[] = {
  {2, 0xA500, 128}, {4, 0x8000, 256}, {5, 0x9A00, 256},
};

static char words[kMaxWords][32];
static int word_count;

static const char *const kNewWords[] = {
  "RECEIVER", "EXCITEMENT", "PORCELAIN", "PEDAL", "SHREDS", "BREATHE", "WRITTEN", "THERE", "STERNWOOD'S", "ITS",
  "ANYTHING", "BULLETPROOF", "LADIES'", "SATURDAY-NIGHT-",
};

static int known(const char *w) {
  for(int i = 0; i < word_count; i++) {
    if(!strcmp(words[i], w)) return 1;
  }
  return 0;
}

static void add_word(const char *w) {
  if(!known(w) && word_count < kMaxWords) {
    snprintf(words[word_count++], sizeof(words[0]), "%s", w);
  }
}

// Calls back with each word as the screen shows it: spaces, line breaks and the window's own wrap end one.
static void for_each_word(const char *text, void (*fn)(const char *, const char *), const char *where) {
  char word[64];
  size_t n = 0;
  int column = 0;
  for(;; text++) {
    char c = *text;
    int line_end = c == '\n' || c == '\f' || c == '\v' || !c;
    int wrap = column == kWidth;
    int word_char = (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '\'' || c == '-';
    if(wrap) column = 0;
    if(!word_char || wrap) {  // punctuation ends a word too
      if(n) {
        word[n] = 0;
        fn(word, where);
        n = 0;
      }
    }
    if(!c) return;
    if(line_end) {
      column = 0;
      continue;
    }
    column++;
    if(word_char && n + 1 < sizeof(word)) {
      word[n++] = c;
    }
  }
}

static void learn(const char *word, const char *where) {
  (void)where;
  add_word(word);
}

static void check_known(const char *word, const char *where) {
  if(!known(word)) {
    fprintf(stderr, "test_text_fixes: %s shows a word it shouldn't: %s\n", where, word);
    failures++;
  }
}

// A full line followed by a line break would leave a blank line; the original has a few, none new may come.
static int full_lines_before_breaks(const char *text) {
  int count = 0, column = 0;
  for(; *text; text++) {
    if(*text == '\n' || *text == '\f' || *text == '\v') {
      count += column == kWidth && *text == '\n';
      column = 0;
    } else {
      column = column == kWidth ? 1 : column + 1;
    }
  }
  return count;
}

static uint16_t entry(const uint8_t *prg, int bank, uint16_t addr) {
  uint32_t at = (uint32_t)bank * 0x4000 + (addr & 0x3FFF);
  return (uint16_t)(prg[at] | prg[at + 1] << 8);
}

int main(int argc, char **argv) {
  if(argc != 2) {
    fprintf(stderr, "usage: test_text_fixes ROM\n");
    return 2;
  }
  FILE *f = fopen(argv[1], "rb");
  if(!f || fread(rom, 1, sizeof(rom), f) != sizeof(rom)) {
    fprintf(stderr, "test_text_fixes: can't read %s\n", argv[1]);
    return 2;
  }
  fclose(f);
  const uint8_t *prg = rom + 16;
  memcpy(fixed, prg, kPrgSize);

  char err[256];
  int changed = text_fixes_apply(fixed, err, sizeof(err));
  if(changed < 0) {
    fprintf(stderr, "test_text_fixes: %s\n", err);
    return 1;
  }

  // Every correction's words are in its message, and every message it names is one the tables reach.
  for(size_t i = 0; i < kTextFixCount; i++) {
    char text[kMaxText];
    CHECK(text_fix_decode(prg, kTextFixes[i].bank, kTextFixes[i].addr, text, sizeof(text)) > 0);
    CHECK(strstr(text, kTextFixes[i].from) != NULL);
  }

  char before[kMaxText], after[kMaxText], where[32];
  for(size_t t = 0; t < sizeof(kTables) / sizeof(kTables[0]); t++) {
    for(int e = 0; e < kTables[t].entries; e++) {
      uint16_t a = entry(prg, kTables[t].bank, (uint16_t)(kTables[t].table + e * 2));
      if(text_fix_decode(prg, kTables[t].bank, a, before, sizeof(before)) > 0) {
        for_each_word(before, learn, "");
      }
    }
  }
  for(size_t i = 0; i < sizeof(kNewWords) / sizeof(kNewWords[0]); i++) {
    add_word(kNewWords[i]);
  }

  int messages = 0, different = 0;
  for(size_t t = 0; t < sizeof(kTables) / sizeof(kTables[0]); t++) {
    int bank = kTables[t].bank;
    for(int e = 0; e < kTables[t].entries; e++) {
      uint16_t a = entry(prg, bank, (uint16_t)(kTables[t].table + e * 2));
      uint16_t b = entry(fixed, bank, (uint16_t)(kTables[t].table + e * 2));
      int old_len = text_fix_decode(prg, bank, a, before, sizeof(before));
      int new_len = text_fix_decode(fixed, bank, b, after, sizeof(after));
      CHECK((old_len < 0) == (new_len < 0));
      if(old_len < 0) {
        CHECK(a == b);
        continue;
      }
      messages++;
      CHECK(!strstr(after, "CABBY"));  // one spelling
      int named = 0;
      for(size_t i = 0; i < kTextFixCount; i++) {
        named |= kTextFixes[i].bank == bank && kTextFixes[i].addr == a;
      }
      if(!named) {
        CHECK(a == b && !strcmp(before, after));
        continue;
      }
      different++;
      CHECK(strcmp(before, after));
      snprintf(where, sizeof(where), "%d:%04X", bank, a);
      for_each_word(after, check_known, where);
      if(full_lines_before_breaks(after) > full_lines_before_breaks(before)) {
        fprintf(stderr, "test_text_fixes: %s has a new blank line\n", where);
        failures++;
      }
    }
  }
  // Nothing but the message tables, the unused space and the changed messages' own bytes moved.
  size_t outside = 0;
  for(size_t i = 0; i < kPrgSize; i++) {
    uint16_t addr = (uint16_t)(0x8000 + (i & 0x3FFF));
    int bank = (int)(i / 0x4000), allowed = 0;
    allowed |= bank == 2 && (addr >= 0xBA26 || (addr >= 0xA500 && addr < 0xA600));
    allowed |= bank == 2 && addr == 0x8D00 + 0xB6 * 8 + 7;  // the last letter of OFREEALL
    allowed |= bank == 4 && (addr >= 0xB61D || addr < 0x8200);
    allowed |= bank == 5 && (addr >= 0xB9D8 || (addr >= 0x9A00 && addr < 0x9C00));
    for(size_t f = 0; f < kTextFixCount && !allowed; f++) {
      if(kTextFixes[f].bank == bank) {
        int size = text_fix_stored_size(prg, bank, kTextFixes[f].addr);
        allowed = addr >= kTextFixes[f].addr && addr < kTextFixes[f].addr + size;
      }
    }
    outside += !allowed && prg[i] != fixed[i];
  }
  CHECK(outside == 0);
  // The item's name becomes OFREEALL, as the medicine list has it, which keeps its spelling.
  static const uint8_t kName[8] = {0xBF, 0xB6, 0xC2, 0xB5, 0xB5, 0xB1, 0xBC, 0xBC};
  uint32_t name = 2 * 0x4000 + ((0x8D00 + 0xB6 * 8) & 0x3FFF);
  CHECK(!memcmp(fixed + name, kName, 8));
  CHECK(prg[name + 7] == 0xFF);
  CHECK(changed > 0 && different >= changed);
  if(failures) {
    fprintf(stderr, "test_text_fixes: %d checks failed\n", failures);
    return 1;
  }
  printf("test_text_fixes: %d corrections to %d messages, of %d, all laid out properly\n", (int)kTextFixCount, changed,
         messages);
  return 0;
}
