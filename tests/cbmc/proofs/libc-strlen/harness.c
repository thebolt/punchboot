/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: strlen (src/lib/libc/strlen.c)
 *
 * Properties (C11 7.24.6.3): no access past the terminating NUL; returns the index of the first
 * NUL. Assumptions: s is a NUL terminated string. Buffers are at most LIBC_MAX (6) bytes: the
 * function works byte by byte, so longer buffers repeat the same steps.
 * Stubs: none (the firmware libc itself, no string models).
 */

#include <libc.h>
#include <string.h>

void harness(void)
{
    size_t size;
    char *s = pb_cbmc_string(&size);

    size_t len = strlen(s);

    __CPROVER_assert(len == spec_strlen(s), "index of the first NUL");
    PB_COVER(len == LIBC_MAX - 1);
}
