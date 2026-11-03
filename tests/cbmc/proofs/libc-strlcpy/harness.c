/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: strlcpy (src/lib/libc/strlcpy.c)
 * Not compiled into the firmware (F-030), proven anyway.
 *
 * Properties (POSIX.1-2024 strlcpy): no access outside the buffers; returns strlen(src); if dsize >
 * 0, dst holds the first min(len, dsize - 1) characters and a NUL, nothing after it changes.
 * Assumptions: src is a NUL terminated string; dst has dsize bytes; different objects. Buffers are
 * at most LIBC_MAX (6) bytes: the function works byte by byte, so longer buffers repeat the same
 * steps. Stubs: none (the firmware libc itself, no string models).
 */

#include <libc.h>
#include <string.h>

void harness(void)
{
    size_t ssize, dsize, j;
    char *src = pb_cbmc_string(&ssize);
    char *dst = (char *)pb_cbmc_buffer(&dsize);
    __CPROVER_assume(j < dsize);
    size_t len = spec_strlen(src);
    size_t copy = len < dsize ? len : dsize - 1;
    char old = dst[j];

    size_t ret = strlcpy(dst, src, dsize);

    __CPROVER_assert(ret == len, "returns strlen(src)");
    if (j < copy)
        __CPROVER_assert(dst[j] == src[j], "copies what fits");
    else if (j == copy)
        __CPROVER_assert(dst[j] == 0, "NUL terminated");
    else
        __CPROVER_assert(dst[j] == old, "nothing after the NUL changes");
    PB_COVER(len >= dsize && dsize > 0);
    PB_COVER(len < dsize);
}
