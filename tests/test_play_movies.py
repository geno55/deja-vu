"""Tests for tests/run_play_movies.py that need no build or ROM, so they run in CI."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import run_play_movies as plays  # noqa: E402


def state():
    return {name: bytes(size) for name, size in plays.LAYOUT}


def changed(s, name, at):
    data = bytearray(s[name])
    data[at] ^= 1
    return dict(s, **{name: bytes(data)})


class Compare(unittest.TestCase):
    def test_split(self):
        s = plays.split_state(bytes(sum(size for _, size in plays.LAYOUT)))
        self.assertEqual(len(s["RAM"]), 0x800)
        self.assertEqual(len(s["OAM"]), 0x100)
        with self.assertRaises(ValueError):
            plays.split_state(bytes(10))

    def test_the_same(self):
        self.assertEqual(plays.differences(state(), state()), ([], []))

    def test_what_must_match(self):
        for name, at in (("save RAM", 5), ("nametables", 5), ("palette", 5), ("OAM", 5), ("RAM", 0x514), ("RAM", 0xA5)):
            bad, other = plays.differences(changed(state(), name, at), state())
            self.assertEqual(len(bad), 1, (name, at))
            self.assertEqual(other, [])

    def test_other_ram_only_noted(self):
        self.assertEqual(plays.differences(changed(state(), "RAM", 0x25), state()), ([], [0x25]))

    def test_stack_left_out(self):
        self.assertEqual(plays.differences(changed(state(), "RAM", 0x1F0), state()), ([], []))


if __name__ == "__main__":
    unittest.main()
