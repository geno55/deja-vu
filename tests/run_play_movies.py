#!/usr/bin/env python3
"""Play scripted movies through the program itself, and say whether it ends where the original does.

usage: run_play_movies.py --dejavu EXE [--harness EXE] [--rom ROM] [--work DIR]

The TAS movies can't check dejavu itself: their presses are timed to the frame, and the program has no lag, so it soon
misses one. The scripts in tests/movies/ (see tools/movie_script.py) leave a margin around each press, so they mean the
same with or without lag, and the two can be compared where each ends, after a wait.

The program alone plays each one with --no-window, and must end with the cursor where FIXED says, which is
what Mesen2 does. So key repeat is checked by plain ctest, where there's no reference.

--harness adds the reference: each movie runs on Mesen2's core and the C side in lockstep, compared at every NMI, and
what Mesen2 ends with must be what the program alone ends with: the game's state, save RAM, nametables, palette and OAM,
and the cursor. Other RAM may differ, as lag moves the counters and the sound along. It prints which.
"""
import argparse
import subprocess
import sys
import tempfile
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "tools"))
import movie_script  # noqa: E402

MOVIES = REPO / "tests" / "movies"
ORIGINAL = ("--config", str(MOVIES / "original.ini"))  # as the reference behaves
LAST_FRAMES = 300  # the frames the harness plays after a movie's last line; the port plays them too
LAYOUT = (("RAM", 0x800), ("save RAM", 0x2000), ("nametables", 0x800), ("palette", 0x20), ("OAM", 0x100))
GAME_STATE = range(0x420, 0x530)  # the flags, the items, the room: what a save file holds
CURSOR = {0xA5: "cursor_x", 0xA6: "cursor_y"}
PLAYS = ("hold_title", "hold_text", "hold_cursor", "key_repeat", "key_repeat_b", "play_basics")
# Where key_repeat.moves leaves the cursor, as Mesen2 does: (cursor_x, cursor_y).
FIXED = {"key_repeat": (26, 186), "key_repeat_b": (26, 170)}


def split_state(data):
    """The memories of a --state-out file, as {name: bytes}."""
    out, at = {}, 0
    for name, size in LAYOUT:
        out[name] = data[at:at + size]
        at += size
    if at != len(data):
        raise ValueError(f"a state file has {len(data)} bytes, not {at}")
    return out


def differences(mine, theirs):
    """(what must match but doesn't, as text; the other RAM that differs, as addresses), of two split states."""
    bad = []
    for name in ("save RAM", "nametables", "palette", "OAM"):
        count = sum(a != b for a, b in zip(mine[name], theirs[name]))
        if count:
            first = next(i for i, (a, b) in enumerate(zip(mine[name], theirs[name])) if a != b)
            bad.append(f"{name}: {count} bytes, the first at +${first:04X}")
    ram_a, ram_b = mine["RAM"], theirs["RAM"]
    for a in GAME_STATE:
        if ram_a[a] != ram_b[a]:
            bad.append(f"game state ${a:04X}: program {ram_a[a]:02X}, Mesen2 {ram_b[a]:02X}")
    for a, name in CURSOR.items():
        if ram_a[a] != ram_b[a]:
            bad.append(f"{name}: program {ram_a[a]}, Mesen2 {ram_b[a]}")
    stack = range(0x100, 0x200)  # what lies on it isn't the same under C
    other = [a for a in range(0x800) if ram_a[a] != ram_b[a] and a not in GAME_STATE and a not in CURSOR and a not in stack]
    return bad, other


class Run:
    def __init__(self, args, work):
        self.args = args
        self.work = work
        self.failures = 0

    def check(self, ok, what):
        print(f"run_play_movies: {'ok  ' if ok else 'FAIL'} {what}", flush=True)
        self.failures += not ok

    def movie(self, name):
        frames = movie_script.expand((MOVIES / f"{name}.moves").read_text())
        path = self.work / f"{name}.inp"
        path.write_text("\n".join(frames) + "\n")
        return path, len(frames) + LAST_FRAMES

    def program(self, name):
        """Plays a movie on the program alone, from no save file. Returns its state, or None."""
        movie, frames = self.movie(name)
        save, state = self.work / f"{name}.sav", self.work / f"{name}.state"
        save.unlink(missing_ok=True)
        state.unlink(missing_ok=True)
        result = subprocess.run([str(self.args.dejavu), "--no-window", str(self.args.rom), "--movie", str(movie), "--frames",
                                 str(frames), "--save", str(save), "--state-out", str(state), *ORIGINAL],
                                capture_output=True, text=True)
        self.check(result.returncode == 0 and state.is_file(), f"{name}: the program plays it")
        if result.returncode or not state.is_file():
            print(result.stdout[-2000:], result.stderr[-2000:])
            return None
        return split_state(state.read_bytes())

    def reference(self, name):
        """Plays a movie in lockstep. Returns Mesen2's state at the end, or None."""
        movie, _ = self.movie(name)
        state = self.work / f"{name}_m.state"
        state.unlink(missing_ok=True)
        result = subprocess.run([str(self.args.harness), "--movie", str(movie), "--compare", "--check-ports", "--rom",
                                 str(self.args.rom), "--home", str(self.work / "mesen-home"), "--state-out", str(state)],
                                capture_output=True, text=True)
        self.check(result.returncode == 0, f"{name}: the C side matches Mesen2 at every NMI")
        if result.returncode:
            print(result.stdout[-2000:], result.stderr[-4000:])
        return split_state(state.read_bytes()) if state.is_file() else None


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--dejavu", type=Path, required=True)
    ap.add_argument("--harness", type=Path)
    ap.add_argument("--rom", type=Path, default=REPO / "Deja Vu (USA).nes")
    ap.add_argument("--work", type=Path)
    args = ap.parse_args()
    args.rom = args.rom.resolve()
    work = (args.work or Path(tempfile.mkdtemp(prefix="dv-plays-"))).resolve()
    work.mkdir(parents=True, exist_ok=True)
    run = Run(args, work)
    for name in PLAYS:
        mine = run.program(name)
        if mine is None:
            continue
        ram = mine["RAM"]
        print(f"run_play_movies: {name}: the program ends with the cursor at {ram[0xA5]},{ram[0xA6]}, "
              f"room {ram[0x514]:02X}")
        if name in FIXED:
            run.check((ram[0xA5], ram[0xA6]) == FIXED[name], f"{name}: the cursor ends at {FIXED[name]}")
        if args.harness:
            theirs = run.reference(name)
            if theirs is None:
                continue
            bad, other = differences(mine, theirs)
            run.check(not bad, f"{name}: the program ends where Mesen2 does" + "".join("\n    " + b for b in bad))
            print(f"run_play_movies: {name}: other RAM that differs: {' '.join(f'{a:03X}' for a in other) or 'none'}")
    print(f"run_play_movies: {run.failures} failed")
    return 1 if run.failures else 0


if __name__ == "__main__":
    sys.exit(main())
