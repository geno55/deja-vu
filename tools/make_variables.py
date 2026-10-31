#!/usr/bin/env python3
"""Write src/variables.h, the C port's names for RAM, from asm/ram.inc.

usage: make_variables.py [--check]

Each variable in asm/ram.inc becomes a macro, as zelda3's variables.h does: one-byte
variables are lvalues (`system_flags |= 2`), longer ones are pointers to their first byte
(`oam_buffer[i]`). Run it after editing asm/ram.inc (with tools/name_ram.py). With --check it
writes nothing and fails if src/variables.h is out of date (the CMake test variables_current).
"""
import argparse
import sys
from pathlib import Path

from name_ram import ASM, DEFINITION

REPO = Path(__file__).resolve().parent.parent
OUT = REPO / "src" / "variables.h"

HEAD = """\
// The C port's names for RAM (zelda3's variables.h), generated from asm/ram.inc by
// tools/make_variables.py: edit asm/ram.inc and run the tool, not this file. One-byte
// variables are lvalues; longer ones ([n] in the comment) point to their first byte.
#ifndef DV_VARIABLES_H
#define DV_VARIABLES_H

#include "nes_hw.h"

#define g_ram (g_nes.ram)
"""


def render():
    lines = [HEAD]
    for line in (ASM / "ram.inc").read_text().splitlines():
        m = DEFINITION.match(line.strip())
        if not m:
            continue
        name, addr, tag, comment = m.group(1), int(m.group(2), 16), m.group(3), m.group(4)
        size = int(tag) if tag and tag.isdigit() else 1
        what = (comment or "").strip()
        if size > 1:
            what = f"[{size}] {what}".strip()
        lines.append(f"#define {name} {'(g_ram + 0x%02X)' % addr if size > 1 else '(g_ram[0x%02X])' % addr}"
                     + (f"  // {what}" if what else ""))
    lines.append("\n#endif  // DV_VARIABLES_H\n")
    return "\n".join(lines)


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--check", action="store_true")
    args = ap.parse_args()
    text = render()
    if args.check:
        if not OUT.exists() or OUT.read_text() != text:
            sys.exit("make_variables: src/variables.h is out of date; run tools/make_variables.py")
        print("make_variables: src/variables.h is current")
        return
    OUT.write_text(text)
    print(f"make_variables: wrote {OUT.relative_to(REPO)}")


if __name__ == "__main__":
    main()
