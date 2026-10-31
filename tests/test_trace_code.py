"""Tests for tools/trace_code.py on a synthetic PRG ROM, so they run in CI."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
import trace_code  # noqa: E402
from nes6502 import BANK_SIZE, rom_offset  # noqa: E402

FIXED = 7 * BANK_SIZE


def make_prg(code, at=0xC000):
    """PRG filled with an invalid opcode, `code` at `at`, all vectors pointing to it."""
    prg = bytearray([0x02]) * (8 * BANK_SIZE)
    o = rom_offset(at, None)
    prg[o:o + len(code)] = code
    for vec in (0xFFFA, 0xFFFC, 0xFFFE):
        v = rom_offset(vec, None)
        prg[v], prg[v + 1] = at & 0xFF, at >> 8
    return prg


def flags_at(t, addr, n=1):
    o = rom_offset(addr, None)
    return list(t.flags[o:o + n])


class Trace(unittest.TestCase):
    def test_constant_branch_does_not_fall_through(self):
        # LDA #0 / STA $10 / BEQ +1 / <data byte> / RTS
        t, report = trace_code.trace(make_prg([0xA9, 0x00, 0x85, 0x10, 0xF0, 0x01, 0x02, 0x60]))
        self.assertEqual(report["problems"], [])
        self.assertFalse(flags_at(t, 0xC006)[0] & trace_code.FLAG_CODE)
        self.assertTrue(flags_at(t, 0xC007)[0] & trace_code.FLAG_OPSTART)

    def test_chr_inline_bytes_are_data_and_code_continues(self):
        code = [0x20, 0x16, 0xE9] + [0x02] * 8 + [0x60]  # JSR $E916, 8 bytes, RTS
        t, report = trace_code.trace(make_prg(code))
        self.assertTrue(all(f & trace_code.FLAG_INLINE for f in flags_at(t, 0xC003, 8)))
        self.assertTrue(flags_at(t, 0xC00B)[0] & trace_code.FLAG_OPSTART)
        self.assertEqual(report["chr_bank_inline_data"], ["7:C003"])

    def test_rle_screen_length(self):
        # JSR $E9D5, then: literal 01, FF 05 03 (run), FF FF 02 (run of FF), FF FF 00 (end)
        rle = [0x01, 0xFF, 0x05, 0x03, 0xFF, 0xFF, 0x02, 0xFF, 0xFF, 0x00]
        t, report = trace_code.trace(make_prg([0x20, 0xD5, 0xE9] + rle + [0x60]))
        self.assertEqual(report["rle_screens"], [{"start": "7:C003", "length": len(rle)}])
        self.assertFalse(flags_at(t, 0xC003 + len(rle))[0] & trace_code.FLAG_CODE)

    def test_jump_table_entries_are_traced(self):
        # JSR $EBE6 followed by two addresses, each pointing to an RTS
        code = [0x20, 0xE6, 0xEB, 0x07, 0xC0, 0x08, 0xC0, 0x60, 0x60]
        t, report = trace_code.trace(make_prg(code))
        self.assertEqual(report["jump_tables"][0]["entries"], ["7:C007", "7:C008"])
        self.assertTrue(all(f & trace_code.FLAG_ADDRTAB for f in flags_at(t, 0xC003, 4)))

    def test_far_call_resolves_through_bank_table(self):
        # LDA #$62 / JSR $F3B5: bank 6, entry 2 of the table at $8000 -> $8123
        prg = make_prg([0xA9, 0x62, 0x20, 0xB5, 0xF3, 0x60])
        entry = 6 * BANK_SIZE + 4
        prg[entry], prg[entry + 1] = 0x23, 0x81
        prg[6 * BANK_SIZE + 0x123] = 0x60
        t, report = trace_code.trace(prg)
        self.assertEqual(report["far_calls"], {"8000:62": "6:8123"})
        self.assertEqual(report["labels"]["6:8123"], "far_8000_62")


class Unreferenced(unittest.TestCase):
    # Candidates start where a run of unclassified bytes starts: here right after reset's RTS.
    def test_code_that_calls_known_code_is_found(self):
        prg = make_prg([0x60, 0x20, 0x00, 0xC0, 0x60])  # RTS, then nothing calls: JSR $C000 / RTS
        t, _ = trace_code.trace(prg)
        self.assertEqual(t.find_unreferenced_code([7]), [rom_offset(0xC001, None)])
        self.assertTrue(flags_at(t, 0xC004)[0] & trace_code.FLAG_OPSTART)

    def test_self_contained_code_that_fills_its_run_is_found(self):
        # RTS, then 12 bytes nothing calls: LDA $10 / CLC / ADC #1 / STA $10 / BCC +2 / INC $11 / RTS
        code = [0xA5, 0x10, 0x18, 0x69, 0x01, 0x85, 0x10, 0x90, 0x02, 0xE6, 0x11, 0x60]
        prg = make_prg([0x60] + code)
        o = rom_offset(0xC001 + len(code), None)
        t, _ = trace_code.trace(prg)
        t.mark(o, 4, trace_code.FLAG_KNOWN)  # classified data right after, so the run ends with the code
        self.assertEqual(t.find_unreferenced_code([7]), [rom_offset(0xC001, None)])

    def test_data_is_left_alone(self):
        prg = make_prg([0x60, 0x10, 0x20, 0x30, 0x40])  # decodes, but calls nothing known and never ends
        t, _ = trace_code.trace(prg)
        self.assertEqual(t.find_unreferenced_code([7]), [])
        self.assertFalse(flags_at(t, 0xC001)[0] & trace_code.FLAG_CODE)


if __name__ == "__main__":
    unittest.main()
