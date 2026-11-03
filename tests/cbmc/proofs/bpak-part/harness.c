/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: bpak_part_offset, bpak_part_size (src/lib/bpak.c)
 *
 * Used to find a part's data in storage and how much to load
 * (image_helpers.c, armv7m_baremetal.c, atf.c).
 * Properties: no undefined behaviour, no wrap: the size is the part's real
 * size (size + pad, or transport_size) whenever that fits a size_t (on 32 bit
 * a larger part can't be loaded, image_helpers.c checks it), and the offset
 * is 4096 plus the real sizes of the parts before it (F-148, F-149).
 * Assumptions: the header passed bpak_valid_header (every caller checks it
 * first) and has at most OFFSET_PARTS parts; the part comes from
 * bpak_get_part.
 * Stubs: none.
 */

#include <bpak/bpak.h>
#include <common.h>

#define OFFSET_PARTS 3

void harness(void)
{
    struct bpak_header hdr;
    bpak_id_t id;
    struct bpak_part_header *part = NULL;

    /* At most OFFSET_PARTS parts: each slot is handled the same way, and
     * the 64 bit sums over all 32 slots are too slow for the solver */
    __CPROVER_assume(hdr.parts[OFFSET_PARTS].id == 0);
    __CPROVER_assume(bpak_valid_header(&hdr) == BPAK_OK);
    __CPROVER_assume(id != 0);
    __CPROVER_assume(bpak_get_part(&hdr, id, &part) == BPAK_OK);

    /* bpak_part_size returns a size_t: callers check that the real size
     * fits before they use it (boot_image_verify_parts on 32 bit) */
    uint64_t real;
    if (part->flags & BPAK_FLAG_TRANSPORT) {
        real = part->transport_size;
    } else {
        __CPROVER_assert(part->size <= UINT64_MAX - part->pad_bytes, "size + pad does not wrap");
        real = part->size + part->pad_bytes;
    }
    if (real <= SIZE_MAX)
        __CPROVER_assert(bpak_part_size(part) == real, "part size is not truncated");

    off_t offset = bpak_part_offset(&hdr, part);
    __CPROVER_assert(offset >= (off_t)sizeof(struct bpak_header), "offset after the header");

    /* The same sum in 64 bit. bpak_valid_header keeps it below INT64_MAX,
     * and bpak_get_part found no empty slot before the part. */
    uint64_t sum = sizeof(struct bpak_header);
    for (struct bpak_part_header *p = hdr.parts; p != part; p++) {
        __CPROVER_assert(p->id != 0, "no empty slot before the part");
        sum += (p->flags & BPAK_FLAG_TRANSPORT) ? p->transport_size : p->size + p->pad_bytes;
    }
    __CPROVER_assert(offset == (off_t)sum, "offset is the sum of the earlier parts");
    PB_COVER(sum > sizeof(struct bpak_header));
    PB_COVER(part != &hdr.parts[0]);
}
