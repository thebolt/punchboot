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
 * Properties: no undefined behaviour, no wrap and no truncation: the size is
 * the part's real size (size + pad, or transport_size), and the offset is
 * 4096 plus the sizes of the parts before it (for a part with no empty slot
 * before it, F-148).
 * Assumptions: any header (the functions don't depend on what
 * bpak_valid_header checks); the part comes from bpak_get_part; for the
 * offset, the part is in one of the first OFFSET_PARTS slots.
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

    __CPROVER_assume(id != 0);
    __CPROVER_assume(bpak_get_part(&hdr, id, &part) == BPAK_OK);

    size_t size = bpak_part_size(part);
    if (part->flags & BPAK_FLAG_TRANSPORT) {
        __CPROVER_assert(size == part->transport_size, "transport size is not truncated");
    } else {
        __CPROVER_assert(part->size <= UINT64_MAX - part->pad_bytes &&
                             size == part->size + part->pad_bytes,
                         "part size is not truncated");
    }

    /* The offset loop is the same for every part; limit it to the first
     * OFFSET_PARTS slots to keep the solver time down. The size checks above
     * hold for every slot. */
    __CPROVER_assume(part - hdr.parts < OFFSET_PARTS);
    off_t offset = bpak_part_offset(&hdr, part);
    __CPROVER_assert(offset >= (off_t)sizeof(struct bpak_header), "offset after the header");

    /* The same sum without wrapping. If it fits and no empty slot comes
     * before the part (F-148: bpak_part_offset stops at the first one), the
     * offset must equal it. */
    __CPROVER_bool fits = 1;
    uint64_t sum = sizeof(struct bpak_header);
    for (struct bpak_part_header *p = hdr.parts; p != part; p++) {
        uint64_t sz;
        if (p->id == 0) {
            fits = 0;
            break;
        }
        if (p->flags & BPAK_FLAG_TRANSPORT) {
            sz = p->transport_size;
        } else if (p->size <= UINT64_MAX - p->pad_bytes) {
            sz = p->size + p->pad_bytes;
        } else {
            fits = 0;
            break;
        }
        /* sum <= INT64_MAX holds here */
        if (sz > SIZE_MAX || sz > (uint64_t)INT64_MAX - sum) {
            fits = 0;
            break;
        }
        sum += sz;
    }
    if (fits)
        __CPROVER_assert(offset == (off_t)sum, "offset is the sum of the earlier parts");
    PB_COVER(fits && sum > sizeof(struct bpak_header));
    PB_COVER(part != &hdr.parts[0]);
}
