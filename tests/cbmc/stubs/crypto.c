/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Stub of the crypto API (src/crypto.c and its providers) for the image
 * proofs. Mbed TLS and the CAAM are not verified (doc/dev/verification.md).
 *
 * Contract: every call can fail with an error code (<= 0). hash_update and
 * hash_update_async read their whole buffer and are recorded in
 * pb_hashed[] (up to PB_CBMC_MAX_HASHED calls); hash_final writes any digest
 * of the given length. dsa_verify reads its buffers, records its arguments
 * in pb_dsa_*, and sets *verified to any value when it returns OK.
 */

#include <pb/crypto.h>
#include <pb/errors.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef PB_CBMC_MAX_HASHED
#define PB_CBMC_MAX_HASHED 4
#endif

struct pb_cbmc_hashed {
    const void *buf;
    size_t length;
};

hash_t pb_hash_alg;
bool pb_hash_open;
unsigned int pb_nhashed;
struct pb_cbmc_hashed pb_hashed[PB_CBMC_MAX_HASHED];

bool pb_dsa_called;
int pb_dsa_rc;
bool pb_dsa_verified;
size_t pb_dsa_sig_length;
size_t pb_dsa_md_length;
hash_t pb_dsa_md_alg;

static int any_rc(void)
{
    int rc;
    __CPROVER_assume(rc <= 0);
    return rc;
}

int hash_init(hash_t alg)
{
    pb_hash_alg = alg;
    pb_hash_open = true;
    pb_nhashed = 0;
    return any_rc();
}

static int record(const void *buf, size_t length)
{
    __CPROVER_assert(pb_hash_open, "hash: after hash_init");
#ifndef PB_CBMC_HASH_RAW_ADDRESSES
    /* Not for images loaded to raw memory addresses (ih-load), which are
     * not CBMC objects */
    __CPROVER_assert(__CPROVER_r_ok(buf, length), "hash: buffer readable");
#endif
    __CPROVER_assert(pb_nhashed < PB_CBMC_MAX_HASHED, "hash: within the recorded calls");
    pb_hashed[pb_nhashed].buf = buf;
    pb_hashed[pb_nhashed].length = length;
    pb_nhashed++;
    return any_rc();
}

int hash_update(const void *buf, size_t length)
{
    return record(buf, length);
}

int hash_update_async(const void *buf, size_t length)
{
    return record(buf, length);
}

int hash_final(uint8_t *digest_output, size_t length)
{
    __CPROVER_assert(pb_hash_open, "hash_final: after hash_init");
    __CPROVER_assert(__CPROVER_w_ok(digest_output, length), "hash_final: digest writable");
    __CPROVER_havoc_slice(digest_output, length);
    pb_hash_open = false;
    return any_rc();
}

int dsa_verify(dsa_t alg,
               const uint8_t *der_signature,
               size_t signature_length,
               const uint8_t *der_key,
               size_t key_length,
               hash_t md_alg,
               uint8_t *md,
               size_t md_length,
               bool *verified)
{
    (void)alg;
    __CPROVER_assert(__CPROVER_r_ok(der_signature, signature_length), "dsa: signature readable");
    __CPROVER_assert(__CPROVER_r_ok(der_key, key_length), "dsa: key readable");
    __CPROVER_assert(__CPROVER_r_ok(md, md_length), "dsa: digest readable");
    pb_dsa_called = true;
    pb_dsa_sig_length = signature_length;
    pb_dsa_md_length = md_length;
    pb_dsa_md_alg = md_alg;
    pb_dsa_rc = any_rc();
    bool v;
    pb_dsa_verified = !!v;
    if (pb_dsa_rc == PB_OK)
        *verified = pb_dsa_verified;
    return pb_dsa_rc;
}
