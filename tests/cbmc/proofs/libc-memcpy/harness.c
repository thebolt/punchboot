/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: memcpy (src/lib/libc/string.c)
 *
 * Properties (C11 7.24.2.1): no access outside the buffers; dst[0, n) is src[0, n), the rest of dst
 * is unchanged; returns dst. Assumptions: dst and src are different objects with at least n bytes
 * (C11 7.24.2.1). Buffers are at most LIBC_MAX (6) bytes: the function works byte by byte, so
 * longer buffers repeat the same steps. Stubs: none (the firmware libc itself, no string models).
 */

#include <libc.h>
#include <string.h>

void harness(void)
{
    size_t dsize, ssize, n, j;
    unsigned char *dst = pb_cbmc_buffer(&dsize);
    unsigned char *src = pb_cbmc_buffer(&ssize);
    __CPROVER_assume(n <= dsize && n <= ssize);
    __CPROVER_assume(j < dsize);
    unsigned char old = dst[j];

    void *ret = memcpy(dst, src, n);

    __CPROVER_assert(ret == dst, "returns dst");
    __CPROVER_assert(dst[j] == (j < n ? src[j] : old), "copies n bytes, nothing more");
    PB_COVER(n == LIBC_MAX);
}
