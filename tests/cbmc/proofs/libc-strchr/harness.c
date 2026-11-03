/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: strchr (src/lib/libc/strchr.c)
 * Not compiled into the firmware (F-030), proven anyway.
 *
 * Properties (C11 7.24.5.2): no access past the terminating NUL; returns the first occurrence of
 * (char)ch, the terminating NUL included, or NULL. Assumptions: s is a NUL terminated string.
 * Buffers are at most LIBC_MAX (6) bytes: the function works byte by byte, so longer buffers repeat
 * the same steps. Stubs: none (the firmware libc itself, no string models).
 */

#include <libc.h>
#include <string.h>

void harness(void)
{
    size_t size;
    int ch;
    char *s = pb_cbmc_string(&size);

    char *ret = strchr(s, ch);

    char *expect = NULL;
    size_t len = spec_strlen(s);
    for (size_t i = 0; i <= len; i++) {
        if (s[i] == (char)ch) {
            expect = s + i;
            break;
        }
    }
    __CPROVER_assert(ret == expect, "first occurrence of (char)ch, including the NUL");
    PB_COVER(ret != NULL && *ret != 0);
    PB_COVER(ret != NULL && *ret == 0);
    PB_COVER(ret == NULL);
}
