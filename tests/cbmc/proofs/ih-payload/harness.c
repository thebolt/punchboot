/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: boot_image_verify_payload (src/boot/image_helpers.c)
 *
 * Properties, for any header and digest buffer: no access outside the
 * header's payload_hash or the 64 byte digest; OK only for a known hash kind
 * and a digest equal to payload_hash in the hash kind's length.
 * Assumptions: the digest buffer is 64 bytes (all callers).
 * Stubs: crypto, rot (not called).
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
    uint8_t *digest = pb_cbmc_alloc(64);

    int rc = boot_image_verify_payload(&hdr, digest);
    PB_COVER(rc == PB_OK);
    PB_COVER(rc == -PB_ERR_BAD_PAYLOAD);
    if (rc == PB_OK) {
        size_t len = hdr.hash_kind == BPAK_HASH_SHA256   ? 32
                     : hdr.hash_kind == BPAK_HASH_SHA384 ? 48
                                                         : 64;
        __CPROVER_assert(hdr.hash_kind == BPAK_HASH_SHA256 || hdr.hash_kind == BPAK_HASH_SHA384 ||
                             hdr.hash_kind == BPAK_HASH_SHA512,
                         "a known hash kind");
        size_t j;
        __CPROVER_assume(j < len);
        __CPROVER_assert(digest[j] == hdr.payload_hash[j], "the digest matches");
    }
}
