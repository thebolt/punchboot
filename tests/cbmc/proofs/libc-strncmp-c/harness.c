/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: strncmp (src/lib/libc/strncmp.c)
 * Not compiled into the firmware (F-030), proven anyway.
 *
 * Properties (C11 7.24.4.4): no access past either NUL or n characters; the sign of the result is
 * that of the first differing character in the first n (as unsigned char), 0 if they are equal.
 * Assumptions: a and b are NUL terminated strings; n is any value up to LIBC_MAX + 1. Buffers are
 * at most LIBC_MAX (6) bytes: the function works byte by byte, so longer buffers repeat the same
 * steps. Stubs: none (the firmware libc itself, no string models).
 */

#include <libc.h>
#include <string.h>

void harness(void)
{
    size_t asize, bsize;
    char *a = pb_cbmc_string(&asize);
    char *b = pb_cbmc_string(&bsize);
    size_t n;
    __CPROVER_assume(n <= LIBC_MAX + 1);

    int ret = strncmp(a, b, n);

    int expect = 0;
    for (size_t i = 0; i < n; i++) {
        unsigned char ca = (unsigned char)a[i], cb = (unsigned char)b[i];
        if (ca != cb) {
            expect = (int)ca - (int)cb;
            break;
        }
        if (ca == 0)
            break;
    }
    __CPROVER_assert(spec_sign(ret) == spec_sign(expect), "sign of the first difference");
    PB_COVER(ret < 0);
    PB_COVER(ret > 0);
    PB_COVER(ret == 0 && a[0] != 0);
}
