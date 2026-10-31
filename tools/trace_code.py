#!/usr/bin/env python3
"""Map code and data in Deja Vu's PRG ROM by following control flow.

Starts from the reset/NMI/IRQ vectors and, when given a Mesen code/data log,
also from every run of executed bytes and every jump or call target Mesen saw.
It understands the game's calling conventions (docs/conventions.md).

usage: trace_code.py [--rom ROM] [--cdl FILE] [--out DIR]
Writes DIR/static.map (one FLAG_* byte per PRG byte) and DIR/trace.json.
"""
import argparse
import copy
import json
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "tools"))
import check_rom  # noqa: E402
from nes6502 import BANK_SIZE, FIXED_BANK, NUM_BANKS, OPCODES, cpu_addr, rom_offset, word  # noqa: E402

FLAG_CODE = 0x01      # part of an instruction
FLAG_OPSTART = 0x02   # first byte of an instruction
FLAG_ADDRTAB = 0x04   # byte of a 16-bit address table entry
FLAG_INLINE = 0x08    # data placed inline after a JSR
FLAG_CDL_CODE = 0x10  # Mesen saw it executed
FLAG_CDL_DATA = 0x20  # Mesen saw it read as data
FLAG_KNOWN = 0x40     # data whose format is known (text, name tables); set by tools/disasm.py
FLAG_TABLE = 0x80     # data in a table that code reads (format not decoded); set by tools/disasm.py

JUMP_TABLE_CALLS = {0xEBE6, 0xFADA}  # index in A, address table follows the JSR, no return
CHR_INLINE_CALL = 0xE916             # 8 CHR bank numbers follow the JSR, code continues after
RLE_SCREEN_CALLS = {0xE9D5, 0xE992}  # RLE nametable follows; returns to the caller's caller
FAR_CALLS = {0xF3B5: 0x8000, 0xF3C5: 0x8020, 0xF3D5: 0x8040, 0xF3E5: 0x8400}  # table base per trampoline
CHR_INLINE_BYTES = 8
# Far-call tables that hold code addresses: (bank, table, entries). Bank 6's $8040
# table has 6 entries (code follows it); bank 2 only uses the $8400 table, since its
# $8000 table lists scene pictures.
FAR_TABLES = [(6, 0x8000, 16), (6, 0x8020, 16), (6, 0x8040, 6), (2, 0x8400, 6)]
MAX_TABLE_ENTRIES = 256


BRANCH_CONDITION = {"BEQ": ("z", True), "BNE": ("z", False), "BMI": ("n", True), "BPL": ("n", False),
                    "BCS": ("c", True), "BCC": ("c", False), "BVS": ("v", True), "BVC": ("v", False)}
NO_FLAG_CHANGE = {"STA", "STX", "STY", "PHA", "PHP", "TXS", "NOP"}


def flags_after(name, mode, operand, known):
    """Flags still known to be constant after one instruction."""
    if mode == "imm" and name in ("LDA", "LDX", "LDY"):
        return {**{k: v for k, v in known.items() if k in "cv"}, "z": operand == 0, "n": operand >= 0x80}
    if name in ("CLC", "SEC"):
        return {**known, "c": name == "SEC"}
    if name == "CLV":
        return {**known, "v": False}
    if name in NO_FLAG_CHANGE:
        return known
    return {}


def loc(offset):
    bank, addr = cpu_addr(offset)
    return f"{bank}:{addr:04X}"


class Tracer:
    def __init__(self, prg, cdl=None):
        self.prg = prg
        self.flags = bytearray(len(prg))
        if cdl:
            for i, b in enumerate(cdl):
                self.flags[i] |= (FLAG_CDL_CODE if b & 0x01 else 0) | (FLAG_CDL_DATA if b & 0x02 else 0)
        self.labels = {}
        self.work = []
        self.pending_tables = []
        self.far_calls = {}
        self.jump_tables = []
        self.rle_blocks = []
        self.chr_inline = []
        self.indirect_jumps = []
        self.problems = []
        self.overlaps = set()

    def problem(self, offset, what):
        self.problems.append({"at": loc(offset), "problem": what})

    def seed(self, offset, name=None):
        if offset is None:
            return
        if name:
            self.labels.setdefault(offset, name)
        self.work.append(offset)

    def run(self):
        while True:
            while self.work:
                self.flow(self.work.pop())
            if not self.pending_tables:
                break
            # Tables are sized last, when as much surrounding code as possible is known.
            self.pending_tables.sort()
            self.jump_table(*self.pending_tables.pop(0))

    def flow(self, o):
        prev = None
        known = {}  # flags made constant by the instructions just before, e.g. LDA #0 -> Z=1
        while True:
            if self.flags[o] & FLAG_OPSTART:
                return
            if self.flags[o] & FLAG_CODE:
                self.overlaps.add(o)
            info = OPCODES.get(self.prg[o])
            if info is None:
                self.problem(o, f"invalid opcode ${self.prg[o]:02X}")
                return
            name, mode, n = info
            if o % BANK_SIZE + n > BANK_SIZE:
                self.problem(o, "instruction crosses the end of the bank")
                return
            if any(self.flags[o + i] & (FLAG_ADDRTAB | FLAG_INLINE) for i in range(n)):
                self.problem(o, "code runs into data")
                return
            self.flags[o] |= FLAG_OPSTART
            for i in range(n):
                self.flags[o + i] |= FLAG_CODE

            bank, addr = cpu_addr(o)
            ctx = None if bank == FIXED_BANK else bank
            nxt = o + n
            if mode == "rel":
                d = self.prg[o + 1]
                flag, want = BRANCH_CONDITION[name]
                taken = None if flag not in known else known[flag] == want
                if taken is not False:
                    self.seed(rom_offset((addr + 2 + d - (256 if d >= 128 else 0)) & 0xFFFF, ctx))
                if taken:  # e.g. LDA #0 / BEQ: an unconditional branch, nothing falls through
                    return
            elif name == "JSR":
                target = word(self.prg, o + 1)
                self.seed(rom_offset(target, ctx))
                if target in JUMP_TABLE_CALLS:
                    self.pending_tables.append((nxt, ctx, o))
                    return
                if target in RLE_SCREEN_CALLS:
                    self.rle_block(nxt)
                    return
                if target == CHR_INLINE_CALL:
                    self.mark(nxt, CHR_INLINE_BYTES, FLAG_INLINE)
                    self.chr_inline.append(loc(nxt))
                    nxt += CHR_INLINE_BYTES
                elif target in FAR_CALLS:
                    self.far_call(prev, FAR_CALLS[target], o)
            elif name == "JMP" and mode == "abs":
                self.seed(rom_offset(word(self.prg, o + 1), ctx))
                return
            elif name == "JMP":
                self.indirect_jumps.append(loc(o))
                return
            elif name in ("RTS", "RTI", "BRK"):
                return
            known = flags_after(name, mode, self.prg[o + 1] if n > 1 else None, known)
            prev = o
            o = nxt
            if o % BANK_SIZE == 0:
                self.problem(o - 1, "code falls off the end of the bank")
                return

    def mark(self, start, length, flag):
        for i in range(start, start + length):
            self.flags[i] |= flag

    def far_call(self, prev, table_base, site):
        if prev is None or self.prg[prev] != 0xA9:
            self.problem(site, "far call whose ID is not an LDA #immediate")
            return
        fid = self.prg[prev + 1]
        bank, index = fid >> 4, fid & 0x0F
        entry = rom_offset(table_base + 2 * index, bank)
        if entry is None:
            self.problem(site, f"far call ${fid:02X} names bank {bank}")
            return
        self.mark(entry, 2, FLAG_ADDRTAB)
        target = rom_offset(word(self.prg, entry), bank)
        key = f"{table_base:04X}:{fid:02X}"
        self.far_calls[key] = loc(target) if target is not None else None
        self.seed(target, name=f"far_{table_base:04X}_{fid:02X}")

    def jump_table(self, start, ctx, site):
        entries, p = [], start
        while len(entries) < MAX_TABLE_ENTRIES and p % BANK_SIZE + 2 <= BANK_SIZE:
            if any(self.flags[p + i] & (FLAG_CODE | FLAG_CDL_CODE | FLAG_INLINE) for i in (0, 1)):
                break
            if p in entries:  # the table ends where the first handler starts
                break
            target = rom_offset(word(self.prg, p), ctx)
            if target is None or self.prg[target] not in OPCODES:
                break
            entries.append(target)
            p += 2
        self.mark(start, p - start, FLAG_ADDRTAB)
        self.jump_tables.append({"call": loc(site), "table": loc(start),
                                 "entries": [loc(e) for e in entries]})
        for e in entries:
            self.seed(e)

    def rle_block(self, start):
        # Format (decoder at $EA18): byte = literal; FF c n = n copies of c;
        # FF FF n = n copies of FF; FF FF 00 = end.
        p, end = start, start - start % BANK_SIZE + BANK_SIZE
        while p < end:
            b = self.prg[p]
            p += 1
            if b != 0xFF:
                continue
            c, p = self.prg[p], p + 1
            n, p = self.prg[p], p + 1
            if c == 0xFF and n == 0:
                break
        self.mark(start, p - start, FLAG_INLINE)
        self.rle_blocks.append({"start": loc(start), "length": p - start})

    def find_unreferenced_code(self, banks):
        """Seed runs of unclassified bytes that can only be code nothing calls.

        A run qualifies when the tracer can follow code from its first byte (jump
        tables and all) without any problem, and the new code reaches an RTS, RTI or
        JMP and calls or jumps to at least one routine that was already known (or
        falls or branches into one). A run that calls nothing also qualifies if it
        decodes exactly, from first byte to last, into self-contained code ending
        with RTS, RTI or JMP. Each candidate is traced on a copy of the state and
        kept only if it passes. Returns the offsets kept.
        """
        found = []
        for bank in banks:
            o = bank * BANK_SIZE
            end_bank = o + BANK_SIZE
            while o < end_bank:
                if self.flags[o] & ~(FLAG_CDL_DATA | FLAG_CDL_CODE):
                    o += 1
                    continue
                run_end = o
                while run_end < end_bank and not self.flags[run_end] & ~(FLAG_CDL_DATA | FLAG_CDL_CODE):
                    run_end += 1
                if self._try_code(o) or (self._fills_run(o, run_end) and self._try_code(o, need_known=False)):
                    found.append(o)
                    o += 1  # the new code may not fill the run; look again after it
                else:
                    o = run_end
        return found

    def _try_code(self, start, name=None, need_known=True):
        """Trace from `start` on a copy of the state; keep it if it passes (see find_unreferenced_code).

        Code that falls or branches into known code (an extra entry point to a known
        routine) passes too. With need_known=False, clean code that ends is enough.
        Either way it needs at least two instructions, none of them on bytes the
        recordings read as data.
        """
        if self.prg[start] not in OPCODES or self.prg[start] == 0x00:
            return False
        saved = copy.deepcopy({k: v for k, v in self.__dict__.items() if k != "prg"})
        known_before = bytes(self.flags)
        problems_before = len(self.problems)
        self.seed(start, name=name or f"unreferenced_{cpu_addr(start)[1]:04X}")
        self.run()
        new = [o for o in range(len(self.prg)) if self.flags[o] & FLAG_OPSTART and not known_before[o] & FLAG_OPSTART]
        reaches_known = ends = False
        for o in new:
            op, mode, n = OPCODES[self.prg[o]]
            bank, addr = cpu_addr(o)
            ctx = None if bank == FIXED_BANK else bank
            targets = []
            if op in ("JSR", "JMP") and mode == "abs":
                targets.append(rom_offset(word(self.prg, o + 1), ctx))
            if mode == "rel":
                d = self.prg[o + 1]
                targets.append(rom_offset((addr + 2 + d - (256 if d >= 128 else 0)) & 0xFFFF, ctx))
            if op in ("RTS", "RTI", "JMP"):
                ends = True
            elif o + n < len(self.prg):
                targets.append(o + n)
            reaches_known |= any(t is not None and known_before[t] & FLAG_OPSTART for t in targets)
        read_as_data = any(self.flags[o + i] & FLAG_CDL_DATA for o in new for i in range(OPCODES[self.prg[o]][2]))
        if (len(self.problems) == problems_before and len(new) >= 2 and not read_as_data
                and (ends or reaches_known) and (reaches_known or not need_known)):
            return True
        self.__dict__.update(saved)
        return False

    def _fills_run(self, start, end, min_length=12):
        """True if start..end decodes exactly into instructions whose branches stay
        inside, ending with RTS/RTI/JMP on the last byte: self-contained code that
        neither calls nor reaches anything known."""
        p, last = start, None
        while p < end:
            info = OPCODES.get(self.prg[p])
            if info is None or info[0] == "BRK" or p + info[2] > end:
                return False
            name, mode, n = info
            if mode == "rel":
                bank, addr = cpu_addr(p)
                d = self.prg[p + 1]
                t = rom_offset((addr + 2 + d - (256 if d >= 128 else 0)) & 0xFFFF,
                               None if bank == FIXED_BANK else bank)
                if t is None or not start <= t < end:
                    return False
            last, p = name, p + n
        return end - start >= min_length and last in ("RTS", "RTI", "JMP")

    def try_entry(self, offset, name):
        """Trace a routine named by hand (asm/labels.txt) that nothing reaches; keep it if it traces cleanly."""
        return not self.flags[offset] & FLAG_CODE and self._try_code(offset, name, need_known=False)

    def inline_argument_candidates(self):
        # PLA / STA zp / PLA / STA zp+1 at a routine start: reads data placed after its JSR.
        found = []
        for o in range(len(self.prg) - 6):
            p = self.prg
            if (self.flags[o] & FLAG_OPSTART and p[o] == 0x68 and p[o + 1] == 0x85
                    and p[o + 3] == 0x68 and p[o + 4] == 0x85 and p[o + 5] == (p[o + 2] + 1) & 0xFF):
                found.append(loc(o))
        return found


def byte_class(x):
    """What one byte counts as in the statistics, strongest evidence first."""
    for flag, name in ((FLAG_CODE, "code"), (FLAG_ADDRTAB, "address_tables"), (FLAG_INLINE, "inline_data"),
                       (FLAG_KNOWN, "known_data"), (FLAG_TABLE, "code_tables"), (FLAG_CDL_DATA, "read_as_data")):
        if x & flag:
            return name
    return "unclassified"


def stats(flags):
    out = {}
    for b in range(NUM_BANKS):
        f = flags[b * BANK_SIZE:(b + 1) * BANK_SIZE]
        s = dict.fromkeys(("code", "address_tables", "inline_data", "known_data", "code_tables", "read_as_data",
                           "unclassified"), 0)
        for x in f:
            s[byte_class(x)] += 1
        s["executed_but_not_traced"] = sum(1 for x in f if x & FLAG_CDL_CODE and not x & FLAG_CODE)
        out[b] = s
    return out


def trace(prg, cdl=None):
    """Trace from the vectors (and the CDL, if given). Returns (tracer, report dict)."""
    t = Tracer(prg, cdl)
    for vec, name in ((0xFFFA, "nmi"), (0xFFFC, "reset"), (0xFFFE, "irq")):
        t.seed(rom_offset(word(prg, rom_offset(vec, None)), None), name)
    if cdl:
        for o, b in enumerate(cdl):
            if b & 0x01 and (b & 0x0C or o % BANK_SIZE == 0 or not cdl[o - 1] & 0x01):
                t.seed(o)
    # Every entry of the far-call tables is an entry point, including ones no
    # LDA #id / JSR far_call_* site uses (the game may never call them).
    for bank, base, count in FAR_TABLES:
        for i in range(count):
            entry = rom_offset(base + 2 * i, bank)
            t.mark(entry, 2, FLAG_ADDRTAB)
            t.seed(rom_offset(word(prg, entry), bank), name=f"far_{base:04X}_{bank << 4 | i:02X}")
    t.run()
    report = {
        "stats_by_bank": stats(t.flags),
        "labels": {loc(o): n for o, n in sorted(t.labels.items())},
        "far_calls": dict(sorted(t.far_calls.items())),
        "jump_tables": t.jump_tables,
        "rle_screens": t.rle_blocks,
        "chr_bank_inline_data": t.chr_inline,
        "indirect_jumps": sorted(set(t.indirect_jumps)),
        "inline_argument_routines": t.inline_argument_candidates(),
        "overlapping_instructions": sorted(loc(o) for o in t.overlaps),
        "problems": t.problems,
    }
    return t, report


def load_prg(rom_path, tool):
    data = rom_path.read_bytes()
    if check_rom.check(data):
        sys.exit(f"{tool}: {rom_path} is not Deja Vu (USA); run tools/check_rom.py")
    return data


def summary(t, report):
    total = {k: sum(s[k] for s in report["stats_by_bank"].values()) for k in report["stats_by_bank"][0]}
    return (", ".join(f"{v} {k.replace('_', ' ')}" for k, v in total.items()) + "\n"
            f"{len(t.far_calls)} far calls, {len(t.jump_tables)} jump tables, "
            f"{len(t.rle_blocks)} RLE screens, {len(report['indirect_jumps'])} indirect jumps, "
            f"{len(t.problems)} problems")


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--rom", type=Path, default=check_rom.DEFAULT_ROM)
    ap.add_argument("--cdl", type=Path, help="Mesen CDL for PRG ROM (from tools/run_mesen.py)")
    ap.add_argument("--out", type=Path, default=REPO / "build" / "trace")
    args = ap.parse_args()

    data = load_prg(args.rom, "trace_code")
    t, report = trace(data[16:16 + NUM_BANKS * BANK_SIZE], args.cdl.read_bytes() if args.cdl else None)
    args.out.mkdir(parents=True, exist_ok=True)
    (args.out / "static.map").write_bytes(t.flags)
    (args.out / "trace.json").write_text(json.dumps(report, indent=1))
    print(summary(t, report))


if __name__ == "__main__":
    main()
