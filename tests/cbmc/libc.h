/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Helpers for the libc proofs (tests/cbmc/proofs/libc-*). The functions under
 * proof are the firmware's own (src/lib/libc), built with -fno-builtin and no
 * string models.
 */

#ifndef TESTS_CBMC_LIBC_H
#define TESTS_CBMC_LIBC_H

#include <common.h>
#include <stdbool.h>

/* Largest buffer in the libc proofs. The functions loop byte by byte, so a
 * longer buffer only repeats the same steps. */
#ifndef LIBC_MAX
#define LIBC_MAX 6
#endif

/* A buffer of 0..LIBC_MAX bytes with any contents; *size is its size */
static inline unsigned char *pb_cbmc_buffer(size_t *size)
{
    size_t n;
    __CPROVER_assume(n <= LIBC_MAX);
    *size = n;
    return pb_cbmc_alloc(n);
}

/* A NUL terminated string in a buffer of 1..LIBC_MAX bytes (it may have
 * more NULs; the first one ends the string) */
static inline char *pb_cbmc_string(size_t *size)
{
    size_t n;
    size_t nul;
    __CPROVER_assume(n > 0 && n <= LIBC_MAX);
    __CPROVER_assume(nul < n);
    char *s = pb_cbmc_alloc(n);
    s[nul] = '\0';
    *size = n;
    return s;
}

/* The C standard's strlen, as a specification */
static inline size_t spec_strlen(const char *s)
{
    size_t n = 0;
    while (s[n] != '\0')
        n++;
    return n;
}

static inline int spec_sign(int x)
{
    return (x > 0) - (x < 0);
}

#endif // TESTS_CBMC_LIBC_H
