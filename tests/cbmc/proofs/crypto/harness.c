/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: the crypto API dispatch (src/crypto.c)
 *
 * The image signature check goes through here to the provider (Mbed TLS or
 * the CAAM, not verified).
 * Properties:
 *  - hash_update, hash_update_async and hash_final fail with STATE before
 *    hash_init and after hash_final;
 *  - hash_init uses a registered provider that has the algorithm, or fails
 *    with NOT_SUPPORTED; every later call goes to that provider with the
 *    caller's arguments and returns its result;
 *  - dsa_verify uses a registered provider that has the algorithm, returns
 *    its result, and verified is only what the provider set; without one it
 *    returns NOT_SUPPORTED and leaves verified alone;
 *  - the provider tables never overflow.
 * Assumptions: up to two hash providers (as many as the config allows) and
 * one DSA provider, with any algorithm bits.
 * Stubs: the providers (any result, they record their arguments).
 */

#include "crypto.c"
#include <common.h>

static int calls;
static const struct hash_ops *last_hash;
static const void *last_buf;
static size_t last_len;

static int any_rc(void)
{
    int rc;
    __CPROVER_assume(rc <= 0);
    return rc;
}

#define HASH_PROVIDER(n)                                     \
    static int h##n##_init(hash_t alg)                       \
    {                                                        \
        (void)alg;                                           \
        calls++;                                             \
        return any_rc();                                     \
    }                                                        \
    static int h##n##_update(const void *buf, size_t length) \
    {                                                        \
        calls++;                                             \
        last_hash = &hash##n;                                \
        last_buf = buf;                                      \
        last_len = length;                                   \
        return any_rc();                                     \
    }                                                        \
    static int h##n##_final(uint8_t *out, size_t length)     \
    {                                                        \
        calls++;                                             \
        last_hash = &hash##n;                                \
        last_buf = out;                                      \
        last_len = length;                                   \
        return any_rc();                                     \
    }

static struct hash_ops hash0, hash1;
HASH_PROVIDER(0)
HASH_PROVIDER(1)

static bool dsa_called;
static bool dsa_verified_set;
static int dsa_rc;
static const uint8_t *dsa_sig;

static int dsa_v(const uint8_t *sig,
                 size_t sig_len,
                 const uint8_t *key,
                 size_t key_len,
                 hash_t md_alg,
                 uint8_t *md,
                 size_t md_len,
                 bool *verified)
{
    (void)sig_len;
    (void)key;
    (void)key_len;
    (void)md_alg;
    (void)md;
    (void)md_len;
    dsa_called = true;
    dsa_sig = sig;
    dsa_rc = any_rc();
    bool v;
    *verified = !!v;
    dsa_verified_set = *verified;
    return dsa_rc;
}

static struct dsa_ops dsa0 = { .name = "dsa0", .verify = dsa_v };

void harness(void)
{
    uint32_t b0, b1, bd;
    hash0 = (struct hash_ops){
        .name = "h0", .alg_bits = b0, .init = h0_init, .update = h0_update, .final = h0_final
    };
    hash1 = (struct hash_ops){
        .name = "h1", .alg_bits = b1, .init = h1_init, .update = h1_update, .final = h1_final
    };
    dsa0.alg_bits = bd;
    bool two;
    __CPROVER_assert(hash_add_ops(&hash0) == PB_OK, "first provider registered");
    bool second = false;
    if (!!two) {
        int arc = hash_add_ops(&hash1);
        second = (arc == PB_OK);
        __CPROVER_assert(second || (arc == -PB_ERR_MEM && CONFIG_CRYPTO_MAX_HASH_OPS == 1),
                         "a full table refuses");
    }
    (void)dsa_add_ops(&dsa0);
    __CPROVER_assert(no_of_hash_ops <= CONFIG_CRYPTO_MAX_HASH_OPS, "hash table bound");

    uint8_t buf[4];
    __CPROVER_assert(hash_update(buf, sizeof(buf)) == -PB_ERR_STATE, "no update before init");

    hash_t alg;
    int rc = hash_init(alg);
    bool h0 = (b0 & alg) != 0, h1 = second && (b1 & alg) != 0;
    if (!h0 && !h1)
        __CPROVER_assert(rc == -PB_ERR_NOT_SUPPORTED && hash_final(buf, 4) == -PB_ERR_STATE,
                         "no provider: not supported, no context");
    else {
        int urc = hash_update(buf, 3);
        __CPROVER_assert(last_hash == (h0 ? &hash0 : &hash1) && last_buf == buf && last_len == 3,
                         "update goes to the provider with the arguments");
        (void)urc;
        (void)hash_final(buf, 4);
        __CPROVER_assert(hash_update(buf, 1) == -PB_ERR_STATE, "no update after final");
    }
    PB_COVER(h0);

    dsa_t dalg;
    bool verified = false;
    uint8_t sig[2], key[2], md[2];
    dsa_called = false;
    rc = dsa_verify(dalg, sig, 2, key, 2, alg, md, 2, &verified);
    if ((bd & dalg) == 0) {
        __CPROVER_assert(rc == -PB_ERR_NOT_SUPPORTED && !verified && !dsa_called,
                         "no provider: not supported, not verified");
    } else {
        __CPROVER_assert(dsa_called && dsa_sig == sig && rc == dsa_rc &&
                             verified == dsa_verified_set,
                         "the provider's result and verified");
    }
    PB_COVER(rc == PB_OK && verified);
}
