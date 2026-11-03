/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: memchr (src/lib/libc/memchr.c)
 *
 * Properties (C11 7.24.5.1): no access outside the buffer; returns the first byte in [0, n) equal
 * to (unsigned char)c (C11 7.24.5.1), or NULL. Assumptions: the buffer has at least n bytes.
 * Buffers are at most LIBC_MAX (6) bytes: the function works byte by byte, so longer buffers repeat
 * the same steps. Stubs: none (the firmware libc itself, no string models).
 */

#include <libc.h>
#include <string.h>

void harness(void)
{
    size_t size, n;
    int c;
    unsigned char *s = pb_cbmc_buffer(&size);
    __CPROVER_assume(n <= size);

    unsigned char *ret = memchr(s, c, n);

    unsigned char *expect = NULL;
    for (size_t i = 0; i < n; i++) {
        if (s[i] == (unsigned char)c) {
            expect = s + i;
            break;
        }
    }
    __CPROVER_assert(ret == expect, "first byte equal to (unsigned char)c");
    PB_COVER(ret != NULL && ret != s);
    PB_COVER(ret == NULL && n == LIBC_MAX);
}
