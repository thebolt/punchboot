/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: der_ecsig_to_rs (src/lib/der_helpers.c)
 *
 * Parses the DER ECDSA signature from the image header before it is verified
 * (imx_caam.c), so the input is attacker controlled.
 * Properties: no undefined behaviour and no access outside sig[0, sig_length)
 * or r/s[0, length) for any input; on OK the first byte is a SEQUENCE tag.
 * Assumptions: sig_length <= MAX_SIG and length <= MAX_LEN, smaller than
 * real signatures (up to 139 bytes) and curves (32, 48, 66 bytes). The parser
 * only moves forward and uses length only in comparisons and as a copy size,
 * so the small sizes reach the same branches (integer shorter than, equal to,
 * longer than length; leading zeros; truncated input; long-form lengths).
 * Stubs: string models.
 */

#include <common.h>
#include <pb/der_helpers.h>
#include <pb/errors.h>

#define MAX_SIG 24
#define MAX_LEN 8

void harness(void)
{
    size_t sig_length;
    size_t length;

    __CPROVER_assume(sig_length <= MAX_SIG);
    __CPROVER_assume(length > 0 && length <= MAX_LEN);

    uint8_t *sig = pb_cbmc_alloc(sig_length);
    uint8_t *r = pb_cbmc_alloc(length);
    uint8_t *s = pb_cbmc_alloc(length);

    int rc = der_ecsig_to_rs(sig, sig_length, r, s, length);

    PB_COVER(rc == PB_OK);
    PB_COVER(rc == -PB_ERR_BUF_TOO_SMALL);
    PB_COVER(rc == -PB_ERR_ASN1 && sig_length > 8);
    PB_COVER(rc == PB_OK && sig[1] & 0x80);

    if (rc == PB_OK)
        __CPROVER_assert(sig_length > 0 && sig[0] == 0x30, "OK implies a SEQUENCE");
}
