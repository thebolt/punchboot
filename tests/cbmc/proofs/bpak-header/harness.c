/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: bpak_valid_header (src/lib/bpak.c)
 *
 * Runs on the header read from storage before the signature is checked
 * (boot_image_auth_header), so the header is attacker controlled.
 * Properties: no undefined behaviour for any 4096 byte header; OK means magic,
 * hash kind, part alignment and metadata bounds hold for every slot before the
 * first empty one.
 * Assumptions: none.
 * Stubs: none.
 */

#include <bpak/bpak.h>
#include <common.h>

void harness(void)
{
    struct bpak_header hdr;
    unsigned int i;

    int rc = bpak_valid_header(&hdr);

    PB_COVER(rc == BPAK_OK);
    PB_COVER(rc == -BPAK_BAD_MAGIC);
    PB_COVER(rc == -BPAK_BAD_ALIGNMENT);
    PB_COVER(rc == -BPAK_SIZE_ERROR);
    PB_COVER(rc == -BPAK_NOT_SUPPORTED);

    if (rc != BPAK_OK)
        return;

    __CPROVER_assert(hdr.magic == BPAK_HEADER_MAGIC, "OK implies magic");
    __CPROVER_assert(hdr.hash_kind != 0, "OK implies a hash kind");

    /* Any slot before the first empty one */
    unsigned int used_meta = BPAK_MAX_META;
    for (unsigned int j = 0; j < BPAK_MAX_META; j++) {
        if (hdr.meta[j].id == 0) {
            used_meta = j;
            break;
        }
    }
    __CPROVER_assume(i < used_meta);
    __CPROVER_assert(hdr.meta[i].offset + hdr.meta[i].size <= BPAK_METADATA_BYTES,
                     "OK implies metadata in bounds");

    unsigned int used_parts = BPAK_MAX_PARTS;
    for (unsigned int j = 0; j < BPAK_MAX_PARTS; j++) {
        if (hdr.parts[j].id == 0) {
            used_parts = j;
            break;
        }
    }
    unsigned int k;
    __CPROVER_assume(k < used_parts);
    /* (size + pad) % 512 without the u64 wrap of the sum */
    __CPROVER_assert(
        (hdr.parts[k].size % BPAK_PART_ALIGN + hdr.parts[k].pad_bytes) % BPAK_PART_ALIGN == 0,
        "OK implies part alignment");
    PB_COVER(used_meta > 0 && used_parts > 0);
}
