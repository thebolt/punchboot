/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: strnlen (src/lib/libc/strnlen.c)
 * Not compiled into the firmware (F-030), proven anyway.
 *
 * Properties (POSIX.1-2008 strnlen): no access past maxlen or the first NUL; returns min(maxlen,
 * index of the first NUL). Assumptions: s has at least maxlen bytes or a NUL before that. Buffers
 * are at most LIBC_MAX (6) bytes: the function works byte by byte, so longer buffers repeat the
 * same steps. Stubs: none (the firmware libc itself, no string models).
 */

#include <libc.h>
#include <string.h>

void harness(void)
{
    size_t size, maxlen;
    unsigned char *s = pb_cbmc_buffer(&size);
    __CPROVER_assume(maxlen <= size);

    size_t len = strnlen((const char *)s, maxlen);

    size_t expect = 0;
    while (expect < maxlen && s[expect] != 0)
        expect++;
    __CPROVER_assert(len == expect, "min(maxlen, strlen)");
    PB_COVER(len == maxlen && maxlen > 0);
    PB_COVER(len < maxlen);
}
