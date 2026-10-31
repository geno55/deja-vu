"""Tests for tools/scenes.py on a synthetic ROM, so they run in CI."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
import scenes  # noqa: E402
from nes6502 import BANK_SIZE, rom_offset  # noqa: E402


def build(counts=(3, 2, 1)):
    """Banks 0-2 each with a pointer table and `count` pictures; picture n is filled with n."""
    prg = bytearray(8 * BANK_SIZE)
    for (bank, first, _), count in zip(scenes.BANKS, counts):
        start = 0x8000 + 2 * count
        for i in range(count):
            addr = start + i * scenes.RECORD
            prg[rom_offset(0x8000 + 2 * i, bank)] = addr & 0xFF
            prg[rom_offset(0x8000 + 2 * i, bank) + 1] = addr >> 8
            o = rom_offset(addr, bank)
            prg[o:o + scenes.RECORD] = bytes([first + i]) * scenes.RECORD
    return prg


class Load(unittest.TestCase):
    def test_banks_and_numbering(self):
        s = scenes.load(build())
        self.assertEqual(sorted(s), [0x00, 0x01, 0x02, 0x4C, 0x4D, 0x98])
        self.assertEqual(s[0x4D]["bank"], 1)
        self.assertEqual(s[0x4D]["tiles"], bytes([0x4D]) * 196)
        self.assertEqual(len(s[0x98]["attrs"]), 16)

    def test_bank_for(self):
        self.assertEqual(scenes.bank_for(0x4B), (0, 0x4B))
        self.assertEqual(scenes.bank_for(0x4C), (1, 0))
        self.assertEqual(scenes.bank_for(0x99), (2, 1))

    def test_check_against_a_log(self):
        s = scenes.load(build())
        good = "P 01 " + "01" * 196 + " " + "01" * 16
        dark = "P 02 " + "FF" * 196 + " " + "00" * 16
        off = "P 4C " + "4C" * 195 + "00 " + "4C" * 16
        log = Path(self.id() + ".log")
        try:
            log.write_text("\n".join((good, dark, off)) + "\n")
            self.assertEqual(scenes.check(s, log), [(1, 0, 0), (2, None, None), (0x4C, 1, 0)])
        finally:
            log.unlink()


class Rooms(unittest.TestCase):
    def test_hotspots_and_item_lists(self):
        import rooms
        prg = bytearray([0xFF]) * (8 * BANK_SIZE)

        def put(addr, data):
            o = rom_offset(addr, rooms.BANK)
            prg[o:o + len(data)] = bytes(data)

        put(rooms.TABLE, [0x04, 0x9C, 0x10, 0x9C])           # 2 rooms
        put(0x9C04, [0, 8, 0, 8, 0x83, 5] + [8, 16, 0, 8, 1, 6])  # room 0: 2 hotspots, then $FF
        put(0x9C10, [0xFF])                                   # room 1: none
        put(rooms.ITEM_LISTS, [0x04, 0xA4, 0x08, 0xA4])       # entry 0 -> $A404, entry 1 -> $A408
        put(0xA404, [0x06, 0xA4])                             # one list at $A406
        put(0xA406, [0x01, 0xFF])
        put(0xA408, [0x0A, 0xA4])                             # one list at $A40A
        put(0xA40A, [0xFF])
        entries, lists = rooms.load(prg)
        self.assertEqual(entries, [0x9C04, 0x9C10])
        self.assertEqual(lists, {0x9C04: 2, 0x9C10: 0})
        tables, item_lists = rooms.load_item_lists(prg)
        self.assertEqual(item_lists, {0xA406: 2, 0xA40A: 1})


if __name__ == "__main__":
    unittest.main()
