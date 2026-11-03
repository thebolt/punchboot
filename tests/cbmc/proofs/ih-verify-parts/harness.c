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
 * layout: no undefined behaviour; OK means every part's load range
 * [load_addr, load_addr + size) is clear of the stack, data, bss, code and
 * no_init regions, and the part fits in the address space above the load
 * address (F-065 regression).
 * Assumptions: the regions are [start, end] with start <= end; one part,
 * its load address the first metadata entry at offset 0 (reading it from
 * anywhere is the ih-verify-parts-read proof; with both symbolic the solver
 * does not finish in 30 minutes).
 * Stubs: crypto, rot (not called). CHECK_OVERLAP records the range the
 * code checks, so the property is about what the code compared (that the
 * address is the metadata value is the ih-verify-parts-read proof).
 */

#include <pb/utils_def.h>
/* The linker region symbols as plain variables, so the harness can give
 * them any values: as objects their addresses have a fixed order in CBMC,
 * which made "start <= end" contradictory and the proof vacuous */
#undef IMPORT_SYM
#define IMPORT_SYM(type, sym, name) static type name
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* The range the code checks against each region, and the regions it found
 * clear of it */
static uintptr_t checked_addr;
static size_t checked_len;
static unsigned int checked_calls;
static unsigned int clear_regions;
static bool check_overlap(uintptr_t addr, size_t len, uintptr_t start, uintptr_t end);

#undef CHECK_OVERLAP
#define CHECK_OVERLAP(addr, len, start, end) check_overlap(addr, len, start, end)

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

static bool overlaps(uintptr_t a, uintptr_t len, uintptr_t start, uintptr_t end)
{
    /* [a, a + len) and [start, end] share a byte; a + len does not wrap */
    return len != 0 && a <= end && a + len > start;
}

/* CHECK_OVERLAP, recording its range. Where the code finds no overlap
 * there is none, checked per call: the five regions in one assertion made
 * the solver take more than 30 minutes at 32 bit. */
static bool check_overlap(uintptr_t addr, size_t len, uintptr_t start, uintptr_t end)
{
    if (checked_calls == 0) {
        checked_addr = addr;
        checked_len = len;
    }
    __CPROVER_assert(addr == checked_addr && len == checked_len,
                     "every region is checked with the same range");
    checked_calls++;
    bool hit = ((addr) <= (end)) && ((addr) + (len) >= (start));
    if (!hit && len <= UINTPTR_MAX - addr) {
        __CPROVER_assert(!overlaps(addr, len, start, end), "a region found clear is clear");
        if (start == stack_start && end == stack_end)
            clear_regions |= 1u;
        if (start == data_start && end == data_end)
            clear_regions |= 2u;
        if (start == bss_start && end == bss_end)
            clear_regions |= 4u;
        if (start == code_start && end == code_end)
            clear_regions |= 8u;
        if (start == no_init_start && end == no_init_end)
            clear_regions |= 16u;
    }
    return hit;
}

void harness(void)
{
    static struct bpak_header hdr;
    __CPROVER_havoc_object(&hdr);
    /* One part, its load address the only metadata entry */
    __CPROVER_assume(hdr.parts[0].id != 0 && hdr.parts[1].id == 0);
    hdr.meta[0].id = BPAK_ID_PB_LOAD_ADDR;
    hdr.meta[0].part_id_ref = hdr.parts[0].id;
    hdr.meta[0].offset = 0;
    hdr.meta[1].id = 0;
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
    PB_COVER(rc == PB_OK);

    if (rc != PB_OK)
        return;
    struct bpak_meta_header *mh = &hdr.meta[0];
    __CPROVER_assert(mh->size >= sizeof(uint64_t), "the load address is a u64");
    __CPROVER_assume(mh->size >= sizeof(uint64_t));
    uint64_t load = *bpak_get_meta_ptr(&hdr, mh, uint64_t);
    struct bpak_part_header *part = &hdr.parts[0];
    uint64_t size = (part->flags & BPAK_FLAG_TRANSPORT) ? part->transport_size
                                                        : part->size + part->pad_bytes;
    PB_COVER(size > 0);
    __CPROVER_assert(load <= UINTPTR_MAX, "the load address fits");
    __CPROVER_assert(size <= UINTPTR_MAX - load, "the part fits above the load address");
    /* The code checked the part's whole range against every region (that
     * the address is the metadata value is the ih-verify-parts-read proof) */
    __CPROVER_assert(checked_len == size, "the checked length is the part size");
    __CPROVER_assert(checked_len <= UINTPTR_MAX - checked_addr, "the checked range does not wrap");
    __CPROVER_assert(clear_regions == 31u, "no part loads over the firmware");
}
