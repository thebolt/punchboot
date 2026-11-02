# Formal verification (CBMC)

Bounded model checking of the central firmware code with [CBMC](https://github.com/diffblue/cbmc)
(D-022). A proof checks every input up to a bound, so a passing proof means: no failed property
for any input within the bound, under the assumptions and stubs listed for that proof. It is not
an unbounded proof, and it says nothing about stubbed code (see Trust boundary).

How to run and add proofs: `tests/cbmc/README.md`.

## Setup

- Harnesses live in `tests/cbmc/proofs/<name>/`, shared rules in `tests/cbmc/Makefile.common`.
- Every proof runs at both word sizes of the firmware targets: `WORD=32` (ILP32, i.MX6UL/RT) and
  `WORD=64` (LP64, i.MX8). goto-cc builds with `-m32`/`-m64` against the firmware libc headers
  (`include/libc`, `include/libc/aarch{32,64}`), not the host libc, so the type widths are those
  of the targets. `tests/cbmc/common.h` asserts the pointer and `size_t` width.
- `config.h` is generated from `configs/host_defconfig` like the host build.
- `LOGLEVEL=0`: the log macros compile away.
- The string functions (`memcpy`, `memmove`, `memset`, `memcmp`) come from
  `tests/cbmc/stubs/string_models.c`. They assert the C preconditions (valid for n bytes, no
  overlap for memcpy). CBMC's own models are not used: CBMC preprocesses them with the host libc
  headers, which do not exist for 32-bit builds. The real `src/lib/libc` is verified by its own
  proofs (planned, Phase 1).
- Default checks: bounds, pointer, pointer-primitive, signed and unsigned overflow, conversion,
  undefined shift, pointer overflow, division by zero, and unwinding assertions (a too-low loop
  bound fails the proof). A proof may turn a check off with `CHECK_FLAGS_OFF` and a comment why.
- Vacuity: every harness has `PB_COVER()` goals for the paths that must be reachable. `make cover`
  fails if one is unreachable or if there are none.
- Mutants: `tests/cbmc/mutants/*.patch` are bugs that a proof must find (only a counterexample
  counts, not a build error). `scripts/pbdev cbmc --mutants` applies each to a scratch copy.
- CI: the `cbmc` job in `build.yml` runs the proofs and the mutants for both word sizes and uploads
  the logs. It is not blocking yet (`continue-on-error`).
- CBMC 6.6.0 (Debian trixie package in the dev container).

## Status

| Proof | Target | Properties | Bounds | Time (32/64) |
|---|---|---|---|---|
| `crc` | `src/lib/crc.c` | Memory safety; one step equals the bitwise CRC-32 (so all lengths, by induction); incremental `crc32(a‖b)` | len ≤ 16, incremental ≤ 2 | 22 s / 22 s |
| `wire` | `src/wire.c` | `valid_command` is exactly magic + range; only IDENTIFIER_READ and AUTHENTICATE need no auth; `init_*2` size checks, no write outside the struct | none (fully symbolic structs) | 3 s / 5 s |

Checks turned off: `crc` has no unsigned overflow check (`while (size--)` wraps on purpose).

## Trust boundary

What the proofs assume rather than prove. Each stub states its contract in its file.

| Not verified | Why | Assumed |
|---|---|---|
| String functions in proofs other than libc | Replaced by models | C standard semantics |
| Mbed TLS, CAAM and other crypto drivers | Out of scope | (planned) hash/DSA stub contracts |
| littlefs, libfdt | Vendored, large | (planned) bounded harnesses later |
| USB, MMC, UART hardware, SoC code | Hardware | (planned) transport delivers what it reports |

## Plan

From the approved plan (2026-11-02), ordered by attacker reachability:

- **Phase 0 (done):** tooling, CI job, vacuity and mutation checks, `crc` and `wire`.
- **Phase 1:** pre-auth parsers: `bpak` (header getters, part offset/size, signature copy),
  `der_ecsig_to_rs`, `slc`/`rot` and the platform `rot_helpers.c`/`slc_helpers.c` (suspected:
  i.MX6UL reads the revoke fuse from the wrong word), vendored `libc` and `uuid`.
- **Phase 2:** `bio` (32-bit `lba_t` arithmetic, `bio_erase` F-055), `gpt`, `image_helpers`
  (hashed range equals loaded range, F-065), `ab_state`, `ramlog`, `tzc380`.
- **Phase 3:** `boot.c` with a ghost model of what was hashed and verified (F-070), and
  `cm_main.c` per-handler harnesses and the auth gate (suspected: an SLC read error skips the
  gate).
- **Phase 4:** host-side parsing in `tools/punchboot`, sequence properties, harnesses reused as
  fuzz targets.
- Not yet done from Phase 0's surroundings: a CI gate that a PR touching a proven file also
  touches its proof, and cbmc-viewer reports.

Findings from proofs go in `findings.md`, each fix in its own `bugfix:` commit with the proof as
its regression test. Fixes that change accepted images or wire behaviour need a decision first
(D-008).
