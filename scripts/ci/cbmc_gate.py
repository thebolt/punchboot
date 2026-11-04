#!/usr/bin/env python3
"""Check that a change to proven code also changes its proof (tests/cbmc).

A source file is proven when a proof's Makefile lists it (SOURCES or a
dependency line) or its harness includes it. When a branch changes such a
file, it must also change one of the proofs that cover it: the harness, its
Makefile, or a mutant for it. A change that does not need that (a comment, a
rename the proof does not see) says so in a commit message line

    CBMC-waiver: <why>

  cbmc_gate.py <base ref>     Check the commits in <base ref>..HEAD
  cbmc_gate.py --list         Print each proven file and its proofs

Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>

SPDX-License-Identifier: BSD-3-Clause
"""

from __future__ import annotations

import argparse
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
PROOFS = ROOT / "tests" / "cbmc" / "proofs"
MUTANTS = ROOT / "tests" / "cbmc" / "mutants"
# Where proven code lives; stubs and models under tests/ are the proofs' own
CODE = ("src/", "tools/", "include/")


def say(text: str) -> None:
    """Print a line of the report."""
    sys.stdout.write(text + "\n")


def makefile_files(makefile: pathlib.Path) -> set[str]:
    """Return the repository files a proof Makefile names."""
    text = makefile.read_text().replace("\\\n", " ")
    variables = {"ROOT": str(ROOT)}
    for name, value in re.findall(r"^(\w+)\s*[:?]?=\s*(.*)$", text, re.MULTILINE):
        variables.setdefault(name, value.strip())

    def expand(value: str) -> str:
        for _ in range(5):
            value = re.sub(r"\$\((\w+)\)", lambda m: variables.get(m.group(1), ""), value)
        return value

    files = set()
    for word in expand(text).split():
        path = pathlib.Path(word.rstrip(":"))
        if path.is_absolute() and path.is_file() and path.is_relative_to(ROOT):
            files.add(str(path.relative_to(ROOT)))
    return files


def harness_files(harness: pathlib.Path, seen: set[pathlib.Path]) -> set[str]:
    """Return the source files a harness includes, through other harnesses too."""
    if harness in seen or not harness.is_file():
        return set()
    seen.add(harness)
    files = set()
    for name in re.findall(r'^#include "([^"]+\.c)"', harness.read_text(), re.MULTILINE):
        for base in (harness.parent, ROOT / "src"):
            path = (base / name).resolve()
            if path.is_file():
                if path.name == "harness.c":
                    files |= harness_files(path, seen)
                else:
                    files.add(str(path.relative_to(ROOT)))
                break
    return files


def proven_files() -> dict[str, set[str]]:
    """Map each proven source file to the proofs that cover it."""
    covered: dict[str, set[str]] = {}
    for proof in sorted(p for p in PROOFS.iterdir() if (p / "Makefile").is_file()):
        files = makefile_files(proof / "Makefile") | harness_files(proof / "harness.c", set())
        for name in files:
            if name.startswith(CODE):
                covered.setdefault(name, set()).add(proof.name)
    return covered


def git(*args: str) -> str:
    """Run git in the repository and return its output."""
    return subprocess.run(
        ["git", *args], cwd=ROOT, check=True, capture_output=True, text=True
    ).stdout


def mutant_proof(name: str) -> str | None:
    """Return the proof a mutant patch names, if the file exists."""
    path = ROOT / name
    if not path.is_file():
        return None
    match = re.match(r"# proof: *(\S+)", path.read_text())
    return match.group(1) if match else None


def check(base: str) -> int:
    """Check the commits in base..HEAD, return the exit code."""
    changed = git("diff", "--name-only", f"{base}...HEAD").split()
    messages = git("log", "--format=%B", f"{base}..HEAD")
    waivers = re.findall(r"^CBMC-waiver:\s*(.+)$", messages, re.MULTILINE)

    touched = set()
    for name in changed:
        parts = pathlib.PurePosixPath(name).parts
        if parts[:3] == ("tests", "cbmc", "proofs") and len(parts) > 3:
            touched.add(parts[3])
        elif name.startswith("tests/cbmc/mutants/"):
            proof = mutant_proof(name)
            if proof:
                touched.add(proof)

    missing = []
    covered = proven_files()
    for name in changed:
        proofs = covered.get(name)
        if proofs and not proofs & touched:
            missing.append((name, proofs))

    if not missing:
        say("cbmc gate: OK")
        return 0
    for name, proofs in missing:
        say(f"{name}: changed, but none of its proofs is: {', '.join(sorted(proofs))}")
    if waivers:
        for waiver in waivers:
            say(f"waived: {waiver}")
        return 0
    say("Update a proof (tests/cbmc/README.md), or add 'CBMC-waiver: <why>' to a commit message.")
    return 1


def main() -> int:
    """Parse the arguments and run."""
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("base", nargs="?", help="base ref of the branch")
    group.add_argument("--list", action="store_true", help="print the proven files")
    args = parser.parse_args()

    if args.list:
        for name, proofs in sorted(proven_files().items()):
            say(f"{name}: {' '.join(sorted(proofs))}")
        return 0
    return check(args.base)


if __name__ == "__main__":
    sys.exit(main())
