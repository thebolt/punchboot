/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: snprintf with format specifiers it does not support
 * (src/lib/libc/snprintf.c)
 *
 * Property: snprintf returns for every format specifier, supported or not
 * ("%c", "%X", "%zu", "%2u" here; "%X" is the uuid upper case format).
 * (F-089 regression: it spun forever on them.)
 * Assumptions: n <= 8.
 * Stubs: none (the firmware libc itself).
 */

#include <libc.h>
#include <stdio.h>

void harness(void)
{
    size_t n;
    unsigned int u;
    int which;
    __CPROVER_assume(n <= 8);
    char *buf = pb_cbmc_alloc(n);

    /* Reached; whether it returns is the property */
    PB_COVER(which == 3);
    if (which == 0)
        snprintf(buf, n, "%c", (int)u);
    else if (which == 1)
        snprintf(buf, n, "%X", u);
    else if (which == 2)
        snprintf(buf, n, "%zu", (size_t)u);
    else
        snprintf(buf, n, "%2u", u);
}
