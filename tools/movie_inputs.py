#!/usr/bin/env python3
"""Convert a TAS movie's player 1 input to the plain format record_cdl.lua plays back.

usage: movie_inputs.py MOVIE OUT.inp
MOVIE is an FCEUX .fm2 or BizHawk .bk2 file, or a TASVideos .zip holding one.

OUT.inp has one line per frame, buttons in the order UDLRTSBA (T = Start,
S = Select): the letter when pressed, "." when not. Frame 0 is the first
frame after power-on.
"""
import io
import sys
import zipfile
from pathlib import Path

ORDER = "UDLRTSBA"
FM2_ORDER = "RLDUTSBA"  # FCEUX gamepad column
BK2_NAMES = {"P1 Up": "U", "P1 Down": "D", "P1 Left": "L", "P1 Right": "R",
             "P1 Start": "T", "P1 Select": "S", "P1 B": "B", "P1 A": "A"}


def canonical(pressed):
    return "".join(b if b in pressed else "." for b in ORDER)


def from_fm2(text):
    frames, notes = [], []
    for line in text.splitlines():
        if not line.startswith("|"):
            continue
        cmd, port0 = line.split("|")[1:3]
        if int(cmd or 0) & ~1 or (int(cmd or 0) & 1 and frames):
            notes.append(f"frame {len(frames)}: command {cmd} ignored")
        frames.append(canonical({FM2_ORDER[i] for i, c in enumerate(port0) if c != "."}))
    return frames, notes


def from_bk2(data):
    log = zipfile.ZipFile(io.BytesIO(data)).read("Input Log.txt").decode()
    keys, frames, notes = None, [], []
    for line in log.splitlines():
        if line.startswith("LogKey:"):
            # "#Power|Reset|#P1 Up|..." -- '#' starts a new column group
            keys = [k.lstrip("#") for k in line[len("LogKey:"):].strip("|").split("|")]
        elif line.startswith("|"):
            chars = line.replace("|", "")
            pressed = {BK2_NAMES[k] for k, c in zip(keys, chars) if c != "." and k in BK2_NAMES}
            if any(c != "." for k, c in zip(keys, chars) if k in ("Power", "Reset")):
                notes.append(f"frame {len(frames)}: power/reset ignored")
            frames.append(canonical(pressed))
    return frames, notes


def load(path):
    data = path.read_bytes()
    name = path.name
    if name.endswith(".zip"):
        z = zipfile.ZipFile(io.BytesIO(data))
        inner = [n for n in z.namelist() if n.endswith((".fm2", ".bk2"))]
        if len(inner) != 1:
            sys.exit(f"movie_inputs: {path} should hold exactly one .fm2 or .bk2")
        name, data = inner[0], z.read(inner[0])
    if name.endswith(".fm2"):
        return from_fm2(data.decode("utf-8", "replace"))
    if name.endswith(".bk2"):
        return from_bk2(data)
    sys.exit(f"movie_inputs: {path}: not an .fm2 or .bk2 movie")


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    frames, notes = load(Path(sys.argv[1]))
    Path(sys.argv[2]).write_text("\n".join(frames) + "\n")
    print(f"movie_inputs: {len(frames)} frames, {sum(f != '.' * 8 for f in frames)} with buttons held")
    for n in notes:
        print(f"movie_inputs: {n}")


if __name__ == "__main__":
    main()
