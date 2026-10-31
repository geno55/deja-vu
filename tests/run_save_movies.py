#!/usr/bin/env python3
"""Play the game's saving through, and carry the save files between the port and Mesen2.

usage: run_save_movies.py --dejavu EXE [--harness EXE] [--mesen] [--rom ROM] [--work DIR]

The two TAS movies never save, so this is what checks saving in real play. It plays the scripts in
tests/movies/ (see tools/movie_script.py) and looks inside the save files they leave: the signature
and sum of each copy, and the items. The port alone runs them all, and it checks that a save file of
the wrong size is refused and left alone, and that the file a run started from is kept as .bak.

--harness adds the reference: each movie runs on Mesen2's core and the C side in lockstep, compared at
every NMI, and Mesen2's own save file must be byte for byte what the port alone wrote. Each run after
the first starts from the file the other side wrote, so saves cross in both directions.

--mesen adds Mesen.exe itself (tools/run_mesen.py, which needs MESEN set or Mesen on the PATH), for the
same check with no code of ours between the emulator and its save file.
"""
import argparse
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "tools"))
import movie_script  # noqa: E402

MOVIES = REPO / "tests" / "movies"
SAVE_SIZE = 0x2000
STATE_SIZE = 0x110
COAT, GUN = 0x60, 0x62  # as slots hold them: $20 above COAT and GUN1's numbers in item_names
GOODS = 0xAE  # goods_slots in the state, which starts at $0420
LAST_FRAMES = 300  # the frames the harness plays after a movie's last line; the port plays them too


def copy_offset(file, copy):
    return 0x10 + file * 0x800 + copy * 0x200


def good_copies(sav, file):
    """How many of the file's 4 copies have the signature and the right sum."""
    good = 0
    for copy in range(4):
        base = copy_offset(file, copy)
        stored = sav[base + 0x126] | sav[base + 0x127] << 8
        good += sav[base + 0x120:base + 0x126] == b"DEJAVU" and \
            stored == sum(sav[base:base + STATE_SIZE]) & 0xFFFF
    return good


def goods(sav, file=0):
    """The items in the file's first good copy."""
    for copy in range(4):
        base = copy_offset(file, copy)
        if sav[base + 0x120:base + 0x126] == b"DEJAVU":
            return [b for b in sav[base + GOODS:base + GOODS + 7] if b != 0xFF]
    return None


def erased(sav, file):
    """No copy of the file has anything in it."""
    return all(b == 0 for b in sav[copy_offset(file, 0):copy_offset(file, 0) + 0x800])


class Run:
    def __init__(self, args, work):
        self.args = args
        self.work = work
        self.failures = 0

    def check(self, ok, what):
        print(f"run_save_movies: {'ok  ' if ok else 'FAIL'} {what}", flush=True)
        self.failures += not ok

    def movie(self, name):
        path = self.work / f"{name}.inp"
        frames = movie_script.expand((MOVIES / f"{name}.moves").read_text())
        path.write_text("\n".join(frames) + "\n")
        return path, len(frames) + LAST_FRAMES

    def port(self, name, start, out):
        """Plays a movie on the port alone, from a copy of `start` (None: no save file)."""
        movie, frames = self.movie(name)
        out.unlink(missing_ok=True)
        if start:
            shutil.copyfile(start, out)
        result = subprocess.run([str(self.args.dejavu), "--no-window", str(self.args.rom), "--movie",
                                 str(movie), "--frames", str(frames), "--save", str(out)],
                                capture_output=True, text=True)
        self.check(result.returncode == 0, f"{name}: the port alone plays it")
        if result.returncode:
            print(result.stdout, result.stderr)
        return out.read_bytes()

    def reference(self, name, start, out, must_run):
        """Plays a movie on the reference and the C side, from `start`; returns Mesen's save file."""
        movie, _ = self.movie(name)
        cmd = [str(self.args.harness), "--movie", str(movie), "--compare", "--check-ports", "--rom",
               str(self.args.rom), "--home", str(self.work / "mesen-home"), "--sav-out", str(out)]
        if start:
            cmd += ["--sav", str(start)]
        result = subprocess.run(cmd, capture_output=True, text=True)
        self.check(result.returncode == 0, f"{name}: the C side matches Mesen2 at every NMI")
        if result.returncode:
            print(result.stdout[-2000:], result.stderr[-4000:])
        never = next((line for line in result.stdout.splitlines() if "never ran" in line), "")
        routines = {word.strip(",") for word in never.split()}
        missed = [r for r in must_run if r in routines]
        self.check(not missed, f"{name}: on it, {', '.join(must_run)} ran" +
                   (f"; these didn't: {missed}" if missed else ""))
        return out.read_bytes()

    def mesen(self, name, start, out):
        """Plays a movie on Mesen.exe itself, from `start`; returns the save file it wrote."""
        movie, _ = self.movie(name)
        cmd = [sys.executable, str(REPO / "tools" / "run_mesen.py"), "--rom", str(self.args.rom), "--movie",
               str(movie), "--out", str(self.work / "mesen-exe"), "--sav-out", str(out)]
        if start:
            cmd += ["--sav", str(start)]
        result = subprocess.run(cmd, capture_output=True, text=True)
        self.check(result.returncode == 0, f"{name}: Mesen.exe plays it")
        if result.returncode:
            print(result.stdout[-2000:], result.stderr[-2000:])
        return out.read_bytes()


def damaged(sav, *copies):
    """The save file with these copies of file 1 spoiled."""
    bad = bytearray(sav)
    for copy in copies:
        bad[copy_offset(0, copy) + 0x10] ^= 0xFF  # the state no longer matches its sum
    return bytes(bad)


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--dejavu", type=Path, required=True)
    ap.add_argument("--harness", type=Path)
    ap.add_argument("--mesen", action="store_true")
    ap.add_argument("--rom", type=Path, default=REPO / "Deja Vu (USA).nes")
    ap.add_argument("--work", type=Path)
    args = ap.parse_args()
    args.rom = args.rom.resolve()
    work = args.work or Path(tempfile.mkdtemp(prefix="dv-saves-"))
    work.mkdir(parents=True, exist_ok=True)
    run = Run(args, work.resolve())
    work = run.work

    # ---- The port alone ----------------------------------------------------------------------
    saved = run.port("new_game_save", None, work / "saved.sav")
    run.check(len(saved) == SAVE_SIZE, "new_game_save: the save file is 8 KB")
    run.check(good_copies(saved, 0) == 4, "new_game_save: all 4 copies of file 1 are good")
    run.check(goods(saved) == [COAT], "new_game_save: file 1 has the coat")
    run.check(erased(saved, 1) and erased(saved, 2), "new_game_save: files 2 and 3 are empty")

    resaved = run.port("continue_save", work / "saved.sav", work / "resaved.sav")
    run.check(good_copies(resaved, 0) == 4, "continue_save: all 4 copies of file 1 are good")
    run.check(goods(resaved) == [COAT, GUN], "continue_save: file 1 has the coat and the gun")
    backup = work / "resaved.sav.bak"
    run.check(backup.is_file() and backup.read_bytes() == saved,
              "continue_save: the file it started from is kept as .bak")
    run.check(not (work / "resaved.sav.new").exists(), "continue_save: no .new file is left")

    menu = run.port("file_menu", work / "saved.sav", work / "menu.sav")
    run.check(good_copies(menu, 0) == 0, "file_menu: file 1 was erased: it has no good copy left")

    # A bad copy after the first: it isn't even read, and the next save rewrites all 4.
    (work / "later_bad.sav").write_bytes(damaged(saved, 2))
    fixed = run.port("continue_save", work / "later_bad.sav", work / "later_bad_out.sav")
    run.check(good_copies(fixed, 0) == 4 and goods(fixed) == [COAT, GUN],
              "a bad later copy: the game goes on, and saving rewrites it")
    # A bad first copy: the game erases the whole file, good copies and all (docs/saves.md). The port
    # must do the same, and .bak is then all that's left of it.
    (work / "first_bad.sav").write_bytes(damaged(saved, 0))
    fixed = run.port("continue_save", work / "first_bad.sav", work / "first_bad_out.sav")
    run.check(good_copies(fixed, 0) == 0, "a bad first copy: the game erases the file")
    run.check((work / "first_bad_out.sav.bak").read_bytes() == damaged(saved, 0),
              "a bad first copy: what it erased is still in .bak")

    # A file that can't be this game's save RAM is refused, not overwritten.
    for size in (0, 100, SAVE_SIZE - 1, SAVE_SIZE + 1, 0x8000):
        wrong = work / "wrong.sav"
        wrong.write_bytes(b"\xA5" * size)
        movie, _ = run.movie("new_game_save")
        result = subprocess.run([str(args.dejavu), "--no-window", str(args.rom), "--movie", str(movie),
                                 "--frames", "120", "--save", str(wrong)], capture_output=True, text=True)
        run.check(result.returncode == 1 and wrong.read_bytes() == b"\xA5" * size,
                  f"a {size}-byte save file is refused and left alone")

    # ---- With the reference: the same again, each from what the other side wrote ------------------
    if args.harness:
        mesen_saved = run.reference("new_game_save", None, work / "m_saved.sav",
                                    ["save_game", "write_save_copy", "save_dialog"])
        run.check(mesen_saved == saved, "new_game_save: Mesen2's save file is byte for byte the port's")

        # Port to Mesen2: Mesen2 starts from the port's file.
        mesen_resaved = run.reference("continue_save", work / "saved.sav", work / "m_resaved.sav",
                                      ["load_game", "read_save_copy", "check_checksum"])
        run.check(mesen_resaved == resaved, "continue_save from the port's save: Mesen2 ends with the "
                                            "same file as the port")
        # Mesen2 to the port: the port starts from Mesen2's file.
        from_mesen = run.port("continue_save", work / "m_saved.sav", work / "from_mesen.sav")
        run.check(from_mesen == mesen_resaved, "continue_save from Mesen2's save: the port ends with the "
                                               "same file as Mesen2")

        mesen_menu = run.reference("file_menu", work / "saved.sav", work / "m_menu.sav",
                                   ["show_file_options", "draw_file_options", "draw_option_cursor",
                                    "erase_save_file"])
        run.check(mesen_menu == menu, "file_menu: Mesen2 ends with the same file as the port")
        mesen_fixed = run.reference("continue_save", work / "later_bad.sav", work / "m_later_bad.sav",
                                    ["load_game", "save_game"])
        run.check(mesen_fixed == (work / "later_bad_out.sav").read_bytes(),
                  "a bad later copy: Mesen2 ends with the same file as the port")
        mesen_fixed = run.reference("continue_save", work / "first_bad.sav", work / "m_first_bad.sav",
                                    ["erase_save_copies"])
        run.check(mesen_fixed == (work / "first_bad_out.sav").read_bytes(),
                  "a bad first copy: Mesen2 erases the file as the port does")

    # ---- Mesen.exe itself ---------------------------------------------------------------------------
    if args.mesen:
        exe_saved = run.mesen("new_game_save", None, work / "x_saved.sav")
        run.check(exe_saved == saved, "new_game_save: Mesen.exe's save file is byte for byte the port's")
        exe_resaved = run.mesen("continue_save", work / "saved.sav", work / "x_resaved.sav")
        run.check(exe_resaved == resaved, "continue_save from the port's save: Mesen.exe ends with the "
                                          "same file as the port")
        from_exe = run.port("continue_save", work / "x_saved.sav", work / "from_exe.sav")
        run.check(from_exe == exe_resaved, "continue_save from Mesen.exe's save: the port ends with the "
                                           "same file as Mesen.exe")

    print(f"run_save_movies: {run.failures} failed")
    return 1 if run.failures else 0


if __name__ == "__main__":
    sys.exit(main())
