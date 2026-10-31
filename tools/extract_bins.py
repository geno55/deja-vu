#!/usr/bin/env python3
"""Extract the game data the assembly includes from your ROM into build/bin.

usage: extract_bins.py [--rom ROM] [--out DIR]

The repository's assembly (asm/*.s) holds the code and small tables; large data
(scene pictures, messages, scripts, sound, graphics) comes from your ROM through
.incbin. asm/bins.txt lists each piece: name, bank, CPU address, length and
SHA-1. Every extracted file is checked against its SHA-1.
"""
import argparse
import hashlib
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "tools"))
import check_rom  # noqa: E402
from nes6502 import BANK_SIZE, NUM_BANKS, rom_offset  # noqa: E402

MANIFEST = REPO / "asm" / "bins.txt"
CHR = "chr"  # bank "chr" in the manifest: offset into CHR ROM instead of a CPU address


def read_manifest(path=MANIFEST):
    """[(name, bank or "chr", address, length, sha1)]"""
    out = []
    for line in path.read_text().splitlines():
        if line.strip() and not line.startswith("#"):
            name, bank, addr, length, sha1 = line.split()
            out.append((name, bank if bank == CHR else int(bank), int(addr, 16), int(length), sha1))
    return out


def piece(rom_body, bank, addr, length):
    if bank == CHR:
        start = NUM_BANKS * BANK_SIZE + addr
    else:
        start = rom_offset(addr, bank)
    return rom_body[start:start + length]


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--rom", type=Path, default=check_rom.DEFAULT_ROM)
    ap.add_argument("--out", type=Path, default=REPO / "build" / "bin")
    args = ap.parse_args()

    data = args.rom.read_bytes()
    problems = check_rom.check(data)
    if problems:
        sys.exit(f"extract_bins: {args.rom} is not Deja Vu (USA):\n  " + "\n  ".join(problems))
    body = data[16:]
    args.out.mkdir(parents=True, exist_ok=True)
    total = 0
    for name, bank, addr, length, sha1 in read_manifest():
        blob = piece(body, bank, addr, length)
        if hashlib.sha1(blob).hexdigest() != sha1:
            sys.exit(f"extract_bins: {name} does not match its SHA-1 in {MANIFEST.name}")
        (args.out / f"{name}.bin").write_bytes(blob)
        total += length
    print(f"extract_bins: wrote {len(read_manifest())} files ({total} bytes) to {args.out}")


if __name__ == "__main__":
    main()
