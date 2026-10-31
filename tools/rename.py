#!/usr/bin/env python3
"""Rename symbols across the assembly.

usage: rename.py OLD NEW [OLD NEW ...]
       rename.py --file PAIRS     (a text file of "OLD NEW [; comment]" lines)

Replaces each OLD as a whole word in asm/*.s, asm/*.inc and asm/labels.txt.
Refuses if NEW is already used anywhere, so two symbols never merge by accident.
Rebuild with tools/build_rom.py afterwards; renames don't change the bytes.
"""
import argparse
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
ASM = REPO / "asm"


def files():
    return sorted(ASM.glob("*.s")) + sorted(ASM.glob("*.inc")) + [ASM / "labels.txt"]


def rename_all(pairs, paths=None):
    texts = {p: p.read_text() for p in (paths or files())}
    for old, new in pairs:
        if not re.fullmatch(r"[A-Za-z_]\w*", new):
            sys.exit(f"rename: {new!r} is not a valid symbol name")
        word_new = re.compile(rf"\b{re.escape(new)}\b")
        if any(word_new.search(t) for t in texts.values()):
            sys.exit(f"rename: {new} is already used; pick another name")
        word_old = re.compile(rf"\b{re.escape(old)}\b")
        hits = sum(len(word_old.findall(t)) for t in texts.values())
        if not hits:
            sys.exit(f"rename: {old} not found")
        texts = {p: word_old.sub(new, t) for p, t in texts.items()}
        print(f"rename: {old} -> {new} ({hits} uses)")
    for p, t in texts.items():
        if p.read_text() != t:
            p.write_text(t)


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("pairs", nargs="*")
    ap.add_argument("--file", type=Path)
    args = ap.parse_args()
    words = list(args.pairs)
    if args.file:
        for line in args.file.read_text().splitlines():
            line = line.split(";")[0].strip()
            if line and not line.startswith("#"):
                words += line.split()
    if not words or len(words) % 2:
        sys.exit(__doc__)
    rename_all(list(zip(words[::2], words[1::2])))


if __name__ == "__main__":
    main()
