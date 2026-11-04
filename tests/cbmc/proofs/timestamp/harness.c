/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: boot timestamps (src/timestamp.c)
 *
 * Properties:
 *  1. ts() never writes past the table, however often it is called.
 *  2. ts_print() and ts_total() only read taken timestamps, also before the
 *     first one.
 *  3. ts_total() is the time from the first to the last timestamp (modulo
 *     2^32, the tick wraps), 0 without timestamps.
 * Assumptions: a table of 2 timestamps (cfg/config.h), up to 3 calls.
 * Stubs: plat_get_us_tick (any value; it records the first and the last),
 * printf (no output).
 */

#include <common.h>
#include <pb/timestamp.h>

#define MAX_CALLS 3

static unsigned int ticks_read;
static unsigned int first_tick;
static unsigned int last_tick;

unsigned int plat_get_us_tick(void)
{
    unsigned int t;

    if (ticks_read == 0)
        first_tick = t;
    last_tick = t;
    ticks_read++;
    return t;
}

void harness(void)
{
    unsigned int n;

    __CPROVER_assume(n <= MAX_CALLS);

    for (unsigned int i = 0; i < n; i++)
        ts("step");

    ts_print();
    unsigned int total = ts_total();

    if (n == 0)
        __CPROVER_assert(total == 0, "no timestamps: total 0");
    else
        __CPROVER_assert(total == last_tick - first_tick, "total from the first to the last");
    PB_COVER(n == 0);
    PB_COVER(n == MAX_CALLS && total != 0);
}
