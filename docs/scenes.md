# Scene pictures

Each location's picture is a 14x14 tile map plus its attribute bytes. Banks 0 and 1 hold
nothing else. `tools/scenes.py` decodes all 156 pictures into `build/scenes/scenes.txt`
(not committed: game data).

Names match [asm/labels.txt](../asm/labels.txt).

## Where pictures live
The scene number in `$0515` picks the bank, and an entry in that bank's pointer table at
`$8000` (`select_scene_bank`, `7:C09F`, leaves the index in `$75`):

| Scenes | Bank | Pictures | Then |
|---|---|---|---|
| `$00-$4B` | 0 | 76 | 120 bytes of free space |
| `$4C-$97` | 1 | 76 | 120 bytes of free space |
| `$98-$9B` | 2 | 4 | the rest of bank 2 (code, names, messages) |

## Picture format (212 bytes)
1. **196 bytes:** 14 rows of 14 tile numbers, top row first. They go to nametable `$2082`,
   so the picture's top left is at tile column 2, row 4.
2. **16 bytes:** attribute bytes for the picture's area. `load_scene_attrs` (`7:C186`) copies
   them into a copy of the attribute table at `$0300`, at the offsets listed in
   `scene_attr_positions` (`7:C1A6`): `$08-$0B`, `$10-$13`, `$18-$1B`, `$20-$23`.

There are two ways to draw one:
- `draw_scene` (`7:C1CC`) copies the tile map to `$0554` and sends it to the PPU in one go.
- `draw_scene_dissolve` (`7:C0DD`) draws it tile by tile over 6 passes, one frame each. In
  each pass, each of the 14 rows draws the columns set in a 16-bit mask from
  `dissolve_row_mask` (`6:9A50`, far call `$6B`): one of three fixed orders at `6:9A94`, 6 x 14
  masks each. With `$7D` set it draws `$FF` everywhere instead: the recordings show each
  picture being blanked this way as well as drawn.

`room_settings` (`7:C21F`, 80 bytes, indexed by the room number in `$0514`) gives each room
two values. The low nibble is a sound: `apply_room_settings` plays it (`play_sound`, `7:F409`)
after loading a scene, except in room `$33`. The high nibble is `dissolve_pattern` (`$0530`),
which of the three orders the dissolve uses (`load_dissolve_pattern`).

Loading a scene (`change_scene`, `change_scene_instant`, `load_scene`, at the start of bank 7)
also sets its CHR banks (`load_scene_chr_banks`, `6:8070`, R0 and R4 from a 2-byte entry per
scene at `6:80AC`) and its palette (`load_scene_palette`, `6:81DE`, through
`scene_palette_ids` and `palette_table`). All of this is ported to C in
[src/scenes.c](../src/scenes.c).

## Checked against the game
`run_mesen.py --log-scenes` reads the nametable two frames after either drawing routine
returns, and `scenes.py --check` compares it with the decoded picture. Across both TAS movies,
**88 different pictures match exactly**. One, scene `$67`, differs in a 3x6 block of tiles at
its top left while its attributes match: something draws over it after the picture, most
likely an object whose state changed. Where such overlays come from is not mapped yet.
The other 67 pictures don't appear in either movie.
