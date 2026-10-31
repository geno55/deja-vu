"""Tests for tools/name_ram.py and tools/rename.py on temporary files."""
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
import name_ram  # noqa: E402
import rename  # noqa: E402

INC = """cursor_x          = $A5
script_ptr        = $B6   ; [2] next script byte
JOY2              = $4017 ; [read]
APU_FRAME         = $4017 ; [write]
MMC3_BANK_SELECT  = $8000 ; [write]
"""


class NameRam(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        inc = Path(self.tmp.name) / "x.inc"
        inc.write_text(INC)
        self.names = name_ram.read_names([inc])

    def tearDown(self):
        self.tmp.cleanup()

    def test_operands(self):
        r = lambda s: name_ram.rename_line(s, self.names)  # noqa: E731
        self.assertEqual(r("        lda     $A5"), "        lda     cursor_x")
        self.assertEqual(r("label:  lda     ($B6),y"), "label:  lda     (script_ptr),y")
        self.assertEqual(r("        sta     $B7"), "        sta     script_ptr+1")
        self.assertEqual(r("        lda     #$A5"), "        lda     #$A5")  # immediate: not an address
        self.assertEqual(r("        lda     $A7"), "        lda     $A7")    # not named

    def test_read_and_write_names(self):
        r = lambda s: name_ram.rename_line(s, self.names)  # noqa: E731
        self.assertEqual(r("        lda     $4017"), "        lda     JOY2")
        self.assertEqual(r("        sta     $4017"), "        sta     APU_FRAME")
        self.assertEqual(r("        sta     $8000"), "        sta     MMC3_BANK_SELECT")
        self.assertEqual(r("        lda     $8000,y"), "        lda     $8000,y")  # a ROM table read


class Rename(unittest.TestCase):
    def test_whole_words_and_refusals(self):
        with tempfile.TemporaryDirectory() as d:
            f = Path(d) / "a.s"
            f.write_text("LC000:  jsr     LC0001\n        jmp     LC000\n")
            rename.rename_all([("LC000", "start")], [f])
            self.assertEqual(f.read_text(), "start:  jsr     LC0001\n        jmp     start\n")
            with self.assertRaises(SystemExit):
                rename.rename_all([("LC0001", "start")], [f])  # "start" is taken
            with self.assertRaises(SystemExit):
                rename.rename_all([("missing", "other")], [f])


if __name__ == "__main__":
    unittest.main()
