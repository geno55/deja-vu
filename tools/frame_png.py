#!/usr/bin/env python3
"""Turn frames the harness dumped (dv_harness --picture-dump) into a PNG to look at.

usage: frame_png.py --out FILE.png A.raw [B.raw]

A frame is 256 x 240 little-endian 16-bit pixels: a palette color (6 bits) with PPUMASK's emphasis
bits above it. With two frames the PNG has both side by side, then a third panel with the pixels
that differ in red over a dimmed picture of the first. The colors come from nes_rgb, which computes
them as ppu_make_palette does (src/ppu_render.c). Comparisons are of palette colors, not of these.
"""
import argparse
import math
import struct
import zlib
from pathlib import Path

WIDTH, HEIGHT = 256, 240
LOW = (0.350, 0.518, 0.962, 1.550)
HIGH = (1.094, 1.506, 1.962, 1.962)
BLACK, WHITE = 0.518, 1.962
HUE_PHASE, SATURATION = 4, 1.56


def nes_rgb(color):
    """The RGB of a palette color (no emphasis): the 12 phases of its signal, decoded as YIQ."""
    level, hue = color >> 4 & 3, color & 15
    low, high = LOW[level], HIGH[level]
    if hue == 0:
        low = high
    elif hue > 12:
        high = low
    y = i = q = 0.0
    for phase in range(12):
        signal = high if (hue + phase) % 12 < 6 else low
        if hue > 13:
            signal = BLACK
        v = (signal - BLACK) / (WHITE - BLACK)
        y += v
        i += v * math.cos(math.pi * (phase + HUE_PHASE) / 6)
        q += v * math.sin(math.pi * (phase + HUE_PHASE) / 6)
    y, i, q = y / 12, i * SATURATION / 12, q * SATURATION / 12
    rgb = (y + 0.946882 * i + 0.623557 * q,
           y - 0.274788 * i - 0.635691 * q,
           y - 1.108545 * i + 1.709007 * q)
    return tuple(max(0, min(255, round(c * 255))) for c in rgb)


def read_frame(path):
    data = Path(path).read_bytes()
    if len(data) != WIDTH * HEIGHT * 2:
        raise SystemExit(f"{path} isn't a frame ({len(data)} bytes)")
    return struct.unpack(f"<{WIDTH * HEIGHT}H", data)


def chunk(kind, data):
    return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))


def write_png(path, width, height, rgb_rows):
    raw = b"".join(b"\0" + bytes(row) for row in rgb_rows)
    Path(path).write_bytes(b"\x89PNG\r\n\x1a\n"
                           + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0))
                           + chunk(b"IDAT", zlib.compress(raw, 9))
                           + chunk(b"IEND", b""))


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--out", required=True)
    parser.add_argument("frames", nargs="+")
    args = parser.parse_args()
    frames = [read_frame(path) for path in args.frames[:2]]
    colors = [nes_rgb(c & 0x3F) for c in range(64)]
    rows = []
    for y in range(HEIGHT):
        row = []
        for frame in frames:
            for x in range(WIDTH):
                row += colors[frame[y * WIDTH + x] & 0x3F]
            row += (64, 64, 64)
        if len(frames) == 2:
            for x in range(WIDTH):
                i = y * WIDTH + x
                if frames[0][i] != frames[1][i]:
                    row += (255, 0, 0)
                else:
                    row += [c // 3 for c in colors[frames[0][i] & 0x3F]]
        rows.append(row)
    write_png(args.out, len(rows[0]) // 3, HEIGHT, rows)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
