# Editable data formats

Every block of data the assembly takes from the ROM (`asm/bins.txt`) has an editable form
and an encoder that turns it back into bytes. `python tools/roundtrip.py` writes all of them
to `build/src/` and checks that encoding each one gives back the original bytes exactly; it
does for every block. The files hold the game's data, so they are never committed.

| Block | File | Form |
|---|---|---|
| `messages_2/4/5_data` | `.txt` | `@ADDR` per message, then the text with `{TAGS}`: `{N}` new line, `{WAIT}`, `{WAITCLEAR}`, `{CLEAR}`, `{END}`, `{WAITEND}`, `{ITEM nn}`, `{WORD nn}`, `{NUMBER $addr}`, `{ITEMAT $addr}`, `{Cnn}` for other codes. The encoder rebuilds the bit stream with the ROM's code table ([text.md](text.md)). |
| `item_names_data`, `word_names_data` | `.txt` | one name per line (8 tiles, `$FF` padding left out) |
| `scripts_*_data` | `.txt` | `ADDR: mnemonic args`, one opcode per line, with the names from [scripts.md](scripts.md) |
| `scenes_0/1/2_data` | `.txt` | `@ADDR scene NN`, 14 rows of 14 tile numbers, then `attrs` and 16 bytes ([scenes.md](scenes.md)) |
| `hotspots_data` | `.txt` | `@ADDR rooms ...`, then `x1 x2 y1 y2 kind id` per hotspot and `end` ([rooms.md](rooms.md)) |
| `layouts_data` | `.txt` | `@ADDR` per layout, then text with `{MOVE x y}`, `{ITEM nn}`, `{CMDnn $addr}`, `{xx}` tiles |
| `sound_data` | `.txt` | headers (`type`, channel addresses, `speed`), channel streams as commands (`nXX` note, `rest`, `wait`, `lenXX`, `loopN`, `endloop`, `breakN`, `inst $addr`, ..., `end`), instruments as bytes ([sound.md](sound.md)) |
| `unused_4/5_data` | `.txt` | hex bytes |
| `chr` (256 KB of graphics) | `.png` | 16 tiles across, 4 gray levels; `tools/chr_png.py` converts either way and accepts 8-bit palette or grayscale PNGs from an image editor |

The code is in `tools/formats.py` and `tools/chr_png.py`; `tests/test_formats.py` checks the
encoders on made-up data, and the `data_roundtrip` CMake test runs the full check when your
ROM is present.

## Not yet: building a modified ROM
The encoders produce correct bytes for edited files, but the build still takes its data
from the ROM. Building from edited files needs one more step: labels inside a block (each
message, script, picture) are fixed offsets in the assembly today, so anything that changes
a block's length or moves things inside it needs those labels generated from the encoded
data, and the bank must have room for the new size.
