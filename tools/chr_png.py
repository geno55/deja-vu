#!/usr/bin/env python3
"""Convert NES CHR graphics (2 bits per pixel) to a PNG and back.

usage: chr_png.py dump CHR.bin OUT.png      (16 tiles across, one row per 16 tiles)
       chr_png.py load IN.png OUT.bin

The PNG uses 4 gray levels (palette entries 0-3, black to white). Loading accepts
8-bit palette or 8-bit grayscale PNGs (grays map to the nearest of the 4 levels),
so an image editor can re-save the file.
"""
import struct
import sys
import zlib
from pathlib import Path

TILES_ACROSS = 16
GRAYS = (0x00, 0x55, 0xAA, 0xFF)


def tiles_to_pixels(chr_data):
    """Rows of pixel values 0-3, TILES_ACROSS tiles wide."""
    count = len(chr_data) // 16
    rows = []
    for tile_row in range((count + TILES_ACROSS - 1) // TILES_ACROSS):
        for y in range(8):
            row = []
            for tx in range(TILES_ACROSS):
                t = tile_row * TILES_ACROSS + tx
                if t >= count:
                    row += [0] * 8
                    continue
                lo, hi = chr_data[t * 16 + y], chr_data[t * 16 + 8 + y]
                row += [(lo >> (7 - x) & 1) | (hi >> (7 - x) & 1) << 1 for x in range(8)]
            rows.append(row)
    return rows


def pixels_to_tiles(rows, count):
    out = bytearray(count * 16)
    for t in range(count):
        tx, ty = t % TILES_ACROSS, t // TILES_ACROSS
        for y in range(8):
            row = rows[ty * 8 + y]
            lo = hi = 0
            for x in range(8):
                p = row[tx * 8 + x]
                lo = lo << 1 | (p & 1)
                hi = hi << 1 | (p >> 1 & 1)
            out[t * 16 + y], out[t * 16 + 8 + y] = lo, hi
    return bytes(out)


def _chunk(kind, data):
    return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))


def write_png(path, rows):
    h, w = len(rows), len(rows[0])
    raw = b"".join(b"\x00" + bytes(r) for r in rows)
    png = (b"\x89PNG\r\n\x1a\n"
           + _chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 3, 0, 0, 0))
           + _chunk(b"PLTE", b"".join(bytes((g, g, g)) for g in GRAYS))
           + _chunk(b"IDAT", zlib.compress(raw, 9))
           + _chunk(b"IEND", b""))
    Path(path).write_bytes(png)


def read_png(path):
    data = Path(path).read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError("not a PNG")
    p, idat, ihdr = 8, b"", None
    while p < len(data):
        n = struct.unpack(">I", data[p:p + 4])[0]
        kind, body = data[p + 4:p + 8], data[p + 8:p + 8 + n]
        if kind == b"IHDR":
            ihdr = struct.unpack(">IIBBBBB", body)
        elif kind == b"IDAT":
            idat += body
        p += 12 + n
    w, h, depth, ctype, _, _, interlace = ihdr
    if depth != 8 or ctype not in (0, 3) or interlace:
        raise ValueError("expected an 8-bit palette or grayscale PNG without interlacing")
    raw, rows, prev = zlib.decompress(idat), [], [0] * w
    for y in range(h):
        f, line = raw[y * (w + 1)], list(raw[y * (w + 1) + 1:(y + 1) * (w + 1)])
        for x in range(w):
            a = line[x - 1] if x else 0
            b = prev[x]
            c = prev[x - 1] if x else 0
            if f == 1:
                line[x] = (line[x] + a) & 0xFF
            elif f == 2:
                line[x] = (line[x] + b) & 0xFF
            elif f == 3:
                line[x] = (line[x] + (a + b) // 2) & 0xFF
            elif f == 4:
                pa, pb, pc = abs(b - c), abs(a - c), abs(a + b - 2 * c)
                line[x] = (line[x] + (a if pa <= pb and pa <= pc else b if pb <= pc else c)) & 0xFF
        prev = line
        rows.append([v & 3 if ctype == 3 else min(range(4), key=lambda i: abs(GRAYS[i] - v)) for v in line])
    return rows


def main():
    if len(sys.argv) != 4 or sys.argv[1] not in ("dump", "load"):
        sys.exit(__doc__)
    if sys.argv[1] == "dump":
        write_png(sys.argv[3], tiles_to_pixels(Path(sys.argv[2]).read_bytes()))
    else:
        rows = read_png(sys.argv[2])
        Path(sys.argv[3]).write_bytes(pixels_to_tiles(rows, len(rows) // 8 * TILES_ACROSS))


if __name__ == "__main__":
    main()
