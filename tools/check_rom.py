#!/usr/bin/env python3
"""Verify that a ROM is Deja Vu (USA) for the NES.

Identity is the SHA-1 of the ROM data without the 16-byte iNES header,
because headers differ between otherwise identical dumps.

usage: check_rom.py [ROM]   (default: "Deja Vu (USA).nes" in the repo root)
exit status: 0 = good ROM, 1 = wrong or damaged ROM, 2 = cannot read it
"""
import hashlib
import sys
import zlib
from pathlib import Path

EXPECTED_SHA1 = "c8ca009cfb56edb86acd8bb8f05d96d19132ad66"
EXPECTED_CRC32 = 0x43D01C10
PRG_SIZE = 8 * 0x4000
CHR_SIZE = 32 * 0x2000
MAPPER = 4  # MMC3

DEFAULT_ROM = Path(__file__).resolve().parent.parent / "Deja Vu (USA).nes"


def parse_header(header):
    """Return (prg_size, chr_size, mapper) from an iNES / NES 2.0 header."""
    if len(header) < 16 or header[:4] != b"NES\x1a":
        raise ValueError("missing iNES header")
    nes2 = (header[7] & 0x0C) == 0x08
    prg_units = header[4] | ((header[9] & 0x0F) << 8 if nes2 else 0)
    chr_units = header[5] | ((header[9] & 0xF0) << 4 if nes2 else 0)
    mapper = (header[6] >> 4) | (header[7] & 0xF0)
    if nes2:
        mapper |= (header[8] & 0x0F) << 8
    return prg_units * 0x4000, chr_units * 0x2000, mapper


def check(data):
    """Return a list of problems with the ROM image; empty means it's good."""
    try:
        prg_size, chr_size, mapper = parse_header(data[:16])
    except ValueError as e:
        return [str(e)]
    problems = []
    if mapper != MAPPER:
        problems.append(f"mapper {mapper}, expected {MAPPER} (MMC3)")
    if (prg_size, chr_size) != (PRG_SIZE, CHR_SIZE):
        problems.append(f"PRG/CHR {prg_size // 1024}/{chr_size // 1024} KB, "
                        f"expected {PRG_SIZE // 1024}/{CHR_SIZE // 1024} KB")
    body = data[16:]
    if len(body) != PRG_SIZE + CHR_SIZE:
        problems.append(f"{len(body)} bytes after the header, expected {PRG_SIZE + CHR_SIZE}")
    sha1 = hashlib.sha1(body).hexdigest()
    if sha1 != EXPECTED_SHA1:
        problems.append(f"SHA-1 {sha1} (CRC32 {zlib.crc32(body):08X}), "
                        f"expected {EXPECTED_SHA1} (CRC32 {EXPECTED_CRC32:08X})")
    return problems


def main(argv):
    path = Path(argv[1]) if len(argv) > 1 else DEFAULT_ROM
    try:
        data = path.read_bytes()
    except OSError as e:
        print(f"check_rom: cannot read {path}: {e.strerror}", file=sys.stderr)
        return 2
    problems = check(data)
    if problems:
        print(f"check_rom: {path} is not Deja Vu (USA):", file=sys.stderr)
        for p in problems:
            print(f"  - {p}", file=sys.stderr)
        return 1
    print(f"check_rom: OK, {path.name} is Deja Vu (USA)")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
