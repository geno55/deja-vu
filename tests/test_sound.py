"""Tests for tools/sound.py and tools/layouts.py on a synthetic ROM, so they run in CI."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
import layouts  # noqa: E402
import sound  # noqa: E402
from nes6502 import BANK_SIZE, rom_offset  # noqa: E402


def put(prg, bank, addr, data):
    o = rom_offset(addr, bank)
    prg[o:o + len(data)] = bytes(data)


class Sound(unittest.TestCase):
    def test_header_streams_and_instrument(self):
        prg = bytearray(8 * BANK_SIZE)
        put(prg, 5, 0x8000, [0x02, 0x80])  # one sound, header at $8002
        # type 3: channels at offsets 3 and 7
        put(prg, 5, 0x8002, [3, 0, 0, 0x10, 0x80, 0, 0, 0x20, 0x80, 0, 0, 6])
        put(prg, 5, 0x8010, [0xE0, 0x30, 0x80, 0x85, 0x40, 0x7E, 0xC2, 0x41, 0xCF, 0xFF])
        put(prg, 5, 0x8020, [0x7F, 0xFF])
        s = sound.load(prg)
        self.assertEqual(s.problems, [])
        self.assertEqual(s.items[0x8002][:2], ("header", 12))
        self.assertEqual(s.items[0x8010][:2], ("stream", 10))
        self.assertEqual(s.items[0x8020][:2], ("stream", 2))
        kind, length, _ = s.items[0x8030]
        self.assertEqual(kind, "instrument")
        self.assertEqual(length, 4 + sound.ENVELOPE_MAX)  # nothing follows it, so the full envelope


class Layouts(unittest.TestCase):
    def test_command_sizes(self):
        prg = bytearray([0xFF]) * (8 * BANK_SIZE)
        put(prg, 6, layouts.TABLE, [0x02, 0xBC])
        # move to 6,3; tiles; item name 5; name at $0534; end
        put(prg, 6, 0xBC02, [0x06, 0x06, 0x03, 0xB1, 0xB2, 0x00, 0x05, 0x12, 0x34, 0x05, 0xFF])
        entries, found = layouts.load(prg)
        self.assertEqual(entries, [0xBC02])
        self.assertEqual(found, {0xBC02: 11})


if __name__ == "__main__":
    unittest.main()
