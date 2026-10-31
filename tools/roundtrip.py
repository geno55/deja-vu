#!/usr/bin/env python3
"""Turn every block of game data into an editable file and back, and check the bytes match.

usage: roundtrip.py [--rom ROM] [--out DIR]

For each block in asm/bins.txt (the data the assembly takes from the ROM), writes
DIR/<name>.txt with tools/formats.py (DIR/chr.png for the graphics), reads that file
back with the matching encoder, and compares the result with the original bytes.
Exit status 0 means every block round-trips exactly. The files contain the game's
data, so DIR (default build/src) is not committed.
"""
import argparse
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "tools"))
import check_rom  # noqa: E402
import chr_png  # noqa: E402
import formats  # noqa: E402
import extract_bins  # noqa: E402
from nes6502 import BANK_SIZE, NUM_BANKS  # noqa: E402


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--rom", type=Path, default=check_rom.DEFAULT_ROM)
    ap.add_argument("--out", type=Path, default=REPO / "build" / "src")
    args = ap.parse_args()

    data = args.rom.read_bytes()
    if check_rom.check(data):
        sys.exit(f"roundtrip: {args.rom} is not Deja Vu (USA); run tools/check_rom.py")
    body, prg = data[16:], data[16:16 + NUM_BANKS * BANK_SIZE]
    args.out.mkdir(parents=True, exist_ok=True)

    failures = 0
    for name, bank, addr, length, _ in extract_bins.read_manifest():
        original = extract_bins.piece(body, bank, addr, length)
        if bank == extract_bins.CHR:
            path = args.out / f"{name}.png"
            chr_png.write_png(path, chr_png.tiles_to_pixels(original))
            rebuilt = chr_png.pixels_to_tiles(chr_png.read_png(path), length // 16)
            how = "PNG"
        else:
            dump, parse = formats.codec_for(name, prg)
            path = args.out / f"{name}.txt"
            try:
                path.write_text(dump(prg, bank, addr, length))
                rebuilt = parse(path.read_text())
            except (ValueError, KeyError, IndexError) as e:
                rebuilt, how = None, f"error: {e}"
            else:
                how = dump.__name__.replace("dump_", "")
        if rebuilt == original:
            status = "identical"
        else:
            failures += 1
            if rebuilt is None:
                status = how
            else:
                diff = next((i for i in range(min(len(rebuilt), len(original))) if rebuilt[i] != original[i]),
                            min(len(rebuilt), len(original)))
                status = f"DIFFERS at +${diff:04X} ({len(rebuilt)} vs {len(original)} bytes)"
        print(f"roundtrip: {name:<20} {length:6} bytes  {how:<10} {status}")
    print(f"roundtrip: {'every block round-trips' if not failures else f'{failures} blocks differ'}; "
          f"files in {args.out}")
    sys.exit(1 if failures else 0)


if __name__ == "__main__":
    main()
