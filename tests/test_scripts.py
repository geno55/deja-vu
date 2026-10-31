"""Tests for tools/scripts.py on a synthetic bank 3, so they run in CI."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
import scripts  # noqa: E402
from nes6502 import BANK_SIZE, rom_offset  # noqa: E402


def build(script, at=0x8100):
    """Bank 3 with 6 top entries -> one $AB table -> one $BC table of 4 scripts, all `script`."""
    prg = bytearray([0xFF]) * (8 * BANK_SIZE)

    def put(addr, data):
        o = rom_offset(addr, scripts.BANK)
        prg[o:o + len(data)] = bytes(data)

    put(0x8000, [0x0C, 0x80] * 6)            # top table -> $800C
    put(0x800C, [0x0E, 0x80])                # $AB table: one entry -> $800E
    put(0x800E, [at & 0xFF, at >> 8] * 4)    # $BC table: 4 scripts
    put(at, script)
    return prg


class Walk(unittest.TestCase):
    def test_tables_and_plain_script(self):
        s = scripts.load(build([0x16, 0x05, 0x07, 0xFF]))  # test_0490 5, test_0450[ab], end
        self.assertEqual(sorted(s.ops), [0x8100, 0x8102, 0x8103])
        # every $AA table points to the same $AB table, so all 6 x 4 entries name this script
        self.assertEqual(len(s.starts[0x8100]), 24)
        self.assertEqual(s.starts[0x8100][0], "0.00.0")
        self.assertEqual(s.problems, [])

    def test_goto_if_follows_both_paths(self):
        # goto_if $8108; message 1; end; (gap) $8108: end
        s = scripts.load(build([0x04, 0x08, 0x81, 0x00, 0x01, 0xFF, 0xEE, 0xEE, 0xFF]))
        self.assertIn(0x8103, s.ops)
        self.assertIn(0x8108, s.ops)
        self.assertNotIn(0x8106, s.ops)

    def test_yield_falls_through(self):
        s = scripts.load(build([0x28, 0x00, 0x02, 0xFF]))
        self.assertEqual(sorted(s.ops), [0x8100, 0x8101, 0x8103])

    def test_switch_cases_and_default(self):
        # switch: (1,2 -> $810E), (FE,00 -> $810F); bytes in between are not code
        s = scripts.load(build([0x29, 0x01, 0x02, 0x0E, 0x81, 0xFE, 0x00, 0x0F, 0x81,
                                0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF]))
        self.assertEqual(s.ops[0x8100], (0x29, 9))
        self.assertIn(0x810E, s.ops)
        self.assertIn(0x810F, s.ops)
        self.assertNotIn(0x8109, s.ops)

    def test_trace_check(self):
        s = scripts.load(build([0x07, 0xFF]))
        log = Path(self.id() + ".log")
        try:
            log.write_text("S 1 00 00 00\nO 3 8100 07\nO 3 8101 FF\nO 3 8105 07\n")
            self.assertEqual(s.check_trace(log), [(3, 0x8105, 0x07)])
        finally:
            log.unlink()


if __name__ == "__main__":
    unittest.main()
