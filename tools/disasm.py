#!/usr/bin/env python3
"""Disassemble Deja Vu's PRG ROM with da65 and check that it rebuilds byte for byte.

usage: disasm.py [--rom ROM] [--cdl FILE] [--out DIR]

1. Traces code with tools/trace_code.py, also using a Mesen CDL when given.
2. Writes a da65 info file per 16 KB bank: traced code as CODE, address tables
   as ADDRTABLE, everything else as BYTETABLE, with names from asm/labels.txt.
3. Runs da65, assembles and links the output with ca65/ld65, and compares each
   bank with the ROM. If all match, also writes DIR/rebuilt.nes.

Output goes to DIR (default build/asm), which git ignores.
"""
import argparse
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "tools"))
import check_rom  # noqa: E402
import layouts  # noqa: E402
import rooms  # noqa: E402
import scenes  # noqa: E402
import scripts  # noqa: E402
import sound  # noqa: E402
import text  # noqa: E402
import trace_code  # noqa: E402
from nes6502 import BANK_SIZE, FIXED_BANK, NUM_BANKS, OPCODES, cpu_addr, rom_offset, word  # noqa: E402

LABELS = REPO / "asm" / "labels.txt"
LINK_CFG = """MEMORY {{ ROM: start = ${base:04X}, size = $4000, fill = yes, file = %O; }}
SEGMENTS {{ CODE: load = ROM, type = ro; }}
"""


def cc65_tool(name):
    for d in (os.environ.get("CC65_HOME"), Path(os.environ.get("LOCALAPPDATA", "")) / "Programs" / "cc65"):
        if d and (Path(d) / "bin" / f"{name}.exe").is_file():
            return str(Path(d) / "bin" / f"{name}.exe")
    found = shutil.which(name)
    if not found:
        sys.exit(f"disasm: {name} not found; install cc65 or set CC65_HOME")
    return found


def read_labels():
    """{(bank, addr): (name, comment)} from asm/labels.txt."""
    labels = {}
    for n, line in enumerate(LABELS.read_text().splitlines(), 1):
        line, _, comment = line.partition(";")
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        where, name = line.split()
        bank, addr = where.split(":")
        key = (int(bank), int(addr, 16))
        if key in labels:
            sys.exit(f"disasm: {LABELS.name}:{n}: {where} named twice")
        labels[key] = (name, comment.strip())
    return labels


def ranges(flags, bank, skip=()):
    """Runs of (type, start offset, end offset) covering one bank. Offsets in `skip`
    (a set of PRG offsets) become SKIP ranges, which da65 leaves out."""
    def kind(o):
        f = flags[o]
        if o in skip:
            return "SKIP"
        if f & trace_code.FLAG_CODE:
            return "CODE"
        if f & trace_code.FLAG_ADDRTAB:
            return "ADDRTABLE"
        return "BYTETABLE"
    start = bank * BANK_SIZE
    out, run_start, run_kind = [], start, kind(start)
    for o in range(start + 1, start + BANK_SIZE):
        k = kind(o)
        if k != run_kind:
            out.append((run_kind, run_start, o - 1))
            run_start, run_kind = o, k
    out.append((run_kind, run_start, start + BANK_SIZE - 1))
    return out


def info_file(bank, flags, names, bin_path, asm_path, skip=(), comments=0):
    base = cpu_addr(bank * BANK_SIZE)[1]
    lines = ["GLOBAL {",
             f'  INPUTNAME "{bin_path.as_posix()}";',
             f'  OUTPUTNAME "{asm_path.as_posix()}";',
             f"  STARTADDR ${base:04X};",
             '  CPU "6502";',
             "  PAGELENGTH 0;",
             f"  COMMENTS {comments};",
             "};"]
    for kind, s, e in ranges(flags, bank, skip):
        lines.append(f"RANGE {{ START ${cpu_addr(s)[1]:04X}; END ${cpu_addr(e)[1]:04X}; TYPE {kind}; }};")
    for (b, addr), (name, comment) in sorted(names.items()):
        # A bank sees its own labels plus the fixed bank's.
        if b == bank or (b == FIXED_BANK and bank != FIXED_BANK):
            text = f'LABEL {{ NAME "{name}"; ADDR ${addr:04X};'
            if comment and b == bank:
                text += ' COMMENT "' + comment.replace('"', "'") + '";'
            lines.append(text + " };")
    return "\n".join(lines) + "\n"


def mark_text(t, prg, names):
    """Classify the text found by tools/text.py: pointer tables, messages, name and code tables."""
    def known(bank, addr, length, label, comment=""):
        o = rom_offset(addr, bank)
        if any(t.flags[i] & trace_code.FLAG_CODE for i in range(o, o + length)):
            sys.exit(f"disasm: text at {bank}:{addr:04X} overlaps traced code")
        t.mark(o, length, trace_code.FLAG_KNOWN)
        if label:
            names.setdefault((bank, addr), (label, comment))

    tables, _ = text.decode_all(prg)
    for tb in tables:
        b, base = tb["bank"], tb["table"]
        t.mark(rom_offset(base, b), 2 * len(tb["entries"]), trace_code.FLAG_ADDRTAB)
        names[(b, base)] = (f"messages_{b}", f"{len(tb['entries'])} message pointers (tools/text.py)")
        for i, p in enumerate(tb["entries"]):
            known(b, p, tb["messages"][p][1], f"msg{b}_{i:02X}")
    rom = text.Rom(prg)
    for kind, base in text.NAME_TABLES.items():
        known(2, base, 8 * text.name_table_size(rom, kind), f"{kind}_names", "8 tiles per entry")
    known(FIXED_BANK, text.CODE_TABLE, 64, "text_code_table", "bit-stream index -> character code")
    known(FIXED_BANK, text.TILE_TABLE, 64, "text_tile_table", "character code -> font tile")


def mark_scripts(t, prg, names):
    """Classify bank 3's script tables and scripts, found by tools/scripts.py."""
    s = scripts.load(prg)
    for addr, entries, kind in s.tables:
        t.mark(rom_offset(addr, scripts.BANK), 2 * len(entries), trace_code.FLAG_ADDRTAB)
        names.setdefault((scripts.BANK, addr), ("script_tables" if kind == "top" else f"scripts_{kind}", ""))
    for r in s.layout()["scripts"]:
        o = rom_offset(r["start"], scripts.BANK)
        t.mark(o, r["end"] - r["start"], trace_code.FLAG_KNOWN)
    for addr, which in sorted(s.starts.items()):
        names.setdefault((scripts.BANK, addr), ("script_" + which[0].replace(".", "_"),
                                                f"also {', '.join(which[1:4])}..." if len(which) > 1 else ""))


def mark_scenes(t, prg, names):
    """Scene pictures in banks 0-2 (tools/scenes.py) and the loader's tables in bank 7."""
    for bank, first, _ in scenes.BANKS:
        entries = scenes.pointer_table(prg, bank)
        t.mark(rom_offset(0x8000, bank), 2 * len(entries), trace_code.FLAG_ADDRTAB)
        names.setdefault((bank, 0x8000), (f"scene_pictures_{bank}", f"scenes ${first:02X}-${first + len(entries) - 1:02X}"))
        for i, addr in enumerate(entries):
            t.mark(rom_offset(addr, bank), scenes.RECORD, trace_code.FLAG_KNOWN)
            names.setdefault((bank, addr), (f"scene_{first + i:02X}", "14x14 tiles, then 16 attribute bytes"))
    for addr, length, label, comment in ((0xC1A6, 16, "scene_attr_positions", "where each attribute byte goes, from $0300"),
                                         (0xC21F, 80, "room_settings", "per room ($0514): low nibble -> 7:F409, high -> $0530")):
        t.mark(rom_offset(addr, None), length, trace_code.FLAG_KNOWN)
        names.setdefault((FIXED_BANK, addr), (label, comment))


def mark_sound(t, prg, names):
    """Music and sound-effect data in bank 5 (tools/sound.py) and the driver's tables in bank 7."""
    s = sound.load(prg)
    for addr, (kind, length, name) in s.items.items():
        o = rom_offset(addr, sound.BANK)
        t.mark(o, length, trace_code.FLAG_ADDRTAB if kind == "table" else trace_code.FLAG_KNOWN)
        names.setdefault((sound.BANK, addr), (name, kind if kind != "table" else "address of each sound's header"))
    for addr, length, label, comment in ((0xFAF5, 64, "note_lengths", "length commands $80-$9F"),
                                         (0xFB1B, 2 * 105, "note_periods", "timer value per note")):
        t.mark(rom_offset(addr, None), length, trace_code.FLAG_KNOWN)
        names.setdefault((FIXED_BANK, addr), (label, comment))


def mark_layouts(t, prg, names):
    """Screen layouts in bank 6 (tools/layouts.py)."""
    entries, found = layouts.load(prg)
    t.mark(rom_offset(layouts.TABLE, layouts.BANK), 2 * len(entries), trace_code.FLAG_ADDRTAB)
    names.setdefault((layouts.BANK, layouts.TABLE), ("screen_layouts", "drawn by 7:FBF0"))
    for i, a in enumerate(entries):
        t.mark(rom_offset(a, layouts.BANK), found[a], trace_code.FLAG_KNOWN)
        names.setdefault((layouts.BANK, a), (f"layout_{i:02X}", ""))


def mark_rooms(t, prg, names):
    """Room hotspots and the coat item lists in bank 2 (tools/rooms.py)."""
    b = rooms.BANK
    entries, lists = rooms.load(prg)
    t.mark(rom_offset(rooms.TABLE, b), 2 * len(entries), trace_code.FLAG_ADDRTAB)
    names.setdefault((b, rooms.TABLE), ("room_hotspots", "per room ($0514): 6-byte records, $FF ends"))
    for room, a in enumerate(entries):
        t.mark(rom_offset(a, b), rooms.RECORD * lists[a] + 1, trace_code.FLAG_KNOWN)
        names.setdefault((b, a), (f"hotspots_{room:02X}", ""))
    tables, item_lists = rooms.load_item_lists(prg)
    for a, e in tables:
        t.mark(rom_offset(a, b), 2 * len(e), trace_code.FLAG_ADDRTAB)
        names.setdefault((b, a), (f"item_lists_{a:04X}", "read by 7:D4FD"))
    for a, n in item_lists.items():
        t.mark(rom_offset(a, b), n, trace_code.FLAG_KNOWN)


def mark_vectors(t, prg, names):
    t.mark(rom_offset(0xFFFA, None), 6, trace_code.FLAG_ADDRTAB)
    names.setdefault((FIXED_BANK, 0xFFFA), ("vectors", "NMI, reset, IRQ"))


def mark_padding(t, prg, names):
    """Runs of 8+ unclassified $FF bytes that no recording read: unused space."""
    unclassified = lambda o: not t.flags[o] & ~trace_code.FLAG_CDL_CODE  # noqa: E731 (CDL data counts as used)
    o = 0
    while o < len(prg):
        end = o
        while end < len(prg) and prg[end] == 0xFF and unclassified(end) and (end == o or end % BANK_SIZE):
            end += 1
        if end - o >= 8:
            t.mark(o, end - o, trace_code.FLAG_KNOWN)
            if end - o >= 32:
                bank, addr = cpu_addr(o)
                names.setdefault((bank, addr), (f"unused_{bank}_{addr:04X}", f"{end - o} bytes of $FF"))
        o = max(end, o + 1)


# Regions checked by hand: nothing reads them and they hold no structure.
UNUSED = [(5, 0xB9D8, 0xC000, "unused_5", "after the last message: $FF with leftover bytes from the build tools"),
          (4, 0xB61D, 0xBECE, "unused_4", "after the last message; only the message printer maps bank 4, "
                                          "so no code can read this")]


def mark_unused(t, prg, names):
    for bank, start, end, label, comment in UNUSED:
        o = rom_offset(start, bank)
        if any(t.flags[i] & (trace_code.FLAG_CODE | trace_code.FLAG_CDL_DATA) for i in range(o, o + end - start)):
            sys.exit(f"disasm: {bank}:{start:04X} is listed as unused but is code or was read")
        t.mark(o, end - start, trace_code.FLAG_KNOWN)
        names.setdefault((bank, start), (label, comment))


def mark_code_tables(t, prg):
    """Unclassified bytes from each address code reads, up to the next classified byte or
    the next such address. "Reads" means absolute addressing, or loading the address into
    a zero-page pointer (LDA #lo / STA zp / LDA #hi / STA zp+1). When bank 7 code points
    into $8000-$BFFF, the bank is the one where recordings read that address and nothing
    else explains the read, if there is exactly one."""
    explained = trace_code.FLAG_CODE | trace_code.FLAG_ADDRTAB | trace_code.FLAG_INLINE | trace_code.FLAG_KNOWN

    def bank_read_at(addr):
        banks = [b for b in range(FIXED_BANK)
                 if t.flags[rom_offset(addr, b)] & trace_code.FLAG_CDL_DATA
                 and not t.flags[rom_offset(addr, b)] & explained]
        return banks[0] if len(banks) == 1 else None

    starts = set()
    for o in range(len(prg)):
        if not t.flags[o] & trace_code.FLAG_OPSTART:
            continue
        name, mode, _ = OPCODES[prg[o]]
        bank, _ = cpu_addr(o)
        addr = None
        if mode in ("abs", "abx", "aby") and name not in ("JSR", "JMP"):
            addr = word(prg, o + 1)
        elif (prg[o:o + 1] == b"\xA9" and prg[o + 2] == 0x85 and prg[o + 4] == 0xA9 and prg[o + 6] == 0x85
              and prg[o + 7] == (prg[o + 3] + 1) & 0xFF):
            addr = prg[o + 1] | prg[o + 5] << 8
        if addr is None:
            continue
        if bank == FIXED_BANK and 0x8000 <= addr < 0xC000:
            target = rom_offset(addr, bank_read_at(addr))
        else:
            target = rom_offset(addr, None if bank == FIXED_BANK else bank)
        if target is not None:
            starts.add(target)
    done = trace_code.FLAG_CODE | trace_code.FLAG_ADDRTAB | trace_code.FLAG_INLINE | trace_code.FLAG_KNOWN
    for s in sorted(starts):
        o = s
        while o < len(prg) and not t.flags[o] & done and (o == s or o not in starts) and (o == s or o % BANK_SIZE):
            t.flags[o] |= trace_code.FLAG_TABLE
            o += 1


def mark_free_space(t, prg, names):
    """The $FF fill at the end of banks 0-6 (bank 7 ends with the vectors)."""
    for bank in range(FIXED_BANK):
        start = end = (bank + 1) * BANK_SIZE
        while start > bank * BANK_SIZE and prg[start - 1] == 0xFF and not t.flags[start - 1] & (
                trace_code.FLAG_CODE | trace_code.FLAG_ADDRTAB | trace_code.FLAG_INLINE | trace_code.FLAG_KNOWN):
            start -= 1
        if end - start >= 8:
            t.mark(start, end - start, trace_code.FLAG_KNOWN)
            names.setdefault(cpu_addr(start), (f"free_space_{bank}", f"{end - start} unused bytes of $FF"))


def classify(prg, cdl=None):
    """Trace the code and classify the data. Returns (tracer, report, names)."""
    t, report = trace_code.trace(prg, cdl)
    names = {cpu_addr(o): (n, "") for o, n in t.labels.items()}
    for mark in (mark_text, mark_scripts, mark_scenes, mark_sound, mark_layouts, mark_rooms,
                 mark_vectors, mark_unused, mark_free_space, mark_padding):
        mark(t, prg, names)
    for (bank, addr), (name, _) in sorted(read_labels().items()):
        t.try_entry(rom_offset(addr, bank), name)
    for o in t.find_unreferenced_code([2, 6, FIXED_BANK]):
        names.setdefault(cpu_addr(o), (t.labels[o], "no caller found"))
    t.run()
    mark_code_tables(t, prg)
    report["stats_by_bank"] = trace_code.stats(t.flags)
    return t, report, names


def unclassified_report(t, prg):
    """Every run of unclassified bytes, largest first."""
    runs, start = [], None
    for o in range(len(prg) + 1):
        u = o < len(prg) and o % BANK_SIZE != 0 or start is None
        u = u and o < len(prg) and trace_code.byte_class(t.flags[o]) == "unclassified"
        if u and start is None:
            start = o
        elif not u and start is not None:
            runs.append((start, o))
            start = None
            if o < len(prg) and trace_code.byte_class(t.flags[o]) == "unclassified":
                start = o
    lines = [f"{sum(e - s for s, e in runs)} unclassified bytes in {len(runs)} runs"]
    for s, e in sorted(runs, key=lambda r: (r[0] - r[1], r[0])):
        bank, addr = cpu_addr(s)
        lines.append(f"{bank}:{addr:04X} {e - s:5}  " + " ".join(f"{b:02X}" for b in prg[s:min(e, s + 16)]))
    return "\n".join(lines) + "\n"


def run(cmd):
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        sys.exit(f"disasm: {' '.join(map(str, cmd))} failed:\n{result.stdout}{result.stderr}")


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--rom", type=Path, default=check_rom.DEFAULT_ROM)
    ap.add_argument("--cdl", type=Path, help="Mesen CDL for PRG ROM (from tools/run_mesen.py)")
    ap.add_argument("--out", type=Path, default=REPO / "build" / "asm")
    args = ap.parse_args()

    data = trace_code.load_prg(args.rom, "disasm")
    prg = data[16:16 + NUM_BANKS * BANK_SIZE]
    t, report, names = classify(prg, args.cdl.read_bytes() if args.cdl else None)
    args.out.mkdir(parents=True, exist_ok=True)
    (args.out / "unclassified.txt").write_text(unclassified_report(t, prg))
    (args.out / "trace.json").write_text(json.dumps(report, indent=1))
    names.update(read_labels())
    da65, ca65, ld65 = cc65_tool("da65"), cc65_tool("ca65"), cc65_tool("ld65")

    rebuilt, all_match = bytearray(), True
    for bank in range(NUM_BANKS):
        stem = args.out / f"bank{bank}"
        original = prg[bank * BANK_SIZE:(bank + 1) * BANK_SIZE]
        stem.with_suffix(".bin").write_bytes(original)
        stem.with_suffix(".info").write_text(
            info_file(bank, t.flags, names, stem.with_suffix(".bin"), stem.with_suffix(".s")))
        stem.with_suffix(".cfg").write_text(LINK_CFG.format(base=cpu_addr(bank * BANK_SIZE)[1]))
        run([da65, "--info", stem.with_suffix(".info")])
        run([ca65, "--cpu", "6502", "-o", stem.with_suffix(".o"), stem.with_suffix(".s")])
        run([ld65, "-C", stem.with_suffix(".cfg"), "-o", stem.with_suffix(".out"), stem.with_suffix(".o")])
        built = stem.with_suffix(".out").read_bytes()
        diffs = [i for i in range(BANK_SIZE) if i >= len(built) or built[i] != original[i]]
        s = report["stats_by_bank"][bank]
        known = BANK_SIZE - s["unclassified"]
        status = "identical" if not diffs else f"{len(diffs)} bytes differ, first at offset ${diffs[0]:04X}"
        print(f"bank {bank}: {s['code']:5} code, {known * 100 // BANK_SIZE:3}% classified, rebuild {status}")
        all_match &= not diffs
        rebuilt += built[:BANK_SIZE]

    print(trace_code.summary(t, report))
    if all_match:
        (args.out / "rebuilt.nes").write_bytes(data[:16] + rebuilt + data[16 + len(prg):])
        print(f"disasm: PRG rebuilds byte for byte; wrote {args.out / 'rebuilt.nes'}")
    else:
        sys.exit("disasm: rebuild does not match the ROM")


if __name__ == "__main__":
    main()
