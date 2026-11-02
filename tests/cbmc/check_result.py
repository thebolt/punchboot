#!/usr/bin/env python3
"""Check a CBMC property log against the proof's known failures.

  check_result.py <cbmc log> <word size> [<expected_failures.txt>]

A proof passes when CBMC finished and every failed property is a known one.
expected_failures.txt lists the known ones, one per line, as a piece of the
property description (the text after the line number, e.g. "memcpy: source
valid for n bytes"). Text after '#' is a comment, by convention the finding id.
A line starting with "32:" or "64:" only applies to that word size.

A known failure that no longer fails is an error too, so the list is updated
when a bug is fixed and the proof becomes its regression test.
"""

from __future__ import annotations

import pathlib
import re
import sys

FAILURE = re.compile(r"^\[(?P<id>[^\]]+)\] (?P<desc>.*): FAILURE$")


def say(text: str) -> None:
    """Print a line of the report."""
    sys.stdout.write(text + "\n")


def expected(path: pathlib.Path | None, word: str) -> list[str]:
    """Return the known failures from expected_failures.txt for a word size."""
    if path is None or not path.exists():
        return []
    result = []
    for line in path.read_text().splitlines():
        text = line.split("#", 1)[0].strip()
        if text[:3] in ("32:", "64:"):
            if text[:2] != word:
                continue
            text = text[3:].strip()
        if text:
            result.append(text)
    return result


def main() -> int:
    """Compare the log with the known failures, return the exit code."""
    log = pathlib.Path(sys.argv[1]).read_text()
    word = sys.argv[2]
    known = expected(pathlib.Path(sys.argv[3]) if len(sys.argv) > 3 else None, word)

    if "VERIFICATION SUCCESSFUL" not in log and "VERIFICATION FAILED" not in log:
        say("cbmc did not finish (see the log)")
        return 1

    failures = [m.group("id", "desc") for m in map(FAILURE.match, log.splitlines()) if m]
    unexpected = [(i, d) for i, d in failures if not any(k in d for k in known)]
    fixed = [k for k in known if not any(k in d for _, d in failures)]

    for i, d in unexpected:
        say(f"new failure: [{i}] {d}")
    for k in fixed:
        say(f"known failure no longer fails, update expected_failures.txt: {k}")
    if failures and not unexpected and not fixed:
        say(f"{len(failures)} known failure(s), see expected_failures.txt")

    return 1 if unexpected or fixed else 0


if __name__ == "__main__":
    sys.exit(main())
