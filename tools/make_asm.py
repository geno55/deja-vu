#!/usr/bin/env python3
"""Write the repository's assembly (asm/*.s) from the ROM, once.

usage: make_asm.py [--rom ROM] [--cdl FILE] [--force]

This is the bootstrap for Phase 2. It runs the same analysis as tools/disasm.py,
then writes one ca65 file per bank where large data (scene pictures, messages,
scripts, sound data, name tables, hotspot lists, unused areas) becomes .incbin of
a file that tools/extract_bins.py cuts from the user's ROM, as listed in
asm/bins.txt. Code and small tables stay in the assembly.

After the first run, asm/*.s are the source: edit them by hand. This tool refuses
to overwrite them unless given --force.
"""
import argparse
import hashlib
import re
import sys
import tempfile
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "tools"))
import disasm  # noqa: E402
import extract_bins  # noqa: E402
import layouts  # noqa: E402
import rooms  # noqa: E402
import scenes  # noqa: E402
import scripts  # noqa: E402
import sound  # noqa: E402
import text  # noqa: E402
import trace_code  # noqa: E402
from nes6502 import BANK_SIZE, FIXED_BANK, NUM_BANKS, OPCODES, cpu_addr, rom_offset  # noqa: E402

ASM = REPO / "asm"
CHR_SIZE = 0x40000
ADDRESS_COMMENT = re.compile(r"\s+; ([0-9A-F]{4}) (?:[0-9A-F]{2} )+.*$")
SINGLE_BYTE = re.compile(r"^\s+\.byte\s+(\$[0-9A-F]{2})$")
BYTE_LIST = re.compile(r"^\s+\.byte\s+(\$[0-9A-F]{2},)*\$[0-9A-F]{2}$")


def blobs(prg, t):
    """[(bank, start, end, name, description)] of data that goes into .incbin files."""
    out = []
    for bank, _, _ in scenes.BANKS:
        e = scenes.pointer_table(prg, bank)
        out.append((bank, min(e), max(e) + scenes.RECORD, f"scenes_{bank}_data", "scene pictures"))
    for tb in text.decode_all(prg)[0]:
        m = tb["messages"]
        out.append((tb["bank"], min(m), max(p + n for p, (_, n) in m.items()),
                    f"messages_{tb['bank']}_data", "compressed messages"))
    rom = text.Rom(prg)
    for kind, base in text.NAME_TABLES.items():
        out.append((2, base, base + 8 * text.name_table_size(rom, kind), f"{kind}_names_data", f"{kind} names"))
    s = scripts.load(prg)
    runs = []
    for r in s.layout()["scripts"]:
        if runs and runs[-1][1] == r["start"]:
            runs[-1][1] = r["end"]
        else:
            runs.append([r["start"], r["end"]])
    for a, e in runs:
        if e - a >= 64:
            out.append((scripts.BANK, a, e, f"scripts_{a:04X}_data", "bytecode scripts"))
    snd = sound.load(prg)
    items = [(a, a + n) for a, (k, n, _) in snd.items.items() if k != "table"]
    out.append((sound.BANK, min(a for a, _ in items), max(e for _, e in items), "sound_data",
                "sound headers, channel streams and instruments"))
    entries, lists = rooms.load(prg)
    out.append((rooms.BANK, min(lists), max(a + rooms.RECORD * n + 1 for a, n in lists.items()),
                "hotspots_data", "room hotspot lists"))
    entries, found = layouts.load(prg)
    out.append((layouts.BANK, min(found), max(a + n for a, n in found.items()), "layouts_data", "screen layouts"))
    for bank, start, end, label, _ in disasm.UNUSED:
        out.append((bank, start, end, f"{label}_data", "unused bytes"))

    blocked = trace_code.FLAG_CODE | trace_code.FLAG_ADDRTAB | trace_code.FLAG_INLINE
    for bank, start, end, name, _ in out:
        o = rom_offset(start, bank)
        if any(t.flags[i] & blocked for i in range(o, o + end - start)):
            sys.exit(f"make_asm: {name} ({bank}:{start:04X}-{end - 1:04X}) overlaps code or an address table")
    return sorted(out)


def fixed_refs(prg, t, bank):
    """Fixed-bank addresses that bank `bank` refers to (operands and address tables), so
    its file only declares the names it uses."""
    refs = set()
    for o in range(bank * BANK_SIZE, (bank + 1) * BANK_SIZE):
        if t.flags[o] & trace_code.FLAG_OPSTART and OPCODES[prg[o]][2] == 3:
            refs.add(prg[o + 1] | prg[o + 2] << 8)
        elif t.flags[o] & trace_code.FLAG_ADDRTAB and not t.flags[o - 1] & trace_code.FLAG_ADDRTAB:
            p = o  # the first entry of an address table; read entries while the table lasts
            while p + 1 < (bank + 1) * BANK_SIZE and t.flags[p] & trace_code.FLAG_ADDRTAB:
                refs.add(prg[p] | prg[p + 1] << 8)
                p += 2
    return {a for a in refs if a >= 0xC000}


def postprocess(raw, bank, bank_blobs, names):
    """da65 output (COMMENTS 4) -> the committed file: header, segment, .incbin blocks,
    address comments removed, single-byte lines merged."""
    base = cpu_addr(bank * BANK_SIZE)[1]
    out = [f"; Deja Vu (USA), NES: PRG bank {bank} (${base:04X}-${base + BANK_SIZE - 1:04X})",
           "; Generated once by tools/make_asm.py; this file is now the source, so edit it directly.",
           "; Large data comes from your ROM via .incbin: see asm/bins.txt and tools/extract_bins.py.",
           "",
           '        .setcpu "6502"',
           f'        .segment "BANK{bank}"']
    lines = raw.splitlines()
    lines = lines[next(i for i, l in enumerate(lines) if l.strip().startswith(".setcpu")) + 1:]
    pending, todo = [], list(bank_blobs)

    def blob_block(b):
        _, start, end, name, desc = b
        block = ["", f"; {desc}: {end - start} bytes from your ROM", f"{name}:", f'        .incbin "{name}.bin"']
        for (nb, addr), (label, _) in sorted(names.items()):
            if nb == bank and start <= addr < end:
                block.append(f"{label:<15} := {name} + ${addr - start:04X}")
        return block + [""]

    for line in lines:
        m = ADDRESS_COMMENT.search(line)
        if not m:
            pending.append(line)
            continue
        addr = int(m.group(1), 16)
        while todo and todo[0][1] <= addr:
            out += blob_block(todo.pop(0))
        out += pending + [ADDRESS_COMMENT.sub("", line).rstrip()]
        pending = []
    for b in todo:
        out += blob_block(b)
    out += pending

    merged = []
    for line in out:
        m = SINGLE_BYTE.match(line)
        prev = merged[-1] if merged else ""
        if m and BYTE_LIST.match(prev) and prev.count("$") < 8:
            merged[-1] = prev + "," + m.group(1)
        elif line.startswith("; ---") and prev.startswith("; ---"):
            continue  # da65 sometimes prints two separators in a row
        else:
            merged.append(line)
    while merged and not merged[-1].strip():
        merged.pop()
    return "\n".join(merged) + "\n"


def linker_config():
    banks = "\n".join(f"    PRG{b}:   start = ${0xC000 if b == FIXED_BANK else 0x8000:04X}, size = $4000, "
                      f"fill = yes, fillval = $FF, file = %O;" for b in range(NUM_BANKS))
    segs = "\n".join(f"    BANK{b}:  load = PRG{b}, type = ro;" for b in range(NUM_BANKS))
    return ("# ld65 configuration: iNES header, 8 PRG banks of 16 KB, 256 KB CHR ROM.\n"
            "MEMORY {\n    HEADER: start = $0000, size = $0010, fill = yes, file = %O;\n"
            f"{banks}\n    CHR:    start = $0000, size = ${CHR_SIZE:05X}, fill = yes, file = %O;\n}}\n"
            "SEGMENTS {\n    HEADER: load = HEADER, type = ro;\n"
            f"{segs}\n    CHR:    load = CHR, type = ro;\n}}\n")


HEADER_S = """; iNES (NES 2.0) header for Deja Vu (USA): mapper 4 (MMC3), 128 KB PRG, 256 KB CHR,
; 8 KB battery-backed PRG RAM, horizontal mirroring.
        .segment "HEADER"
        .byte   "NES", $1A
        .byte   $08             ; PRG ROM: 8 x 16 KB
        .byte   $20             ; CHR ROM: 32 x 8 KB
        .byte   $42             ; mapper 4 (low nibble), battery
        .byte   $08             ; NES 2.0
        .byte   $00, $00
        .byte   $70             ; 8 KB battery-backed PRG RAM
        .byte   $00, $00, $00, $00
        .byte   $01             ; default expansion device: standard controllers
"""

CHR_S = """; CHR ROM: all 256 KB of tile graphics come from your ROM.
        .segment "CHR"
        .incbin "chr.bin"
"""


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--rom", type=Path, default=extract_bins.check_rom.DEFAULT_ROM)
    ap.add_argument("--cdl", type=Path, help="Mesen CDL for PRG ROM (from tools/run_mesen.py)")
    ap.add_argument("--force", action="store_true", help="overwrite existing asm/*.s")
    args = ap.parse_args()

    existing = sorted(ASM.glob("bank*.s"))
    if existing and not args.force:
        sys.exit("make_asm: asm/bank*.s already exist and may have hand edits; use --force to overwrite")
    data = trace_code.load_prg(args.rom, "make_asm")
    prg = data[16:16 + NUM_BANKS * BANK_SIZE]
    t, report, names = disasm.classify(prg, args.cdl.read_bytes() if args.cdl else None)
    names.update(disasm.read_labels())
    all_blobs = blobs(prg, t)
    da65 = disasm.cc65_tool("da65")

    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)
        for bank in range(NUM_BANKS):
            mine = [b for b in all_blobs if b[0] == bank]
            skip = set()
            for _, start, end, _, _ in mine:
                o = rom_offset(start, bank)
                skip.update(range(o, o + end - start))
            stem = tmp / f"bank{bank}"
            stem.with_suffix(".bin").write_bytes(prg[bank * BANK_SIZE:(bank + 1) * BANK_SIZE])
            refs = fixed_refs(prg, t, bank)
            visible = {k: v for k, v in names.items() if k[0] == bank or (k[0] == FIXED_BANK and k[1] in refs)}
            stem.with_suffix(".info").write_text(disasm.info_file(
                bank, t.flags, visible, stem.with_suffix(".bin"), stem.with_suffix(".s"), skip=skip, comments=4))
            disasm.run([da65, "--info", stem.with_suffix(".info")])
            (ASM / f"bank{bank}.s").write_text(postprocess(stem.with_suffix(".s").read_text(), bank, mine, names))

    manifest = ["# Data the assembly takes from your ROM (tools/extract_bins.py).",
                "# name  bank  address  length  sha1   (bank \"chr\": offset into CHR ROM)"]
    for bank, start, end, name, _ in all_blobs:
        blob = prg[rom_offset(start, bank):rom_offset(start, bank) + end - start]
        manifest.append(f"{name} {bank} {start:04X} {end - start} {hashlib.sha1(blob).hexdigest()}")
    chr_data = data[16 + len(prg):16 + len(prg) + CHR_SIZE]
    manifest.append(f"chr chr 0000 {CHR_SIZE} {hashlib.sha1(chr_data).hexdigest()}")
    (ASM / "bins.txt").write_text("\n".join(manifest) + "\n")
    (ASM / "dejavu.cfg").write_text(linker_config())
    (ASM / "header.s").write_text(HEADER_S)
    (ASM / "chr.s").write_text(CHR_S)
    in_blobs = sum(e - s for _, s, e, _, _ in all_blobs)
    print(f"make_asm: wrote asm/bank0-7.s, header.s, chr.s, dejavu.cfg and bins.txt; "
          f"{len(all_blobs)} PRG blobs ({in_blobs} bytes) + CHR come from the ROM")


if __name__ == "__main__":
    main()
