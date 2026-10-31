# Text format

All of Deja Vu's messages are compressed with a simple variable-length code. `tools/text.py`
decodes every one of them (636 messages, 26,139 bytes) into `build/text/messages.txt`.
That file contains the game's text, so it stays in `build/` and isn't committed.

Names in this document match [asm/labels.txt](../asm/labels.txt).

## Where text lives
| Table | Entries | Printed by | Notes |
|---|---|---|---|
| `5:9A00` | 256 | `print_message_5` (`7:C4AE`), A = id | object descriptions, story text |
| `4:8000` | 256 | `print_message_4` (`7:C4EF`), A = id | |
| `2:A500` | 128 | `print_message_2_or_5` (`7:C501`), A = id + $80 | ids below $80 go to `5:9A00` entries $80-$FF |

Each table is a list of 16-bit pointers to messages in the same bank; the messages follow
the table. Some entries share a message (for example `5:9A00` #93-#95).
`print_message_from` (`7:C4BC`) takes the bank in `$84` and the table in `$70/$71`.

Two name tables in bank 2 hold fixed-width strings: 8 font tiles each, padded with `$FF`.
- `item_names` (`2:8D00`, 256 entries): item and object names by item id (COIN, LIGHTER, ...).
- `word_names` (`2:9500`, 224 entries): word fragments for building sentences
  (HAVE / HAVEN'T, ARE / AREN'T, ...).

## The code
`print_message` (`7:DF74`) reads the message pointed to by `$00/$01` as a stream of bits,
most significant bit first (`text_read_bit`, `7:DF4C`; buffer in `$3E`, bits used in `$3D`).

Each character (`text_read_code`, `7:DF00`):
1. Count 0 bits until a 1 bit: that count is the high part of an index.
2. Read 3 more bits: the low part. `index = count * 8 + bits` (0-63).
3. `text_code_table` (`7:E4FC`) turns the index into a character code. It's ordered by
   frequency (space, E, T, O, A, I, S, N, ...), so common characters take 4 bits.

This is a Golomb-Rice code with k = 3, the same idea as in Kemco's *Uninvited*
(see hUninvited), with one lookup table instead of two.

## Character codes
| Code | Meaning |
|---|---|
| `$00-$19` | A-Z |
| `$1A` | space |
| `$1B` | fill the page with blank lines and wait for a button (`<wait>`) |
| `$1C` | wait, then clear the window (`<wait><clear>`) |
| `$1D` | new line |
| `$1E` | end of message |
| `$1F` | wait, then end of message |
| `$20-$3A` | punctuation and digits, through `text_tile_table` (`7:E213`): `. - * = : , " ! ? ; ' $ ( )` and 0-9 |
| `$3B` | clear the window |
| `$3C` | the next 8 bits are an `item_names` index; print that name |
| `$3D` | the next 8 bits are a `word_names` index; print that word |
| `$3E` | the next 2 bytes (byte-aligned, after the current byte) are a RAM address; print the number there in decimal |
| `$3F` | the next 2 bytes are a RAM address holding an item id; print its name |

After `$3E` and `$3F` the printer drops the unread bits of the current byte and continues at
the byte after the address. `$3E` reads two bytes, but `number_to_digits` clears the high one,
so a number is only ever its low byte: up to three digits, without leading zeros.

The printer handles more than a message can ask for: `text_code_table` has no entry for `$1B`,
`$1E`, `$3B`, `$3C` or `$3D`, so the bit stream can't produce them. Every message ends in `$1F`.
That leaves `print_item_name_arg`, `print_word_arg` and `text_read_byte` with no way to run,
which is what both TAS movies show (the port keeps them, and the fuzz checks them).

Letters, spaces, punctuation and names all go through `print_tile` (`7:E1F2`). The window is
26 characters wide and 4 lines tall. The printer wraps by itself after 26 characters, so the
text relies on that: "PALM" + "IS" with no space between them shows as two lines.
`messages.txt` shows those automatic line breaks too.

## The window
The window is in the second nametable, and the sprite-0 split shows it below the picture
(`clear_text_window` sets `split_nametable`; a message's end clears it, unless `text_keep_open`
says more messages follow). `text_window` keeps what it shows as 8 rows of 26 tiles, a blank
row above each line of text, so that it can scroll. A tile goes to VRAM through the queue and
to `text_window`. The cursor starts on the last text row, so text scrolls up from the bottom.

Pressing A while printing sets `text_fast`, which finishes the page at once. Otherwise each tile
comes with the arrow beside it, a click (sound `$31`) and `text_delay` frames. Two settings are
only ever cleared, by name in the code: `text_wait_enabled`, which would turn the end of a page
into timed pauses (`text_pause`) instead of waiting for A, and `text_no_wrap`, which would keep
the cursor on the last column. Neither path ran in the TAS movies.

## Font tiles
From Data Crystal's table: `$B1-$CA` A-Z, `$7E` space, `$80-$89` 0-9, `$8A` !, `$8B` ?,
`$A1` ., `$A4` -, `$A5` *, `$CB` =, `$CC` $, `$CD` ', `$CE` &, `$D5` ", `$D6` :, `$D7` ;,
`$D8` ,, `$D9` (, `$DA` ). Names also use `$B0` as a space and, once, `$D2` (unknown).
