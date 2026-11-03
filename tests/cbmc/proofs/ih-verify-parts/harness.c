/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: boot_image_verify_parts (src/boot/image_helpers.c)
 *
 * Checks that no part of a signed image loads over the running firmware.
 * Properties, for any header bpak_valid_header accepts and any memory
 * layout: no undefined behaviour (the load address is read as a u64 from
 * the part's pb-load-addr metadata); OK means every part's load range
 * [load_addr, load_addr + size) is clear of the stack, data, bss, code and
 * no_init regions, and the load address fits in uintptr_t.
 * Known failures: F-065 (CHECK_OVERLAP wraps, the u64 read has no size
 * check, the load address is truncated on 32 bit), F-148/F-149.
 * Assumptions: the regions are [start, end] with start <= end; at most 2
 * parts in use (each is checked the same way).
 * Stubs: crypto, rot (not called).
 */

#include <pb/utils_def.h>
/* The linker region symbols as plain variables, so the harness can give
 * them any values: as objects their addresses have a fixed order in CBMC,
 * which made "start <= end" contradictory and the proof vacuous */
#undef IMPORT_SYM
#define IMPORT_SYM(type, sym, name) static type name
#include "boot/image_helpers.c"

#include <boot/image_helpers.h>
#include <bpak/bpak.h>
#include <bpak/id.h>
#include <common.h>
#include <pb/crypto.h>
#include <pb/pb.h>
#include <pb/rot.h>
#include <string.h>

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

static bool overlaps(uint64_t a, uint64_t len, uintptr_t start, uintptr_t end)
{
    /* [a, a + len) and [start, end] share a byte. A range that would pass
     * 2^64 runs to the top of the address space. */
    if (len == 0 || a > end)
        return false;
    if (len - 1 > UINT64_MAX - a)
        return true;
    return a + (len - 1) >= start;
}

void harness(void)
{
    static struct bpak_header hdr;
    __CPROVER_havoc_object(&hdr);
    /* At most 2 parts in use: each part is checked on its own, and with all
     * 32 the nested metadata lookups do not fit in memory */
    __CPROVER_assume(hdr.parts[2].id == 0);
    /* Any layout of the firmware regions, each with start <= end */
    uintptr_t a, b, c, d, e, f, g, h, i2, j2;
    __CPROVER_assume(a <= b && c <= d && e <= f && g <= h && i2 <= j2);
    stack_start = a;
    stack_end = b;
    data_start = c;
    data_end = d;
    bss_start = e;
    bss_end = f;
    code_start = g;
    code_end = h;
    no_init_start = i2;
    no_init_end = j2;

    int rc = boot_image_verify_parts(&hdr);
    PB_COVER(rc == PB_OK && hdr.parts[0].id != 0);

    if (rc != PB_OK)
        return;
    /* Any part before the first empty slot */
    unsigned int i;
    __CPROVER_assume(i < 2);
    for (unsigned int j = 0; j < 2; j++)
        if (j <= i)
            __CPROVER_assume(hdr.parts[j].id != 0);
    struct bpak_meta_header *mh;
    __CPROVER_assume(bpak_get_meta(&hdr, BPAK_ID_PB_LOAD_ADDR, hdr.parts[i].id, &mh) == BPAK_OK);
    uint64_t load;
    __CPROVER_assume(mh->size >= sizeof(load));
    memcpy(&load, &hdr.metadata[mh->offset], sizeof(load));
    uint64_t size = bpak_part_size(&hdr.parts[i]);
#if PB_CBMC_WORD == 32
    PB_COVER(load > UINTPTR_MAX);
#endif
    PB_COVER(size > 0);
    __CPROVER_assert(load <= UINTPTR_MAX, "the load address fits");
    __CPROVER_assert(!overlaps(load, size, stack_start, stack_end) &&
                         !overlaps(load, size, data_start, data_end) &&
                         !overlaps(load, size, bss_start, bss_end) &&
                         !overlaps(load, size, code_start, code_end) &&
                         !overlaps(load, size, no_init_start, no_init_end),
                     "no part loads over the firmware");
}
