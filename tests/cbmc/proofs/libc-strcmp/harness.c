/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: strcmp (src/lib/libc/strcmp.c)
 *
 * Properties (C11 7.24.4.2): no access past either terminating NUL; the sign of the result is that
 * of the first differing character (as unsigned char), 0 for equal strings. Assumptions: a and b
 * are NUL terminated strings. Buffers are at most LIBC_MAX (6) bytes: the function works byte by
 * byte, so longer buffers repeat the same steps. Stubs: none (the firmware libc itself, no string
 * models).
 */

#include <libc.h>
#include <string.h>

void harness(void)
{
    size_t asize, bsize;
    char *a = pb_cbmc_string(&asize);
    char *b = pb_cbmc_string(&bsize);

    int ret = strcmp(a, b);

    int expect = 0;
    for (size_t i = 0;; i++) {
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
