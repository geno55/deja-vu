#!/usr/bin/env python3
"""Say how much of the original game's code the comparisons reach.

usage: coverage.py [--jobs N] [--no-run] [--no-build] [--work DIR] [--out FILE]

"Compared" means one of three things, and the report keeps them apart:
- movie: --pass-through on TAS #1260 and #7044. The original's code ran against Mesen2 with every NMI
  compared. The ported run of the same movies also matches Mesen2 at every NMI, so the port took the same path,
  as far as its state shows.
- fuzz: --fuzz-ports 5000 on every ported routine. The code ran on made-up states, in samples where the port
  ran from the same state and the results were compared. Such a state may be one the game can't reach, so
  a path only the fuzz takes is checked, but perhaps never played. Its made-up scripts mean nothing, so it doesn't
  count for scripts.
- sweep: --sweep-scripts, from five points in the movies. Script code ran from every opcode of every script on the
  state there, with its flags as they were and then at random, all as original code but for the waits and the NMI, and
  the port ran from the same state. Real scripts and real state, but flags in combinations play may never reach, and
  starting partway through a script skips what came before.

It counts, from a ca65 listing of asm/*.s (what it has as instructions: a part of the ROM the disassembly has as
data doesn't count, and neither does anything in C):
- instructions that ran;
- conditional branches seen going both ways, one way, or not at all;
- routines (from the last label that isn't an L0123) with any instruction run;
- script opcodes that ran, of those tools/scripts.py finds (without the $FF that ends a script);
- goto_if script opcodes seen going both ways;
- which of the script interpreter's opcode numbers ran.

Prints the summary, and writes the details to FILE (default: WORK/report.txt): every routine that isn't fully
covered, with its unrun branches, and every script opcode that never ran. Runs the harness first, without
--no-run, with --coverage to WORK/NAME.cov (default: build/coverage). It needs what tools/run_checks.py needs, and
cc65. A job that fails or differs is reported and makes this exit with 1, as its marks then mean less.

It doesn't measure C: a branch that exists only in a port isn't seen.
"""
import argparse
import os
import re
import subprocess
import sys
from collections import Counter, defaultdict
from dataclasses import dataclass
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "tools"))
import check_rom  # noqa: E402
import disasm  # noqa: E402
import run_checks  # noqa: E402
import scripts  # noqa: E402
from nes6502 import BANK_SIZE, NUM_BANKS, rom_offset  # noqa: E402

RAN, BRANCHED, FELL, SCRIPT_OP, SCRIPT_NEXT, SCRIPT_JUMP = 0x01, 0x02, 0x04, 0x08, 0x10, 0x20  # emu_set_coverage
BRANCHES = {"bcc", "bcs", "beq", "bne", "bmi", "bpl", "bvc", "bvs"}
MNEMONICS = BRANCHES | {
    "adc", "and", "asl", "bit", "brk", "clc", "cld", "cli", "clv", "cmp", "cpx", "cpy", "dec", "dex", "dey", "eor",
    "inc", "inx", "iny", "jmp", "jsr", "lda", "ldx", "ldy", "lsr", "nop", "ora", "pha", "php", "pla", "plp", "rol",
    "ror", "rti", "rts", "sbc", "sec", "sed", "sei", "sta", "stx", "sty", "tax", "tay", "tsx", "txa", "txs", "tya"}
LISTING_LINE = re.compile(r"^([0-9A-F]{6})r (\d+) {2}(.{12}) ?(.*)$")
LABEL = re.compile(r"^([A-Za-z_@][A-Za-z0-9_@]*):\s*(.*)$")
PLAIN_LABEL = re.compile(r"L[0-9A-F]{4}")


@dataclass
class Instruction:
    offset: int      # in PRG
    addr: int        # where it runs
    bank: int
    routine: str
    mnemonic: str


def parse_listing(text, bank):
    """The instructions in a bank's ca65 listing."""
    base = 0xC000 if bank == NUM_BANKS - 1 else 0x8000
    routine = f"bank{bank}_start"
    out = []
    for line in text.splitlines():
        m = LISTING_LINE.match(line)
        if not m or m[2] != "1":  # a line of an included file has no code
            continue
        source = m[4].split(";")[0].strip()
        label = LABEL.match(source)
        if label:
            if not PLAIN_LABEL.fullmatch(label[1]) and not label[1].startswith("@"):
                routine = label[1]
            source = label[2]
        word = source.split()[0].lower() if source else ""
        if word in MNEMONICS:
            at = int(m[1], 16)
            out.append(Instruction(bank * BANK_SIZE + at, base + at, bank, routine, word))
    return out


def read_listings(work):
    """Assembles every bank to a listing, in work, and returns its instructions."""
    bins = work / "bin"
    subprocess.run([sys.executable, str(REPO / "tools/extract_bins.py"), "--out", str(bins)], check=True,
                   capture_output=True)
    ca65 = disasm.cc65_tool("ca65")
    out = []
    for bank in range(NUM_BANKS):
        listing = work / f"bank{bank}.lst"
        subprocess.run([str(ca65), "--cpu", "6502", "--bin-include-dir", str(bins), "-l", str(listing), "-o",
                        str(work / f"bank{bank}.o"), str(REPO / f"asm/bank{bank}.s")], check=True, capture_output=True)
        out += parse_listing(listing.read_text(errors="replace"), bank)
    return out


def merge(paths):
    """The OR of coverage files, or all zeros with none."""
    size = NUM_BANKS * BANK_SIZE
    merged = bytearray(size)
    for path in paths:
        data = path.read_bytes()
        if len(data) != size:
            sys.exit(f"coverage: {path} has {len(data)} bytes, not {size}")
        for i, flags in enumerate(data):
            merged[i] |= flags
    return merged


def code_counts(instructions, flags):
    """(run, in all, branches both ways, one way, not at all) for these instructions."""
    run = both = one = none = branches = 0
    for i in instructions:
        f = flags[i.offset]
        run += bool(f & RAN)
        if i.mnemonic in BRANCHES:
            branches += 1
            ways = bool(f & BRANCHED) + bool(f & FELL)
            both += ways == 2
            one += ways == 1
            none += ways == 0
    return {"run": run, "all": len(instructions), "branches": branches, "both": both, "one": one, "none": none}


def script_counts(ops, flags):
    """Coverage of the script opcodes, given {PRG offset: (addr, opcode, size)}, from a movie's flags."""
    counted = {o: v for o, v in ops.items() if v[1] != scripts.END}
    ran = {o: v for o, v in counted.items() if flags[o] & SCRIPT_OP}
    gotos = {o for o, v in counted.items() if v[1] in scripts.GOTO_IF}
    both = sum(flags[o] & SCRIPT_NEXT != 0 and flags[o] & SCRIPT_JUMP != 0 for o in gotos)
    one = sum((flags[o] & SCRIPT_NEXT != 0) != (flags[o] & SCRIPT_JUMP != 0) for o in gotos)
    numbers = Counter(v[1] for v in ran.values())
    return {"run": len(ran), "all": len(counted), "gotos": len(gotos), "both": both, "one": one,
            "numbers": numbers, "unrun": sorted(o for o in counted if o not in ran),
            "all_numbers": {v[1] for v in counted.values()}}


def percent(part, whole):
    return f"{100 * part / whole:.1f}%" if whole else "-"


def summary(name, c):
    return (f"{name:<14} instructions {c['run']:>6} of {c['all']} ({percent(c['run'], c['all'])}); "
            f"branches {c['both']:>5} both ways, {c['one']} one way, {c['none']} never, of {c['branches']} "
            f"({percent(c['both'], c['branches'])} both ways)")


def script_line(name, s):
    return (f"{name:<14} script opcodes {s['run']} of {s['all']} ({percent(s['run'], s['all'])}); goto_ifs {s['both']} "
            f"both ways, {s['one']} one way, {s['gotos'] - s['both'] - s['one']} never, of {s['gotos']}; "
            f"{len(s['numbers'])} of {len(s['all_numbers'])} opcode numbers the scripts use")


def report(instructions, ops, movie, fuzz, sweep):
    """Returns (the summary's lines, the details' lines)."""
    both_sources = bytes(a | b | c for a, b, c in zip(movie, fuzz, sweep))
    lines = [summary("movies", code_counts(instructions, movie)),
             summary("fuzz", code_counts(instructions, fuzz)),
             summary("sweep", code_counts(instructions, sweep)),
             summary("any", code_counts(instructions, both_sources))]
    routines = defaultdict(list)
    for i in instructions:
        routines[(i.bank, i.routine)].append(i)
    for name, flags in (("movies", movie), ("fuzz", fuzz), ("sweep", sweep), ("any", both_sources)):
        reached = sum(any(flags[i.offset] & RAN for i in r) for r in routines.values())
        lines.append(f"{name:<14} routines with something run: {reached} of {len(routines)}")
    for bank in range(NUM_BANKS):
        in_bank = [i for i in instructions if i.bank == bank]
        if in_bank:
            lines.append(summary(f"any, bank {bank}", code_counts(in_bank, both_sources)))
    lines.append(script_line("movies", script_counts(ops, movie)))
    lines.append(script_line("sweep", script_counts(ops, sweep)))
    s = script_counts(ops, bytes(a | b for a, b in zip(movie, sweep)))
    lines.append(script_line("both", s))

    details = ["Routines not fully covered, by anything. Each line: bank:addr of the routine, "
               "name, instructions run/all, then the branches that weren't seen both ways.", ""]
    for (bank, name), body in sorted(routines.items(), key=lambda kv: kv[1][0].offset):
        c = code_counts(body, both_sources)
        if c["run"] == c["all"] and c["both"] == c["branches"]:
            continue
        unseen = []
        for i in body:
            if i.mnemonic in BRANCHES:
                f = both_sources[i.offset]
                if not (f & BRANCHED and f & FELL):
                    way = "never ran" if not f & (BRANCHED | FELL) else ("never jumped" if f & FELL else "always jumped")
                    unseen.append(f"{i.addr:04X} {i.mnemonic} {way}")
        details.append(f"{bank}:{body[0].addr:04X} {name} {c['run']}/{c['all']}" + ("" if not unseen else ": " + ", ".join(unseen)))
    details += ["", "Script opcode numbers the scripts use that never ran: " +
                " ".join(f"{n:02X}" for n in sorted(s["all_numbers"] - set(s["numbers"]))),
                "", "Script opcodes that never ran in the movies or the sweep (bank 3):"]
    for o in s["unrun"]:
        addr, op, _ = ops[o]
        details.append(f"  {addr:04X} {scripts.NAMES.get(op, f'op_{op:02X}')}")
    return lines, details


def harness_jobs(args, work):
    """(name, command) for the runs, each with a coverage file of its own."""
    jobs = []
    for movie, offset in run_checks.MOVIES:
        jobs.append(run_checks.harness_job(
            work, f"{movie}_pass", "--movie", str(REPO / f"build/movies/{movie}.inp"), "--offset", str(offset), "--compare",
            "--pass-through", "--coverage", str(work / f"movie_{movie}.cov")))
    count = max(args.jobs - 1, 1)
    for i, group in enumerate(run_checks.shards(run_checks.ported_routines(), count)):
        jobs.append(run_checks.harness_job(
            work, f"fuzz{i}", "--movie", str(REPO / "build/movies/tas1260.inp"), "--compare", "--frames", "0", "--fuzz-ports",
            str(run_checks.FUZZ_SAMPLES), "--fuzz-only", ",".join(group), "--coverage", str(work / f"fuzz_{i}.cov")))
    for movie, offset, frames in run_checks.SWEEPS:
        jobs.append(run_checks.sweep_job(work, movie, offset, frames, "--coverage", str(work / f"sweep_{movie}_{frames}.cov")))
    return jobs


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--jobs", type=int, default=(os.cpu_count() or 2) // 2 or 1)
    ap.add_argument("--no-run", action="store_true", help="report on the coverage files already there")
    ap.add_argument("--no-build", action="store_true")
    ap.add_argument("--work", type=Path, default=REPO / "build/coverage")
    ap.add_argument("--out", type=Path)
    args = ap.parse_args()
    args.work.mkdir(parents=True, exist_ok=True)
    (args.work / "home").mkdir(exist_ok=True)
    failed = []
    if not args.no_run:
        for old in args.work.glob("*.cov"):
            old.unlink()
        if not args.no_build:
            subprocess.run(["cmake", "--build", "build/harness", "--config", "Release", "--target", "dv_harness"], cwd=REPO,
                           check=True, capture_output=True)
        results = run_checks.run_jobs(harness_jobs(args, args.work), args.work, args.jobs)
        failed = [r[0] for r in results if r[2]]
    movie_files = sorted(args.work.glob("movie_*.cov"))
    fuzz_files = sorted(args.work.glob("fuzz_*.cov"))
    sweep_files = sorted(args.work.glob("sweep_*.cov"))
    if not movie_files or not fuzz_files or not sweep_files:
        sys.exit(f"coverage: no coverage files in {args.work}: run it without --no-run")
    prg = check_rom.DEFAULT_ROM.read_bytes()[16:16 + NUM_BANKS * BANK_SIZE]
    walked = scripts.load(prg)
    ops = {rom_offset(a, scripts.BANK): (a, op, n) for a, (op, n) in walked.ops.items()}
    instructions = read_listings(args.work)
    lines, details = report(instructions, ops, merge(movie_files), merge(fuzz_files), merge(sweep_files))
    print("\n".join("coverage: " + line for line in lines))
    out = args.out or args.work / "report.txt"
    out.write_text("\n".join(lines + [""] + details) + "\n")
    print(f"coverage: details in {out}")
    if failed:
        print("coverage: these runs failed or differed: " + ", ".join(failed))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
