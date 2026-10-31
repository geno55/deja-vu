# The game's data and its saves (Phase 6)

The port needs two files from outside the repo: your ROM once, and your save file. Neither is ever
committed.

## The game's data

Everything the game shows and plays comes from your own dump of Deja Vu (USA). It is checked by the
CRC of its data, whatever its 16-byte header says, and then kept in `dejavu_assets.dat`
next to the program, so you only need the dump once ([src/assets.h](../src/assets.h)):

```
dejavu path/to/your/rom.nes
```

After that `dejavu` runs from any folder. It looks for the pack next to itself and then in the current
folder, and then for `Deja Vu (USA).nes` in the current folder and next to itself. A build where CMake
knows your ROM makes the pack as its last step; `dejavu --extract-assets ROM` does it by hand. The plan
had this as a tool in `tools/`, but the game does it itself so that a player doesn't need Python.

The pack is a 16-byte header of its own (`DVAS` and a format version), then the PRG and CHR ROM as they
are in the dump. It is a copy of the game's data, so it's for your machine only: don't share it. A damaged
pack, or one from another version of the format, is refused, and the dump makes a new one.

## Save files

A save file is the game's 8 KB of save RAM, nothing else, as emulators keep it. So saves carry over in
both directions: copy an emulator's file to `saves/dejavu.sav` next to the program (or name it with
`--save`), and copy `saves/dejavu.sav` to where the emulator looks for its save, under the name it
expects.

The `saves` folder follows zelda3, which keeps its save in `saves/`, and it means a shortcut or a ROM
dragged onto the program can't leave a save somewhere else. A `dejavu.sav` in the folder the game starts
from, as saves were before, stays in use until `saves/dejavu.sav` exists. It's never moved or copied.

| Emulator | Where it keeps the save | Checked? |
|---|---|---|
| Mesen2 | `Saves/<ROM's name>.sav` in its folder (`Documents\Mesen2` on Windows) | yes, below |
| FCEUX, Nestopia, others | a raw 8 KB `.sav` too (a guess: as far as I know, not checked) | not tried |

[src/save_file.h](../src/save_file.h) keeps a save from being lost:

- **A file that isn't 8 KB is refused, and left alone.** It can't be this game's save RAM, and the game
  would write over it. The game says so and stops.
- **A write can't leave half a file.** The new save goes to `dejavu.sav.new`, which then replaces the old
  one in a single step.
- **`dejavu.sav.bak` is the file as it was when the game last started,** written just before the first
  change. It is the way back from anything that goes wrong in a run, and the next run replaces it.

The game writes the file a second after it changes, and when it stops. That includes more than your
saves: the game keeps a backup of the state in save RAM as you play.

### What the game does with a save file

The layout is in [conventions.md](conventions.md), "Saving": three files, each 4 copies of the game's
state with a signature and a sum, and a backup copy at `$7E00`.

The 4 copies protect less than they seem to, and this is the original game's behavior, kept as it is. At
the title `check_save_file` goes through the copies adding up a sum that it doesn't clear from one to the
next, so a copy after a bad one can't pass:

- **One wrong byte in a file's first copy erases the whole file,** good copies and all. If that ever
  happens, `dejavu.sav.bak` still has it, though the first copy would need repairing by hand to be any use.
- **A bad copy after the first does no harm.** It is never read, and the next save rewrites all 4.

Whether to fix this is a question for Phase 7, behind a flag as its bug fixes are.

## What is verified

`tests/run_save_movies.py` plays the game's saving through. The two TAS movies never save, so before this
`save_game` and `load_game` had only been compared on made-up states. Its scripts are in
[tests/movies/](../tests/movies), in a form that is easy to read and change
([tools/movie_script.py](../tools/movie_script.py)):

- `new_game_save`: a new game in file 1, take the coat, save.
- `continue_save`: continue file 1, take the gun from the coat, save again.
- `file_menu`: every way through the file menu, ending with file 1 erased.

The test looks inside the files they leave, at each copy's signature and sum and at the items. The test
`save_movies` runs them on the port alone. `harness_saves`, in a harness build, adds the reference, and
`--mesen`, by hand, adds Mesen.exe. With all of them:

- **Saves cross in both directions, and the files are the same to the byte.** Mesen2's core and Mesen.exe
  each write the same file as the port alone for `new_game_save`. `continue_save` from the port's file on
  Mesen2 and on Mesen.exe, and from each of theirs on the port, all end with the same file too. So do
  `file_menu` and both damaged files below on the reference. That the port has no lag frames makes no
  difference to any of them.
- **Saving matches at every NMI.** On the reference each movie is compared with the C side at every NMI,
  with the port checks on, and all match, with no lag NMI placed late: 6,287 NMIs in `new_game_save`
  (3,252 of them lag NMIs), 3,771 in `continue_save` (2,061) and 3,287 in `file_menu` (669). Nearly all
  of those land in the controller reads of `get_input`'s spin, as these movies mostly wait for a button;
  only a handful land in the saving code. The run checks that `save_game`, `load_game`, `save_dialog`,
  `show_file_options`, `erase_save_file` and the rest of what it's meant to reach actually ran.
- **Damaged files.** A wrong byte in a later copy: the game goes on, and the save rewrites it. In the first
  copy: the file is erased, on Mesen2 and the port alike, and `.bak` has what was there.
- **Refusals.** Files of 0, 100, 8,191, 8,193 and 32,768 bytes are refused and unchanged, and
  `tests/test_save_file.c` goes through the rest with no ROM: no file, nothing to write, the `.bak`, a
  place that can't be written. `tests/test_assets.c` does the same for the pack.

Not checked: other emulators' files, which should be the same format but haven't been tried, and a save
made on real hardware.
