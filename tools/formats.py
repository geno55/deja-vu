"""Editable text forms of Deja Vu's data, and the encoders that turn them back into bytes.

Each codec has dump(prg, block) -> str and parse(str) -> bytes, where block is one
entry of asm/bins.txt (name, bank, address, length). parse(dump(...)) must give back
the block's bytes exactly; tools/roundtrip.py checks every block. The CHR graphics
use PNG instead (tools/chr_png.py).

Formats (one item per line; ";" starts a comment):
- messages:  "@ADDR" then the message: text plus {TAGS} for control codes
- names:     one 8-tile entry per line, $FF padding left out
- scripts:   "ADDR: mnemonic args" per opcode ($XX byte or $XXXX word arguments)
- scenes:    "@ADDR scene NN", 14 rows of 14 tile bytes, "attrs" + 16 bytes
- hotspots:  "@ADDR", then "x1 x2 y1 y2 kind id" per hotspot, "end"
- layouts:   "@ADDR" then the layout: text plus {TAGS}
- sound:     "@ADDR kind", then header fields, stream commands or raw bytes
- raw:       hex bytes, 16 per line
"""
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import layouts as layouts_mod  # noqa: E402
import rooms  # noqa: E402
import scenes as scenes_mod  # noqa: E402
import scripts as scripts_mod  # noqa: E402
import sound as sound_mod  # noqa: E402
import text  # noqa: E402
from nes6502 import rom_offset, word  # noqa: E402

TAG = re.compile(r"\{([^}]*)\}")


def block_bytes(prg, bank, addr, length):
    o = rom_offset(addr, bank)
    return prg[o:o + length]


def lines(src):
    """Non-empty lines with comments removed (a ";" inside {...} or quotes is not special here)."""
    for line in src.splitlines():
        line = line.split(";", 1)[0].rstrip()
        if line.strip():
            yield line


def hexbytes(data, per_line=16):
    return "\n".join(" ".join(f"{b:02X}" for b in data[i:i + per_line]) for i in range(0, len(data), per_line))


def parse_hexbytes(src):
    return bytes(int(tok, 16) for line in lines(src) for tok in line.split())


# ---- raw -------------------------------------------------------------------------

def dump_raw(prg, bank, addr, length):
    return f"; {length} bytes from {bank}:{addr:04X}\n" + hexbytes(block_bytes(prg, bank, addr, length)) + "\n"


def parse_raw(src):
    return parse_hexbytes(src)


# ---- tiles as text (names, layouts) ----------------------------------------------

CHAR_TILE = {c: t for t, c in text.TILES.items() if t != 0xB0}  # ' ' is $7E; $B0 is written {B0}


def tiles_to_text(tiles):
    out = []
    for t in tiles:
        c = text.TILES.get(t)
        out.append(c if c is not None and t != 0xB0 and c not in "{}" else f"{{{t:02X}}}")
    return "".join(out)


def text_to_tiles(s):
    out, pos = [], 0
    for m in TAG.finditer(s):
        out += [CHAR_TILE[c] for c in s[pos:m.start()]]
        out.append(int(m.group(1), 16))
        pos = m.end()
    return out + [CHAR_TILE[c] for c in s[pos:]]


# ---- names -----------------------------------------------------------------------

def dump_names(prg, bank, addr, length):
    data = block_bytes(prg, bank, addr, length)
    out = [f"; {length // 8} names of 8 tiles from {bank}:{addr:04X}; $FF padding left out"]
    for i in range(0, length, 8):
        entry = bytes(data[i:i + 8])
        body = entry.rstrip(b"\xFF")
        out.append(f"{i // 8:02X} " + tiles_to_text(body) if body else f"{i // 8:02X}")
    return "\n".join(out) + "\n"


def parse_names(src):
    out = bytearray()
    for line in src.splitlines():
        if not line.strip() or line.startswith(";"):
            continue
        tiles = text_to_tiles(line[3:]) if len(line) > 3 else []
        if len(tiles) > 8:
            raise ValueError(f"name longer than 8 tiles: {line!r}")
        out += bytes(tiles) + b"\xFF" * (8 - len(tiles))
    return bytes(out)


# ---- messages --------------------------------------------------------------------

def _code_text_maps(prg):
    tiles = [prg[rom_offset(text.TILE_TABLE + i, None)] for i in range(64)]
    printable = {}
    for c in list(range(0, 27)) + list(range(0x20, 0x3B)):
        ch = text.TILES.get(tiles[c])
        if ch and not (ch == " " and c != 0x1A) and ch not in "{}":
            printable[c] = ch
    return printable, {ch: c for c, ch in printable.items()}


CONTROL_TAGS = {text.WAIT: "WAIT", text.WAIT_CLEAR: "WAITCLEAR", text.NEWLINE: "N", text.END: "END",
                text.WAIT_END: "WAITEND", text.CLEAR: "CLEAR"}
ARG8_TAGS = {text.ITEM_NAME: "ITEM", text.WORD: "WORD"}
ARG16_TAGS = {text.NUMBER: "NUMBER", text.ITEM_NAME_AT: "ITEMAT"}


def _message_codes(prg, bank, addr):
    """[(code, arg)] of one message, read like the printer reads it, and its length."""
    m = text.Message(text.Rom(prg), bank, addr)
    out = []
    while True:
        c = m.code()
        if c in ARG8_TAGS:
            out.append((c, m.byte8()))
        elif c in ARG16_TAGS:
            out.append((c, m.inline_word()))
            m.count = 0
        else:
            out.append((c, None))
        if c in (text.END, text.WAIT_END):
            return out, m.ptr - addr


def message_starts(prg, bank):
    for b, base, n in text.MESSAGE_TABLES:
        if b == bank:
            return sorted(set(text.message_table(text.Rom(prg), b, base, n)))
    raise KeyError(bank)


def dump_messages(prg, bank, addr, length):
    printable, _ = _code_text_maps(prg)
    out = [f"; messages {bank}:{addr:04X}-{addr + length - 1:04X}: text, {{N}} new line, {{WAIT}}, "
           "{WAITCLEAR}, {CLEAR}, {END}, {WAITEND}, {ITEM nn}, {WORD nn}, {NUMBER $addr}, "
           "{ITEMAT $addr}, {Cnn} other codes"]
    pos = addr
    for start in [s for s in message_starts(prg, bank) if addr <= s < addr + length]:
        if start != pos:
            raise ValueError(f"gap before message {bank}:{start:04X}")
        codes, n = _message_codes(prg, bank, start)
        s = []
        for c, arg in codes:
            if c in CONTROL_TAGS:
                s.append(f"{{{CONTROL_TAGS[c]}}}")
            elif c in ARG8_TAGS:
                s.append(f"{{{ARG8_TAGS[c]} {arg:02X}}}")
            elif c in ARG16_TAGS:
                s.append(f"{{{ARG16_TAGS[c]} ${arg:04X}}}")
            elif c in printable:
                s.append(printable[c])
            else:
                s.append(f"{{C{c:02X}}}")
        out += [f"@{start:04X}", "".join(s)]
        pos = start + n
    if pos != addr + length:
        raise ValueError("messages don't fill the block")
    return "\n".join(out) + "\n"


class BitWriter:
    def __init__(self):
        self.out, self.acc, self.n = bytearray(), 0, 0

    def bits(self, value, count):
        for i in range(count - 1, -1, -1):
            self.acc = self.acc << 1 | (value >> i & 1)
            self.n += 1
            if self.n == 8:
                self.out.append(self.acc)
                self.acc = self.n = 0

    def flush(self):
        if self.n:
            self.bits(0, 8 - self.n)


def make_parse_messages(prg):
    """parse() needs the ROM's code table (to turn codes into bit patterns) and tile table."""
    code_table = [prg[rom_offset(text.CODE_TABLE + i, None)] for i in range(64)]
    _, from_char = _code_text_maps(prg)
    tag_codes = {v: k for k, v in {**CONTROL_TAGS, **ARG8_TAGS, **ARG16_TAGS}.items()}

    def put_code(w, c):
        index = code_table.index(c)
        w.bits(0, index >> 3)
        w.bits(1, 1)
        w.bits(index & 7, 3)

    def parse(src):
        out = bytearray()
        for line in src.splitlines():
            if not line or line.startswith((";", "@")):
                continue
            w, pos = BitWriter(), 0
            tokens = []  # ("char", c) or ("tag", text inside the braces)
            for m in TAG.finditer(line):
                tokens += [("char", c) for c in line[pos:m.start()]] + [("tag", m.group(1))]
                pos = m.end()
            tokens += [("char", c) for c in line[pos:]]
            for kind, tok in tokens:
                if kind == "char":
                    put_code(w, from_char[tok])
                    continue
                name, _, arg = tok.partition(" ")
                if name.startswith("C") and len(name) == 3 and not arg:
                    put_code(w, int(name[1:], 16))
                    continue
                c = tag_codes[name]
                put_code(w, c)
                if c in ARG8_TAGS:
                    w.bits(int(arg, 16), 8)
                elif c in ARG16_TAGS:
                    w.flush()
                    v = int(arg.lstrip("$"), 16)
                    w.out += bytes((v & 0xFF, v >> 8))
            w.flush()
            out += w.out
        return bytes(out)
    return parse


# ---- scripts ---------------------------------------------------------------------

def _mnemonics():
    names = dict(scripts_mod.NAMES)
    taken = {}
    for op, n in sorted(names.items()):
        taken.setdefault(n, op)
    unique = {op: n for op, n in names.items() if taken[n] == op}
    return unique, {n: op for op, n in unique.items()}


def dump_scripts(prg, bank, addr, length):
    s = scripts_mod.load(prg)
    unique, _ = _mnemonics()
    out = [f"; scripts {bank}:{addr:04X}-{addr + length - 1:04X}: \"ADDR: mnemonic args\"; op_XX for "
           "opcodes without a unique name; $XX byte and $XXXX word arguments"]
    pos = addr
    for a in sorted(x for x in s.ops if addr <= x < addr + length):
        if a != pos:
            raise ValueError(f"bytes at {bank}:{pos:04X} are not opcodes")
        op, n = s.ops[a]
        args = block_bytes(prg, bank, a + 1, n - 1)
        name = "end" if op == scripts_mod.END else unique.get(op, f"op_{op:02X}")
        if op in scripts_mod.GOTO_IF or op in scripts_mod.GOTO:
            argtext = f"${word(prg, rom_offset(a + 1, bank)):04X}"
        elif op == scripts_mod.SWITCH:
            argtext = " ".join(f"${args[i]:02X} ${args[i + 1]:02X} ${args[i + 2] | args[i + 3] << 8:04X}"
                               for i in range(0, len(args), 4))
        else:
            argtext = " ".join(f"${b:02X}" for b in args)
        if a in s.starts:
            out.append(f"; script {', '.join(s.starts[a][:6])}{' ...' if len(s.starts[a]) > 6 else ''}")
        out.append(f"{a:04X}: {name} {argtext}".rstrip())
        pos = a + n
    if pos != addr + length:
        raise ValueError("opcodes don't fill the block")
    return "\n".join(out) + "\n"


def parse_scripts(src):
    _, by_name = _mnemonics()
    out = bytearray()
    for line in lines(src):
        _, _, rest = line.partition(":")
        toks = rest.split()
        name, args = toks[0], toks[1:]
        if name == "end":
            op = scripts_mod.END
        elif name.startswith("op_"):
            op = int(name[3:5], 16)
        else:
            op = by_name[name]
        out.append(op)
        for a in args:
            v = int(a.lstrip("$"), 16)
            out += bytes((v & 0xFF, v >> 8)) if len(a) == 5 else bytes((v,))
    return bytes(out)


# ---- scenes ----------------------------------------------------------------------

def dump_scenes(prg, bank, addr, length):
    first = next(f for b, f, _ in scenes_mod.BANKS if b == bank)
    entries = scenes_mod.pointer_table(prg, bank)
    out = [f"; scene pictures {bank}:{addr:04X}: 14 rows of 14 tiles, then 16 attribute bytes"]
    pos = addr
    for i, a in sorted(enumerate(entries), key=lambda e: e[1]):
        if a != pos:
            raise ValueError(f"gap before picture at {bank}:{a:04X}")
        rec = block_bytes(prg, bank, a, scenes_mod.RECORD)
        out.append(f"@{a:04X} scene {first + i:02X}")
        out += [" ".join(f"{t:02X}" for t in rec[r * 14:(r + 1) * 14]) for r in range(14)]
        out.append("attrs " + " ".join(f"{t:02X}" for t in rec[196:]))
        pos = a + scenes_mod.RECORD
    return "\n".join(out) + "\n"


def parse_scenes(src):
    return bytes(int(t, 16) for line in lines(src) if not line.startswith("@")
                 for t in line.replace("attrs", "").split())


# ---- hotspots --------------------------------------------------------------------

def dump_hotspots(prg, bank, addr, length):
    entries, lists = rooms.load(prg)
    out = [f"; room hotspots {bank}:{addr:04X}: \"x1 x2 y1 y2 kind id\" per hotspot, \"end\" after each room"]
    pos = addr
    for a in sorted(lists):
        if a != pos:
            raise ValueError(f"gap before hotspot list {bank}:{a:04X}")
        out.append(f"@{a:04X} room" + ("s" if entries.count(a) > 1 else "") + " "
                   + " ".join(f"{i:02X}" for i, e in enumerate(entries) if e == a))
        for r in range(lists[a]):
            out.append(" ".join(f"{b:02X}" for b in block_bytes(prg, bank, a + 6 * r, 6)))
        out.append("end")
        pos = a + 6 * lists[a] + 1
    return "\n".join(out) + "\n"


def parse_hotspots(src):
    out = bytearray()
    for line in lines(src):
        if line.startswith("@"):
            continue
        out += b"\xFF" if line.strip() == "end" else bytes(int(t, 16) for t in line.split())
    return bytes(out)


# ---- layouts ---------------------------------------------------------------------

def dump_layouts(prg, bank, addr, length):
    entries, found = layouts_mod.load(prg)
    out = [f"; screen layouts {bank}:{addr:04X}: text plus {{MOVE x y}}, {{ITEM nn}}, {{CMDnn $addr}}, {{xx}} tiles"]
    pos = addr
    for a in sorted(found):
        if a != pos:
            raise ValueError(f"gap before layout {bank}:{a:04X}")
        data = block_bytes(prg, bank, a, found[a])
        s, p = [], 0
        while data[p] != layouts_mod.END:
            c = data[p]
            n = layouts_mod.size(c)
            if c == 0x00:
                s.append(f"{{ITEM {data[p + 1]:02X}}}")
            elif c == 0x06:
                s.append(f"{{MOVE {data[p + 1]:02X} {data[p + 2]:02X}}}")
            elif n == 3:
                s.append(f"{{CMD{c:02X} ${data[p + 1] | data[p + 2] << 8:04X}}}")
            else:
                s.append(tiles_to_text([c]))
            p += n
        out += [f"@{a:04X}", "".join(s)]
        pos = a + found[a]
    return "\n".join(out) + "\n"


def parse_layouts(src):
    out = bytearray()
    for line in src.splitlines():
        if not line or line.startswith((";", "@")):
            continue
        pos = 0
        for m in TAG.finditer(line):
            out += bytes(text_to_tiles(line[pos:m.start()]))
            t = m.group(1).split()
            if t[0] == "ITEM":
                out += bytes((0x00, int(t[1], 16)))
            elif t[0] == "MOVE":
                out += bytes((0x06, int(t[1], 16), int(t[2], 16)))
            elif t[0].startswith("CMD"):
                v = int(t[1].lstrip("$"), 16)
                out += bytes((int(t[0][3:], 16), v & 0xFF, v >> 8))
            else:
                out.append(int(t[0], 16))
            pos = m.end()
        out += bytes(text_to_tiles(line[pos:])) + bytes((layouts_mod.END,))
    return bytes(out)


# ---- sound -----------------------------------------------------------------------

STREAM_WORDS = {0xE0: "inst", 0xE1: "inst1", 0xE2: "sweep", 0xE3: "env"}


def _stream_tokens(data):
    toks, p = [], 0
    while True:
        c = data[p]
        if c == 0xFF:
            return toks + ["end"], p + 1
        if c >= 0xE0:
            toks.append(f"{STREAM_WORDS.get(c, f'cmd{c:02X}')} ${data[p + 1] | data[p + 2] << 8:04X}")
            p += 3
            continue
        toks.append("rest" if c == 0x7F else "wait" if c == 0x7E else f"n{c:02X}" if c < 0x80
                    else f"len{c & 0x3F:02X}" if c < 0xC0 else "endloop" if c == 0xCF
                    else f"loop{c & 0x0F:X}" if c < 0xD0 else f"break{c & 0x0F:X}")
        p += 1


def dump_sound(prg, bank, addr, length):
    s = sound_mod.load(prg)
    data = block_bytes(prg, bank, addr, length)
    out = [f"; sound data {bank}:{addr:04X}-{addr + length - 1:04X}: headers, streams (nXX note, rest, wait, "
           "lenXX, loopN, endloop, breakN, inst/inst1/sweep/env/cmdXX $addr, end) and raw bytes"]
    pos = addr
    for a in sorted(s.items):
        kind, n, name = s.items[a]
        if a < pos:
            continue  # inside the item before it (an envelope within an instrument's bytes)
        if a > pos:
            out += [f"@{pos:04X} gap", hexbytes(data[pos - addr:a - addr])]
        chunk = data[a - addr:a - addr + n]
        if kind == "header":
            ptrs = " ".join(f"${chunk[i] | chunk[i + 1] << 8:04X}" for i in (1, 3, 5, 7))
            out += [f"@{a:04X} header {name}", f"type {chunk[0]:02X} ch {ptrs} spare {chunk[9]:02X} "
                    f"{chunk[10]:02X} speed {chunk[11]:02X}"]
        elif kind in ("stream", "unused_stream"):
            toks, used = _stream_tokens(chunk)
            if used != n:
                raise ValueError(f"stream {a:04X} length mismatch")
            out += [f"@{a:04X} {kind} {name}", " ".join(toks)]
        else:
            out += [f"@{a:04X} {kind} {name}", hexbytes(chunk)]
        pos = a + n
    if pos < addr + length:
        out += [f"@{pos:04X} gap", hexbytes(data[pos - addr:])]
    return "\n".join(out) + "\n"


def parse_sound(src):
    out, kind = bytearray(), None
    for line in lines(src):
        if line.startswith("@"):
            kind = line.split()[1]
            continue
        t = line.split()
        if kind == "header":
            # type TT ch $a $b $c $d spare XX YY speed ZZ
            out.append(int(t[1], 16))
            for x in t[3:7]:
                w = int(x.lstrip("$"), 16)
                out += bytes((w & 0xFF, w >> 8))
            out += bytes((int(t[8], 16), int(t[9], 16), int(t[11], 16)))
        elif kind in ("stream", "unused_stream"):
            i = 0
            while i < len(t):
                tok = t[i]
                words = {v: k for k, v in STREAM_WORDS.items()}
                if tok in words or tok.startswith("cmd"):
                    v = int(t[i + 1].lstrip("$"), 16)
                    out += bytes((words.get(tok) or int(tok[3:], 16), v & 0xFF, v >> 8))
                    i += 2
                    continue
                out.append(0xFF if tok == "end" else 0x7F if tok == "rest" else 0x7E if tok == "wait"
                           else 0xCF if tok == "endloop" else int(tok[1:], 16) if tok[0] == "n"
                           else 0x80 | int(tok[3:], 16) if tok.startswith("len")
                           else 0xC0 | int(tok[4:], 16) if tok.startswith("loop")
                           else 0xD0 | int(tok[5:], 16))
                i += 1
        else:
            out += bytes(int(x, 16) for x in t)
    return bytes(out)


def codec_for(name, prg):
    """(dump, parse) for a block of asm/bins.txt, chosen by its name."""
    if name.startswith("messages_"):
        return dump_messages, make_parse_messages(prg)
    if name.endswith("_names_data"):
        return dump_names, parse_names
    if name.startswith("scripts_"):
        return dump_scripts, parse_scripts
    if name.startswith("scenes_"):
        return dump_scenes, parse_scenes
    if name == "hotspots_data":
        return dump_hotspots, parse_hotspots
    if name == "layouts_data":
        return dump_layouts, parse_layouts
    if name == "sound_data":
        return dump_sound, parse_sound
    return dump_raw, parse_raw
