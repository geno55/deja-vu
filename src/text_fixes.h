// Corrections to the original's text, for the setting FixText (docs/text.md). The messages come from the
// player's ROM, so this holds only the words to change: each changed message is decoded, corrected, encoded
// again, in its own place if it fits, and otherwise in the bank's unused space, with its entries in the
// message tables pointing there.
#ifndef DV_TEXT_FIXES_H
#define DV_TEXT_FIXES_H

#include <stddef.h>
#include <stdint.h>

typedef struct TextFix {
  uint8_t bank;
  uint16_t addr;    // the message, as the original has it
  const char *from, *to;  // in the text, \n is a new line and \f a wait and clear
} TextFix;

extern const TextFix kTextFixes[];
extern const size_t kTextFixCount;

// A message as text, from the bit stream at addr in that bank of `prg`, with \n and \f as above, and \v for
// the wait that ends it. Returns its length, or -1 if it isn't plain text (it names an address to print).
int text_fix_decode(const uint8_t *prg, int bank, uint16_t addr, char *out, size_t size);

// How many bytes that message takes in the ROM, or -1 if it isn't plain text.
int text_fix_stored_size(const uint8_t *prg, int bank, uint16_t addr);

// Applies every correction to `prg` (128 KB). Returns how many messages changed, or -1 with a reason in err,
// leaving `prg` as it was: a correction whose words aren't in the message, or no room.
int text_fixes_apply(uint8_t *prg, char *err, size_t err_size);

#endif  // DV_TEXT_FIXES_H
