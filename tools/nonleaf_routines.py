#!/usr/bin/env python3
"""Print the ported routines that call another ported routine, comma-separated, for --emulate.

usage: dv_harness --movie MOVIE --compare --check-ports --emulate "$(python tools/nonleaf_routines.py)"

--check-ports only sees calls from original code, and the game now runs in C, so on its own it checks
little. With these run as original code, their calls of the leaves are checked, and main and reset
start the game as original code too.

A routine counts as calling what the static helpers it calls call. Calls through a table (the opcodes,
the far calls) aren't seen, which only leaves out a routine whose only calls are those.
"""
import re
import sys
from pathlib import Path

SRC = Path(__file__).resolve().parent.parent / "src"
FUNCTION = re.compile(r"^(?:static )?(?:void|int|uint8_t|uint16_t) (\w+)\([^)]*\) \{\n(.*?)^\}", re.M | re.S)


def nonleaf(sources):
    """Returns the names, given {file name: text}."""
    ported = set(re.findall(r"(?:ENTRY|BANKED)\((\w+),", sources["dv_rtl.c"]))
    names = ["main", "reset"]
    for file, text in sources.items():
        if file in ("dv_rtl.c", "dv_cpu_infra.c"):
            continue
        functions = {m.group(1): m.group(2) for m in FUNCTION.finditer(text)}

        def calls(name, seen):
            found = set()
            for callee in re.findall(r"\b(\w+)\(", functions.get(name, "")):
                if callee in ported and callee != name:
                    found.add(callee)
                elif callee in functions and callee not in seen:
                    seen.add(callee)
                    found |= calls(callee, seen)
            return found

        names += [name for name in functions if name in ported and calls(name, {name})]
    return names


def main():
    sources = {p.name: p.read_text() for p in sorted(SRC.glob("*.c"))}
    print(",".join(nonleaf(sources)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
