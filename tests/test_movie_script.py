"""Tests for tools/movie_script.py."""
import sys
import unittest
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "tools"))
import movie_script  # noqa: E402


class Expand(unittest.TestCase):
    def test_steps_become_a_line_per_frame(self):
        self.assertEqual(movie_script.expand(".:2 A:1"), ["........", "........", ".......A"])

    def test_buttons_go_in_the_movie_order(self):
        self.assertEqual(movie_script.expand("AU:1"), ["U......A"])
        self.assertEqual(movie_script.expand("UDLRTSBA:1"), ["UDLRTSBA"])

    def test_comments_and_blank_lines_are_nothing(self):
        self.assertEqual(movie_script.expand("# a comment\n\nT:1  # another\n"), ["....T..."])

    def test_a_count_repeats_the_line(self):
        self.assertEqual(movie_script.expand("2x A:1 .:1"),
                         [".......A", "........", ".......A", "........"])

    def test_it_says_where_a_step_is_wrong(self):
        for bad in ("A", "A:x", "Q:1", "A:1 B"):
            with self.assertRaisesRegex(ValueError, "line 2"):
                movie_script.expand("# fine\n" + bad)

    def test_the_scripts_in_tests_movies_expand(self):
        scripts = sorted((REPO / "tests" / "movies").glob("*.moves"))
        self.assertTrue(scripts)
        for script in scripts:
            self.assertGreater(len(movie_script.expand(script.read_text())), 1000, script.name)


if __name__ == "__main__":
    unittest.main()
