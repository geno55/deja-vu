#!/usr/bin/env python3
"""Run the whole set of checks on the port at once: the tests, both movies each way, the fuzz, the saves.

usage: run_checks.py [--jobs N] [--only WORD[,WORD...]] [--no-build] [--work DIR]

It runs them side by side, which is what makes it fast: nearly all of the time is one core emulating, and
the fuzz splits into a process per group of routines, as each routine's samples are seeded by its name.
--jobs 1 runs them one after another, with the fuzz in one process.

The checks, in build/harness's dv_harness, with a Mesen2 home of its own for each:
- ctest in build/: the tests with no reference.
- For TAS #1260 and #7044, --compare with --check-ports, --picture and --apu; again with --emulate and
  every routine that calls another (tools/nonleaf_routines.py); and with --pass-through.
- --fuzz-ports 5000 on every ported routine.
- --sweep-scripts from every opcode of every script (tools/scripts.py), from five points in the movies.
- --check-palette, and tests/run_save_movies.py with --harness and, where MESEN is set, --mesen.
- tests/run_play_movies.py with --harness: dejavu itself against Mesen2 on scripted movies.

Every job's output goes to WORK/NAME.txt (default: build/checks). A job fails if it exits with anything but
0, or says that any call or sample differed, and then this exits with 1. The one exception: TAS #7044's
glitch reads $2007 while the PPU draws, which the renderer doesn't model (docs/harness.md, "The picture"), so
a run may exit with 1 for frames that differ if every one of them has such a read and all else matches. It
builds both first, without
--no-build. It needs what those need: the ROM, build/movies/tas1260.inp and tas7044.inp, and MESEN for the
Mesen.exe part. It only reads dejavu.sav's folder's name: every run has a save file of its own.
"""
import argparse
import concurrent.futures
import os
import re
import subprocess
import sys
import time
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "tools"))
import check_rom  # noqa: E402
import scripts  # noqa: E402
from nes6502 import BANK_SIZE, NUM_BANKS  # noqa: E402

HARNESS = REPO / "build/harness/harness/Release/dv_harness"
DEJAVU = REPO / "build/Release/dejavu"
MOVIES = (("tas1260", 0), ("tas7044", 1))
FUZZ_SAMPLES = 5000
SWEEPS = (("tas1260", 0, 5000), ("tas1260", 0, 15000), ("tas1260", 0, 30000), ("tas7044", 1, 8000), ("tas7044", 1, 20000))
FUZZ_LINE = re.compile(r"port fuzz( through waits)?: (\d+) samples compared.*?(\d+) differed")
KNOWN_PICTURE = re.compile(r"of the (\d+) that differ, (\d+) had reads of \$2007")
MATCHED = re.compile(r"matched the reference at (\d+) of (\d+) NMIs")
APU_DIFFERED = re.compile(r"apu: \d+ NMIs compared, ([1-9]\d*) differ")
DIFFERED = re.compile(r"(?:port checks: \d+ calls checked against the original, |samples compared.*?)([1-9]\d*) differed")


def ported_routines():
    """The names in kPortedRoutines, which --fuzz-only takes."""
    text = (REPO / "src/dv_rtl.c").read_text()
    names = re.findall(r"^\s+(?:ENTRY|BANKED|BANKED_INLINE|INLINE)\((\w+)", text, re.M)
    return list(dict.fromkeys(names))


def shards(names, count):
    """The names in `count` groups, a routine in turn to each, so that neighbors don't share a group."""
    return [names[i::count] for i in range(count)]


def fuzz_totals(text):
    """(first pass compared, through waits compared) from a run's output."""
    first = waits = 0
    for line in text.splitlines():
        m = FUZZ_LINE.search(line)
        if m:
            if m[1]:
                waits += int(m[2])
            else:
                first += int(m[2])
    return first, waits


def only_known_picture_differences(text):
    """Whether what's wrong with a run is only frames that differ and all have reads of $2007."""
    reads = KNOWN_PICTURE.search(text)
    matched = MATCHED.search(text)
    return bool(reads and matched and reads[1] == reads[2] and int(reads[1]) > 0 and matched[1] == matched[2])


def job_failed(returncode, text):
    """Why a job failed, or None."""
    m = DIFFERED.search(text) or APU_DIFFERED.search(text)
    if m:
        return f"{m[1]} differed"
    if returncode and not (returncode == 1 and only_known_picture_differences(text)):
        return f"exit {returncode}"
    return None


def run_job(name, cmd, work, env=None):
    """Runs a job; returns (name, seconds, why it failed or None, its output)."""
    start = time.monotonic()
    result = subprocess.run(cmd, cwd=REPO, capture_output=True, text=True, errors="replace", env=env)
    text = result.stdout + result.stderr
    (work / f"{name}.txt").write_text(text)
    return name, time.monotonic() - start, job_failed(result.returncode, text), text


def harness_job(work, name, *args):
    return name, [str(HARNESS), *args, "--home", str(work / "home" / name)]


def write_script_places(work):
    """The address of every opcode of every script, for --sweep-scripts. Returns the file's path."""
    prg = check_rom.DEFAULT_ROM.read_bytes()[16:16 + NUM_BANKS * BANK_SIZE]
    path = work / "script_places.txt"
    path.write_text("".join(f"{addr:04X}\n" for addr in sorted(scripts.load(prg).ops)))
    return path


def sweep_job(work, movie, offset, frames, *more):
    return harness_job(work, f"sweep_{movie}_{frames}", "--movie", str(REPO / f"build/movies/{movie}.inp"), "--offset",
                       str(offset), "--compare", "--frames", str(frames), "--sweep-scripts",
                       str(write_script_places(work)), *more)


def make_jobs(args, work):
    jobs = [("ctest", ["ctest", "--test-dir", "build", "-C", "Release", "--output-on-failure", "-j",
                       str(max(args.jobs // 2, 1))])]
    emulate = subprocess.run([sys.executable, str(REPO / "tools/nonleaf_routines.py")], capture_output=True,
                             text=True, check=True).stdout.strip()
    for movie, offset in MOVIES:
        base = ["--movie", str(REPO / f"build/movies/{movie}.inp"), "--offset", str(offset), "--compare"]
        jobs.append(harness_job(work, f"{movie}_all", *base, "--check-ports", "--picture", "--apu"))
        jobs.append(harness_job(work, f"{movie}_emulate", *base, "--check-ports", "--emulate", emulate))
        jobs.append(harness_job(work, f"{movie}_pass", *base, "--pass-through"))
    fuzz = ["--movie", str(REPO / "build/movies/tas1260.inp"), "--compare", "--frames", "0", "--fuzz-ports",
            str(FUZZ_SAMPLES)]
    count = max(args.jobs - 1, 1) if args.jobs > 1 else 1
    if count == 1:
        jobs.append(harness_job(work, "fuzz", *fuzz))
    else:
        for i, group in enumerate(shards(ported_routines(), count)):
            jobs.append(harness_job(work, f"fuzz{i}", *fuzz, "--fuzz-only", ",".join(group)))
    jobs += [sweep_job(work, *sweep) for sweep in SWEEPS]
    jobs.append(harness_job(work, "palette", "--check-palette"))
    saves = [sys.executable, str(REPO / "tests/run_save_movies.py"), "--dejavu", str(DEJAVU), "--harness",
             str(HARNESS), "--work", str(work / "saves")]
    if os.environ.get("MESEN"):
        saves.append("--mesen")
    jobs.append(("saves", saves))
    jobs.append(("plays", [sys.executable, str(REPO / "tests/run_play_movies.py"), "--dejavu", str(DEJAVU), "--harness",
                           str(HARNESS), "--work", str(work / "plays")]))
    if args.only:
        words = args.only.split(",")
        jobs = [j for j in jobs if any(w in j[0] for w in words)]
    return jobs


def run_jobs(jobs, work, workers):
    """Runs (name, cmd) pairs; returns the results in the order they finish, printing each."""
    results = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
        for future in concurrent.futures.as_completed(pool.submit(run_job, n, c, work) for n, c in jobs):
            result = future.result()
            print(f"run_checks: {'FAIL' if result[2] else 'ok  '} {result[0]:16} {result[1]:6.0f} s"
                  f"{'  ' + result[2] if result[2] else ''}", flush=True)
            results.append(result)
    return results


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--jobs", type=int, default=(os.cpu_count() or 2) // 2 or 1,
                    help="how many at once (default: half the logical processors)")
    ap.add_argument("--only", help="only the jobs whose names have one of these words")
    ap.add_argument("--no-build", action="store_true")
    ap.add_argument("--work", type=Path, default=REPO / "build/checks")
    args = ap.parse_args()
    args.work.mkdir(parents=True, exist_ok=True)
    (args.work / "home").mkdir(exist_ok=True)
    start = time.monotonic()
    if not args.no_build:
        for folder, target in (("build", "ALL_BUILD"), ("build/harness", "dv_harness")):
            subprocess.run(["cmake", "--build", folder, "--config", "Release", "--target", target], cwd=REPO,
                           check=True, capture_output=True)
    results = run_jobs(make_jobs(args, args.work), args.work, args.jobs)
    first, waits = 0, 0
    for name, _, _, text in results:
        if name.startswith("fuzz"):
            a, b = fuzz_totals(text)
            first, waits = first + a, waits + b
    if first or waits:
        print(f"run_checks: the fuzz compared {first} samples, and {waits} through waits")
    failed = [r[0] for r in results if r[2]]
    print(f"run_checks: {len(results) - len(failed)} of {len(results)} passed in {time.monotonic() - start:.0f} s, "
          f"{sum(r[1] for r in results):.0f} s of work")
    if failed:
        print("run_checks: failed: " + ", ".join(failed) + f" (see {args.work})")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
