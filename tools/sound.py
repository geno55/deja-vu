#!/usr/bin/env python3
"""Map Deja Vu's music and sound-effect data (bank 5, $8000-$99FF).

usage: sound.py [--rom ROM] [--cdl FILE] [--out DIR]
Writes DIR/sound.json (where each table, header, channel stream and instrument
sits, for tools/disasm.py) and DIR/sound.txt (a listing). With --cdl, reports
any byte the game read in the sound area that the map doesn't explain.

Format (driver at 7:F538, run from the NMI; see docs/sound.md):
- play_sound (7:F409) takes a sound number n (1-based) and reads entry n-1 of
  the address table at 5:8000.
- A header is 12 bytes: type, four channel stream addresses (bytes 1-8; which
  ones a type uses depends on it), 2 unused bytes, and the speed (byte 11).
  Type 0 and 1 are music on the four music channels (1 repeats), 2 is a sound
  effect on three channels, 3 and 4 on two.
- A channel stream is a list of commands, ended by $FF:
    $00-$7D, $7F  note ($7F: rest)          $7E       wait (extends the note)
    $80-$BF       length, from 7:FAF5       $C0-$CE   loop start, n times
    $CF           loop end                  $D0-$DF   leave the loop on pass n
    $E0-$FE       3 bytes: command + address; $E0/$E1 point to an instrument
                  (4 bytes + volume envelope), $E2 to 4 bytes, $E3 to an envelope
"""
import argparse
import json
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "tools"))
import check_rom  # noqa: E402
from nes6502 import BANK_SIZE, NUM_BANKS, rom_offset, word  # noqa: E402

BANK = 5
TABLE = 0x8000
AREA_END = 0x9A00  # the message table starts here
HEADER = 12
CHANNELS = {0: (1, 3, 5, 7), 1: (1, 3, 5, 7), 2: (1, 3, 5), 3: (3, 7), 4: (3, 7)}  # pointer offsets
ENVELOPE_MAX = 64  # the driver stops advancing an envelope at index $3F


class Sound:
    def __init__(self, prg):
        self.prg = prg
        self.items = {}  # start address -> (kind, length, name)
        self.problems = []

    def rd(self, a):
        return self.prg[rom_offset(a, BANK)]

    def rw(self, a):
        return word(self.prg, rom_offset(a, BANK))

    def add(self, start, length, kind, name):
        old = self.items.get(start)
        if old and old[1] >= length:
            return
        self.items[start] = (kind, length, name)

    def walk(self):
        entries = [self.rw(TABLE)]
        while TABLE + 2 * len(entries) < min(entries):
            entries.append(self.rw(TABLE + 2 * len(entries)))
        self.add(TABLE, 2 * len(entries), "table", "sound_table")
        for n, h in enumerate(entries, 1):
            t = self.rd(h)
            if t not in CHANNELS:
                self.problems.append(f"sound {n}: header ${h:04X} has type {t}")
                continue
            self.add(h, HEADER, "header", f"sound_{n:02X}")
            for ch, off in enumerate(CHANNELS[t]):
                s = self.rw(h + off)
                if s:
                    self.stream(s, f"sound_{n:02X}_ch{ch}")
        return entries

    def stream(self, start, name):
        a, instruments = start, []
        while True:
            c = self.rd(a)
            if c == 0xFF:
                a += 1
                break
            if c >= 0xE0:
                target = self.rw(a + 1)
                if c in (0xE0, 0xE1):
                    instruments.append((target, "instrument", 4))
                elif c == 0xE2:
                    instruments.append((target, "sweep", 4))
                elif c == 0xE3:
                    instruments.append((target, "envelope", 0))
                a += 3
            else:
                a += 1
            if a >= AREA_END:
                self.problems.append(f"{name}: stream at ${start:04X} runs past ${AREA_END:04X}")
                return
        self.add(start, a - start, "stream", name)
        for target, kind, fixed in instruments:
            self.add(target, fixed, kind, f"{kind}_{target:04X}")

    def unused(self):
        """Gaps that hold sound data no table points to: a stream that ends exactly at the
        end of the gap, or a 68-byte block laid out like an instrument."""
        covered = self.coverage()
        a = TABLE
        while a < AREA_END:
            if a in covered:
                a += 1
                continue
            end = a
            while end < AREA_END and end not in covered:
                end += 1
            if self.rd(a) != 0xFF:
                p = a
                while p < end and self.rd(p) != 0xFF:
                    p += 3 if self.rd(p) >= 0xE0 else 1
                if p == end - 1:
                    self.add(a, end - a, "unused_stream", f"unused_stream_{a:04X}")
                elif end - a == 4 + ENVELOPE_MAX:
                    self.add(a, end - a, "unused_instrument", f"unused_instrument_{a:04X}")
            a = end

    def finish(self):
        """Give each envelope the room up to the next known item (at most 64 bytes)."""
        starts = sorted(self.items)
        for i, a in enumerate(starts):
            kind, length, name = self.items[a]
            if kind in ("instrument", "envelope"):
                nxt = starts[i + 1] if i + 1 < len(starts) else AREA_END
                fixed = 4 if kind == "instrument" else 0
                self.items[a] = (kind, max(length, min(nxt - a, fixed + ENVELOPE_MAX)), name)

    def coverage(self):
        covered = set()
        for a, (_, length, _) in self.items.items():
            covered.update(range(a, a + length))
        return covered


def load(prg):
    s = Sound(prg)
    s.walk()
    s.finish()
    s.unused()
    return s


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--rom", type=Path, default=check_rom.DEFAULT_ROM)
    ap.add_argument("--cdl", type=Path)
    ap.add_argument("--out", type=Path, default=REPO / "build" / "sound")
    args = ap.parse_args()

    data = args.rom.read_bytes()
    if check_rom.check(data):
        sys.exit(f"sound: {args.rom} is not Deja Vu (USA); run tools/check_rom.py")
    prg = data[16:16 + NUM_BANKS * BANK_SIZE]
    s = load(prg)

    args.out.mkdir(parents=True, exist_ok=True)
    items = [{"start": a, "length": n, "kind": k, "name": nm} for a, (k, n, nm) in sorted(s.items.items())]
    (args.out / "sound.json").write_text(json.dumps(items, indent=1))
    (args.out / "sound.txt").write_text("".join(f"{i['start']:04X} {i['length']:4} {i['kind']:<10} {i['name']}\n"
                                                for i in items))
    covered = s.coverage()
    kinds = {}
    for i in items:
        kinds[i["kind"]] = kinds.get(i["kind"], 0) + 1
    print(f"sound: {', '.join(f'{v} {k}s' for k, v in kinds.items())}; {len(covered)} of "
          f"{AREA_END - TABLE} bytes in 5:{TABLE:04X}-{AREA_END - 1:04X} mapped, {len(s.problems)} problems")
    for p in s.problems:
        print(f"sound: {p}")
    if args.cdl:
        cdl = args.cdl.read_bytes()
        unexplained = [a for a in range(TABLE, AREA_END) if cdl[rom_offset(a, BANK)] & 2 and a not in covered]
        print(f"sound: {len(unexplained)} bytes the game read in the sound area are outside the map"
              + (f", first at ${unexplained[0]:04X}" if unexplained else ""))
    gaps, start = [], None
    for a in range(TABLE, AREA_END + 1):
        if a < AREA_END and a not in covered:
            start = a if start is None else start
        elif start is not None:
            gaps.append((start, a))
            start = None
    print(f"sound: {len(gaps)} unmapped runs ({sum(e - b for b, e in gaps)} bytes)"
          + "".join(f"\nsound:   {b:04X}-{e - 1:04X} ({e - b})" for b, e in gaps[:10]))


if __name__ == "__main__":
    main()
