/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: memset (src/lib/libc/memset.c)
 *
 * Properties (C11 7.24.6.1): no access outside the buffer; dst[0, n) is (unsigned char)val, the
 * rest unchanged; returns dst. Assumptions: dst has at least n bytes. Buffers are at most LIBC_MAX
 * (6) bytes: the function works byte by byte, so longer buffers repeat the same steps. Stubs: none
 * (the firmware libc itself, no string models).
 */

#include <libc.h>
#include <string.h>

void harness(void)
{
    size_t size, n, j;
    int val;
    unsigned char *dst = pb_cbmc_buffer(&size);
    __CPROVER_assume(n <= size && j < size);
    unsigned char old = dst[j];

    void *ret = memset(dst, val, n);

    __CPROVER_assert(ret == dst, "returns dst");
    __CPROVER_assert(dst[j] == (j < n ? (unsigned char)val : old), "sets n bytes, nothing more");
    PB_COVER(n == LIBC_MAX);
}
