"""Tests for tools/make_asm.py and tools/extract_bins.py that need no ROM or cc65."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
import extract_bins  # noqa: E402
import make_asm  # noqa: E402
from nes6502 import BANK_SIZE  # noqa: E402

# A trimmed da65 listing (COMMENTS 4) of bank 5: a table, then a skipped blob at $8004,
# then a labelled routine after it.
RAW = """; da65 V2.19
; Created:    2026-10-29 14:19:02

        .setcpu "6502"

; ----------------------------------------------------------------------------
; ----------------------------------------------------------------------------
table:  .addr   msg_a                           ; 8000 04 80                    ..
        .addr   msg_b                           ; 8002 06 80                    ..
; after the blob
after:
        .byte   $FF                             ; 8008 FF                       .
        .byte   $FF                             ; 8009 FF                       .
        rts                                     ; 800A 60                       `
"""


class Postprocess(unittest.TestCase):
    def setUp(self):
        blob = (5, 0x8004, 0x8008, "msgs_data", "messages")
        names = {(5, 0x8004): ("msg_a", ""), (5, 0x8006): ("msg_b", ""), (5, 0x8000): ("table", "")}
        self.out = make_asm.postprocess(RAW, 5, [blob], names).splitlines()

    def test_header_and_segment(self):
        self.assertTrue(self.out[0].startswith("; Deja Vu (USA), NES: PRG bank 5"))
        self.assertIn('        .segment "BANK5"', self.out)
        self.assertFalse(any("Created:" in line for line in self.out))

    def test_blob_goes_before_the_next_items_label_and_comment(self):
        incbin = self.out.index('        .incbin "msgs_data.bin"')
        self.assertLess(incbin, self.out.index("; after the blob"))
        self.assertLess(incbin, self.out.index("after:"))
        self.assertIn("msg_b           := msgs_data + $0002", self.out)

    def test_address_comments_removed_and_bytes_merged(self):
        self.assertIn("table:  .addr   msg_a", self.out)
        self.assertIn("        .byte   $FF,$FF", self.out)
        self.assertEqual(sum(line.startswith("; ---") for line in self.out), 1)


class Manifest(unittest.TestCase):
    def test_read_and_cut(self):
        path = Path(self.id() + ".txt")
        try:
            path.write_text("# comment\nmsgs 5 8004 4 abc\nchr chr 0010 2 def\n")
            self.assertEqual(extract_bins.read_manifest(path),
                             [("msgs", 5, 0x8004, 4, "abc"), ("chr", "chr", 0x10, 2, "def")])
        finally:
            path.unlink()
        body = bytes(range(256)) * ((8 * BANK_SIZE + 0x100) // 256)
        self.assertEqual(extract_bins.piece(body, 5, 0x8004, 4), body[5 * BANK_SIZE + 4:5 * BANK_SIZE + 8])
        self.assertEqual(extract_bins.piece(body, "chr", 0x10, 2), body[8 * BANK_SIZE + 0x10:8 * BANK_SIZE + 0x12])


if __name__ == "__main__":
    unittest.main()
