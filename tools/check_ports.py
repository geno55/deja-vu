#!/usr/bin/env python3
"""Check the addresses the C port names against the assembled ROM.

usage: check_ports.py [--rom ROM] [--labels FILE]

A RESUME_ORIGINAL_IF_LAG address is only used when a lag NMI lands in that routine's code right
after that call, so most of them never run in a movie, and a wrong one would go unnoticed until
one does. This checks them without running anything:

- each ported routine in src/dv_rtl.c has the address of its label, and its range is not empty;
- each resume address in src/*.c is inside the routine it names, right after a JSR (in that
  routine's bank: 7, or the one BANKED gives);
- each TAIL_ORIGINAL_IF_LAG address is inside the routine it names, at a label;
- each JUMP_TABLE_OR_ORIGINAL address is inside the routine it names, at a JSR jump_table, and
  each jump_table(0xADDR, ...) gives such a JSR's last byte;
- each far-call table in src/bank_calls.c has the routine the ROM's table points to in that slot, and
  no slot left empty points to a ported routine;
- each "name (B:ADDR)" at the start of a comment in src/*.c gives the label's address.

--labels is ld65's label file, which tools/build_rom.py writes next to the ROM it builds
(default: build/dejavu.labels). Exits 1 and lists what is wrong. Addresses given by expression,
not as a literal (kAfterWait[i], entry + 0x10), aren't checked.
"""
import argparse
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
SRC = REPO / "src"
JSR = 0x20

ENUM = re.compile(r"^\s*(k\w+) = 0x([0-9A-F]{4}),", re.M)
ENTRY = re.compile(r"^\s*(?:ENTRY|INLINE|BANKED|BANKED_INLINE)\((\w+), (k\w+), (k\w+)(?:, (\d))?\),", re.M)
RESUME = re.compile(r"RESUME_ORIGINAL_IF_LAG\((k\w+), 0x([0-9A-F]{4}), (\d+)\)")
RESUME_BY_ARGS = re.compile(r"\((k\w+), 0x([0-9A-F]{4})\);")
TAIL = re.compile(r"TAIL_ORIGINAL_IF_LAG\((k\w+), 0x([0-9A-F]{4}), (\d+)\)")
JUMP = re.compile(r"JUMP_TABLE_OR_ORIGINAL\((k\w+), 0x([0-9A-F]{4}), (\d+),")
JUMP_CALL = re.compile(r"jump_table\(0x([0-9A-F]{4}),")
COMMENT = re.compile(r"^// (\w+) \(([0-7]):([0-9A-F]{4})", re.M)
FAR_TABLE = re.compile(r"kFar(\d)_([0-9A-F]{4})\[16\] = \{(.*?)\};", re.S)


def read_labels(path):
    labels = {}
    for line in Path(path).read_text().splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[0] == "al":
            labels[parts[2].lstrip(".")] = int(parts[1], 16) & 0xFFFF
    return labels


def rom_byte(rom, bank, addr):
    return rom[16 + bank * 0x4000 + (addr & 0x3FFF)]


def check(sources, enums, labels, rom):
    """Returns the problems found: sources is {file name: text}, enums {kName: address}."""
    problems = []
    ranges = {}
    for match in ENTRY.finditer(sources["dv_rtl.c"]):
        name, start, end, bank = match.groups()
        if start not in enums or end not in enums:
            problems.append(f"dv_rtl.c: {name}: {start} or {end} isn't in funcs.h")
            continue
        ranges[start] = (name, enums[start], enums[end], int(bank or 7))
        if enums[start] >= enums[end]:
            problems.append(f"dv_rtl.c: {name}: empty range ${enums[start]:04X}-${enums[end]:04X}")
        if name in labels and labels[name] != enums[start]:
            problems.append(f"dv_rtl.c: {name} is at ${labels[name]:04X}, not ${enums[start]:04X}")

    def check_resume(file, routine, addr):
        if routine not in ranges:
            problems.append(f"{file}: resume at ${addr:04X}: {routine} isn't a ported routine")
            return
        name, start, end, bank = ranges[routine]
        if not start <= addr < end:
            problems.append(f"{file}: resume at ${addr:04X} is outside {name} "
                            f"(${start:04X}-${end:04X})")
        elif rom_byte(rom, bank, addr - 3) != JSR:
            problems.append(f"{file}: resume at ${addr:04X} in {name} doesn't follow a JSR")

    def inside(file, what, routine, addr):
        if routine not in ranges:
            problems.append(f"{file}: {what} at ${addr:04X}: {routine} isn't a ported routine")
            return None
        name, start, end, bank = ranges[routine]
        if not start <= addr < end:
            problems.append(f"{file}: {what} at ${addr:04X} is outside {name} "
                            f"(${start:04X}-${end:04X})")
            return None
        return name, bank

    # Addresses of the labels each bank's assembly defines (the label file has no banks).
    at_label = {}
    for bank in range(8):
        names = re.findall(r"^(\w+):", (REPO / "asm" / f"bank{bank}.s").read_text(), re.M)
        at_label[bank] = {labels[n] for n in names if n in labels}
    jump_table = labels.get("jump_table")
    checked = 0
    ported_in_bank = {(bank, start) for _, start, _, bank in ranges.values()}
    for file, text in sources.items():
        for bank, table, entries in FAR_TABLE.findall(text):
            bank, table = int(bank), int(table, 16)
            names = re.findall(r"\w+", entries)
            for slot in range(16):
                target = (rom_byte(rom, bank, table + slot * 2) |
                          rom_byte(rom, bank, table + slot * 2 + 1) << 8)
                if slot >= len(names) or names[slot] == "NULL":
                    if (bank, target) in ported_in_bank:
                        problems.append(f"{file}: far table {bank}:{table:04X} slot {slot} is empty, "
                                        f"but the ROM has ${target:04X}, a ported routine")
                elif names[slot] not in labels:
                    problems.append(f"{file}: far table {bank}:{table:04X} slot {slot}: no label "
                                    f"{names[slot]}")
                elif labels[names[slot]] != target:
                    problems.append(f"{file}: far table {bank}:{table:04X} slot {slot} has "
                                    f"{names[slot]}, but the ROM has ${target:04X}")
                checked += 1
        for routine, addr, _ in TAIL.findall(text):
            addr = int(addr, 16)
            found = inside(file, "tail", routine, addr)
            if found and addr not in at_label[found[1]]:
                problems.append(f"{file}: tail at ${addr:04X} in {found[0]} isn't at a label")
            checked += 1
        jsrs = set()
        for routine, addr, _ in JUMP.findall(text):
            addr = int(addr, 16)
            found = inside(file, "jump table", routine, addr)
            if found:
                bank = found[1]
                target = rom_byte(rom, bank, addr + 1) | rom_byte(rom, bank, addr + 2) << 8
                if rom_byte(rom, bank, addr) != JSR or target != jump_table:
                    problems.append(f"{file}: jump table at ${addr:04X} in {found[0]} isn't a "
                                    f"JSR jump_table")
                jsrs.add(addr)
            checked += 1
        for pushed in JUMP_CALL.findall(text):
            if int(pushed, 16) - 2 not in jsrs:
                problems.append(f"{file}: jump_table(0x{pushed}, ...) isn't the last byte of a "
                                f"JUMP_TABLE_OR_ORIGINAL JSR")
            checked += 1
        for routine, addr, _ in RESUME.findall(text):
            check_resume(file, routine, int(addr, 16))
            checked += 1
        # Helpers given the routine and its address, like print_table_name.
        for routine, addr in RESUME_BY_ARGS.findall(text):
            check_resume(file, routine, int(addr, 16))
            checked += 1
        for name, bank, addr in COMMENT.findall(text):
            if name in labels and labels[name] != int(addr, 16):
                problems.append(f"{file}: the comment on {name} says {bank}:{addr}, "
                                f"the label is at {labels[name]:04X}")
    return problems, len(ranges), checked


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--rom", default=REPO / "Deja Vu (USA).nes")
    parser.add_argument("--labels", default=REPO / "build" / "dejavu.labels")
    args = parser.parse_args()
    sources = {p.name: p.read_text() for p in sorted(SRC.glob("*.c"))}
    funcs = (SRC / "funcs.h").read_text()
    enums = {name: int(addr, 16) for name, addr in ENUM.findall(funcs)}
    problems, routines, resumes = check(sources, enums, read_labels(args.labels),
                                        Path(args.rom).read_bytes())
    for problem in problems:
        print(problem)
    print(f"check_ports: {routines} routines and {resumes} resume addresses, "
          f"{len(problems)} problems")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
