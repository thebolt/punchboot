/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: uuid_parse (src/lib/uuid/parse.c, vendored)
 *
 * Parses partition UUID strings (bio_get_part_by_uu_str).
 * Properties: no access past the string for any NUL terminated input; it
 * accepts exactly the canonical form (36 characters, 8-4-4-4-12 hex digits
 * with '-' between the groups); on success the UUID holds the hex digits in
 * order.
 * Assumptions: strings in a buffer of up to MAX bytes (one more than the
 * canonical length, so longer strings are covered too).
 * Stubs: none (firmware libc: strlen, strtoul).
 */

#include <common.h>
#include <stdbool.h>
#include <uuid.h>

#define MAX 38

static int hexval(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

void harness(void)
{
    size_t size, nul;
    __CPROVER_assume(size > 0 && size <= MAX && nul < size);
    char *s = pb_cbmc_alloc(size);
    s[nul] = '\0';
    uuid_t uu;

    int rc = uuid_parse(s, uu);

    size_t len = 0;
    while (s[len] != '\0')
        len++;
    bool canonical = (len == 36);
    for (size_t i = 0; i < 36 && canonical; i++) {
        if (i == 8 || i == 13 || i == 18 || i == 23)
            canonical = (s[i] == '-');
        else
            canonical = (hexval(s[i]) >= 0);
    }
    __CPROVER_assert((rc == 0) == canonical, "accepts exactly the canonical form");

    if (rc == 0) {
        /* Byte k is hex digits 2k, 2k+1 of the 32, skipping the dashes */
        unsigned int k;
        __CPROVER_assume(k < 16);
        unsigned int d = 2 * k;
        unsigned int pos = d + (d >= 8) + (d >= 12) + (d >= 16) + (d >= 20);
        __CPROVER_assert(uu[k] == (unsigned char)(hexval(s[pos]) * 16 + hexval(s[pos + 1])),
                         "bytes are the hex digits in order");
    }
    PB_COVER(rc == 0);
    PB_COVER(rc != 0 && len == 36);
}
