# Deja Vu for PC

Play the NES version of *Deja Vu*, the point-and-click detective mystery, as a native program on your
computer, with no emulator. It's the original game rewritten in C, and it looks, sounds and plays like
the NES version.

You need your own copy of the game. The pictures, text, music and other large data aren't included: they
come from your own ROM.

## Download for Windows
1. Download the zip from the [releases page](https://github.com/geno55/deja-vu/releases) and unzip it.
2. Copy your dump of *Deja Vu (USA)* for the NES into that folder, and name it `Deja Vu (USA).nes`.
3. Run `dejavu.exe`.

Windows may warn you about an unrecognized program, because the download isn't signed.

After the first run, the game no longer needs your ROM. It keeps what it needs in `dejavu_assets.dat` next
to the program. That file is a copy of the game's data, so please don't share it.

## Build it yourself
On other systems, or to work on the game, build it from source. It takes a few minutes.

**1. Install the tools.** You need [Git](https://git-scm.com/downloads),
[CMake](https://cmake.org/download/) 3.20 or newer, [Python 3](https://www.python.org/downloads/) and a C
compiler (on Windows,
[Visual Studio](https://visualstudio.microsoft.com/downloads/) with "Desktop development with C++").

**2. Get the code.**

```bash
git clone https://github.com/geno55/deja-vu
```

```bash
cd deja-vu
```

**3. Add your game.** Copy your dump of *Deja Vu (USA)* for the NES into the `deja-vu` folder, and name it
`Deja Vu (USA).nes`.

**4. Install SDL2,** which provides the window, sound and controller support.

| System | Command |
|---|---|
| Windows | `python tools/fetch_sdl2.py` |
| Debian or Ubuntu | `sudo apt install libsdl2-dev` |
| macOS | `brew install sdl2` |

**5. Build the game.**

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

```bash
cmake --build build --config Release
```

**6. Play.** On Windows, run `build/Release/dejavu`. On Linux and macOS, run `build/dejavu`.

It has been played on Windows. Linux builds are tested automatically, and macOS builds less often.

## Controls

| NES | Keyboard | Game controller |
|---|---|---|
| D-pad | Arrow keys | D-pad or left stick |
| A | X | Right face button |
| B | Z | Bottom face button |
| Start | Enter | Start or Menu |
| Select | Right Shift | Back or View |

Use the d-pad to move the cursor around the picture, the commands and your inventory, and press A to
click.

To quit, close the window. Your save is written when the game stops.

## Saves
Your save is `saves/dejavu.sav` next to the program. To use another file, choose it with `--save`:

```bash
dejavu --save path/to/my-game.sav
```

Keep your own copy of any save you care about. The game also keeps `saves/dejavu.sav.bak`, the file as it
was when the game last started, but the next run replaces it.

**Emulator saves work in both directions.** The file is the game's 8 KB of save RAM, the same as an
emulator's `.sav`. Copy one over and rename it, in either direction. This has been tested with Mesen2;
other emulators should work the same way, but haven't been tried. More in [docs/saves.md](docs/saves.md).

## Options
In these examples, `dejavu` means the program, so use its full path unless you're in its folder.

| Option | What it does |
|---|---|
| `dejavu path/to/rom.nes` | Uses this ROM, if yours isn't in the `deja-vu` folder when you build. You only need to do this once. |
| `--scale N` | Sets the window to N times the NES picture (the default is 3). You can also resize the window by dragging it. |
| `--save FILE` | Uses this save file. |
| `--config FILE` | Uses this settings file. |
| `--help` | Lists every option. |

## Settings
Settings are in `dejavu.ini` next to the program, and the file says what each one does. To keep yours when
you update, copy it to `dejavu.user.ini` there: the game reads that one instead.

| Setting | Default | What it does |
|---|---|---|
| `FixText` | `0` | With `1`, corrects the original's typos, grammar, punctuation and inconsistent spellings. It doesn't affect saves. |
| `FixBugs` | `0` | With `1`, fixes the original's save check, where a damaged save file is erased or hangs the game, and the capsule warp, which breaks the room you are in when you use the capsules on medicine you haven't taken. Speedruns use that warp. |

## How close is it to the NES?
The game was compared frame by frame with the original running in the Mesen2 emulator, using two recorded
speedruns. The picture matched pixel for pixel, apart from 11 frames in the run that relies on glitches.
The colors and sound were also compared by eye and ear. The game's own bugs, typos and quirks are kept, unless you turn on the fixes under
[Settings](#settings).

There are a few differences:
- **Slow moments are faster.** Work that took the NES more than a frame now finishes at once, most
  noticeably when the game writes to save RAM.
- **Starting up is slightly quicker,** by a fraction of a second.
- **Sound timing is slightly different within a frame.** The notes are the same and come at the same
  times, and you shouldn't be able to hear the difference.

The speedruns don't reach every path through the game, so if something plays differently from the NES, it
may be a bug in this version.

## If something goes wrong

| What you see | What to do |
|---|---|
| `built without SDL2, so there's no window` | Do step 4, then repeat step 5. |
| `this needs your own Deja Vu (USA) ROM` | Put your ROM next to the program, named `Deja Vu (USA).nes`, or run the game once as `dejavu path/to/rom.nes`. |
| `isn't Deja Vu (USA)` or `isn't the size of Deja Vu (USA)'s ROM` | It must be the USA version for the NES. To check it, run `python tools/check_rom.py path/to/rom.nes`. The header doesn't matter, so any correct dump passes. |
| `isn't 8 KB, so it isn't a save of this game` | The game leaves that file alone, so move it away or name another with `--save`. |
| A save you wanted is gone | Look in the `saves` folder next to the program. Saves from older versions stay as `dejavu.sav` in the folder you ran the game from, and the game goes on using one if you start it there. |

## For developers
This game was rebuilt from a matching disassembly and checked against the original.
[docs/development.md](docs/development.md) covers building the tests, rebuilding the ROM from the assembly
and running the comparison harness.

## License
MIT; see [LICENSE](LICENSE). *Deja Vu* itself belongs to its owners, and the large data has to come from your
own ROM. Mesen2 (GPL-3.0) is used only by the comparison harness and isn't part of the game.
