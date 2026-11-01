# Development

How the port was made and how to work on it: a matching disassembly, C that mirrors it, and a
frame-by-frame comparison against the original running in an emulator, the way
[snesrev/zelda3](https://github.com/snesrev/zelda3) was built.

## Your ROM
This repository contains no game data. Put your dump at `Deja Vu (USA).nes` in the repo root
(or pass `-DDV_ROM=path` to CMake) and check it:

```
python tools/check_rom.py
```

The check uses the SHA-1 of the ROM data without its 16-byte header
(`c8ca009cfb56edb86acd8bb8f05d96d19132ad66`), so any correct dump passes whatever its header says.

## Build and test
Needs CMake 3.20+, a C11 compiler and Python 3. The window needs [SDL2](https://libsdl.org): on Windows,
`python tools/fetch_sdl2.py` downloads it into `external/` and checks it; elsewhere, install the
package (`libsdl2-dev`, or `brew install sdl2`). Without it everything else still builds.

```
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The `rom_hash` test runs only when CMake finds your ROM; `rom_rebuild` also needs cc65.

## Making a release
By hand, as zelda3 does: there's no release workflow, and the version is the tag and the zip's name, with
no version in the program. On Windows, with SDL2 fetched and not `DV_HARNESS`:

```
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
python tools/make_release.py v0.1.0
```

That writes `build/dejavu_v0.1.0.zip`, from a fixed list of files so that the build folder's
`dejavu_assets.dat` can't go in. It refuses a program with no window, one that needs Visual C++'s runtime,
or one holding a run of the ROM's bytes. Then tag the commit and attach the zip to a GitHub release.

## Running every check
With a harness build, the ROM and the converted movies, this runs the lot side by side and says what failed:

```
python tools/run_checks.py
```

That is the tests, both movies with port checks, the picture and the sound, again with `--emulate` and with
`--pass-through`, the fuzz, the palette and the save movies (with `Mesen.exe` too, if `MESEN` is set). Each
job's output is in `build/checks/`. Its docstring has the options, and `--only` runs some of them.

## How much the checks reach
Passing says little about code that never ran. This says what ran:

```
python tools/coverage.py
```

It runs the movies in `--pass-through`, the fuzz and the script sweep with `--coverage`, in about 7 minutes, and counts
instructions, branch directions, routines and script opcodes from a listing of `asm/`. `build/coverage/report.txt` lists
every routine with something unrun, every branch not seen both ways and every script opcode that never ran. Its
docstring says what counts as compared. Each source is counted apart, as a path only the fuzz or the sweep takes may be
one the game can't reach.

On 2026-11-01:

| | Instructions (of 9,820) | Branches both ways (of 824) | Script opcodes (of 4,668) | `goto_if`s both ways (of 972) | Opcode numbers (of 91) |
|---|---|---|---|---|---|
| Movies | 78.9% | 47.1% | 25.3% | 55 | 65 |
| Fuzz | 87.3% | 72.7% | not counted | not counted | not counted |
| Sweep | 53.5% | 29.1% | 96.8% | 937 | 90 |
| Any | 94.0% | 81.2% | 96.9% (movies and sweep) | 939 | 90 |

The movies are speedruns, so on their own they reach a quarter of the scripts. The script sweep (`--sweep-scripts`) starts
from every opcode of every script, on the state at five points in the movies, with the flags as they are and then at
random. It runs the 6502 all the way, but for the waits and the NMI, and the port from the same state, and compares them: 107,788
runs, no differences, 5,032 skipped. With `op_4A` broken on purpose it named the two script places that use it, while
the movie's own 5,000 frames still matched. It tests the original's path against the C's in states play may never reach, and
a script started partway has skipped what came before, so it doesn't replace playing the game. The one
opcode number that never ran is `$0F`, in all 12 of its places: every run from them was skipped (not looked into).

The record agrees with `Mesen.exe` on the same movies. The script opcodes that ran are the same in both (from
`run_mesen.py --trace-scripts`). So are the instructions (from its code log), but for one in `ending_sequence` in #1260. That run
of Mesen.exe plays 300 frames past the movie's end, which is probably why (a guess: not looked into).

It doesn't measure the C: a branch that exists only in a port isn't seen.

## Rebuilding the ROM from the assembly
`asm/` holds the game's code and small tables as ca65 source, one file per 16 KB bank
(`bank0.s` ... `bank7.s`), plus the header, the CHR ROM include and the linker configuration.
Large data (scene pictures, messages, scripts, sound, graphics) isn't in the repository: it is
cut from your ROM at build time, as listed in `asm/bins.txt`. This follows the split used by
other disassemblies, such as spannerisms' *A Link to the Past* disassembly.

```
cmake --build build --target rom
```

or `python tools/build_rom.py`. Either one extracts the data, assembles and links everything
into `build/rom/dejavu.nes` (or `build/dejavu.nes`), and checks it against the original.
The files in `asm/` are the source now: edit them by hand. `tools/make_asm.py` generated them
once and won't overwrite them without `--force`.

### Naming things
- RAM variables live in `asm/ram.inc` and hardware registers in `asm/nes.inc`. After adding a
  name there, `python tools/name_ram.py` replaces the raw addresses in the bank files.
- `python tools/rename.py OLD NEW` renames a label everywhere (it refuses names already in use).
- Rebuild afterwards: names never change the bytes, so `build_rom.py` must still match. It also
  writes `build/dejavu.labels` (every symbol's address) and fails if one name has two addresses.

## Disassembly (Phase 1)
Needs [Mesen 2](https://github.com/SourMesen/Mesen2) and [cc65](https://cc65.github.io/).

```
python tools/run_mesen.py --frames 30000 --seed 1 2 3 4 5 6 7 8
python tools/movie_inputs.py randil5-dejavu.fm2.zip build/movies/tas1260.inp
python tools/run_mesen.py --movie build/movies/tas1260.inp
python tools/movie_inputs.py adelikat-dejavu-nes.bk2.zip build/movies/tas7044.inp
python tools/run_mesen.py --movie build/movies/tas7044.inp --movie-offset 1
python tools/disasm.py --cdl build/cdl/merged.cdl
```

`run_mesen.py` plays the game headless, with a button-pressing bot or a TAS movie, and records
which bytes run as code or are read as data. The movies come from TASVideos (download them
yourself from the publication pages):
- [#1260](https://tasvideos.org/1260M) by Randil (FCEUX `.fm2`) plays in sync in Mesen to
  the end of the game, with all five addresses found.
- [#7044](https://tasvideos.org/7044M) by adelikat (BizHawk `.bk2`) needs `--movie-offset 1`
  and still loses sync partway (it relies on glitches); it adds a little extra coverage.

`disasm.py` follows the code from the reset vector and that log, writes one `.s` file
per 16 KB bank to `build/asm/`, and checks that they rebuild into the identical ROM.
Names come from [asm/labels.txt](../asm/labels.txt); what's known about the code is in
[conventions.md](conventions.md).

`python tools/text.py` decodes every message into `build/text/messages.txt`; the format is in
[text.md](text.md). `python tools/scripts.py` lists every game-logic script in
`build/scripts/scripts.txt` ([scripts.md](scripts.md)); add
`--trace build/cdl/*.scripts` to check it against runs recorded with
`run_mesen.py --trace-scripts`. `python tools/scenes.py` decodes the scene pictures
([scenes.md](scenes.md)); `--check build/cdl/*.scenes` compares them with what the
game drew in runs recorded with `run_mesen.py --log-scenes`. `tools/sound.py`,
`tools/rooms.py` and `tools/layouts.py` map the music, room hotspots and screen layouts
([sound.md](sound.md), [rooms.md](rooms.md)).

## Comparison harness (Phase 3)
The harness runs the original game in [Mesen2](https://github.com/SourMesen/Mesen2)'s core
as the reference for the port. Mesen2 is GPL-3.0, so it is never part of the repository or a
release: the off-by-default CMake option `DV_HARNESS` clones it at a pinned tag into
`external/` and builds `dv_harness` against it.

```
cmake -S . -B build/harness -DDV_HARNESS=ON
cmake --build build/harness --config Release --target dv_harness
build/harness/harness/Release/dv_harness --movie build/movies/tas1260.inp --compare
```

`dv_harness` plays a movie on the reference; `--compare` runs the C side alongside and checks
registers, memory and APU writes at every NMI, and `--log` writes the state at every NMI
(`run_mesen.py --log-nmi` writes the same log from Mesen.exe). The C side runs the routines
ported so far ([src/funcs.h](../src/funcs.h)) and the original code for the rest;
`--pass-through` runs it all as original code, and `--emulate NAME,...` just the routines
named. `--check-ports` also runs every call of a port from original code against its
original and names what differs, and `--fuzz-ports N` does the same on made-up states. See
[harness.md](harness.md).

## Editable data
`python tools/roundtrip.py` writes every block of game data to `build/src/` as text (or a
PNG for the graphics) and checks that it encodes back to the same bytes; see
[formats.md](formats.md).

## License
MIT; see [LICENSE](../LICENSE). The development-only comparison harness may build against
Mesen2's GPL-3.0 core, which is never part of a release build: it's fetched into `external/` at a pinned
tag, built only with the off-by-default option `DV_HARNESS`, and its binaries are never published.
