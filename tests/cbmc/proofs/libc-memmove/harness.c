/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: memmove (src/lib/libc/memmove.c)
 *
 * Properties (C11 7.24.2.2): no access outside the buffers; for any overlap, dst[0, n) holds what
 * src[0, n) held before; bytes outside dst[0, n) are unchanged; returns dst. Assumptions: dst and
 * src are valid for n bytes, in the same object (any overlap) or in different objects. Buffers are
 * at most LIBC_MAX (6) bytes: the function works byte by byte, so longer buffers repeat the same
 * steps. Stubs: none (the firmware libc itself, no string models).
 */

#include <libc.h>
#include <string.h>

void harness(void)
{
    size_t size, a, b, n, j, k;
    bool same;
    unsigned char *buf = pb_cbmc_buffer(&size);
    unsigned char *other = pb_cbmc_alloc(LIBC_MAX);
    __CPROVER_assume(a <= size && b <= size);
    __CPROVER_assume(n <= size - a && n <= size - b);
    unsigned char *dst = buf + a;
    unsigned char *src = !!same ? buf + b : other + b;
    __CPROVER_assume(j < n);
    __CPROVER_assume(k < size);
    unsigned char src_j = src[j];
    unsigned char old_k = buf[k];

    void *ret = memmove(dst, src, n);

    __CPROVER_assert(ret == dst, "returns dst");
    __CPROVER_assert(dst[j] == src_j, "dst holds the old src");
    if (k < a || k >= a + n)
        __CPROVER_assert(buf[k] == old_k, "nothing outside dst changes");
    PB_COVER(!!same && a > b && n > 1);
    PB_COVER(!!same && a < b && n > 1);
    PB_COVER(!same && n > 0);
}
