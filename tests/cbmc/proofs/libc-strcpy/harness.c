/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: strcpy (src/lib/libc/strcpy.c)
 * Not compiled into the firmware (F-030), proven anyway.
 *
 * Properties (C11 7.24.2.3): no access outside the buffers; dst[0, len] is src[0, len] (with the
 * NUL), the rest of dst unchanged; returns dst. Assumptions: src is a NUL terminated string, dst a
 * different object with room for it. Buffers are at most LIBC_MAX (6) bytes: the function works
 * byte by byte, so longer buffers repeat the same steps. Stubs: none (the firmware libc itself, no
 * string models).
 */

#include <libc.h>
#include <string.h>

void harness(void)
{
    size_t ssize, dsize, j;
    char *src = pb_cbmc_string(&ssize);
    size_t len = spec_strlen(src);
    __CPROVER_assume(dsize > len && dsize <= LIBC_MAX);
    char *dst = pb_cbmc_alloc(dsize);
    __CPROVER_assume(j < dsize);
    char old = dst[j];

    char *ret = strcpy(dst, src);

    __CPROVER_assert(ret == dst, "returns dst");
    __CPROVER_assert(dst[j] == (j <= len ? src[j] : old), "copies the string and the NUL only");
    PB_COVER(len == LIBC_MAX - 1);
}
