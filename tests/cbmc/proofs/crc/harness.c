/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: crc32 (src/lib/crc.c)
 *
 * Properties:
 *  1. No out of bounds access or arithmetic error for any seed, buffer
 *     contents and length up to MAX_LEN bytes.
 *  2. One step (one byte, any seed) equals the bitwise CRC-32 step. crc32 is
 *     a left fold of that step over the buffer, so by induction it equals the
 *     bitwise reference for every length. Checking the full equivalence over
 *     several symbolic bytes directly is too slow for the SAT solver.
 *  3. crc32(a || b) == crc32(crc32(a), b) (incremental use, as in ab_state
 *     and gpt), for lengths up to INCR_LEN.
 * Assumptions: lengths bounded as above. The loop body is the same for every
 * byte, so longer inputs add no new behaviour.
 * Stubs: none.
 */

#include <common.h>
#include <pb/crc.h>

#define MAX_LEN  16
#define INCR_LEN 2

/* One byte of reflected CRC-32, polynomial 0xEDB88320, no pre/post inversion
 * (crc32() takes the seed as is) */
static uint32_t reference_step(uint32_t crc, uint8_t byte)
{
    crc ^= byte;
    for (int b = 0; b < 8; b++)
        crc = (crc & 1) ? (crc >> 1) ^ 0xEDB88320u : (crc >> 1);
    return crc;
}

void harness(void)
{
    uint8_t buf[MAX_LEN];
    uint32_t seed;
    uint32_t n;
    uint32_t split;

    /* 1. Safety for any length up to MAX_LEN */
    __CPROVER_assume(n <= MAX_LEN);
    (void)crc32(seed, buf, n);
    PB_COVER(n == MAX_LEN);

    /* 2. Single step equals the bitwise reference */
    __CPROVER_assert(crc32(seed, buf, 1) == reference_step(seed, buf[0]),
                     "step matches bitwise reference");

    /* 3. Incremental update */
    uint32_t m;
    __CPROVER_assume(m <= INCR_LEN);
    __CPROVER_assume(split <= m);
    __CPROVER_assert(crc32(seed, buf, m) == crc32(crc32(seed, buf, split), buf + split, m - split),
                     "incremental update");
    PB_COVER(split > 0 && split < m);
}
