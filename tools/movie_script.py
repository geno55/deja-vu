#!/usr/bin/env python3
"""Turn a script of button presses into an .inp movie, which dejavu and dv_harness can play.

usage: movie_script.py SCRIPT.moves OUT.inp

A movie has a line for every frame, thousands of them; a script says what is going on. Each line
of a script has steps, and # starts a comment:

    BUTTONS:FRAMES   hold these buttons, any of UDLRTSBA, or . for none, for this many frames
    Nx               at the start of a line: the line's steps N times

    # Press A for each page of the opening text.
    40x A:3 .:60

Scripts in tests/movies/ work on the original and on the port alone, which reach the same point at
different frames (the port has no lag), so they leave a good margin after each press.
"""
import argparse
import re
import sys
from pathlib import Path

ORDER = "UDLRTSBA"
STEP = re.compile(r"^([UDLRTSBA]+|\.):(\d+)$")
REPEAT = re.compile(r"^(\d+)x$")


def expand(text):
    """The script's lines as movie lines. Raises ValueError for a step it can't read."""
    frames = []
    for number, line in enumerate(text.splitlines(), 1):
        words = line.split("#", 1)[0].split()
        repeat = 1
        if words and REPEAT.match(words[0]):
            repeat = int(words.pop(0)[:-1])
        steps = []
        for word in words:
            step = STEP.match(word)
            if not step:
                raise ValueError(f"line {number}: can't read {word!r}")
            steps += ["".join(b if b in step[1] else "." for b in ORDER)] * int(step[2])
        frames += steps * repeat
    return frames


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("script", type=Path)
    parser.add_argument("out", type=Path)
    args = parser.parse_args()
    try:
        frames = expand(args.script.read_text())
    except ValueError as e:
        sys.exit(f"movie_script: {args.script}: {e}")
    args.out.write_text("\n".join(frames) + "\n")
    print(f"movie_script: {args.out}: {len(frames)} frames")


if __name__ == "__main__":
    main()
