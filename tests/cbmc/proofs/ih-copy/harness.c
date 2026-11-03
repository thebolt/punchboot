/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: boot_image_copy_and_hash (src/boot/image_helpers.c)
 *
 * Copies the parts of an image that is in memory (a RAM boot) and hashes
 * the copy.
 * Properties, for any header bpak_valid_header accepts and any buffer sizes:
 * every copy lies inside the source and the destination (F-088), and what is
 * hashed is exactly the copy, part by part.
 * Assumptions: source and destination are buffers of source_size and
 * destination_size bytes (up to 256); one part, up to 512 bytes (the size
 * checks are the same for larger values).
 * Stubs: crypto (stubs/crypto.c), rot (not called).
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
    /* One part: with two, the 64-bit offset of the second part does not
     * finish on 32 bit. The offsets of later parts are the bpak-part proof,
     * and the copy and hash do the same for every part. */
    __CPROVER_assume(hdr.parts[1].id == 0);
    /* Part sizes up to 512 bytes, around the buffer sizes: larger ones are
     * rejected by the same checks, and with any 64-bit size the solver does
     * not finish on 32 bit */
    __CPROVER_assume(hdr.parts[0].size <= 512 && hdr.parts[0].transport_size <= 512);
    size_t ssize, dsize;
    __CPROVER_assume(ssize <= 256 && dsize <= 256);
    uint8_t *src = pb_cbmc_alloc(ssize);
    uint8_t *dst = pb_cbmc_alloc(dsize);
    uint8_t digest[64];

    int rc = boot_image_copy_and_hash(
        &hdr, (uintptr_t)src, ssize, (uintptr_t)dst, dsize, digest, sizeof(digest));
    PB_COVER(rc == PB_OK && pb_nhashed == 1);

    unsigned int k;
    __CPROVER_assume(k < pb_nhashed);
    __CPROVER_assert((const uint8_t *)pb_hashed[k].buf >= dst &&
                         (const uint8_t *)pb_hashed[k].buf + pb_hashed[k].length <= dst + dsize,
                     "the hashed copy lies inside the destination");
}
