"""Tests for tools/check_rom.py that need no ROM, so they run in CI."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
import check_rom  # noqa: E402


def make_header(prg_units=8, chr_units=32, mapper=4, nes2=True):
    h = bytearray(b"NES\x1a" + bytes(12))
    h[4], h[5] = prg_units, chr_units
    h[6] = (mapper & 0x0F) << 4 | 0x02  # battery
    h[7] = (mapper & 0xF0) | (0x08 if nes2 else 0)
    return bytes(h)


class ParseHeader(unittest.TestCase):
    def test_deja_vu_layout(self):
        self.assertEqual(check_rom.parse_header(make_header()),
                         (check_rom.PRG_SIZE, check_rom.CHR_SIZE, 4))

    def test_plain_ines(self):
        self.assertEqual(check_rom.parse_header(make_header(nes2=False))[2], 4)

    def test_bad_magic(self):
        with self.assertRaises(ValueError):
            check_rom.parse_header(b"XYZ\x1a" + bytes(12))


class Check(unittest.TestCase):
    def test_right_shape_wrong_data(self):
        rom = make_header() + bytes(check_rom.PRG_SIZE + check_rom.CHR_SIZE)
        problems = check_rom.check(rom)
        self.assertEqual(len(problems), 1)
        self.assertIn("SHA-1", problems[0])

    def test_wrong_mapper_and_size(self):
        rom = make_header(prg_units=2, chr_units=1, mapper=0) + bytes(0x4000 * 2 + 0x2000)
        problems = check_rom.check(rom)
        self.assertTrue(any("mapper" in p for p in problems))
        self.assertTrue(any("PRG/CHR" in p for p in problems))

    def test_not_a_rom(self):
        self.assertEqual(check_rom.check(b"hello"), ["missing iNES header"])


if __name__ == "__main__":
    unittest.main()
