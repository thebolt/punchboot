/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: the device UUID (src/device_uuid.c, src/lib/uuid/uuid3.c)
 *
 * Properties:
 *  1. No out of bounds access for any length the platform reports within its
 *     contract and any hash result.
 *  2. A call that returns OK gives an RFC 4122 version 3 UUID; every later
 *     call gives the same one without asking the platform again.
 *  3. A failed call returns the error (the platform's, if it failed) and
 *     caches nothing: the next call asks the platform again.
 * Assumptions: two calls.
 * Stubs: plat_get_unique_id (any error, or up to *length bytes and their
 * count, like every platform), the crypto API (stubs/crypto.c: any result,
 * any digest).
 */

#include <common.h>
#include <pb/device_uuid.h>
#include <pb/errors.h>
#include <string.h>
#include <uuid.h>

const char *platform_ns_uuid = "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10";

static unsigned int plat_calls;
static int plat_rc;

int plat_get_unique_id(uint8_t *output, size_t *length)
{
    int rc;
    size_t n;

    plat_calls++;
    __CPROVER_assume(rc <= 0);
    plat_rc = rc;
    if (rc != PB_OK)
        return rc;
    __CPROVER_assume(n <= *length);
    __CPROVER_havoc_slice(output, n);
    *length = n;
    return PB_OK;
}

void harness(void)
{
    uuid_t first;
    uuid_t second;

    int rc1 = device_uuid(first);
    unsigned int calls1 = plat_calls;
    __CPROVER_assert(plat_rc == PB_OK || rc1 == plat_rc, "a platform error is returned");
    int rc2 = device_uuid(second);

    __CPROVER_assert(calls1 == 1, "the first call asks the platform");
    if (rc1 == PB_OK) {
        __CPROVER_assert((first[6] & 0xf0) == 0x30, "version 3");
        __CPROVER_assert((first[8] & 0xc0) == 0x80, "RFC 4122 variant");
        __CPROVER_assert(rc2 == PB_OK && plat_calls == 1, "cached");
        __CPROVER_assert(memcmp(first, second, 16) == 0, "the same UUID");
    } else {
        __CPROVER_assert(plat_calls == 2, "an error is not cached");
    }
    PB_COVER(rc1 == PB_OK);
    PB_COVER(rc1 != PB_OK && rc2 == PB_OK);
}
