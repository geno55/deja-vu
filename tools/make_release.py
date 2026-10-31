#!/usr/bin/env python3
"""Make the Windows release zip from a Release build, by hand as zelda3 does its own.

usage: make_release.py VERSION [--build DIR] [--rom PATH] [--out FILE]

VERSION is the tag the zip is for (v0.1.0, or v0.3 or v0.2-beta); the zip is
build/dejavu_VERSION.zip. There's no version in the program: the tag and the zip's name are the version.

Only the files in the list go in, so the build folder's dejavu_assets.dat, a copy of the game's data,
can't. It refuses a build that a player couldn't run or that we couldn't give out: one with no window (no
SDL2), one that needs Visual C++'s runtime, or, where --rom or the repo's ROM exists, one holding a run of
the ROM's bytes. It publishes nothing: attach the zip to a GitHub release yourself.
"""
import argparse
import re
import sys
import zipfile
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
ROM_RUN = 32  # bytes of the ROM in a row that count as holding its data; shorter runs happen by chance


def rom_run_in(exe, rom):
    """The offset of a ROM_RUN-byte run of the ROM in exe (runs of few values don't count), or None."""
    seen = {exe[i:i + ROM_RUN] for i in range(len(exe) - ROM_RUN + 1)}
    for i in range(len(rom) - ROM_RUN + 1):
        run = rom[i:i + ROM_RUN]
        if run in seen and len(set(run)) > 4:
            return i
    return None


def check_exe(exe, rom=None):
    """Raises ValueError if this program shouldn't go out."""
    if b"sdl2.dll" not in exe.lower():
        raise ValueError("the program has no window: build it with SDL2 (tools/fetch_sdl2.py)")
    if b"vcruntime" in exe.lower():
        raise ValueError("the program needs Visual C++'s runtime: is this a build with DV_HARNESS?")
    if rom is not None:
        offset = rom_run_in(exe, rom)
        if offset is not None:
            raise ValueError(f"the program holds the ROM's bytes at ROM offset ${offset:X}")


def release_files(build, repo):
    """(name in the zip, file) for everything that goes in."""
    sdl_licenses = sorted(repo.glob("external/SDL2-*/LICENSE.txt"))
    if not sdl_licenses:
        raise ValueError("no SDL2 license in external/ (tools/fetch_sdl2.py)")
    files = [
        ("dejavu.exe", build / "Release" / "dejavu.exe"),
        ("SDL2.dll", build / "Release" / "SDL2.dll"),
        ("README.txt", repo / "tools" / "release_readme.txt"),
        ("LICENSE.txt", repo / "LICENSE"),
        ("LICENSE-SDL2.txt", sdl_licenses[-1]),
    ]
    for _, path in files:
        if not path.exists():
            raise ValueError(f"no {path}")
    return files


def make_zip(out, files):
    with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as z:
        for name, path in files:
            z.write(path, name)


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("version")
    ap.add_argument("--build", type=Path, default=REPO / "build")
    ap.add_argument("--rom", type=Path, default=REPO / "Deja Vu (USA).nes")
    ap.add_argument("--out", type=Path)
    args = ap.parse_args()
    if not re.fullmatch(r"v\d+\.\d+(\.\d+)?(-beta)?", args.version):
        sys.exit(f"make_release: {args.version} isn't a version like v0.1.0")
    out = args.out or args.build / f"dejavu_{args.version}.zip"
    try:
        files = release_files(args.build, REPO)
        rom = args.rom.read_bytes()[16:] if args.rom.exists() else None
        if rom is None:
            print("make_release: no ROM, so the program wasn't checked for its bytes")
        check_exe(files[0][1].read_bytes(), rom)
    except ValueError as e:
        sys.exit(f"make_release: {e}")
    make_zip(out, files)
    print(f"make_release: wrote {out}")


if __name__ == "__main__":
    main()
