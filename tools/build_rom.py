#!/usr/bin/env python3
"""Assemble asm/*.s into a ROM and check it matches the original byte for byte.

usage: build_rom.py [--rom ROM] [--out FILE]

Extracts the data the assembly includes from your ROM (tools/extract_bins.py),
assembles every asm/*.s with ca65, links them with asm/dejavu.cfg, and compares
the result's PRG and CHR with Deja Vu (USA). Exit status 0 means identical.
Also writes FILE.labels (ld65's label file: every symbol with its address).
"""
import argparse
import hashlib
import re
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "tools"))
import check_rom  # noqa: E402
import disasm  # noqa: E402

ASM = REPO / "asm"
SOURCES = ["header"] + [f"bank{b}" for b in range(8)] + ["chr"]


def run(cmd):
    result = subprocess.run([str(c) for c in cmd], capture_output=True, text=True)
    if result.returncode != 0:
        sys.exit(f"build_rom: {' '.join(map(str, cmd))} failed:\n{result.stdout}{result.stderr}")


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--rom", type=Path, default=check_rom.DEFAULT_ROM)
    ap.add_argument("--out", type=Path, default=REPO / "build" / "dejavu.nes")
    args = ap.parse_args()

    build = args.out.parent
    bins, objs = build / "bin", build / "obj"
    objs.mkdir(parents=True, exist_ok=True)
    run([sys.executable, REPO / "tools" / "extract_bins.py", "--rom", args.rom, "--out", bins])
    ca65, ld65 = disasm.cc65_tool("ca65"), disasm.cc65_tool("ld65")
    for name in SOURCES:
        run([ca65, "--cpu", "6502", "-g", "--bin-include-dir", bins, "-o", objs / f"{name}.o", ASM / f"{name}.s"])
    labels = args.out.with_suffix(".labels")
    run([ld65, "-C", ASM / "dejavu.cfg", "-o", args.out, "-Ln", labels] + [objs / f"{name}.o" for name in SOURCES])

    # A named symbol must mean one address everywhere: a bank's equate for a fixed-bank
    # routine has to match the label in bank7.s (catches hand-placed labels in the wrong spot).
    seen = {}
    for line in labels.read_text().splitlines():
        _, addr, name = line.split()
        name = name.lstrip(".")
        if re.fullmatch(r"L[0-9A-F]{4}", name):
            continue
        if seen.setdefault(name, addr) != addr:
            sys.exit(f"build_rom: {name} is ${int(seen[name], 16):04X} in one file and ${int(addr, 16):04X} in another")

    built = args.out.read_bytes()
    sha1 = hashlib.sha1(built[16:]).hexdigest()
    if sha1 != check_rom.EXPECTED_SHA1:
        original = args.rom.read_bytes()[16:]
        diff = next((i for i in range(min(len(built) - 16, len(original))) if built[16 + i] != original[i]), None)
        sys.exit(f"build_rom: {args.out.name} differs from the original"
                 + (f", first at ROM offset ${diff:05X}" if diff is not None else f" (size {len(built)})"))
    print(f"build_rom: {args.out} matches Deja Vu (USA) byte for byte")


if __name__ == "__main__":
    main()
