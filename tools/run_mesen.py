#!/usr/bin/env python3
"""Record code/data logs (CDL) by running the ROM headless in Mesen 2.

usage: run_mesen.py [--rom ROM] [--frames N] [--seed S ...] [--jobs J] [--out DIR]
       run_mesen.py --movie INP [--movie-offset K] [--sav FILE] [--sav-out FILE] [--out DIR]

Each seed is one bot run (see tools/mesen/record_cdl.lua); up to J run at once.
With --movie, a TAS movie converted by tools/movie_inputs.py plays instead.
Runs write DIR/cdl_<run>.cdl plus DIR/cdl_<run>.cdl.ram (RAM at the end, used to
report how far the game got), and DIR/merged.cdl is the OR of every .cdl in DIR.
--trace-scripts, --log-scenes and --log-nmi add logs next to the .cdl (see their help).

Mesen runs on a copy of the ROM named dv-cdl-<run>.nes, so its save file and
debugger workspace never touch a real save of the game. That copy's save file is
deleted before and after every run so each run starts from the same blank save RAM,
which keeps runs repeatable; --sav starts from that file instead, and --sav-out keeps
what Mesen writes as it stops. The Lua I/O permission the script needs is passed as a
command-line switch; Mesen's test runner never saves settings, so the user's
Mesen configuration is unchanged.
"""
import argparse
import concurrent.futures
import os
import shutil
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "tools"))
import check_rom  # noqa: E402

LUA = REPO / "tools" / "mesen" / "record_cdl.lua"
WINGET_MESEN = (Path(os.environ.get("LOCALAPPDATA", "")) / "Microsoft" / "WinGet" / "Packages"
                / "SourMesen.Mesen2_Microsoft.Winget.Source_8wekyb3d8bbwe" / "Mesen.exe")


def find_mesen():
    for candidate in (os.environ.get("MESEN"), shutil.which("Mesen"), WINGET_MESEN):
        if candidate and Path(candidate).is_file():
            return Path(candidate)
    sys.exit("run_mesen: Mesen 2 not found; set MESEN to Mesen.exe")


def mesen_home(mesen):
    # Portable installs keep settings.json next to Mesen.exe; otherwise it's Documents\Mesen2.
    if (mesen.parent / "settings.json").is_file():
        return mesen.parent
    return Path.home() / "Documents" / "Mesen2"


def run_one(mesen, rom_data, out_dir, name, frames, env_extra, sav=None, sav_out=None):
    rom_copy = out_dir / f"dv-cdl-{name}.nes"
    rom_copy.write_bytes(rom_data)
    battery = mesen_home(mesen) / "Saves" / (rom_copy.stem + ".sav")
    battery.unlink(missing_ok=True)
    if sav:
        shutil.copyfile(sav, battery)
    out_file = out_dir / f"cdl_{name}.cdl"
    out_file.unlink(missing_ok=True)
    env = dict(os.environ, DV_OUT=str(out_file), DV_FRAMES=str(frames), **env_extra)
    for var, suffix in (("DV_SCRIPT_LOG", ".scripts"), ("DV_SCENE_LOG", ".scenes"),
                        ("DV_NMI_LOG", ".nmi")):
        if env.get(var) == "yes":
            env[var] = str(out_file) + suffix
    # Hashing 12 KB of memory in Lua at every NMI makes --log-nmi runs several times slower.
    timeout_s = 120 + frames // (20 if env.get("DV_NMI_LOG") else 100)
    cmd = [str(mesen), "--testrunner", f"--timeout={timeout_s}",
           "--debug.scriptWindow.allowIoOsAccess=true", str(rom_copy), str(LUA)]
    result = subprocess.run(cmd, env=env, capture_output=True, text=True, errors="replace",
                            timeout=timeout_s + 30)
    if result.returncode != 0 or not out_file.is_file():
        err = Path(str(out_file) + ".err")
        detail = err.read_text() if err.is_file() else result.stdout[-2000:]
        raise RuntimeError(f"{name} failed (exit {result.returncode})\n{detail}")
    if sav_out:
        shutil.copyfile(battery, sav_out)
    battery.unlink(missing_ok=True)
    return name, progress(Path(str(out_file) + ".ram").read_bytes())


def progress(ram):
    """How far the game got, from RAM addresses on Data Crystal's RAM map."""
    flag_bits = sum(bin(b).count("1") for b in ram[0x0420:0x04B0])
    items = sum(1 for b in ram[0x04C0:0x04F0] if b != 0xFF)
    return f"{flag_bits} story/object flag bits set, {items} address book + inventory slots used"


def merge(out_dir):
    merged = None
    for f in sorted(out_dir.glob("cdl_*.cdl")):
        data = f.read_bytes()
        merged = bytearray(data) if merged is None else bytearray(a | b for a, b in zip(merged, data))
    (out_dir / "merged.cdl").write_bytes(merged)
    return merged


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--rom", type=Path, default=check_rom.DEFAULT_ROM)
    ap.add_argument("--frames", type=int, default=20000)
    ap.add_argument("--seed", type=int, nargs="+", default=[1])
    ap.add_argument("--jobs", type=int, default=4)
    ap.add_argument("--movie", type=Path, help=".inp file from tools/movie_inputs.py")
    ap.add_argument("--movie-offset", type=int, default=0)
    ap.add_argument("--extra-frames", type=int, default=300,
                    help="frames to keep running after the movie's last input (the ending, for example)")
    ap.add_argument("--trace-scripts", action="store_true",
                    help="also log every script the game runs to DIR/cdl_<run>.cdl.scripts")
    ap.add_argument("--log-scenes", action="store_true",
                    help="also log each scene picture as drawn to DIR/cdl_<run>.cdl.scenes")
    ap.add_argument("--log-nmi", action="store_true",
                    help="also log the state at every NMI entry to DIR/cdl_<run>.cdl.nmi, in the "
                         "format harness/dv_harness --log writes")
    ap.add_argument("--sav", type=Path, help="with --movie: start from this save file")
    ap.add_argument("--sav-out", type=Path,
                    help="with --movie: keep the save file Mesen writes as it stops")
    ap.add_argument("--out", type=Path, default=REPO / "build" / "cdl")
    args = ap.parse_args()
    if (args.sav or args.sav_out) and not args.movie:
        ap.error("--sav and --sav-out go with --movie")

    data = args.rom.read_bytes()
    if check_rom.check(data):
        sys.exit(f"run_mesen: {args.rom} is not Deja Vu (USA); run tools/check_rom.py")
    # Mesen's Lua opens these paths from its own working directory, so make them absolute.
    args.out = args.out.resolve()
    args.out.mkdir(parents=True, exist_ok=True)

    if args.movie:
        movie_frames = len(args.movie.read_text().splitlines())
        name = f"movie_{args.movie.stem}" + (f"_off{args.movie_offset}" if args.movie_offset else "")
        runs = [(name, movie_frames - args.movie_offset + args.extra_frames,
                 {"DV_SEED": "0", "DV_MOVIE": str(args.movie.resolve()),
                  "DV_MOVIE_OFFSET": str(args.movie_offset)})]
    else:
        runs = [(f"seed{s}", args.frames, {"DV_SEED": str(s)}) for s in args.seed]

    for _, _, env in runs:
        if args.trace_scripts:
            env["DV_SCRIPT_LOG"] = "yes"
        if args.log_scenes:
            env["DV_SCENE_LOG"] = "yes"
        if args.log_nmi:
            env["DV_NMI_LOG"] = "yes"

    mesen = find_mesen()
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futures = [pool.submit(run_one, mesen, data, args.out, name, frames, env, args.sav, args.sav_out)
                   for name, frames, env in runs]
        for f in concurrent.futures.as_completed(futures):
            try:
                name, how_far = f.result()
                print(f"run_mesen: {name}: done; {how_far}", flush=True)
            except RuntimeError as e:
                sys.exit(f"run_mesen: {e}")

    merged = merge(args.out)
    code = sum(1 for b in merged if b & 1)
    data_b = sum(1 for b in merged if b & 2 and not b & 1)
    print(f"run_mesen: merged {len(merged)} bytes: {code} code, {data_b} data, "
          f"{len(merged) - code - data_b} never touched")


if __name__ == "__main__":
    main()
