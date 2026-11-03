/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: strtoul overflow (src/lib/libc/strtoul.c)
 *
 * Properties (C11 7.22.1.4p8): for a subject sequence of 1..MAX_DIGITS
 * digits valid in base 10, 16 or 36, the value is the number they spell, or
 * ULONG_MAX if it does not fit in unsigned long; *endptr is after the last
 * digit.
 * Assumptions: digits only (the rest of the parse is libc-strtoul).
 * MAX_DIGITS is 7: seven base 36 digits overflow a 32-bit unsigned long.
 * Stubs: none (the firmware libc itself).
 */

#include <common.h>
#include <limits.h>
/* The firmware libc declares strtoul in <string.h> */
#include <string.h>

#define MAX_DIGITS 7

static unsigned int digit(char c)
{
    if (c >= '0' && c <= '9')
        return (unsigned int)(c - '0');
    if (c >= 'a' && c <= 'z')
        return (unsigned int)(c - 'a' + 10);
    if (c >= 'A' && c <= 'Z')
        return (unsigned int)(c - 'A' + 10);
    return 99;
}

static void check(int base)
{
    size_t k;
    char *end;
    __CPROVER_assume(k > 0 && k <= MAX_DIGITS);
    char *num = pb_cbmc_alloc(k + 1);
    unsigned long long ref = 0;
    for (size_t i = 0; i < k; i++) {
        __CPROVER_assume(digit(num[i]) < (unsigned int)base);
        ref = ref * (unsigned long long)base + digit(num[i]);
    }
    num[k] = '\0';

    unsigned long v = strtoul(num, &end, base);

    __CPROVER_assert(v == (ref > ULONG_MAX ? ULONG_MAX : (unsigned long)ref),
                     "value, or ULONG_MAX if it does not fit");
    __CPROVER_assert(end == num + k, "endptr after the last digit");
    PB_COVER(ref <= ULONG_MAX && k == MAX_DIGITS);
#if PB_CBMC_WORD == 32
    /* Only a 32-bit unsigned long overflows with MAX_DIGITS digits */
    PB_COVER(ref > ULONG_MAX);
#endif
}

void harness(void)
{
    /* A constant base keeps the multiplications cheap for the solver. Base
     * 36 reaches the 32-bit overflow with 7 digits. */
    check(10);
    check(16);
    check(36);
}
