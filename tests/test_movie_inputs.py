"""Tests for tools/movie_inputs.py on synthetic movies, so they run in CI."""
import io
import sys
import unittest
import zipfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
import movie_inputs  # noqa: E402


class Fm2(unittest.TestCase):
    def test_buttons_map_to_udlrtsba(self):
        text = "version 3\nport0 1\n|1|........|........||\n|0|R..U...A|........||\n|0|...T....|||\n"
        frames, notes = movie_inputs.from_fm2(text)
        # FM2 columns are RLDUTSBA; "...T...." is Start in FM2's own order (column 3 = U)
        self.assertEqual(frames, ["........", "U..R...A", "U......."])
        self.assertEqual(notes, [])  # a reset on frame 0 is normal and not reported

    def test_later_reset_is_reported(self):
        frames, notes = movie_inputs.from_fm2("|0|........|||\n|1|........|||\n")
        self.assertEqual(notes, ["frame 1: command 1 ignored"])


class Bk2(unittest.TestCase):
    def make_bk2(self, lines):
        log = ("[Input]\nLogKey:#Power|Reset|#P1 Up|P1 Down|P1 Left|P1 Right|"
               "P1 Start|P1 Select|P1 B|P1 A|\n" + "".join(l + "\n" for l in lines) + "[/Input]\n")
        buf = io.BytesIO()
        with zipfile.ZipFile(buf, "w") as z:
            z.writestr("Input Log.txt", log)
        return buf.getvalue()

    def test_buttons_map_to_udlrtsba(self):
        frames, notes = movie_inputs.from_bk2(self.make_bk2(["|..|........|", "|..|U...S..A|"]))
        self.assertEqual(frames, ["........", "U...T..A"])
        self.assertEqual(notes, [])

    def test_power_is_reported(self):
        frames, notes = movie_inputs.from_bk2(self.make_bk2(["|P.|........|"]))
        self.assertEqual(notes, ["frame 0: power/reset ignored"])


if __name__ == "__main__":
    unittest.main()
