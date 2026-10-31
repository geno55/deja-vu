#!/usr/bin/env python3
"""Use the names from asm/ram.inc and asm/nes.inc in asm/bank*.s.

usage: name_ram.py [--check]

Replaces raw operands ($A5, $0515,x, ($B6),y, $2006, ...) with names from
asm/ram.inc (RAM variables) and asm/nes.inc (hardware registers), as name+n inside
multi-byte variables, and makes sure every bank file includes both files. Names
marked [read] or [write] only replace operands of loads or of stores. Running it
again changes nothing. With --check it only reports how many operands it would
change. Rebuild with tools/build_rom.py afterwards: names don't change the bytes,
so the ROM must still match.
"""
import argparse
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
ASM = REPO / "asm"
INCLUDES = [ASM / "nes.inc", ASM / "ram.inc"]
DEFINITION = re.compile(r"^(\w+)\s*=\s*\$([0-9A-F]+)\s*(?:;\s*(?:\[(\d+|read|write)\])?(.*))?$")
# mnemonic, then an operand that is a plain hex address (optionally indexed or indirect)
OPERAND = re.compile(r"^(?P<head>\s*(?:\w+:)?\s+(?P<op>[a-z]{3})\s+\(?)\$(?P<addr>[0-9A-F]{2}|[0-9A-F]{4})"
                     r"(?P<tail>(?:,[xy])?\)?(?:,[xy])?\s*(?:;.*)?)$")
STORES = {"sta", "stx", "sty"}


def read_names(paths=INCLUDES):
    """{(address, access): (name, offset)}; access is "read", "write" or None (either)."""
    names, owner = {}, {}
    for path in paths:
        for n, line in enumerate(path.read_text().splitlines(), 1):
            m = DEFINITION.match(line.strip())
            if not m:
                continue
            name, addr, tag = m.group(1), int(m.group(2), 16), m.group(3)
            size = int(tag) if tag and tag.isdigit() else 1
            access = tag if tag in ("read", "write") else None
            for i in range(size):
                key = (addr + i, access)
                if key in owner or (access is None and any(k[0] == addr + i for k in owner)):
                    sys.exit(f"name_ram: {path.name}:{n}: {name} overlaps {owner.get(key, '?')} at ${addr + i:04X}")
                owner[key] = name
                names[key] = (name, i)
    return names


def rename_line(line, names):
    m = OPERAND.match(line)
    if not m or line.lstrip().startswith(";"):
        return line
    addr = int(m.group("addr"), 16)
    access = "write" if m.group("op") in STORES else "read"
    hit = names.get((addr, None)) or names.get((addr, access))
    if not hit:
        return line
    name, offset = hit
    return f"{m.group('head')}{name}{f'+{offset}' if offset else ''}{m.group('tail')}"


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--check", action="store_true")
    args = ap.parse_args()
    names = read_names()
    total = 0
    for path in sorted(ASM.glob("bank*.s")):
        lines = path.read_text().splitlines()
        new = [rename_line(line, names) for line in lines]
        total += sum(a != b for a, b in zip(lines, new))
        seg = next(i for i, line in enumerate(new) if line.strip().startswith(".segment"))
        for inc in reversed(INCLUDES):
            if not any(line.strip() == f'.include "{inc.name}"' for line in new):
                new.insert(seg + 1, f'        .include "{inc.name}"')
        if not args.check and new != lines:
            path.write_text("\n".join(new) + "\n")
    print(f"name_ram: {total} operands {'would change' if args.check else 'named'}")


if __name__ == "__main__":
    main()
