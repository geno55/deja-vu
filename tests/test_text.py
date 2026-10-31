"""Tests for tools/text.py on a synthetic ROM, so they run in CI."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
import text  # noqa: E402
from nes6502 import BANK_SIZE, rom_offset  # noqa: E402


def encode(codes, code_table):
    """Bit stream for character codes: run of 0s + 1, then 3 bits (inverse of the decoder)."""
    bits = ""
    for c in codes:
        index = code_table.index(c)
        bits += "0" * (index >> 3) + "1" + format(index & 7, "03b")
    bits += "0" * (-len(bits) % 8)
    return bytes(int(bits[i:i + 8], 2) for i in range(0, len(bits), 8))


class Decode(unittest.TestCase):
    def setUp(self):
        self.prg = bytearray(8 * BANK_SIZE)
        # Code table: index i -> character code i, so short codes go to A, B, C...
        self.code_table = list(range(64))
        o = rom_offset(text.CODE_TABLE, None)
        self.prg[o:o + 64] = bytes(self.code_table)
        tiles = [0xB1 + i for i in range(26)] + [0x7E] * 6 + [0xA1] + [0x7E] * 31
        o = rom_offset(text.TILE_TABLE, None)
        self.prg[o:o + 64] = bytes(tiles)

    def decode(self, data, bank=5, addr=0x9C00):
        o = rom_offset(addr, bank)
        self.prg[o:o + len(data)] = data
        m = text.Message(text.Rom(self.prg), bank, addr)
        return m.decode(), m.ptr - addr

    def test_letters_space_and_end(self):
        codes = [7, 8, 26, 0, 32, text.WAIT_END]  # "HI A." then end
        out, length = self.decode(encode(codes, self.code_table))
        self.assertEqual(out, "HI A.<wait>")
        self.assertEqual(length, len(encode(codes, self.code_table)))

    def test_long_lines_wrap_at_26(self):
        out, _ = self.decode(encode([0] * 27 + [text.END], self.code_table))
        self.assertEqual(out, "A" * 26 + "\n" + "A")

    def test_number_reads_inline_word_and_drops_rest_of_byte(self):
        head = encode([text.NUMBER], self.code_table)  # the code, padded to whole bytes
        tail = encode([1, text.END], self.code_table)
        out, length = self.decode(head + bytes([0x35, 0x05]) + tail)
        self.assertEqual(out, "<number at $0535>B")
        self.assertEqual(length, len(head) + 2 + len(tail))


if __name__ == "__main__":
    unittest.main()
