"""Round-trip tests for tools/formats.py and tools/chr_png.py on synthetic data (no ROM)."""
import random
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
import chr_png  # noqa: E402
import formats  # noqa: E402
import text  # noqa: E402
from nes6502 import BANK_SIZE, rom_offset  # noqa: E402
from test_scripts import build as build_scripts  # noqa: E402


def put(prg, bank, addr, data):
    o = rom_offset(addr, bank)
    prg[o:o + len(data)] = bytes(data)


class Messages(unittest.TestCase):
    def setUp(self):
        self.prg = bytearray(8 * BANK_SIZE)
        put(self.prg, 7, text.CODE_TABLE, range(64))  # index i -> code i
        tiles = [0xB1 + i for i in range(26)] + [0x7E] * 6 + [0xA1, 0xA4] + [0x7E] * 14 + \
                [0x80 + i for i in range(10)] + [0x7E] * 6
        put(self.prg, 7, text.TILE_TABLE, tiles)

    def test_encode_matches_a_hand_built_stream_and_decodes_back(self):
        parse = formats.make_parse_messages(self.prg)
        data = parse("@9C00\nHI {NUMBER $0535}.{N}{WAITEND}\n")
        put(self.prg, 5, 0x9C00, data)
        codes, length = formats._message_codes(self.prg, 5, 0x9C00)
        self.assertEqual(length, len(data))
        self.assertEqual([c for c, _ in codes], [7, 8, 26, text.NUMBER, 32, text.NEWLINE, text.WAIT_END])
        self.assertEqual(codes[3][1], 0x0535)


class Names(unittest.TestCase):
    def test_round_trip(self):
        prg = bytearray([0xFF]) * (8 * BANK_SIZE)
        put(prg, 2, 0x8D00, [0xB3, 0xBF, 0xB9, 0xBE, 0xFF, 0xFF, 0xFF, 0xFF] + [0xFF] * 8
            + [0xC4, 0xB0, 0xCC, 0xFF, 0x82, 0xFF, 0xFF, 0xFF])
        dump = formats.dump_names(prg, 2, 0x8D00, 24)
        self.assertIn("00 COIN", dump)
        self.assertEqual(formats.parse_names(dump), bytes(prg[rom_offset(0x8D00, 2):rom_offset(0x8D00, 2) + 24]))


class Scripts(unittest.TestCase):
    def test_round_trip(self):
        prg = build_scripts([0x04, 0x08, 0x81, 0x00, 0x01, 0xFF, 0x29, 0x01, 0x02, 0x0E, 0x81,
                             0xFE, 0x00, 0x0F, 0x81, 0xFF])
        # opcodes at $8100-$8105 and the switch at $8106: dump just the first, contiguous run
        dump = formats.dump_scripts(prg, 3, 0x8100, 6)
        self.assertIn("goto_if $8108", dump)
        self.assertEqual(formats.parse_scripts(dump), bytes([0x04, 0x08, 0x81, 0x00, 0x01, 0xFF]))


class SoundStreams(unittest.TestCase):
    def test_tokens_round_trip(self):
        stream = bytes([0xE0, 0x42, 0x8A, 0x85, 0x2A, 0x7F, 0x7E, 0xC3, 0x30, 0xD1, 0xCF, 0xE5, 0x00, 0x90, 0xFF])
        toks, used = formats._stream_tokens(stream)
        self.assertEqual(used, len(stream))
        src = "@8000 stream test\n" + " ".join(toks) + "\n"
        self.assertEqual(formats.parse_sound(src), stream)


class Layouts(unittest.TestCase):
    def test_round_trip(self):
        prg = bytearray([0xFF]) * (8 * BANK_SIZE)
        put(prg, 6, 0xBC00, [0x02, 0xBC])
        put(prg, 6, 0xBC02, [0x06, 0x06, 0x03, 0xB1, 0xB0, 0x00, 0x05, 0x12, 0x34, 0x05, 0xFF])
        dump = formats.dump_layouts(prg, 6, 0xBC02, 11)
        self.assertEqual(formats.parse_layouts(dump), bytes(prg[rom_offset(0xBC02, 6):rom_offset(0xBC02, 6) + 11]))


class ChrPng(unittest.TestCase):
    def test_round_trip(self):
        rng = random.Random(1)
        data = bytes(rng.randrange(256) for _ in range(16 * 40))  # 40 tiles: a partial last row
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / "t.png"
            chr_png.write_png(path, chr_png.tiles_to_pixels(data))
            self.assertEqual(chr_png.pixels_to_tiles(chr_png.read_png(path), 40), data)


if __name__ == "__main__":
    unittest.main()
