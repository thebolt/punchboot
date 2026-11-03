/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: uuid_unparse and uuid_parse (src/lib/uuid/unparse.c, parse.c,
 * vendored), on top of the firmware snprintf and strtoul
 *
 * Properties: for any UUID, uuid_unparse writes a 36 character lower case
 * canonical string inside its 37 byte buffer, and the two hex digits of
 * each byte are that byte (dashes between the groups). uuid-parse proves
 * that uuid_parse reads exactly those digits back, so together the two give
 * parse(unparse(u)) == u.
 * Assumptions: none on the UUID.
 * Stubs: none (firmware libc).
 */

#include <common.h>
#include <stdio.h>
#include <string.h>
#include <uuid.h>

/* CBMC (6.6) does not apply the default argument promotions to variadic
 * arguments narrower than int, so va_arg(unsigned int) on the uint16_t and
 * uint8_t fields reads past them. unparse.c is included here with its one
 * snprintf call promoting the arguments explicitly, which is what C does
 * anyway; the code is otherwise unchanged. */
#define snprintf(o, n, f, a, b, c, d, e, g, h, i, j, k, l) \
    snprintf(o,                                            \
             n,                                            \
             f,                                            \
             (unsigned int)(a),                            \
             (unsigned int)(b),                            \
             (unsigned int)(c),                            \
             (unsigned int)(d),                            \
             (unsigned int)(e),                            \
             (unsigned int)(g),                            \
             (unsigned int)(h),                            \
             (unsigned int)(i),                            \
             (unsigned int)(j),                            \
             (unsigned int)(k),                            \
             (unsigned int)(l))
#include "uuid/unparse.c"
#undef snprintf

static int hexval(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

void harness(void)
{
    uuid_t uu;
    char *str = pb_cbmc_alloc(37);

    uuid_unparse(uu, str);

    __CPROVER_assert(str[36] == '\0', "36 characters and a NUL");
    unsigned int i;
    __CPROVER_assume(i < 36);
    if (i == 8 || i == 13 || i == 18 || i == 23)
        __CPROVER_assert(str[i] == '-', "dashes between the groups");
    else
        __CPROVER_assert((str[i] >= '0' && str[i] <= '9') || (str[i] >= 'a' && str[i] <= 'f'),
                         "lower case hex digits");

    /* Byte k is hex digits 2k, 2k+1 of the 32, skipping the dashes */
    unsigned int k;
    __CPROVER_assume(k < 16);
    unsigned int d = 2 * k;
    unsigned int pos = d + (d >= 8) + (d >= 12) + (d >= 16) + (d >= 20);
    __CPROVER_assert(hexval(str[pos]) * 16 + hexval(str[pos + 1]) == uu[k],
                     "the hex digits of each byte");
    PB_COVER(uu[0] != 0);
}
