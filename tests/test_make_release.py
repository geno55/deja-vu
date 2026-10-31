"""Tests for tools/make_release.py that need no ROM or build, so they run in CI."""
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
import make_release  # noqa: E402

GOOD_EXE = b"MZ imports SDL2.dll and KERNEL32.dll"


class CheckExe(unittest.TestCase):
    def test_good(self):
        make_release.check_exe(GOOD_EXE)

    def test_no_window(self):
        with self.assertRaises(ValueError):
            make_release.check_exe(b"MZ imports KERNEL32.dll")

    def test_runtime_dll(self):
        with self.assertRaises(ValueError):
            make_release.check_exe(GOOD_EXE + b" VCRUNTIME140.dll")

    def test_rom_bytes(self):
        rom = bytes(range(200))
        with self.assertRaises(ValueError):
            make_release.check_exe(GOOD_EXE + rom[50:100], rom)

    def test_short_or_plain_runs_pass(self):
        rom = bytes(range(200)) + bytes(100)
        make_release.check_exe(GOOD_EXE + rom[50:50 + make_release.ROM_RUN - 1], rom)
        make_release.check_exe(GOOD_EXE + bytes(64), rom)


class Zip(unittest.TestCase):
    def make_tree(self, root):
        (root / "build/Release").mkdir(parents=True)
        (root / "tools").mkdir()
        (root / "external/SDL2-2.0").mkdir(parents=True)
        for name in ("build/Release/dejavu.exe", "build/Release/SDL2.dll", "tools/release_readme.txt",
                     "LICENSE", "external/SDL2-2.0/LICENSE.txt", "build/Release/dejavu_assets.dat",
                     "build/Release/dejavu.sav"):
            (root / name).write_bytes(b"x")

    def test_only_the_list_goes_in(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            self.make_tree(root)
            out = root / "out.zip"
            make_release.make_zip(out, make_release.release_files(root / "build", root))
            with zipfile.ZipFile(out) as z:
                self.assertEqual(sorted(z.namelist()), ["LICENSE-SDL2.txt", "LICENSE.txt", "README.txt",
                                                        "SDL2.dll", "dejavu.exe"])

    def test_missing_file(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            self.make_tree(root)
            (root / "build/Release/SDL2.dll").unlink()
            with self.assertRaises(ValueError):
                make_release.release_files(root / "build", root)


if __name__ == "__main__":
    unittest.main()
