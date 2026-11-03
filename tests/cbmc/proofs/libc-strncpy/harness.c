/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: strncpy (src/lib/libc/string.c)
 *
 * Properties (C11 7.24.2.4): copies not more than n characters from src to
 * dst, stopping after a null character; if src is shorter than n, null
 * characters are appended until n have been written; nothing past dst[n)
 * changes; returns dst. No access outside the buffers.
 * Assumptions: dst has n bytes; src need not be a string, it is valid up to
 * its first null character or n bytes, whichever comes first; different
 * objects. Buffers are at most LIBC_MAX (6) bytes: the
 * function works byte by byte, so longer buffers repeat the same steps.
 * Stubs: none (the firmware libc itself, no string models).
 */

#include <libc.h>
#include <string.h>

void harness(void)
{
    size_t ssize, dsize, n, j;
    char *src = (char *)pb_cbmc_buffer(&ssize);
    char *dst = (char *)pb_cbmc_buffer(&dsize);
    __CPROVER_assume(n <= dsize && j < dsize);
    /* src is valid up to its first null character or n bytes */
    size_t len = 0;
    while (len < ssize && src[len] != '\0')
        len++;
    __CPROVER_assume(len < ssize || n <= ssize);
    char old = dst[j];

    char *ret = strncpy(dst, src, n);

    __CPROVER_assert(ret == dst, "returns dst");
    __CPROVER_assert(dst[j] == (j < n ? (j < len ? src[j] : 0) : old),
                     "copies up to n characters, then pads with null characters");
    PB_COVER(n > len + 1);
    PB_COVER(n < len);
    PB_COVER(len == ssize && n == ssize && n > 0);
}
