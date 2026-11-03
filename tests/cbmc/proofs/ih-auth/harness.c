/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: boot_image_auth_header (src/boot/image_helpers.c)
 *
 * Checks the signature of the image header read from storage, so the header
 * is attacker controlled.
 * Properties, for any 4 KiB header:
 *  - no undefined behaviour;
 *  - it returns OK only if dsa_verify returned OK and set verified;
 *  - dsa_verify gets the header's signature (its length as in the header,
 *    at most 512 bytes), a digest of the length of the header's hash kind
 *    (32, 48, 64) and that hash kind;
 *  - exactly the 4 KiB header is hashed, once, with the signature and its
 *    size zeroed (as it was signed);
 *  - the key comes from rot_get_dsa_key for the header's key id, and only
 *    if rot_read_key_status said it is not revoked.
 * Assumptions: none on the header.
 * Stubs: crypto (stubs/crypto.c), rot (any status, any key).
 */

#include <boot/image_helpers.h>
#include <bpak/bpak.h>
#include <common.h>
#include <pb/crypto.h>
#include <pb/pb.h>
#include <pb/rot.h>

extern bool pb_dsa_called;
extern int pb_dsa_rc;
extern bool pb_dsa_verified;
extern size_t pb_dsa_sig_length;
extern size_t pb_dsa_md_length;
extern hash_t pb_dsa_md_alg;
extern unsigned int pb_nhashed;

struct pb_cbmc_hashed {
    const void *buf;
    size_t length;
};
extern struct pb_cbmc_hashed pb_hashed[];

/* The linker symbols image_helpers.c imports */
char _code_start[1], _code_end[1], _data_region_start[1], _data_region_end[1];
char _ro_data_region_start[1], _ro_data_region_end[1], _stack_start[1], _stack_end[1];
char _zero_region_start[1], _zero_region_end[1], _no_init_start[1], _no_init_end[1];

static bool status_ok;
static key_id_t key_asked;
static uint8_t key[8];

int rot_read_key_status(key_id_t id)
{
    key_asked = id;
    int rc;
    __CPROVER_assume(rc <= 0);
    status_ok = (rc == PB_OK);
    return rc;
}

int rot_get_dsa_key(key_id_t id, dsa_t *dsa_kind, const uint8_t **der_data, size_t *der_data_length)
{
    __CPROVER_assert(status_ok && id == key_asked, "a key whose status is OK");
    int rc;
    __CPROVER_assume(rc <= 0);
    if (rc == PB_OK) {
        dsa_t k;
        *dsa_kind = k;
        *der_data = key;
        *der_data_length = sizeof(key);
    }
    return rc;
}

void harness(void)
{
    static struct bpak_header hdr;
    __CPROVER_havoc_object(&hdr);
    uint16_t sig_sz = hdr.signature_sz;
    uint8_t hash_kind = hdr.hash_kind;
    key_id_t key_id = hdr.key_id;

    int rc = boot_image_auth_header(&hdr);
    PB_COVER(rc == PB_OK);
    PB_COVER(rc == -PB_ERR_AUTHENTICATION_FAILED);

    if (rc == PB_OK)
        __CPROVER_assert(pb_dsa_called && pb_dsa_rc == PB_OK && pb_dsa_verified,
                         "OK only when the signature verified");
    if (pb_dsa_called) {
        __CPROVER_assert(key_asked == key_id, "the header's key");
        __CPROVER_assert(pb_dsa_sig_length == sig_sz && sig_sz > 0 && sig_sz <= 512,
                         "the header's signature");
        size_t want = hash_kind == BPAK_HASH_SHA256 ? 32 : hash_kind == BPAK_HASH_SHA384 ? 48 : 64;
        __CPROVER_assert(hash_kind == BPAK_HASH_SHA256 || hash_kind == BPAK_HASH_SHA384 ||
                             hash_kind == BPAK_HASH_SHA512,
                         "a known hash kind");
        __CPROVER_assert(pb_dsa_md_length == want, "the digest length of the hash kind");
        __CPROVER_assert(pb_nhashed == 1 && pb_hashed[0].buf == &hdr &&
                             pb_hashed[0].length == sizeof(hdr),
                         "the whole header is hashed, once");
        unsigned int i;
        __CPROVER_assume(i < BPAK_SIGNATURE_MAX_BYTES);
        __CPROVER_assert(hdr.signature[i] == 0 && hdr.signature_sz == 0,
                         "hashed with the signature zeroed");
    }
}
