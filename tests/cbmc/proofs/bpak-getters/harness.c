/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: bpak_get_meta, bpak_get_meta_anyref, bpak_get_part (src/lib/bpak.c)
 *
 * The getters run on a header that bpak_valid_header accepted (and, in the
 * boot flow, after its signature was checked).
 * Properties: no undefined behaviour; a returned entry is one of the header's
 * slots, has the requested id (and part ref), and for metadata its
 * [offset, offset + size) lies inside the metadata array, so
 * bpak_get_meta_ptr() is in bounds.
 * Assumptions: bpak_valid_header returned OK; the requested id is not 0 (all
 * callers pass a fixed nonzero id).
 * Stubs: none.
 */

#include <bpak/bpak.h>
#include <common.h>

void harness(void)
{
    struct bpak_header hdr;
    bpak_id_t id;
    bpak_id_t ref;
    struct bpak_meta_header *meta = NULL;
    struct bpak_part_header *part = NULL;
    int rc;

    __CPROVER_assume(bpak_valid_header(&hdr) == BPAK_OK);
    __CPROVER_assume(id != 0);

    /* Slots before the first empty one were checked by bpak_valid_header */
    unsigned int used_meta = BPAK_MAX_META;
    for (unsigned int j = 0; j < BPAK_MAX_META; j++) {
        if (hdr.meta[j].id == 0) {
            used_meta = j;
            break;
        }
    }

    rc = bpak_get_meta(&hdr, id, ref, &meta);
    PB_COVER(rc == BPAK_OK);
    if (rc == BPAK_OK && (unsigned int)(meta - hdr.meta) < used_meta) {
        __CPROVER_assert(meta->offset + meta->size <= BPAK_METADATA_BYTES,
                         "checked meta data inside the metadata array");
    }
    if (rc == BPAK_OK) {
        __CPROVER_assert(meta >= &hdr.meta[0] && meta < &hdr.meta[BPAK_MAX_META],
                         "meta is a header slot");
        __CPROVER_assert(meta->id == id && meta->part_id_ref == ref, "meta matches");
        __CPROVER_assert(meta->offset + meta->size <= BPAK_METADATA_BYTES,
                         "meta data inside the metadata array");
    } else {
        __CPROVER_assert(meta == NULL, "meta is NULL when not found");
    }

    rc = bpak_get_meta_anyref(&hdr, id, &meta);
    PB_COVER(rc == BPAK_OK);
    if (rc == BPAK_OK) {
        __CPROVER_assert(meta->id == id, "anyref meta matches");
        __CPROVER_assert(meta->offset + meta->size <= BPAK_METADATA_BYTES,
                         "anyref meta data inside the metadata array");
    }

    rc = bpak_get_part(&hdr, id, &part);
    PB_COVER(rc == BPAK_OK);
    if (rc == BPAK_OK) {
        __CPROVER_assert(part >= &hdr.parts[0] && part < &hdr.parts[BPAK_MAX_PARTS],
                         "part is a header slot");
        __CPROVER_assert(part->id == id, "part matches");
        __CPROVER_assert((part->size % BPAK_PART_ALIGN + part->pad_bytes) % BPAK_PART_ALIGN == 0,
                         "part is aligned");
    }
}
