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
  fails if one is unreachable or if there are none. In the cover build the harness assertions are
  compiled out, because CBMC turns assertions into assumptions there and a known failure would
  cut every path after it.
- Globals start as zero in CBMC like in C. Harnesses give stub state and module state any value
  explicitly where that matters (`pb_ocotp_havoc()`).
- The log macros print through `tests/cbmc/stubs/log.c` (no output, arguments still evaluated).
- Mutants: `tests/cbmc/mutants/*.patch` are bugs that a proof must find (only a counterexample
  counts, not a build error). `scripts/pbdev cbmc --mutants` applies each to a scratch copy.
- CI: the `cbmc` job in `build.yml` runs the proofs and the mutants for both word sizes and uploads
  the logs. It is not blocking yet (`continue-on-error`).
- CBMC 6.6.0 (Debian trixie package in the dev container).

## Status

| Proof | Target | Properties | Bounds | Known failures | Time (32/64) |
|---|---|---|---|---|---|
| `crc` | `src/lib/crc.c` | Memory safety; one step equals the bitwise CRC-32 (so all lengths, by induction); incremental `crc32(a‖b)` | len ≤ 16, incremental ≤ 2 | | 22 s / 22 s |
| `wire` | `src/wire.c` | `valid_command` is exactly magic + range; only IDENTIFIER_READ and AUTHENTICATE need no auth; `init_*2` size checks, no write outside the struct | none (fully symbolic structs) | | 3 s / 5 s |
| `bpak-header` | `bpak_valid_header` (pre-auth) | No UB for any 4 KiB header; OK implies magic, hash kind, alignment and metadata bounds for the slots before the first empty one | none | F-149 | 55 s / 55 s |
| `bpak-signature` | `bpak_copyz_signature` (pre-auth) | No access outside the header or the output buffer; OK only for 0 < sz ≤ min(512, buffer); signature copied, area and size zeroed; header unchanged on error | buffer ≤ 1 KiB | | 5 s / 5 s |
| `bpak-getters` | `bpak_get_meta`, `_anyref`, `bpak_get_part` | Returned entry is a header slot with the requested id/ref; checked slots have metadata in bounds | valid header | F-148, F-149 | 83 s / 83 s |
| `bpak-part` | `bpak_part_size`, `bpak_part_offset` | No wrap or truncation; offset = 4096 + earlier part sizes | any header; offset for the first 3 slots | F-148, F-149 | 42 s / 223 s |
| `der-ecsig` | `der_ecsig_to_rs` (pre-auth, CAAM) | No access outside sig, r, s for any input; OK implies a SEQUENCE | sig ≤ 24 bytes, curve length ≤ 8 (see harness) | | 28 s / 38 s |
| `der-pubkey` | `der_ec_public_key_data` (trusted keystore) | Memory safety | key ≤ 160 bytes | F-066 | 3 s / 5 s |
| `slc` | `src/slc.c` | Without a config or callback: NOT_SUPPORTED, no NULL call | none | F-001 | 1 s / 1 s |
| `rot` | `src/rot.c` | `rot_init` accepts exactly a matching key map; status/revoke act on the requested key; `rot_get_dsa_key` only returns a key the board reports OK, with its data and kind; no error codes as counts or ids | 3 keys | F-153 | 1 s / 1 s |
| `rot-imx6ul` | i.MX6UL `rot_helpers.c`, `slc_helpers.c` | Revoked key reads as revoked; revoke burns only the key's bit; LOCKED exactly when HAB reports secure boot; lock fuse burnt only from an unlocked state | param1 < 4 | **F-150** | 1 s / 1 s |
| `rot-imx8x` | i.MX8X `rot_helpers.c`, `slc_helpers.c` | REVOKED exactly when the key's SECO bit is set; LOCKED exactly when SECO reports OEM closed; fail closed when SECO can't be asked; forward only with the SRK fused | param1 < 4 | **F-152** | 1 s / 1 s |
| `slc-imxrt` | i.MX RT `slc_helpers.c`, `rot_helpers.c` | LOCKED exactly when SEC_CONFIG is set; NOT_CONFIGURED means no SRK bits; no revocation support | none | F-151 | 1 s / 1 s |

Checks turned off: `crc` has no unsigned overflow check (`while (size--)` wraps on purpose).

Known failures are listed per proof in `expected_failures.txt`. A proof fails on any other
failure, and also when a known one stops failing (then the list is updated and the proof becomes
the fix's regression test).

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
- **Phase 1:** done: `bpak`, `der`, `slc`/`rot` and the platform helpers (found F-148..F-153).
  Left: every string and memory function in `src/lib/libc`, and `uuid`.
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
