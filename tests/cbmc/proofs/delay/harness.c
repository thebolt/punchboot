/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: timeouts (src/delay.c)
 *
 * Properties:
 *  1. pb_timeout_init_us refuses 0 and 0xffffffff and accepts every other
 *     timeout, for any start tick.
 *  2. pb_timeout_has_expired is true exactly when at least timeout_us
 *     microseconds have passed since the init, for any start tick and any
 *     elapsed time below 2^32 us (the tick is a 32-bit microsecond counter
 *     that wraps), also when the tick wrapped between two polls.
 *  3. pb_delay_us returns, and only once the delay has passed.
 * Assumptions: the tick counts up and wraps at 2^32, a poll comes less than
 * 2^32 us (71 minutes) after the init. pb_delay_us: the tick advances by the
 * same 1 to MAX_STEP us between two reads, and the delay is at most two steps,
 * so it polls at most twice (the loop does the same for more polls).
 * Stubs: plat_get_us_tick.
 */

#include <common.h>
#include <pb/delay.h>
#include <pb/pb.h>

#define MAX_STEP 0x40000000u

static uint32_t tick;
static uint32_t step;
static uint64_t elapsed; /* since the first read, not wrapped */

unsigned int plat_get_us_tick(void)
{
    uint32_t now = tick;

    tick = (uint32_t)(((uint64_t)tick + step) & 0xffffffffu); /* wraps like the counter */
    elapsed += step;
    return now;
}

void harness(void)
{
    struct pb_timeout to;
    uint32_t start;
    uint32_t timeout_us;
    uint32_t passed;

    /* 1, 2: one init, one poll after passed us */
    tick = start;
    step = passed;
    int rc = pb_timeout_init_us(&to, timeout_us);

    __CPROVER_assert((rc == PB_OK) == (timeout_us != 0 && timeout_us != 0xffffffff),
                     "init refuses exactly 0 and 0xffffffff");

    if (rc == PB_OK) {
        bool expired = pb_timeout_has_expired(&to);
        __CPROVER_assert(expired == (passed >= timeout_us), "expired exactly after timeout_us");
        PB_COVER(expired && tick < start);
        PB_COVER(!expired && tick < start);
        PB_COVER(expired && tick > start);
        PB_COVER(!expired && tick > start);
    }

    /* 3: pb_delay_us with any start tick and step */
    uint32_t delay;
    uint32_t step2;
    tick = start;
    __CPROVER_assume(step2 >= 1 && step2 <= MAX_STEP);
    __CPROVER_assume(delay <= 2u * step2);
    step = step2;
    elapsed = 0;
    pb_delay_us(delay);
    /* The last read is the one that saw the delay pass; elapsed counts the
     * step after it too */
    __CPROVER_assert(delay == 0 || elapsed - step2 >= delay, "delay waited long enough");
    PB_COVER(delay > 0 && elapsed > step2);
}
