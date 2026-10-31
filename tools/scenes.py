#!/usr/bin/env python3
"""Decode Deja Vu's scene pictures (banks 0, 1 and 2).

usage: scenes.py [--rom ROM] [--check LOG ...] [--out DIR]
Writes DIR/scenes.json (where each picture sits, for tools/disasm.py) and
DIR/scenes.txt (each picture's tile map in hex). With --check (a .scenes log from
`run_mesen.py --log-scenes`), compares every picture the game drew with the decoded one.

Format (loader at 7:C076; see docs/scenes.md):
- The scene number ($0515) picks a bank: 0 for $00-$4B, 1 for $4C-$97, 2 for $98
  and up, and an entry in that bank's pointer table at $8000.
- Each picture is 212 bytes: a 14x14 map of tile numbers (row by row), drawn at
  nametable $2082, then the 16 attribute bytes for that area, in the order of the
  table at 7:C1A6.
"""
import argparse
import json
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "tools"))
import check_rom  # noqa: E402
from nes6502 import BANK_SIZE, NUM_BANKS, rom_offset, word  # noqa: E402

SIZE = 14                                   # tiles across and down
RECORD = SIZE * SIZE + 16                   # tile map + attribute bytes
BANKS = ((0, 0x00, 0x4C), (1, 0x4C, 0x98), (2, 0x98, 0x100))  # bank, first scene, end


def bank_for(scene):
    for bank, first, end in BANKS:
        if first <= scene < end:
            return bank, scene - first
    raise ValueError(scene)


def pointer_table(prg, bank):
    """Entries of the table at $8000, ending where the first picture starts."""
    entries = [word(prg, rom_offset(0x8000, bank))]
    while 0x8000 + 2 * len(entries) < min(entries):
        entries.append(word(prg, rom_offset(0x8000 + 2 * len(entries), bank)))
    return entries


def load(prg):
    """{scene: {"bank", "addr", "tiles" (196 bytes), "attrs" (16 bytes)}}"""
    scenes = {}
    for bank, first, end in BANKS:
        for i, addr in enumerate(pointer_table(prg, bank)):
            o = rom_offset(addr, bank)
            scenes[first + i] = {"bank": bank, "addr": addr,
                                 "tiles": prg[o:o + SIZE * SIZE], "attrs": prg[o + SIZE * SIZE:o + RECORD]}
    return scenes


def check(scenes, log_path):
    """(scene, tiles that differ, attribute bytes that differ) for every logged picture."""
    out = []
    for line in Path(log_path).read_text().splitlines():
        p = line.split()
        if len(p) != 4 or p[0] != "P":
            continue
        n, tiles, attrs = int(p[1], 16), bytes.fromhex(p[2]), bytes.fromhex(p[3])
        s = scenes[n]
        if tiles == b"\xFF" * len(tiles):
            out.append((n, None, None))  # drawn blank: the room is dark ($7D set)
            continue
        out.append((n, sum(a != b for a, b in zip(tiles, s["tiles"])),
                    sum(a != b for a, b in zip(attrs, s["attrs"]))))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--rom", type=Path, default=check_rom.DEFAULT_ROM)
    ap.add_argument("--check", type=Path, nargs="*", default=[])
    ap.add_argument("--out", type=Path, default=REPO / "build" / "scenes")
    args = ap.parse_args()

    data = args.rom.read_bytes()
    if check_rom.check(data):
        sys.exit(f"scenes: {args.rom} is not Deja Vu (USA); run tools/check_rom.py")
    scenes = load(data[16:16 + NUM_BANKS * BANK_SIZE])

    args.out.mkdir(parents=True, exist_ok=True)
    layout = [{"scene": n, "bank": s["bank"], "addr": s["addr"]} for n, s in sorted(scenes.items())]
    (args.out / "scenes.json").write_text(json.dumps(layout, indent=1))
    lines = []
    for n, s in sorted(scenes.items()):
        lines.append(f"scene {n:02X} at {s['bank']}:{s['addr']:04X}, attributes {s['attrs'].hex(' ').upper()}")
        lines += ["  " + s["tiles"][r * SIZE:(r + 1) * SIZE].hex(" ").upper() for r in range(SIZE)]
    (args.out / "scenes.txt").write_text("\n".join(lines) + "\n")
    print(f"scenes: {len(scenes)} pictures ({len(scenes) * RECORD} bytes) in banks "
          f"{', '.join(str(b) for b, _, _ in BANKS)}")

    for log in args.check:
        results = check(scenes, log)
        exact = sum(1 for _, t, a in results if t == 0 and a == 0)
        dark = sum(1 for _, t, _ in results if t is None)
        differ = [(n, t, a) for n, t, a in results if t]
        print(f"scenes: {log.name}: {len(results)} pictures drawn: {exact} match exactly, "
              f"{dark} drawn dark, {len(differ)} differ")
        for n, t, a in differ:
            print(f"scenes:   scene {n:02X}: {t} of 196 tiles and {a} of 16 attribute bytes differ")


if __name__ == "__main__":
    main()
