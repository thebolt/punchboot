/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: strtoul (src/lib/libc/strtoul.c)
 *
 * Properties (C11 7.22.1.4): for any NUL terminated string and the bases 0,
 * 2, 8, 10, 16 and 36, the value and *endptr are those the standard defines:
 *  - leading white-space (isspace in the "C" locale) is skipped, then an
 *    optional sign, then for base 16 (or 0) an optional 0x/0X prefix;
 *  - base 0 picks 16 for a 0x/0X prefix, 8 for a leading 0, 10 otherwise;
 *  - the subject sequence is the longest initial run of the expected form;
 *    a minus sign negates the value in unsigned long;
 *  - a value that does not fit returns ULONG_MAX;
 *  - *endptr is the first character after the subject sequence, or nptr if
 *    there is none (then the value is 0).
 * No access past the NUL. errno is not checked: the firmware has no errno,
 * so the ERANGE part of the standard does not apply.
 * Assumptions: strings up to LIBC_MAX (6) bytes. The overflow path needs
 * longer digit strings and is proven in libc-strtoul-overflow.
 * Stubs: none (the firmware libc itself).
 */

#include <libc.h>
#include <limits.h>
/* The firmware libc declares strtoul in <string.h> */
#include <string.h>

/* Value of a digit or letter (C11 7.22.1.4p3), 99 for anything else */
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

static bool spec_isspace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

/* C11 7.22.1.4, written from the standard text */
static unsigned long spec_strtoul(const char *nptr, const char **endptr, int base)
{
    const char *s = nptr;
    bool neg = false;

    while (spec_isspace(*s))
        s++;
    if (*s == '+' || *s == '-') {
        neg = (*s == '-');
        s++;
    }
    if ((base == 0 || base == 16) && s[0] == '0' && (s[1] == 'x' || s[1] == 'X') &&
        digit(s[2]) < 16) {
        s += 2;
        base = 16;
    } else if (base == 0) {
        base = (s[0] == '0') ? 8 : 10;
    }

    const char *first = s;
    unsigned long acc = 0;
    bool overflow = false;
    while (digit(*s) < (unsigned int)base) {
        unsigned long d = digit(*s);
        if (acc > (ULONG_MAX - d) / (unsigned long)base)
            overflow = true;
        else
            acc = acc * (unsigned long)base + d;
        s++;
    }

    if (s == first) {
        *endptr = nptr;
        return 0;
    }
    *endptr = s;
    if (overflow)
        return ULONG_MAX;
    return neg ? 0ul - acc : acc;
}

static void check(int base)
{
    size_t size;
    char *end;
    const char *spec_end;
    char *s = pb_cbmc_string(&size);

    unsigned long v = strtoul(s, &end, base);
    unsigned long expect = spec_strtoul(s, &spec_end, base);

    __CPROVER_assert(v == expect, "value as in C11 7.22.1.4");
    __CPROVER_assert(end == spec_end, "endptr as in C11 7.22.1.4");
    PB_COVER(s[0] == '-' && expect != 0);
    PB_COVER(spec_isspace(s[0]) && spec_end != s);
    PB_COVER(s[0] == '0' && (s[1] == 'x' || s[1] == 'X') && spec_end > s + 2);
}

void harness(void)
{
    /* One call per base: a constant base keeps the multiplications cheap
     * for the solver. 0 and 16 are the bases with their own rules, 2 and 36
     * the extremes; strtoul treats the others like these. */
    check(0);
    check(2);
    check(8);
    check(10);
    check(16);
    check(36);
}
