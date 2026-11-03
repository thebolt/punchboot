/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: memcmp (src/lib/libc/memcmp.c)
 *
 * Properties (C11 7.24.4.1): no access outside the buffers; the sign of the result is that of the
 * first differing byte pair (as unsigned char), 0 if the first n bytes are equal. Assumptions: both
 * buffers have at least n bytes. Buffers are at most LIBC_MAX (6) bytes: the function works byte by
 * byte, so longer buffers repeat the same steps. Stubs: none (the firmware libc itself, no string
 * models).
 */

#include <libc.h>
#include <string.h>

void harness(void)
{
    size_t asize, bsize, n;
    unsigned char *a = pb_cbmc_buffer(&asize);
    unsigned char *b = pb_cbmc_buffer(&bsize);
    __CPROVER_assume(n <= asize && n <= bsize);

    int ret = memcmp(a, b, n);

    int expect = 0;
    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            expect = (int)a[i] - (int)b[i];
            break;
        }
    }
    __CPROVER_assert(spec_sign(ret) == spec_sign(expect), "sign of the first difference");
    PB_COVER(ret < 0);
    PB_COVER(ret > 0);
    PB_COVER(ret == 0 && n == LIBC_MAX);
}
