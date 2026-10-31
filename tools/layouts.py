#!/usr/bin/env python3
"""Map Deja Vu's screen layouts: the tile lists that draw menus and title text (bank 6).

usage: layouts.py [--rom ROM]
Prints each layout's address and size. tools/disasm.py uses load() to classify them.

Format (drawn by 7:FBF0 with A = layout number; commands at 7:FC88):
- An address table at 6:BC00 points to each layout.
- A layout is a list of commands, ended by $FF:
    $00 n          print item name n (from the table at 2:8D00)    2 bytes
    $01-$05 a a    print something found through address a          3 bytes
    $06 x y        move to x, y                                     3 bytes
    $12 a a        print the item name whose number is at address a 3 bytes
    anything else  one font tile ($90 and $A0 are skipped)          1 byte
"""
import argparse
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "tools"))
import check_rom  # noqa: E402
from nes6502 import BANK_SIZE, NUM_BANKS, rom_offset, word  # noqa: E402

BANK = 6
TABLE = 0xBC00
END = 0xFF


def size(cmd):
    if cmd == 0x00:
        return 2
    if 0x01 <= cmd <= 0x06 or cmd == 0x12:
        return 3
    return 1


def load(prg):
    """(table entries, {layout address: length in bytes})"""
    entries = [word(prg, rom_offset(TABLE, BANK))]
    while TABLE + 2 * len(entries) < min(entries):
        entries.append(word(prg, rom_offset(TABLE + 2 * len(entries), BANK)))
    layouts = {}
    for start in entries:
        a = start
        while prg[rom_offset(a, BANK)] != END:
            a += size(prg[rom_offset(a, BANK)])
        layouts[start] = a + 1 - start
    return entries, layouts


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--rom", type=Path, default=check_rom.DEFAULT_ROM)
    args = ap.parse_args()
    data = args.rom.read_bytes()
    if check_rom.check(data):
        sys.exit(f"layouts: {args.rom} is not Deja Vu (USA); run tools/check_rom.py")
    entries, layouts = load(data[16:16 + NUM_BANKS * BANK_SIZE])
    print(f"layouts: {len(entries)} entries, {len(layouts)} layouts, {sum(layouts.values())} bytes")
    for a, n in sorted(layouts.items()):
        print(f"layouts:   {BANK}:{a:04X} {n} bytes")


if __name__ == "__main__":
    main()
