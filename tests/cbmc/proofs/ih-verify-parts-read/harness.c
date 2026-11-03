/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: part_load_addr (src/boot/image_helpers.c), the load address read
 * of boot_image_verify_parts and boot_image_load_and_hash
 *
 * Properties, for any header bpak_valid_header accepts: no undefined
 * behaviour reading the load address from anywhere in the metadata; OK
 * means the pb-load-addr entry of the part is a u64, the address returned
 * is its value, it fits a uintptr_t and the part fits above it (F-065
 * regression).
 * Assumptions: at most 4 metadata entries in use (two load addresses and two
 * other entries cover every order of the lookups; the getters stop at the
 * first empty slot, F-148).
 */

#include <pb/utils_def.h>
#undef IMPORT_SYM
#define IMPORT_SYM(type, sym, name) static type name
#include "boot/image_helpers.c"

#include <bpak/bpak.h>
#include <bpak/id.h>
#include <common.h>
#include <pb/pb.h>

void harness(void)
{
    static struct bpak_header hdr;
    __CPROVER_havoc_object(&hdr);
    __CPROVER_assume(hdr.meta[4].id == 0);
    __CPROVER_assume(hdr.parts[0].id != 0 && hdr.parts[1].id == 0);
    __CPROVER_assume(bpak_valid_header(&hdr) == BPAK_OK);

    struct bpak_part_header *part = &hdr.parts[0];
    uintptr_t addr;
    int rc = part_load_addr(&hdr, part, &addr);
    PB_COVER(rc == PB_OK);
    PB_COVER(rc == -PB_ERR_MEM);

    if (rc != PB_OK)
        return;
    struct bpak_meta_header *mh;
    __CPROVER_assume(bpak_get_meta(&hdr, BPAK_ID_PB_LOAD_ADDR, part->id, &mh) == BPAK_OK);
    __CPROVER_assert(mh->size >= sizeof(uint64_t), "the load address is a u64");
    __CPROVER_assume(mh->size >= sizeof(uint64_t));
    uint64_t load = *bpak_get_meta_ptr(&hdr, mh, uint64_t);
    uint64_t size = (part->flags & BPAK_FLAG_TRANSPORT) ? part->transport_size
                                                        : part->size + part->pad_bytes;
    __CPROVER_assert(addr == load, "the address is the metadata value");
    __CPROVER_assert(size <= UINTPTR_MAX - load, "the part fits above the load address");
}
