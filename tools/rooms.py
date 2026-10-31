#!/usr/bin/env python3
"""Map Deja Vu's room hotspots: the clickable areas of each room (bank 2, $9C00).

usage: rooms.py [--rom ROM] [--cdl FILE]
Prints a summary; tools/disasm.py uses load() to classify the data.

Format (read by 7:C462 and 7:CE13; records tested against the cursor by 7:C283):
- An address table at 2:9C00, indexed by the room number ($0514).
- Each room has a list of 6-byte records, ended by a byte $FF:
    x1 x2 y1 y2   the cursor ($A5, $A6) is inside when x1 <= x < x2 and y1 <= y < y2
    kind          bit 7 set: 7:C462 also runs far call $60 with $BD = kind & $7F, $BE = id;
                  7:CE13 compares kind & $7F with $AA (the script table to use)
    id            the object number, compared with $AB
"""
import argparse
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "tools"))
import check_rom  # noqa: E402
from nes6502 import BANK_SIZE, NUM_BANKS, rom_offset, word  # noqa: E402

BANK = 2
TABLE = 0x9C00
RECORD = 6
END = 0xFF


def load(prg):
    """(table entries, {list address: number of records})"""
    rd = lambda a: prg[rom_offset(a, BANK)]  # noqa: E731
    entries = [word(prg, rom_offset(TABLE, BANK))]
    while TABLE + 2 * len(entries) < min(entries):
        entries.append(word(prg, rom_offset(TABLE + 2 * len(entries), BANK)))
    lists = {}
    for start in entries:
        n = 0
        while rd(start + RECORD * n) != END:
            n += 1
        lists[start] = n
    return entries, lists


ITEM_LISTS = 0xA400  # read by 7:D4FD


def load_item_lists(prg):
    """Item-number lists that 7:D4FD copies into the coat slots ($04C7).

    2:A400 holds two addresses (entry 0 when $AF = 1, else entry 1), each of a table
    indexed by $B0 whose entries point to lists of item numbers ended by $FF.
    Returns (tables as (address, entries), {list address: length including the $FF}).
    """
    rd = lambda a: prg[rom_offset(a, BANK)]  # noqa: E731
    rw = lambda a: word(prg, rom_offset(a, BANK))  # noqa: E731
    tables, lists = [(ITEM_LISTS, [rw(ITEM_LISTS), rw(ITEM_LISTS + 2)])], {}
    for sub in tables[0][1]:
        entries = [rw(sub)]
        while sub + 2 * len(entries) < min(entries):
            entries.append(rw(sub + 2 * len(entries)))
        tables.append((sub, entries))
        for start in entries:
            n = 0
            while rd(start + n) != END:
                n += 1
            lists[start] = n + 1
    return tables, lists


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--rom", type=Path, default=check_rom.DEFAULT_ROM)
    ap.add_argument("--cdl", type=Path)
    args = ap.parse_args()
    data = args.rom.read_bytes()
    if check_rom.check(data):
        sys.exit(f"rooms: {args.rom} is not Deja Vu (USA); run tools/check_rom.py")
    prg = data[16:16 + NUM_BANKS * BANK_SIZE]
    entries, lists = load(prg)
    covered = set(range(TABLE, TABLE + 2 * len(entries)))
    for a, n in lists.items():
        covered |= set(range(a, a + RECORD * n + 1))
    lo, hi = TABLE, max(covered) + 1
    print(f"rooms: {len(entries)} rooms, {len(lists)} lists, {sum(lists.values())} hotspots; "
          f"${lo:04X}-${hi - 1:04X}: {len(covered)} of {hi - lo} bytes covered")
    tables, item_lists = load_item_lists(prg)
    item_cov = set()
    for a, e in tables:
        item_cov |= set(range(a, a + 2 * len(e)))
    for a, n in item_lists.items():
        item_cov |= set(range(a, a + n))
    print(f"rooms: item lists: {len(tables)} tables, {len(item_lists)} lists, {len(item_cov)} bytes "
          f"from ${ITEM_LISTS:04X} to ${max(item_cov):04X}")
    if args.cdl:
        cdl = args.cdl.read_bytes()
        for name, cov, a0, a1 in (("hotspots", covered, lo, hi), ("item lists", item_cov, ITEM_LISTS, max(item_cov) + 1)):
            outside = [a for a in range(a0, a1) if cdl[rom_offset(a, BANK)] & 2 and a not in cov]
            print(f"rooms: {name}: {len(outside)} bytes the game read in that range are outside the map")


if __name__ == "__main__":
    main()
