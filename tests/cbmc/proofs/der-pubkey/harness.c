/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: der_ec_public_key_data (src/lib/der_helpers.c)
 *
 * Parses a public key from the keystore that is compiled into the firmware
 * (imx_caam.c), so the input is trusted. It has no input length parameter.
 * Properties: no undefined behaviour and no access outside the key or the
 * output buffer, for any key bytes in a buffer of up to MAX_KEY bytes.
 * Known failures: F-066 (the parser trusts the lengths in the data).
 * Assumptions: key buffer <= MAX_KEY bytes (a P-521 SubjectPublicKeyInfo is
 * 158 bytes).
 * Stubs: string models.
 */

#include <common.h>
#include <pb/der_helpers.h>
#include <pb/errors.h>

#define MAX_KEY 160

void harness(void)
{
    size_t key_length;
    size_t out_length;
    dsa_t kind;

    __CPROVER_assume(key_length > 0 && key_length <= MAX_KEY);
    __CPROVER_assume(out_length <= 160);

    uint8_t *key = pb_cbmc_alloc(key_length);
    uint8_t *out = pb_cbmc_alloc(out_length);

    int rc = der_ec_public_key_data(key, out, out_length, &kind);

    PB_COVER(rc == PB_OK);
    PB_COVER(rc != PB_OK);
}
