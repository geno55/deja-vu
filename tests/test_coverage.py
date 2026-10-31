"""Tests for tools/coverage.py that need no build or ROM, so they run in CI."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
import coverage  # noqa: E402
import scripts  # noqa: E402

LISTING = """ca65 V2.19
Main file   : asm/bank7.s

000000r 1               ; a comment
000000r 1                       .segment "BANK7"
000000r 2               nes_register := $2000
000000r 1  8A           change_scene:  txa
000001r 1  D0 02                bne     L0010
000003r 1  12 34                .byte   $12,$34
000005r 1  60           L0010:  rts
000006r 1  00 03 FF 00          .incbin "scripts.bin"
00000Ar 1  A9 01        other:
00000Ar 1  A9 01                lda     #$01 ; note
"""


class Listing(unittest.TestCase):
    def test_instructions(self):
        got = coverage.parse_listing(LISTING, 7)
        self.assertEqual([(i.offset - 7 * 0x4000, i.addr, i.routine, i.mnemonic) for i in got],
                         [(0, 0xC000, "change_scene", "txa"), (1, 0xC001, "change_scene", "bne"),
                          (5, 0xC005, "change_scene", "rts"), (0xA, 0xC00A, "other", "lda")])

    def test_switched_bank(self):
        self.assertEqual(coverage.parse_listing(LISTING, 2)[0].addr, 0x8000)


class Counts(unittest.TestCase):
    def test_code(self):
        body = coverage.parse_listing(LISTING, 0)
        flags = bytearray(0x20)
        flags[0] = coverage.RAN
        flags[1] = coverage.RAN | coverage.FELL
        c = coverage.code_counts(body, flags)
        self.assertEqual(c, {"run": 2, "all": 4, "branches": 1, "both": 0, "one": 1, "none": 0})
        flags[1] |= coverage.BRANCHED
        self.assertEqual(coverage.code_counts(body, flags)["both"], 1)

    def test_scripts(self):
        ops = {0: (0x8000, 0x04, 3), 3: (0x8003, 0x04, 3), 6: (0x8006, 0x00, 2), 8: (0x8008, scripts.END, 1)}
        flags = bytearray(16)
        flags[0] = coverage.SCRIPT_OP | coverage.SCRIPT_NEXT | coverage.SCRIPT_JUMP
        flags[3] = coverage.SCRIPT_OP | coverage.SCRIPT_NEXT
        c = coverage.script_counts(ops, flags)
        self.assertEqual((c["run"], c["all"], c["gotos"], c["both"], c["one"]), (2, 3, 2, 1, 1))
        self.assertEqual(c["unrun"], [6])
        self.assertEqual(set(c["numbers"]), {0x04})


if __name__ == "__main__":
    unittest.main()
