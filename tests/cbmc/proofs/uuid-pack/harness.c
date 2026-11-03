/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: uuid pack/unpack, compare, copy, clear, is_null (src/lib/uuid,
 * vendored)
 *
 * Properties: unpack then pack gives the same bytes; uuid_compare orders
 * like memcmp over the 16 bytes (big endian fields, so the sign of the first
 * differing byte); compare(a, a) == 0; copy copies 16 bytes; clear gives a
 * null UUID; is_null is true exactly for 16 zero bytes.
 * Assumptions: none on the UUIDs.
 * Stubs: none (firmware libc).
 */

#include "../../../../src/lib/uuid/uuidP.h"
#include <common.h>
#include <stdbool.h>
#include <string.h>
#include <uuid.h>

static int sign(int x)
{
    return (x > 0) - (x < 0);
}

void harness(void)
{
    uuid_t a, b, c;
    struct uuid u;
    unsigned int k;
    __CPROVER_assume(k < 16);

    uuid_unpack(a, &u);
    uuid_pack(&u, c);
    __CPROVER_assert(c[k] == a[k], "unpack then pack is the identity");

    int expect = 0;
    for (unsigned int i = 0; i < 16; i++) {
        if (a[i] != b[i]) {
            expect = (int)a[i] - (int)b[i];
            break;
        }
    }
    __CPROVER_assert(sign(uuid_compare(a, b)) == sign(expect), "orders like the bytes");
    __CPROVER_assert(uuid_compare(a, a) == 0, "equal to itself");

    uuid_copy(c, a);
    __CPROVER_assert(c[k] == a[k], "copy");

    bool zero = true;
    for (unsigned int i = 0; i < 16; i++)
        zero = zero && a[i] == 0;
    __CPROVER_assert(!!uuid_is_null(a) == zero, "is_null exactly for zero bytes");
    uuid_clear(c);
    __CPROVER_assert(uuid_is_null(c), "clear gives a null UUID");
    PB_COVER(expect < 0);
    PB_COVER(expect > 0 && a[0] == b[0]);
}
