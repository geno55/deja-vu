"""Tests for tools/run_checks.py that need no build or ROM, so they run in CI."""
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
import run_checks  # noqa: E402


class Shards(unittest.TestCase):
    def test_every_routine_once(self):
        names = run_checks.ported_routines()
        self.assertGreater(len(names), 500)
        self.assertEqual(len(names), len(set(names)))
        groups = run_checks.shards(names, 7)
        self.assertEqual(sorted(n for g in groups for n in g), sorted(names))


class Output(unittest.TestCase):
    def test_a_difference_fails(self):
        for line in ("dv_harness: port checks: 76299 calls checked against the original, 3 differed",
                     "dv_harness: port fuzz: 100 samples compared, 5 skipped (the original waited), 2 differed; 1 routines",
                     "dv_harness: port fuzz through waits: 10 samples compared, 5 skipped (x), 1 differed; 0 routines"):
            self.assertIsNotNone(run_checks.job_failed(0, line), line)

    def test_clean_passes(self):
        text = ("dv_harness: port checks: 76299 calls checked against the original, 0 differed\n"
                "dv_harness: port fuzz: check_save_file    4159 compared    841 skipped    0 differed (0.5 s)\n"
                "dv_harness: port fuzz: 100 samples compared, 5 skipped (the original waited), 0 differed; 1 routines\n")
        self.assertIsNone(run_checks.job_failed(0, text))

    def test_exit_code_fails(self):
        self.assertIsNotNone(run_checks.job_failed(1, "no difference here"))

    def test_known_picture_differences_pass(self):
        text = ("dv_harness: the C side matched the reference at 34083 of 34083 NMIs compared (34083 in all)\n"
                "dv_harness: apu: 34083 NMIs compared, 0 differ (0 fields in all)\n"
                "dv_harness: picture: 12 frames had reads of PPU registers (glitches), 12 of $2007; "
                "of the 11 that differ, 11 had reads of $2007\n")
        self.assertIsNone(run_checks.job_failed(1, text))

    def test_other_picture_differences_fail(self):
        text = ("dv_harness: the C side matched the reference at 34083 of 34083 NMIs compared (34083 in all)\n"
                "dv_harness: picture: 12 frames had reads of PPU registers (glitches), 12 of $2007; "
                "of the 12 that differ, 11 had reads of $2007\n")
        self.assertIsNotNone(run_checks.job_failed(1, text))

    def test_memory_differences_fail_whatever_the_picture(self):
        text = ("dv_harness: the C side matched the reference at 34082 of 34083 NMIs compared (34083 in all)\n"
                "dv_harness: picture: of the 11 that differ, 11 had reads of $2007\n")
        self.assertIsNotNone(run_checks.job_failed(1, text))

    def test_sound_differences_fail(self):
        self.assertIsNotNone(run_checks.job_failed(0, "dv_harness: apu: 34083 NMIs compared, 2 differ (3 fields in all)"))

    def test_fuzz_totals(self):
        text = ("dv_harness: port fuzz: a    1 compared    1 skipped    0 differed (0.1 s)\n"
                "dv_harness: port fuzz: 100 samples compared, 5 skipped (x), 0 differed\n"
                "dv_harness: port fuzz through waits: 10 samples compared, 5 skipped (x), 0 differed\n")
        self.assertEqual(run_checks.fuzz_totals(text), (100, 10))


class Jobs(unittest.TestCase):
    def test_runs_and_reports_failures(self):
        with tempfile.TemporaryDirectory() as d:
            work = Path(d)
            jobs = [("good", [sys.executable, "-c", "print('fine')"]),
                    ("bad", [sys.executable, "-c", "import sys; print('no'); sys.exit(3)"])]
            results = {r[0]: r for r in run_checks.run_jobs(jobs, work, 2)}
            self.assertIsNone(results["good"][2])
            self.assertEqual(results["bad"][2], "exit 3")
            self.assertIn("fine", (work / "good.txt").read_text())


if __name__ == "__main__":
    unittest.main()
