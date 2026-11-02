# CBMC proofs

Bounded model checking of the firmware with CBMC. What is proven, the assumptions and the plan
are in `doc/dev/verification.md`.

## Running

```sh
scripts/pbdev cbmc                      # all proofs, 32 and 64 bit
scripts/pbdev cbmc --word 32 wire crc   # some proofs, one word size
scripts/pbdev cbmc --mutants            # every mutant must be found
```

Output goes to `build-cbmc/` (`BUILD_DIR`): `<proof>-<word>.log` (property results),
`.cover.log` (cover goals) and `.make.log` (everything). `JOBS` sets how many proofs run in
parallel.

Without the container, with `cbmc` and `goto-cc` on the PATH:

```sh
make -C tests/cbmc/proofs/wire WORD=32
```

## Adding a proof

1. `tests/cbmc/proofs/<name>/Makefile`:

   ```make
   PROOF   = <name>
   SOURCES = $(ROOT)/src/<file>.c
   UNWIND  = <largest loop bound + 1>
   include ../../Makefile.common
   ```

   See the top of `Makefile.common` for `CBMC_EXTRA` (e.g. `--unwindset`), `CHECK_FLAGS_OFF`
   (with a comment why), `EXTRA_INC`, `EXTRA_DEFS` and `MODELS`.

2. `harness.c` with a `void harness(void)` entry point and a header comment listing the
   target, the properties, the assumptions (bounds) and the stubs. Include `<common.h>`.
   Inputs are uninitialized locals (CBMC treats them as any value). Constrain them with
   `__CPROVER_assume()`, state properties with `__CPROVER_assert()`, and add `PB_COVER()`
   goals for every path that must be reachable, so an over-constrained harness fails instead of
   passing vacuously.

3. Check that the proof can fail: add a mutant, `tests/cbmc/mutants/<name>-<what>.patch`, whose
   first line is `# proof: <name>`, then a description, then a `diff -u` against the tree with
   `a/` and `b/` prefixes. `scripts/pbdev cbmc --mutants <mutant>` must report it as KILLED.

4. Run both word sizes and add the proof to the status table in `doc/dev/verification.md`.

Stubs and models shared between proofs go in `stubs/`. A stub returns any value its real function
can return, including errors, and documents its contract.
