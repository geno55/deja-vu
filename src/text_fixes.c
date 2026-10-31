// See text_fixes.h. The format is in docs/text.md. The window wraps by itself after 26 characters, and the
// text relies on that, so a correction that changes a line's length often has to move a line break too.
#include "text_fixes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  kCodeTable = 0xE4FC,  // 7: bit-stream index -> character code
  kTileTable = 0xE213,  // 7: character code -> tile
  kCodeWaitClear = 0x1C,
  kCodeNewLine = 0x1D,
  kCodeWaitEnd = 0x1F,
  kCodeNumber = 0x3E,
  kCodeItemAt = 0x3F,
  kBankSize = 0x4000,
  kMaxMessage = 2048,
  kItemNames = 0x8D00,  // 2: 8 tiles a name, $FF padding
  kItemOfreeall = 0xB6,
  kTileL = 0xBC,
  kPaddingTile = 0xFF,
  kPrgSize_ = 8 * kBankSize,
};

// Where each bank's messages are, and the space nothing reads: $FF at the end of banks 2 and 4, and what the
// assembly calls unused_4 and unused_5, which no routine reaches.
static const struct {
  uint8_t bank;
  uint16_t table, entries, free_start, free_end;
} kBanks[] = {
  {2, 0xA500, 128, 0xBA26, 0xC000},
  {4, 0x8000, 256, 0xB61D, 0xC000},
  {5, 0x9A00, 256, 0xB9D8, 0xBFC0},
};

const TextFix kTextFixes[] = {
  // Spelling.
  {4, 0x842E, "RECIEVER", "RECEIVER"},
  {5, 0xA863, "EXCITMENT BUILDS! YOU COMEUP", "EXCITEMENT BUILDS! YOU\nCOME UP"},
  {4, 0xA2B6, "PORCELIAN", "PORCELAIN"},
  {4, 0x8229, "PEDDLE. I BETIT'D", "PEDAL. I BET\nIT'D"},
  {4, 0x86AC, "SHREADS", "SHREDS"},
  {4, 0x872F, "SHREADS", "SHREDS"},
  {5, 0xB296, "ANTITDOTE", "ANTIDOTE"},
  {5, 0xB296, "BISODIUMITS", "BISODIUMITIS"},
  {4, 0xB3E1, "SEIGEL", "SIEGEL"},
  {2, 0xB872, "PENTATHOL", "PENTOTHAL"},
  {4, 0x9741, "PENTATHOL", "PENTOTHAL"},
  {4, 0x93F1, "ME ANY\nTHING", "ME\nANYTHING"},
  // Grammar.
  {4, 0x9741, "IT'S EFFECT", "ITS EFFECT"},
  {2, 0xB918, "TO BREATH A", "TO BREATHE A"},
  {4, 0x920B, "MUST HAVE WROTE\nTHIS", "MUST HAVE\nWRITTEN THIS"},
  {5, 0xB847, "THEY ARE NO", "THERE ARE NO"},
  {4, 0x89AC, "STERNWOODS", "STERNWOOD'S"},
  // Punctuation and spacing.
  {2, 0xAA6C, "IT' S", "IT'S"},
  {2, 0xB6F4, "SATURDAY- NIGHT", "SATURDAY-NIGHT"},
  {2, 0xB58E, "VISIT,BUT", "VISIT, BUT"},
  {4, 0x8C00, "FARE,\"1", "FARE, \"1"},
  {5, 0xA774, "MRS.STERNWOOD", "MRS. STERNWOOD"},
  {4, 0x9383, "GAUDY, PAIR", "GAUDY PAIR"},
  {4, 0xB200, "BULLETS.", "BULLETS?"},
  {4, 0xB3E1, "GUILTY. THE", "GUILTY, THE"},
  {4, 0xB010, "OK. HOTSHOT", "OK, HOTSHOT"},
  {4, 0xB217, "LAWYER,\nMADE", "LAWYER\nMADE"},
  {4, 0xB217, "LETTER, PROVIDEDTHE", "LETTER PROVIDED\nTHE"},
  {4, 0xA7F0, "- \"2:15", "- 2:15"},
  {4, 0x9741, "\fDIETHANOL TRIMENE", "\f\"DIETHANOL TRIMENE"},
  {4, 0x9741, "COUNTERED.\f", "COUNTERED.\"\f"},
  {4, 0x9741, "THE TRUTH.\f", "THE TRUTH.\"\f"},
  {5, 0xB296, "UNCONSCIOUS.\f", "UNCONSCIOUS.\"\f"},
  {5, 0xB296, "NECESSARY.", "NECESSARY.\""},
  {2, 0xAC6D, "BAR.  YOU", "BAR. YOU"},
  {4, 0xAE3F, "GONE.  WHAT", "GONE. WHAT"},
  {4, 0xABED, " YOU SHOOT", "YOU SHOOT"},
  // One spelling of each.
  {2, 0xB167, "CABBY SAYS, CHUCKLING,\"MISTER, DON'T ACT STUPID.THIS IS WHERE YOU WANT TO\nGO",
   "CABBIE SAYS,\nCHUCKLING, \"MISTER, DON'T\nACT STUPID. THIS IS WHERE\nYOU WANT TO GO"},
  {4, 0x8D7A, "CABBY", "CABBIE"},
  {4, 0x8D9C, "CABBY SAYS, \"WHAT CAN\nI DO", "CABBIE SAYS, \"WHAT\nCAN I DO"},
  {4, 0x8DB5, "CABBY", "CABBIE"},
  {4, 0x8E37, "CABBY SAYS, \"WHERE DO\nYOU", "CABBIE SAYS, \"WHERE\nDO YOU"},
  {5, 0xB11D, "CABBY", "CABBIE"},
  {5, 0xB586, "CABBY", "CABBIE"},
  {5, 0xB5BC, "CABBY SAYS, \"HEY BUB,\nYOU HAVEN'T PAID YET! PAY\nBEFORE GETTING OUT",
   "CABBIE SAYS, \"HEY\nBUB, YOU HAVEN'T PAID\nYET! PAY BEFORE GETTING\nOUT"},
  {5, 0xB77F, "CABBY", "CABBIE"},
  {5, 0xB7AD, "CABBY", "CABBIE"},
  {5, 0xB7DF, "CABBY", "CABBIE"},
  {4, 0x8D01, "BULLET-PROOF", "BULLETPROOF"},
  {5, 0xB047, "BULLET-PROOF", "BULLETPROOF"},
  {5, 0xA26B, "LADIES ROOM", "LADIES' ROOM"},
  {5, 0xA36D, "LADIES TOILET", "LADIES' TOILET"},
};
const size_t kTextFixCount = sizeof(kTextFixes) / sizeof(kTextFixes[0]);

static uint32_t bank_offset(int bank, uint16_t addr) {
  return (uint32_t)bank * kBankSize + (addr & (kBankSize - 1));
}

// The tile for a character of the text, or 0 if there's none.
static uint8_t char_tile(char c) {
  if(c >= 'A' && c <= 'Z') return (uint8_t)(0xB1 + c - 'A');
  if(c >= '0' && c <= '9') return (uint8_t)(0x80 + c - '0');
  static const struct { char c; uint8_t tile; } kOthers[] = {
    {' ', 0x7E}, {'!', 0x8A}, {'?', 0x8B}, {'.', 0xA1}, {'-', 0xA4}, {'*', 0xA5}, {'=', 0xCB}, {'$', 0xCC},
    {'\'', 0xCD}, {'"', 0xD5}, {':', 0xD6}, {';', 0xD7}, {',', 0xD8}, {'(', 0xD9}, {')', 0xDA},
  };
  for(size_t i = 0; i < sizeof(kOthers) / sizeof(kOthers[0]); i++) {
    if(kOthers[i].c == c) return kOthers[i].tile;
  }
  return 0;
}

static char code_char(const uint8_t *prg, uint8_t code) {
  if(code == kCodeNewLine) return '\n';
  if(code == kCodeWaitClear) return '\f';
  if(code == kCodeWaitEnd) return '\v';
  uint8_t tile = prg[bank_offset(7, (uint16_t)(kTileTable + code))];
  for(int c = ' '; c <= 'Z'; c++) {
    if(char_tile((char)c) == tile) return (char)c;
  }
  // No character for it (the game's # is one): kept as it is, by its code.
  return (char)(0x80 | code);
}

static int decode_message(const uint8_t *prg, int bank, uint16_t addr, char *out, size_t size, uint32_t *stored) {
  uint32_t start = bank_offset(bank, addr), pos = start;
  uint8_t bits = 0;
  int count = 0;
  size_t n = 0;
  for(;;) {
    int run = 0, index = 0;
    for(int phase = 0; phase < 4;) {
      if(!count) {
        if(pos >= (uint32_t)(bank + 1) * kBankSize) return -1;
        bits = prg[pos++];
      }
      int bit = bits >> 7;
      bits = (uint8_t)(bits << 1);
      count = (count + 1) & 7;
      if(!phase) {
        if(bit) {
          phase = 1;
        } else if(++run > 7) {
          return -1;
        }
      } else {
        index = index << 1 | bit;
        phase++;
      }
    }
    index |= run << 3;
    uint8_t code = prg[bank_offset(7, (uint16_t)(kCodeTable + index))];
    if(code == kCodeNumber || code == kCodeItemAt || n + 2 > size) {
      return -1;
    }
    out[n++] = code_char(prg, code);
    if(code == kCodeWaitEnd) {
      out[n] = 0;
      *stored = pos - start;
      return (int)n;
    }
  }
}

int text_fix_decode(const uint8_t *prg, int bank, uint16_t addr, char *out, size_t size) {
  uint32_t stored;
  return decode_message(prg, bank, addr, out, size, &stored);
}

int text_fix_stored_size(const uint8_t *prg, int bank, uint16_t addr) {
  char text[kMaxMessage];
  uint32_t stored;
  return decode_message(prg, bank, addr, text, sizeof(text), &stored) < 0 ? -1 : (int)stored;
}

// The index in the bit stream's code table for a character of the text, or -1.
static int char_index(const uint8_t *prg, char c) {
  int code;
  if(c == '\n') {
    code = kCodeNewLine;
  } else if(c == '\f') {
    code = kCodeWaitClear;
  } else if(c == '\v') {
    code = kCodeWaitEnd;
  } else if(c & 0x80) {
    code = c & 0x7F;
  } else {
    uint8_t tile = char_tile(c);
    for(code = 0; code < 0x40; code++) {
      if(code != kCodeNewLine && code != kCodeWaitClear && tile &&
         prg[bank_offset(7, (uint16_t)(kTileTable + code))] == tile) {
        break;
      }
    }
    if(code == 0x40) return -1;
  }
  for(int i = 0; i < 64; i++) {
    if(prg[bank_offset(7, (uint16_t)(kCodeTable + i))] == code) return i;
  }
  return -1;
}

// Encodes the text. Returns its bytes, or -1.
static int encode(const uint8_t *prg, const char *text, uint8_t *out, size_t size) {
  size_t n = 0;
  int count = 0;
  uint8_t bits = 0;
#define PUT_BIT(b)                 \
  do {                             \
    bits = (uint8_t)(bits << 1 | (b)); \
    if(++count == 8) {             \
      if(n >= size) return -1;     \
      out[n++] = bits;             \
      count = 0;                   \
      bits = 0;                    \
    }                              \
  } while(0)
  for(; *text; text++) {
    int index = char_index(prg, *text);
    if(index < 0) return -1;
    for(int i = 0; i < index >> 3; i++) PUT_BIT(0);
    PUT_BIT(1);
    for(int i = 2; i >= 0; i--) PUT_BIT((index >> i) & 1);
  }
  while(count) PUT_BIT(0);
#undef PUT_BIT
  return (int)n;
}

// Replaces the only `from` in `text`. Returns 0, or -1 if it isn't there once.
static int replace_once(char *text, size_t size, const char *from, const char *to) {
  char *at = strstr(text, from);
  if(!at || strstr(at + 1, from)) return -1;
  size_t from_len = strlen(from), to_len = strlen(to), len = strlen(text);
  if(len - from_len + to_len + 1 > size) return -1;
  memmove(at + to_len, at + from_len, len - (size_t)(at - text) - from_len + 1);
  memcpy(at, to, to_len);
  return 0;
}

int text_fixes_apply(uint8_t *prg, char *err, size_t err_size) {
  uint8_t *copy = malloc(kPrgSize_);
  if(!copy) {
    snprintf(err, err_size, "no memory for the text corrections");
    return -1;
  }
  memcpy(copy, prg, kPrgSize_);
  uint16_t free_at[sizeof(kBanks) / sizeof(kBanks[0])];
  for(size_t b = 0; b < sizeof(kBanks) / sizeof(kBanks[0]); b++) {
    free_at[b] = kBanks[b].free_start;
  }
  // The item's name lacks the last letter that the medicine list and the other versions have, and fits it.
  uint32_t ofreeall_end = bank_offset(2, (uint16_t)(kItemNames + kItemOfreeall * 8 + 7));
  if(copy[ofreeall_end - 1] != kTileL || copy[ofreeall_end] != kPaddingTile) {
    snprintf(err, err_size, "the name of item $%02X isn't what the correction expects", kItemOfreeall);
    free(copy);
    return -1;
  }
  copy[ofreeall_end] = kTileL;

  int changed = 0;
  for(size_t i = 0; i < kTextFixCount; i++) {
    const TextFix *fix = &kTextFixes[i];
    int first = 1;
    for(size_t j = 0; j < i; j++) {
      first &= kTextFixes[j].bank != fix->bank || kTextFixes[j].addr != fix->addr;
    }
    if(!first) continue;

    char text[kMaxMessage];
    uint8_t coded[kMaxMessage];
    size_t b = 0;
    while(kBanks[b].bank != fix->bank) b++;
    uint32_t stored = 0;
    const char *problem =
        decode_message(prg, fix->bank, fix->addr, text, sizeof(text), &stored) < 0 ? "can't read it" : NULL;
    for(size_t j = i; j < kTextFixCount && !problem; j++) {
      if(kTextFixes[j].bank == fix->bank && kTextFixes[j].addr == fix->addr &&
         replace_once(text, sizeof(text), kTextFixes[j].from, kTextFixes[j].to)) {
        problem = kTextFixes[j].from;
      }
    }
    int len = problem ? -1 : encode(prg, text, coded, sizeof(coded));
    if(!problem && len < 0) {
      problem = "can't encode it";
    }
    if(!problem && (uint32_t)len > stored && free_at[b] + len > kBanks[b].free_end) {
      problem = "no room";
    }
    if(problem) {
      snprintf(err, err_size, "the correction to the message at %d:%04X doesn't apply: %s", fix->bank, fix->addr,
               problem);
      memcpy(prg, copy, kPrgSize_);
      free(copy);
      return -1;
    }
    if((uint32_t)len <= stored) {
      memcpy(copy + bank_offset(fix->bank, fix->addr), coded, (size_t)len);
      changed++;
      continue;
    }
    memcpy(copy + bank_offset(fix->bank, free_at[b]), coded, (size_t)len);
    for(int e = 0; e < kBanks[b].entries; e++) {
      uint32_t entry = bank_offset(fix->bank, (uint16_t)(kBanks[b].table + e * 2));
      if((prg[entry] | prg[entry + 1] << 8) == fix->addr) {
        copy[entry] = (uint8_t)free_at[b];
        copy[entry + 1] = (uint8_t)(free_at[b] >> 8);
      }
    }
    free_at[b] = (uint16_t)(free_at[b] + len);
    changed++;
  }
  memcpy(prg, copy, kPrgSize_);
  free(copy);
  return changed;
}
