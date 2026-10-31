#!/usr/bin/env python3
"""Disassemble Deja Vu's bytecode scripts (bank 3).

usage: scripts.py [--rom ROM] [--trace LOG ...] [--out DIR]
Writes DIR/scripts.txt (every script as a listing) and DIR/scripts.json (where
the tables and scripts sit, for tools/disasm.py). With --trace (a .scripts log
from `run_mesen.py --trace-scripts`), checks that every opcode the game ran is
one the walker found, with the same opcode byte.

Format (interpreter at 7:CBB1; see docs/scripts.md):
- Bank 3 starts with 6 pointers (index $AA & $7F). Each points to a table
  indexed by $AB, whose entries point to small tables indexed by $BC; those
  hold the script addresses.
- A script is a list of opcodes, each followed by its operand bytes. $FF ends
  it. Opcode sizes come from the handlers: which of the operand fetchers
  (1-4 bytes) each one calls. The sizes agree with every opcode a full TAS
  playthrough executes.
"""
import argparse
import json
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "tools"))
import check_rom  # noqa: E402
from nes6502 import BANK_SIZE, NUM_BANKS, rom_offset, word  # noqa: E402

BANK = 3
TOP_TABLE = 0x8000
TOP_ENTRIES = 6
END = 0xFF

# Size of each opcode including its operands, $00-$68 (handlers listed at 7:CC1A).
SIZES = [
    2, 2, 2, 3, 3, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1,  # 00-0F
    1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 3, 1, 2, 2,  # 10-1F
    1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 2, 2, 2, 2, 2,  # 20-2F
    2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 2,  # 30-3F
    1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1,  # 40-4F
    1, 1, 2, 2, 2, 2, 2, 1, 1, 1, 2, 1, 3, 2, 2, 2,  # 50-5F
    1, 1, 2, 1, 1, 3, 1, 1, 2,                       # 60-68
]

# Opcodes whose effect on the script's flow is known; the rest just continue.
NAMES = {0x00: "message_5", 0x01: "message_4", 0x02: "message_2_or_5",
         0x03: "goto_if_alt", 0x04: "goto_if", 0x05: "goto_if_not", 0x1C: "goto",
         0x28: "yield", 0x29: "switch", 0x48: "yield_b5_80",
         0x65: "push_goto", 0x66: "pop_goto"}
# Bit-flag opcodes: test (sets the condition flag), set or clear bit n of a flag array.
# "[ab]" ones use the current object ($AB) as n; the others take n as an operand.
for _base, _ops in ((0x0450, (0x07, 0x08, 0x09, 0x52)), (0x0480, (0x13, 0x14, 0x15, 0x2E, 0x2F, 0x30)),
                    (0x0490, (None, None, None, 0x16, 0x17, 0x18)), (0x045C, (0x20, 0x21, 0x22, None, 0x53)),
                    (0x047A, (0x23, 0x24, 0x25, None, 0x54))):
    for _op, _what in zip(_ops, ("test", "set", "clear") * 2):
        if _op is not None:
            NAMES[_op] = f"{_what}_{_base:04X}" + ("[ab]" if _op in _ops[:3] else "")
GOTO_IF = {0x03, 0x04, 0x05}  # 2-byte target; also falls through
GOTO = {0x1C, 0x65}           # 2-byte target only (0x65 also pushes it for a later 0x66)
STOP = {0x66}                 # no fall-through
# 0x28 and 0x48 stop the current run, but the game later resumes at the next opcode
# (7:CB5C jumps back into script_loop with $B6 unchanged), so for the walk they fall through.
SWITCH = 0x29                 # cases of 4 bytes: a, b, target; a = $FE is the default


class Scripts:
    def __init__(self, prg):
        self.prg = prg
        self.tables = []      # (addr, entries, kind)
        self.starts = {}      # script address -> list of "aa.ab.bc" names
        self.ops = {}         # opcode address -> (opcode, length)
        self.problems = []

    def rd(self, addr):
        return self.prg[rom_offset(addr, BANK)]

    def rw(self, addr):
        return word(self.prg, rom_offset(addr, BANK))

    def walk_tables(self):
        top = [self.rw(TOP_TABLE + 2 * i) for i in range(TOP_ENTRIES)]
        self.tables.append((TOP_TABLE, top, "top"))
        for aa, mid in enumerate(top):
            # The $AB table ends where the first $BC table starts; $BC tables are evenly spaced.
            entries = [self.rw(mid)]
            while mid + 2 * len(entries) < min(entries):
                entries.append(self.rw(mid + 2 * len(entries)))
            self.tables.append((mid, entries, f"aa{aa}"))
            width = (entries[1] - entries[0]) // 2 if len(entries) > 1 else 4
            for ab, low in enumerate(entries):
                scripts = [self.rw(low + 2 * bc) for bc in range(width)]
                self.tables.append((low, scripts, f"aa{aa}_ab{ab:02X}"))
                for bc, s in enumerate(scripts):
                    self.starts.setdefault(s, []).append(f"{aa}.{ab:02X}.{bc}")

    def find_unreferenced(self):
        """Walk gaps between known script data as scripts; keep a walk only if it hits no
        problem and every opcode it finds lies in that gap. Returns the starts kept."""
        known = set()
        for t, e, _ in self.tables:
            known.update(range(t, t + 2 * len(e)))
        for a, (_, n) in self.ops.items():
            known.update(range(a, a + n))
        last = max(known) + 1
        kept, a = [], 0x8000
        while a < last:
            if a in known or self.rd(a) == END:
                a += 1
                continue
            end = a
            while end < last and end not in known:
                end += 1
            ops, problems = dict(self.ops), list(self.problems)
            self.walk_scripts([a])
            new = [x for x in self.ops if x not in ops]
            if len(self.problems) == len(problems) and all(a <= x < end for x in new):
                kept.append(a)
                self.starts.setdefault(a, []).append(f"unreferenced_{a:04X}")
            else:
                self.ops, self.problems = ops, problems
            a = end
        return kept

    def walk_scripts(self, starts=None):
        work = sorted(self.starts) if starts is None else list(starts)
        while work:
            a = work.pop()
            while a not in self.ops:
                if not 0x8000 <= a < 0xC000:
                    self.problems.append(f"flow leaves bank 3 at ${a:04X}")
                    break
                op = self.rd(a)
                if op == END:
                    self.ops[a] = (op, 1)
                    break
                if op >= len(SIZES):
                    self.problems.append(f"${a:04X}: opcode ${op:02X} has no handler")
                    break
                if op == SWITCH:
                    n, targets = 1, []
                    while True:
                        case = a + n
                        targets.append(self.rw(case + 2))
                        n += 4
                        if self.rd(case) == 0xFE:
                            break
                    self.ops[a] = (op, n)
                    work += targets
                    break
                size = SIZES[op]
                self.ops[a] = (op, size)
                if op in GOTO_IF or op in GOTO:
                    work.append(self.rw(a + 1))
                if op in GOTO or op in STOP:
                    break
                a += size

    def check_trace(self, path):
        """Opcodes the game ran that the walk did not find (or found with another byte)."""
        bad = set()
        for line in Path(path).read_text().splitlines():
            p = line.split()
            if p and p[0] == "O":
                bank, addr, op = int(p[1]), int(p[2], 16), int(p[3], 16)
                if bank != BANK or addr not in self.ops or self.ops[addr][0] != op:
                    bad.add((bank, addr, op))
        return sorted(bad)

    def listing(self):
        out = []
        for a, (op, n) in sorted(self.ops.items()):
            if a in self.starts:
                out.append(f"\nscript {', '.join(self.starts[a])}:")
            args = " ".join(f"{self.rd(a + i):02X}" for i in range(1, n))
            name = "end" if op == END else NAMES.get(op, f"op_{op:02X}")
            if op in GOTO_IF or op in GOTO:
                args += f"  -> ${self.rw(a + 1):04X}"
            out.append(f"  {a:04X}: {name:<16} {args}".rstrip())
        return "\n".join(out) + "\n"

    def layout(self):
        """Contiguous byte ranges of script code, for the disassembler."""
        runs = []
        for a, (_, n) in sorted(self.ops.items()):
            if runs and runs[-1][1] == a:
                runs[-1][1] = a + n
            else:
                runs.append([a, a + n])
        return {"bank": BANK,
                "tables": [{"start": t, "entries": len(e), "name": k} for t, e, k in self.tables],
                "scripts": [{"start": s, "end": e} for s, e in runs],
                "entry_points": {f"{a:04X}": names for a, names in sorted(self.starts.items())}}


def load(prg):
    s = Scripts(prg)
    s.walk_tables()
    s.walk_scripts()
    s.find_unreferenced()
    return s


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--rom", type=Path, default=check_rom.DEFAULT_ROM)
    ap.add_argument("--trace", type=Path, nargs="*", default=[])
    ap.add_argument("--out", type=Path, default=REPO / "build" / "scripts")
    args = ap.parse_args()

    data = args.rom.read_bytes()
    if check_rom.check(data):
        sys.exit(f"scripts: {args.rom} is not Deja Vu (USA); run tools/check_rom.py")
    s = load(data[16:16 + NUM_BANKS * BANK_SIZE])

    args.out.mkdir(parents=True, exist_ok=True)
    (args.out / "scripts.txt").write_text(s.listing())
    lay = s.layout()
    (args.out / "scripts.json").write_text(json.dumps(lay, indent=1))
    script_bytes = sum(r["end"] - r["start"] for r in lay["scripts"])
    table_bytes = sum(2 * t["entries"] for t in lay["tables"])
    print(f"scripts: {len(s.starts)} entry points, {len(s.ops)} opcodes in {script_bytes} bytes, "
          f"{len(s.tables)} tables ({table_bytes} bytes), {len(s.problems)} problems")
    for p in s.problems[:20]:
        print(f"scripts: {p}")
    for t in args.trace:
        bad = s.check_trace(t)
        print(f"scripts: {t.name}: {'every executed opcode matches' if not bad else f'{len(bad)} mismatches'}")
        for b in bad[:10]:
            print(f"scripts:   bank {b[0]} ${b[1]:04X} opcode ${b[2]:02X}")


if __name__ == "__main__":
    main()
