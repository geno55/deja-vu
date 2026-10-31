#!/usr/bin/env python3
"""Decode Deja Vu's compressed text.

usage: text.py [--rom ROM] [--out DIR]
Writes DIR/messages.txt (every message, readable) and DIR/text.json (where each
pointer table, message and name table sits, for tools/disasm.py).

Format (decoder at 7:DF00, printer at 7:DF74; see docs/text.md):
- Messages are bit streams, read most significant bit first. Each character is
  a run of 0 bits ended by a 1 (the run length is the high part of an index),
  then 3 more bits (the low part). The 6-bit index picks a character code from
  the table at 7:E4FC; frequent characters get short codes.
- Character codes 0-25 are A-Z, 26 is a space, 32-58 are punctuation and digits
  (tile numbers from the table at 7:E213); the rest are control codes.
- Some control codes take arguments: 8 more bits from the stream, or 2 bytes
  placed right after the current byte.
"""
import argparse
import json
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "tools"))
import check_rom  # noqa: E402
from nes6502 import BANK_SIZE, NUM_BANKS, cpu_addr, rom_offset, word  # noqa: E402

CODE_TABLE = 0xE4FC  # 64 entries: bit-stream index -> character code
TILE_TABLE = 0xE213  # 64 entries: character code -> tile number
# (bank, table address, entries). Entry points: 7:C4AE, 7:C4EF, 7:C501.
MESSAGE_TABLES = [(5, 0x9A00, 256), (4, 0x8000, 256), (2, 0xA500, 128)]
# Name tables in bank 2: 8 tile numbers per name, $FF ends a shorter one.
NAME_TABLES = {"item": 0x8D00, "word": 0x9500}

# Tile numbers to text (Data Crystal's table; $8A is shown there as "!!").
TILES = {0x7E: " ", 0x7F: "|", 0x8A: "!", 0x8B: "?", 0xA1: ".", 0xA4: "-", 0xA5: "*", 0xB0: " ",
         0xCB: "=", 0xCC: "$", 0xCD: "'", 0xCE: "&", 0xD5: '"', 0xD6: ":", 0xD7: ";",
         0xD8: ",", 0xD9: "(", 0xDA: ")"}
TILES.update({0x80 + i: str(i) for i in range(10)})
TILES.update({0xB1 + i: chr(ord("A") + i) for i in range(26)})

# Control codes (character codes the printer handles itself).
WAIT = 0x1B          # fill the page with blank lines, wait for a button
WAIT_CLEAR = 0x1C    # same, then clear the window
NEWLINE = 0x1D
END = 0x1E
WAIT_END = 0x1F
CLEAR = 0x3B
ITEM_NAME = 0x3C     # 8-bit argument from the stream: name from the "item" table
WORD = 0x3D          # 8-bit argument from the stream: entry of the "word" table
NUMBER = 0x3E        # 2 bytes: address of a 16-bit number to print in decimal
ITEM_NAME_AT = 0x3F  # 2 bytes: address of a RAM byte holding an "item" table index
LINE_WIDTH = 26


def tile_text(tile):
    return TILES.get(tile, f"<tile {tile:02X}>")


class Rom:
    def __init__(self, prg):
        self.prg = prg

    def read(self, bank, addr):
        return self.prg[rom_offset(addr, bank)]

    def name(self, table, index):
        out = []
        for i in range(8):
            t = self.read(2, NAME_TABLES[table] + index * 8 + i)
            if t == 0xFF:
                break
            out.append(tile_text(t))
        return "".join(out).rstrip()


class Message:
    """Decodes one message exactly the way the 6502 printer reads it."""

    def __init__(self, rom, bank, addr):
        self.rom, self.bank, self.ptr = rom, bank, addr
        self.bits, self.count = 0, 0  # $3E (bit buffer) and $3D (bits used of it)
        self.names_used = {"item": set(), "word": set()}

    def bit(self):
        if self.count == 0:
            self.bits = self.rom.read(self.bank, self.ptr)
            self.ptr += 1
        b = self.bits >> 7
        self.bits = self.bits << 1 & 0xFF
        self.count = (self.count + 1) % 8
        return b

    def code(self):
        run = 0
        while not self.bit():
            run += 1
        index = run << 3 | self.bit() << 2 | self.bit() << 1 | self.bit()
        if index >= 64:
            raise ValueError(f"code index {index} out of range")
        return self.rom.read(7, CODE_TABLE + index)

    def byte8(self):
        v = 0
        for _ in range(8):
            v = v << 1 | self.bit()
        return v

    def inline_word(self):
        v = self.rom.read(self.bank, self.ptr) | self.rom.read(self.bank, self.ptr + 1) << 8
        self.ptr += 2
        return v

    def decode(self):
        """The message as the screen shows it, with control codes as <tags>.

        The window is 26 characters wide and wraps by itself, so the text has
        line breaks there too. After a name or number of unknown length the
        column is unknown, and no wrap is shown until the next line break.
        """
        out, col = [], 0

        def text(s):
            nonlocal col
            for ch in s:
                out.append(ch)
                if col is not None:
                    col += 1
                    if col == LINE_WIDTH:
                        out.append("\n")
                        col = 0

        def line_break(tag=""):
            nonlocal col
            out.append(tag + "\n")
            col = 0

        while True:
            c = self.code()
            if c == 0xFF:
                raise ValueError("unused code index")
            if c == END:
                return "".join(out)
            if c == WAIT_END:
                return "".join(out) + "<wait>"
            if c == WAIT:
                line_break("<wait>")
            elif c == WAIT_CLEAR:
                line_break("<wait><clear>")
            elif c == NEWLINE:
                line_break()
            elif c == CLEAR:
                line_break("<clear>")
            elif c in (ITEM_NAME, WORD):
                table = "item" if c == ITEM_NAME else "word"
                i = self.byte8()
                self.names_used[table].add(i)
                out.append(f"<{table} {i:02X}: {self.rom.name(table, i)}>")
                col = None
            elif c in (ITEM_NAME_AT, NUMBER):
                what = "item" if c == ITEM_NAME_AT else "number"
                out.append(f"<{what} at ${self.inline_word():04X}>")
                self.count = 0  # the printer drops the rest of the current byte here
                col = None
            else:
                text(tile_text(self.rom.read(7, TILE_TABLE + c)))


def message_table(rom, bank, base, max_entries):
    """Pointer entries of one table, stopping where its first message starts."""
    entries, first_msg = [], 0x10000
    for i in range(max_entries):
        a = base + 2 * i
        if a >= first_msg:
            break
        p = word(rom.prg, rom_offset(a, bank))
        if not 0x8000 <= p < 0xC000:
            break
        entries.append(p)
        if p > base:
            first_msg = min(first_msg, p)
    return entries


def decode_all(prg):
    rom = Rom(prg)
    tables, names_used = [], {"item": set(), "word": set()}
    for bank, base, max_entries in MESSAGE_TABLES:
        entries = message_table(rom, bank, base, max_entries)
        decoded = {}
        for p in sorted(set(entries)):
            m = Message(rom, bank, p)
            try:
                text = m.decode()
            except (ValueError, IndexError) as e:
                text = f"<decode error: {e}>"
            decoded[p] = (text, m.ptr - p)
            for k in names_used:
                names_used[k] |= m.names_used[k]
        tables.append({"bank": bank, "table": base, "entries": entries, "messages": decoded})
    names = {k: [rom.name(k, i) for i in range(name_table_size(rom, k))] for k in NAME_TABLES}
    return tables, names


def name_table_size(rom, table):
    """Entries until one holds a byte outside the font tiles ($7E-$DF) and $FF padding.

    Unused entries are all $FF; the table can hold at most 256 (an index is one byte).
    """
    n = 0
    while n < 256:
        entry = [rom.read(2, NAME_TABLES[table] + n * 8 + i) for i in range(8)]
        if not all(0x7E <= t <= 0xDF or t == 0xFF for t in entry):
            break
        n += 1
    return n


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--rom", type=Path, default=check_rom.DEFAULT_ROM)
    ap.add_argument("--out", type=Path, default=REPO / "build" / "text")
    args = ap.parse_args()

    data = args.rom.read_bytes()
    if check_rom.check(data):
        sys.exit(f"text: {args.rom} is not Deja Vu (USA); run tools/check_rom.py")
    tables, names = decode_all(data[16:16 + NUM_BANKS * BANK_SIZE])

    args.out.mkdir(parents=True, exist_ok=True)
    lines, layout, errors, count = [], {"message_tables": [], "messages": [], "name_tables": []}, 0, 0
    for t in tables:
        b = t["bank"]
        lines.append(f"==== bank {b}, table ${t['table']:04X}, {len(t['entries'])} entries ====")
        layout["message_tables"].append({"bank": b, "start": t["table"], "entries": len(t["entries"])})
        for i, p in enumerate(t["entries"]):
            text, length = t["messages"][p]
            lines.append(f"--- {b}:{t['table']:04X} #{i:02X} at {b}:{p:04X} ({length} bytes)")
            lines.append(text)
            errors += "<decode error" in text
        for p, (text, length) in sorted(t["messages"].items()):
            layout["messages"].append({"bank": b, "start": p, "length": length})
            count += 1
    for k, v in names.items():
        lines.append(f"==== {k} names (bank 2 ${NAME_TABLES[k]:04X}) ====")
        lines += [f"{i:02X} {n}" for i, n in enumerate(v)]
        layout["name_tables"].append({"bank": 2, "start": NAME_TABLES[k], "length": 8 * len(v), "table": k})
    (args.out / "messages.txt").write_text("\n".join(lines) + "\n")
    (args.out / "text.json").write_text(json.dumps(layout, indent=1))
    total = sum(m["length"] for m in layout["messages"])
    print(f"text: {count} messages ({total} bytes) in {len(tables)} tables, {errors} decode errors; "
          f"{', '.join(f'{len(v)} {k} names' for k, v in names.items())}")


if __name__ == "__main__":
    main()
