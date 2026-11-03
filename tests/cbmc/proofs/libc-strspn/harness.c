/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: strspn (src/lib/libc/strspn.c)
 * Not compiled into the firmware (F-030), proven anyway.
 *
 * Properties (C11 7.24.5.6): no access past either NUL; returns the length of the initial segment
 * of s made of characters in accept. Assumptions: s and accept are NUL terminated strings. Buffers
 * are at most LIBC_MAX (6) bytes: the function works byte by byte, so longer buffers repeat the
 * same steps. Stubs: none (the firmware libc itself, no string models).
 */

#include <libc.h>
#include <string.h>

void harness(void)
{
    size_t ssize, csize;
    char *s = pb_cbmc_string(&ssize);
    char *set = pb_cbmc_string(&csize);

    size_t ret = strspn(s, set);

    size_t expect = 0;
    size_t slen = spec_strlen(s), clen = spec_strlen(set);
    for (; expect < slen; expect++) {
        bool in = false;
        for (size_t k = 0; k < clen; k++)
            in |= (s[expect] == set[k]);
        if (!in)
            break;
    }
    __CPROVER_assert(ret == expect, "length of the initial segment");
    PB_COVER(ret > 0 && ret < slen);
}
