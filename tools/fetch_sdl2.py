#!/usr/bin/env python3
"""Fetch SDL2's Windows development files into external/, for the window (src/platform_sdl.c).

usage: fetch_sdl2.py

Downloads the official release from libsdl-org/SDL on GitHub, checks it against the SHA-256 below,
and unpacks it to external/SDL2-<version>, where CMake looks for it. external/ is never committed.
It does nothing if that is there already. Run CMake's configure step again afterwards.

Elsewhere, install SDL2 as a package instead (Debian and Ubuntu: libsdl2-dev; macOS: brew install
sdl2). Without SDL2 the build still works, but `dejavu` has no window.
"""
import hashlib
import sys
import urllib.request
import zipfile
from pathlib import Path

VERSION = "2.32.10"
ARCHIVE = f"SDL2-devel-{VERSION}-VC.zip"
URL = f"https://github.com/libsdl-org/SDL/releases/download/release-{VERSION}/{ARCHIVE}"
SHA256 = "af347939395a58b365846aaea27391e69f9ec9d4dd650d6ac40802159b418a6e"

EXTERNAL = Path(__file__).resolve().parent.parent / "external"


def main():
    target = EXTERNAL / f"SDL2-{VERSION}"
    if (target / "cmake" / "sdl2-config.cmake").exists():
        print(f"fetch_sdl2: {target} is there already")
        return 0
    download = EXTERNAL / "sdl2-download" / ARCHIVE
    download.parent.mkdir(parents=True, exist_ok=True)
    if not download.exists():
        print(f"fetch_sdl2: downloading {URL}")
        urllib.request.urlretrieve(URL, download)
    digest = hashlib.sha256(download.read_bytes()).hexdigest()
    if digest != SHA256:
        download.unlink()
        print(f"fetch_sdl2: {ARCHIVE} has SHA-256 {digest}, not {SHA256}: deleted it", file=sys.stderr)
        return 1
    with zipfile.ZipFile(download) as archive:
        archive.extractall(EXTERNAL)
    print(f"fetch_sdl2: unpacked {target}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
