#!/usr/bin/env python3
"""Compare the sound of the APU with the reference's (dv_harness --apu --audio-wav PREFIX).

usage: compare_audio.py PREFIX_c.wav PREFIX_ref.wav [--start SECONDS] [--seconds N]

The two come from different mixers and filters, so they can't match sample for sample. This finds the
delay between them, a fraction of a sample included, and reports band by band how the reference
compares with this APU: its level as a ratio (1 is the same) and the coherence (1 means one is just a
filtered copy of the other). Then what is left over of the whole signal, per second, in decibels below
it. Needs numpy.
"""
import argparse
import sys
from pathlib import Path

import numpy as np

BANDS = [10, 20, 40, 80, 160, 320, 640, 1280, 2560, 5120, 8000, 12000, 16000, 20000, 24000]
BLOCK = 1 << 16


def read_wav(path):
    """Returns (rate, samples of the first channel, as floats)."""
    data = Path(path).read_bytes()
    if data[:4] != b"RIFF" or data[8:12] != b"WAVE":
        raise SystemExit(f"{path} isn't a WAV file")
    pos, rate, channels = 12, 0, 1
    while pos + 8 <= len(data):
        kind, size = data[pos:pos + 4], int.from_bytes(data[pos + 4:pos + 8], "little")
        if kind == b"fmt ":
            channels = int.from_bytes(data[pos + 10:pos + 12], "little")
            rate = int.from_bytes(data[pos + 12:pos + 16], "little")
        elif kind == b"data":
            samples = np.frombuffer(data[pos + 8:pos + 8 + (size & ~1)], dtype=np.int16)
            return rate, samples[::channels].astype(np.float64)
        pos += 8 + size + (size & 1)
    raise SystemExit(f"{path} has no sound")


def delayed(x, delay):
    n = len(x)
    return np.fft.irfft(np.fft.rfft(x) * np.exp(-2j * np.pi * np.fft.rfftfreq(n) * delay), n)


def residual_db(x, y):
    """How far y is from x scaled to fit it, in dB below y, with the ends left out."""
    x, y = x[200:-200], y[200:-200]
    gain = (x * y).sum() / (x * x).sum()
    rest = y - gain * x
    return 10 * np.log10((rest * rest).sum() / (y * y).sum())


def find_delay(x, y):
    """The delay of x, in samples, at which it fits y best."""
    spectrum = np.fft.irfft(np.fft.rfft(y) * np.conj(np.fft.rfft(x)))
    whole = int(np.argmax(spectrum))
    if whole > len(x) // 2:
        whole -= len(x)
    fractions = np.arange(whole - 1, whole + 1.0001, 0.02)
    return min(fractions, key=lambda d: residual_db(delayed(x, d), y))


def report_bands(x, y, rate):
    window = np.hanning(BLOCK)
    size = BLOCK // 2 + 1
    xx, yy, xy = np.zeros(size), np.zeros(size), np.zeros(size, dtype=complex)
    for i in range(0, len(x) - BLOCK, BLOCK // 2):
        fx, fy = np.fft.rfft(x[i:i + BLOCK] * window), np.fft.rfft(y[i:i + BLOCK] * window)
        xx += np.abs(fx) ** 2
        yy += np.abs(fy) ** 2
        xy += fy * np.conj(fx)
    freq = np.fft.rfftfreq(BLOCK, 1 / rate)
    print("    band (Hz)    level  coherence  share of the reference's power (dB)")
    for lo, hi in zip(BANDS, BANDS[1:]):
        m = (freq >= lo) & (freq < hi)
        level = abs(xy[m].sum()) / xx[m].sum()
        coherence = abs(xy[m].sum()) ** 2 / (xx[m].sum() * yy[m].sum())
        print(f"  {lo:6d}-{hi:6d}  {level:6.3f}  {coherence:9.4f}  {10 * np.log10(yy[m].sum() / yy.sum()):7.1f}")


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("mine")
    parser.add_argument("reference")
    parser.add_argument("--start", type=float, default=10.0)
    parser.add_argument("--seconds", type=float, default=180.0)
    args = parser.parse_args()
    rate, a = read_wav(args.mine)
    rate_b, b = read_wav(args.reference)
    if rate != rate_b:
        raise SystemExit(f"rates differ: {rate}, {rate_b}")
    start, stop = int(args.start * rate), int((args.start + args.seconds) * rate)
    stop = min(stop, len(a), len(b))
    x, y = a[start:stop] - a[start:stop].mean(), b[start:stop] - b[start:stop].mean()
    print(f"{len(x) / rate:.0f} s of {len(a) / rate:.0f}")
    delay = find_delay(x[:30 * rate], y[:30 * rate])
    print(f"this APU is {-delay:.2f} samples later than the reference")
    x = delayed(x, delay)
    report_bands(x, y, rate)
    print(f"all of it: {residual_db(x, y):.1f} dB left over")
    seconds = []
    for s in range(0, len(x) - rate, rate):
        p, q = x[s:s + rate], y[s:s + rate]
        if (q * q).mean() > 100 ** 2:  # not silence
            seconds.append(residual_db(p, q))
    seconds.sort()
    if seconds:
        print(f"per second with sound ({len(seconds)}): best {seconds[0]:.1f} dB, median "
              f"{seconds[len(seconds) // 2]:.1f}, worst {seconds[-1]:.1f}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
