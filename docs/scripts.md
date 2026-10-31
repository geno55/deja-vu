# Scripts

Deja Vu's game logic is bytecode, run by an interpreter in the fixed bank. `tools/scripts.py`
walks every script from the tables in bank 3 and writes a listing to
`build/scripts/scripts.txt` (not committed: it is the game's logic). The walk agrees with
every opcode the game ran in five recordings (TAS #1260 to the end of the game, TAS #7044 and
three bot runs).

Names match [asm/labels.txt](../asm/labels.txt).

## Choosing a script
`7:CB64` switches to bank 3 and follows three tables:

1. `script_tables` (`3:8000`): 6 pointers, indexed by `$AA & $7F`.
2. One table per `$AA` value, indexed by `$AB`. `$AB` is the object the player is acting on:
   the "[ab]" flag opcodes use it as their bit number. Sizes: 120, 13, 5, 93, 18 and 78 entries.
3. Small tables indexed by `$BC`: 4 entries each (6 for `$AA` = 4). 327 of them.

The result is one of **616 distinct scripts**. Many table entries share a script (a common
"nothing happens" message, for example). The walker also finds 2 short fragments no table
points to, at `3:97FA` and `3:B63D`, by walking the gaps between known scripts.

`$AA` is the kind of target (it comes from the hotspot record), `$AB` the object, and `$BC`
a slot that depends on the command, as the next section shows.

### From a click to a script
Pressing A runs `press_a` (bank 6):
1. `click_at_cursor` works out which screen area the cursor is in (`find_screen_area`:
   picture, commands or inventory; stored in `screen_area`).
2. **Picture:** `click_picture` walks the room's hotspots (`find_clicked_hotspot`, see
   [rooms.md](rooms.md)); a hit sets `script_table` (`$AA`) and `object_id` (`$AB`) from the
   hotspot record. If no command is chosen yet, `pick_default_command` picks one.
   **Commands:** `click_command_area` sets `command` (`$AC`) and redraws the highlight.
   **Inventory:** `click_inventory_area` picks an item the same way.
3. Once a command and an object are both set, `run_command` (`7:CB4B`) asks
   `script_slot_for_command` for the slot: entry `11 * $AA + command` of a table at `6:A985`
   (11 commands per script table, `$FF` for "no script"). With a slot it runs `script_start`;
   without, far call `$64` gives the default response.

Steps 1 and 2 are ported to C in [src/commands.c](../src/commands.c),
but for `click_picture` and its hotspots, in [src/rooms.c](../src/rooms.c), and `run_command`, the
interpreter and all 105 opcodes in [src/scripts.c](../src/scripts.c), [src/flags.c](../src/flags.c)
and [src/opcodes.c](../src/opcodes.c).

**Choices inside a script:** a script that needs the player to pick something sets event
flag `$80` and yields (opcode `$28`). While that flag is set, clicks store the chosen table
and object in `switch_key` (`$AD`/`$AE`) instead of `$AA`/`$AB`, and `press_a` resumes the
script with `script_resume`. Its next opcode is usually the switch (`$29`), which branches on
what was chosen.

## Running a script
`script_loop` (`7:CBB1`) fetches an opcode at `($B6)`. `$FF` ends the script; `$00-$68` go
through the 105-entry jump table at `7:CC1A`. Operands follow the opcode; handlers read them
with `7:CBC2` / `7:CBD8` / `7:CBE8` / `7:CBFD` (1-4 bytes, into `$B8-$BB`).

**Yield:** opcodes `$28` and `$48` stop the current run but leave `$B6` pointing at the next
opcode. `7:CB5C` later switches to bank 3 and jumps back into `script_loop`, so the script
carries on from there. The walker treats them as falling through.

## Control flow
- A **condition flag** is bit 2 of `$B5`: test opcodes clear it (`7:CFE6`) and set it
  (`7:CFDF`) when the test passes.
- `$03`, `$04`: goto the 2-byte operand if the condition is set; `$05`: if it is clear.
- `$1C`: goto. `$65`: push the target (on the bank stack) and goto it; `$66`: pop an address
  and goto it, which together make loops.
- `$29`: switch. 4-byte cases follow: two bytes compared with `$AD`/`$AE`, then a target. A
  first byte of `$FE` is the default case and ends the list.

## Flags
Most tests change bits in five flag arrays (bit n = byte `n >> 3`, mask `1 << (n & 7)`):

| Array | Test / set / clear bit `$AB` | Test / set / clear bit n (operand) | Data Crystal calls it |
|---|---|---|---|
| `$0450` | `$07` / `$08` / `$09` | `$52` (test) | story event flags |
| `$0480` | `$13` / `$14` / `$15` | `$2E` / `$2F` / `$30` | doors, windows |
| `$0490` | | `$16` / `$17` / `$18` | more event flags |
| `$045C` | `$20` / `$21` / `$22` | `$53` (set) | |
| `$047A` | `$23` / `$24` / `$25` | `$54` (set) | |

`$0A-$12` work on the item status bytes at `$0468` (bit 0 obtained, bit 1 open).

## Opcodes
Sizes include the opcode. They come from which operand fetchers each handler calls, and
match every opcode the recordings ran ("Ran" column). Names are given only where the
handler has been read.

| Op | Size | Handler | Name | Ran |
|---|---|---|---|---|
| `00` | 2 | `CF87` | message_5 | yes |
| `01` | 2 | `CF8E` | message_4 | yes |
| `02` | 2 | `CF95` | message_2_or_5 | yes |
| `03` | 3 | `CF9C` | goto_if_alt (same handler as `$04`; never used) |  |
| `04` | 3 | `CF9C` | goto_if | yes |
| `05` | 3 | `CFA6` | goto_if_not | yes |
| `06` | 2 | `D078` |  | yes |
| `07` | 1 | `D109` | test_0450[ab] | yes |
| `08` | 1 | `D112` | set_0450[ab] | yes |
| `09` | 1 | `D12B` | clear_0450[ab] |  |
| `0A` | 1 | `D177` |  | yes |
| `0B` | 1 | `D18F` |  | yes |
| `0C` | 1 | `D1A5` |  |  |
| `0D` | 1 | `D1B8` |  | yes |
| `0E` | 1 | `D1D6` |  | yes |
| `0F` | 1 | `D1EF` |  |  |
| `10` | 1 | `D208` |  | yes |
| `11` | 1 | `D220` |  | yes |
| `12` | 1 | `D233` |  |  |
| `13` | 1 | `D246` | test_0480[ab] | yes |
| `14` | 1 | `D255` | set_0480[ab] |  |
| `15` | 1 | `D271` | clear_0480[ab] |  |
| `16` | 2 | `D298` | test_0490 | yes |
| `17` | 2 | `D2A2` | set_0490 | yes |
| `18` | 2 | `D2B9` | clear_0490 | yes |
| `19` | 2 | `D305` |  | yes |
| `1A` | 1 | `D321` |  | yes |
| `1B` | 1 | `D33F` |  |  |
| `1C` | 3 | `CFD8` | goto | yes |
| `1D` | 1 | `D351` |  | yes |
| `1E` | 2 | `D418` |  | yes |
| `1F` | 2 | `D421` |  | yes |
| `20` | 1 | `D42A` | test_045C[ab] |  |
| `21` | 1 | `D439` | set_045C[ab] | yes |
| `22` | 1 | `D44B` | clear_045C[ab] |  |
| `23` | 1 | `D468` | test_047A[ab] |  |
| `24` | 1 | `D477` | set_047A[ab] |  |
| `25` | 1 | `D489` | clear_047A[ab] |  |
| `26` | 1 | `D4A6` |  | yes |
| `27` | 1 | `D5E4` |  | yes |
| `28` | 1 | `CE01` | yield | yes |
| `29` | varies | `CDE0` | switch | yes |
| `2A` | 1 | `D5F0` |  |  |
| `2B` | 2 | `D1BD` |  |  |
| `2C` | 2 | `D1DB` |  | yes |
| `2D` | 2 | `D1F4` |  |  |
| `2E` | 2 | `D24B` | test_0480 | yes |
| `2F` | 2 | `D25A` | set_0480 | yes |
| `30` | 2 | `D276` | clear_0480 | yes |
| `31` | 1 | `CE0B` |  | yes |
| `32` | 1 | `CE7A` |  | yes |
| `33` | 2 | `D315` |  | yes |
| `34` | 2 | `D333` |  | yes |
| `35` | 2 | `D5F6` |  | yes |
| `36` | 2 | `D31B` |  |  |
| `37` | 2 | `D339` |  |  |
| `38` | 2 | `D5FD` |  | yes |
| `39` | 2 | `D606` |  | yes |
| `3A` | 2 | `D637` |  |  |
| `3B` | 1 | `D651` |  |  |
| `3C` | 2 | `D660` |  | yes |
| `3D` | 1 | `D6A4` |  |  |
| `3E` | 1 | `D6CA` |  |  |
| `3F` | 2 | `D73D` |  | yes |
| `40` | 1 | `D6F0` |  | yes |
| `41` | 1 | `D746` |  | yes |
| `42` | 2 | `D7D9` |  | yes |
| `43` | 1 | `D7C0` |  | yes |
| `44` | 1 | `D7A6` |  | yes |
| `45` | 1 | `D7B6` |  | yes |
| `46` | 2 | `D812` |  | yes |
| `47` | 1 | `D81B` |  |  |
| `48` | 1 | `D8A0` | yield_b5_80 | yes |
| `49` | 1 | `D8AB` |  |  |
| `4A` | 1 | `D8D2` |  |  |
| `4B` | 2 | `D8DE` |  | yes |
| `4C` | 1 | `D8ED` |  | yes |
| `4D` | 1 | `D8FE` |  | yes |
| `4E` | 1 | `D90F` |  |  |
| `4F` | 1 | `D926` |  | yes |
| `50` | 1 | `D935` |  |  |
| `51` | 1 | `D965` |  |  |
| `52` | 2 | `D103` | test_0450 | yes |
| `53` | 2 | `D433` | set_045C |  |
| `54` | 2 | `D471` | set_047A |  |
| `55` | 2 | `D99A` |  | yes |
| `56` | 2 | `D626` |  |  |
| `57` | 1 | `D646` |  |  |
| `58` | 1 | `D974` |  |  |
| `59` | 1 | `D98B` |  |  |
| `5A` | 2 | `DA28` |  | yes |
| `5B` | 1 | `CF20` |  | yes |
| `5C` | 3 | `DA31` |  | yes |
| `5D` | 2 | `D171` |  | yes |
| `5E` | 2 | `DA3D` |  | yes |
| `5F` | 2 | `CDD9` |  | yes |
| `60` | 1 | `CDC4` |  | yes |
| `61` | 1 | `CD9D` |  |  |
| `62` | 2 | `D617` |  |  |
| `63` | 1 | `CD21` |  |  |
| `64` | 1 | `CD0D` |  | yes |
| `65` | 3 | `CFB9` | push_goto |  |
| `66` | 1 | `CFCA` | pop_goto |  |
| `67` | 1 | `CCF7` |  |  |
| `68` | 2 | `CCEC` |  | yes |
