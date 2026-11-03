/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: boot_image_load_and_hash (src/boot/image_helpers.c)
 *
 * Loads the parts of a signed image to their load addresses in chunks and
 * hashes them.
 * Properties, for any header bpak_valid_header accepts, any chunk size and
 * any read results:
 *  - each chunk is read to load_addr + offset within its part, from image
 *    offset part_offset - 4096 + offset (the parts follow the header);
 *  - what is hashed is exactly what was just read, chunk by chunk (no byte
 *    that was not read is hashed, and the hashed chunks cover the parts);
 *  - a read error stops the load; OK means every chunk was read and hashed
 *    and hash_final was called.
 * Assumptions: at most 2 parts in use, each at most 3 chunks; the load
 * addresses are any 32-bit values (the memory behind them is not modelled).
 * Stubs: crypto (stubs/crypto.c), read_f records its arguments.
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

#define MAX_READS 6

static unsigned int nreads;
static int64_t read_off[MAX_READS];
static size_t read_len[MAX_READS];
static uintptr_t read_buf[MAX_READS];
static bool read_failed;

static int read_f(int64_t offset, size_t length, void *buf)
{
    __CPROVER_assert(nreads < MAX_READS, "within the recorded reads");
    read_off[nreads] = offset;
    read_len[nreads] = length;
    read_buf[nreads] = (uintptr_t)buf;
    nreads++;
    int rc;
    __CPROVER_assume(rc <= 0);
    if (rc != PB_OK)
        read_failed = true;
    return rc;
}

void harness(void)
{
    static struct bpak_header hdr;
    __CPROVER_havoc_object(&hdr);
    __CPROVER_assume(hdr.parts[2].id == 0);
    size_t chunk;
    __CPROVER_assume(chunk > 0 && chunk <= SIZE_MAX / 4);
    for (unsigned int i = 0; i < 2; i++)
        __CPROVER_assume((uint64_t)bpak_part_size(&hdr.parts[i]) <= 3 * (uint64_t)chunk);
    uint8_t digest[64];

    int rc = boot_image_load_and_hash(&hdr, chunk, read_f, NULL, digest, sizeof(digest));
    PB_COVER(rc == PB_OK && nreads > 1);

    /* Every hashed chunk is the chunk read just before it */
    unsigned int k;
    __CPROVER_assume(k < pb_nhashed);
    __CPROVER_assert(k < nreads && (uintptr_t)pb_hashed[k].buf == read_buf[k] &&
                         pb_hashed[k].length == read_len[k],
                     "hashes exactly what was read");
    if (rc == PB_OK)
        __CPROVER_assert(!read_failed && pb_nhashed == nreads, "OK: every chunk read and hashed");
}
