#!/usr/bin/env bash
#
# Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Run the CBMC proofs in tests/cbmc. Meant to run inside the punchboot-dev
# container (see scripts/pbdev), both locally and in CI.
#
#   scripts/ci/cbmc.sh [--word 32|64] [proof...]   Run proofs (default: all, both word sizes)
#   scripts/ci/cbmc.sh --mutants [mutant...]       Check that every mutant is caught
#
# Environment:
#   BUILD_DIR  Output directory (default: build-cbmc)
#   JOBS       Proofs run in parallel (default: nproc)

set -euo pipefail

cd "$(dirname "$0")/../.."
ROOT="$PWD"

BUILD_DIR="${BUILD_DIR:-build-cbmc}"
JOBS="${JOBS:-$(nproc)}"
WORDS="32 64"
MODE=proofs

while [ $# -gt 0 ]; do
    case "$1" in
        --word)
            WORDS="$2"
            shift 2
            ;;
        --mutants)
            MODE=mutants
            shift
            ;;
        *)
            break
            ;;
    esac
done

echo "=== $(cbmc --version | head -n 1)"

# Run one proof for one word size. Prints a single PASS/FAIL line, the full
# output is in $BUILD/<proof>-<word>.{log,cover.log,make.log}.
run_proof() {
    local root="$1" build="$2" proof="$3" word="$4"
    local start=$SECONDS
    if make -C "$root/tests/cbmc/proofs/$proof" WORD="$word" BUILD="$build" \
        > "$build/$proof-$word.make.log" 2>&1; then
        echo "PASS  $proof ($word bit, $((SECONDS - start)) s)"
    else
        echo "FAIL  $proof ($word bit, $((SECONDS - start)) s), see $build/$proof-$word.make.log"
        return 1
    fi
}
export -f run_proof

if [ "$MODE" = proofs ]; then
    mkdir -p "$BUILD_DIR"
    BUILD="$(cd "$BUILD_DIR" && pwd)"

    if [ $# -gt 0 ]; then
        PROOFS="$*"
    else
        PROOFS="$(cd tests/cbmc/proofs && ls -d -- */ | tr -d / | tr "\n" " ")"
    fi

    # Generate config.h once, before the proofs run in parallel
    make -s -C tests/cbmc/proofs/"${PROOFS%% *}" BUILD="$BUILD" "$BUILD/config.h"

    echo "=== Running proofs"
    for proof in $PROOFS; do
        for word in $WORDS; do
            echo "$proof $word"
        done
    done | xargs -P "$JOBS" -n 2 bash -c 'run_proof "$0" "$1" "$2" "$3"' "$ROOT" "$BUILD"
    exit $?
fi

# Mutants: each tests/cbmc/mutants/<name>.patch is a bug that a proof must
# find. The first line of the patch names it: "# proof: <proof>". The patch is
# applied to a scratch copy of the tree, never to the working tree.
echo "=== Checking mutants"
if [ $# -gt 0 ]; then
    MUTANTS="$*"
else
    MUTANTS="$(cd tests/cbmc/mutants && ls -- *.patch | sed 's/\.patch$//' | tr "\n" " ")"
fi

failed=0
for mutant in $MUTANTS; do
    patch_file="$ROOT/tests/cbmc/mutants/$mutant.patch"
    proof="$(sed -n '1s/^# proof: *//p' "$patch_file")"
    if [ -z "$proof" ]; then
        echo "ERROR $mutant: first line must be '# proof: <name>'"
        failed=1
        continue
    fi

    scratch="$(mktemp -d)"
    cp -r include src scripts configs "$scratch/"
    mkdir -p "$scratch/tests"
    cp -r tests/cbmc "$scratch/tests/"
    patch -s -d "$scratch" -p1 < "$patch_file"
    mkdir -p "$scratch/build"

    for word in $WORDS; do
        # Only a counterexample counts, not a build error or a timeout
        run_proof "$scratch" "$scratch/build" "$proof" "$word" > /dev/null || true
        if grep -q "VERIFICATION FAILED" "$scratch/build/$proof-$word.log" 2> /dev/null; then
            echo "KILLED   $mutant ($proof, $word bit)"
        else
            echo "SURVIVED $mutant ($proof, $word bit): see the log below"
            tail -n 20 "$scratch/build/$proof-$word.make.log"
            failed=1
        fi
    done
    rm -rf "$scratch"
done
exit $failed
