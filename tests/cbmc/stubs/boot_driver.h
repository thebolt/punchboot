/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Shared by the boot driver proofs (boot-linux, boot-atf), included after
 * the driver source.
 *
 * libfdt is a contract on the device tree's address and size: the tree is
 * at an address from the signed metadata, so CBMC cannot follow it as a
 * pointer. Every libfdt call must be on the dtb part's load address, after
 * the header check on a part that holds the header, with a tree whose total
 * size (any value) is inside the part. A tree has at most 3 nodes.
 *
 * The header: BD_PARTS parts and load address entries at offsets 0, 8, ...,
 * each part and entry for one of the ids or another one, entries of any
 * size, then empty slots. The harness reads that layout by constant index:
 * through the getters each read is a choice over all 64 slots and 2048
 * bytes, and the proof runs out of memory (the getters are bpak-getters,
 * reading entries from anywhere is ih-verify-parts-read).
 */

#ifndef PB_CBMC_BOOT_DRIVER_H
#define PB_CBMC_BOOT_DRIVER_H

#include <bpak/bpak.h>
#include <bpak/id.h>
#include <common.h>
#include <libfdt.h>
#include <pb/rot.h>
#include <pb/slc.h>
#include <string.h>
#include <uuid.h>

/* The dtb part as the harness sees it, and the tree in it */
static bool dtb_known;
static uintptr_t dtb_addr_g;
static size_t dtb_len_g;
static uint32_t tree_size;
static bool header_checked;
static unsigned int nodes;

static void in_dtb(const void *fdt)
{
    __CPROVER_assert(header_checked, "libfdt only after the header check");
    __CPROVER_assert(dtb_known && (uintptr_t)fdt == dtb_addr_g, "libfdt only on the dtb part");
    __CPROVER_assert(tree_size <= dtb_len_g, "the tree is inside the dtb part");
}

int fdt_check_header(const void *fdt)
{
    __CPROVER_assert(dtb_known && (uintptr_t)fdt == dtb_addr_g, "the header is the dtb part's");
    __CPROVER_assert(dtb_len_g >= sizeof(struct fdt_header), "the header is inside the dtb part");
    int rc;
    __CPROVER_assume(rc <= 0);
    header_checked = (rc == 0);
    return rc;
}

static uint32_t cbmc_fdt_totalsize(const void *fdt)
{
    __CPROVER_assert(header_checked && (uintptr_t)fdt == dtb_addr_g,
                     "the total size is read from the checked header");
    return tree_size;
}

int fdt_next_node(const void *fdt, int offset, int *depth)
{
    in_dtb(fdt);
    int next;
    if (nodes++ == 3)
        return -FDT_ERR_NOTFOUND;
    __CPROVER_assume(next < 0 || next > offset);
    return next;
}

const char *fdt_get_name(const void *fdt, int nodeoffset, int *lenp)
{
    in_dtb(fdt);
    int which;
    if (which == 0)
        return NULL;
    return which == 1 ? "chosen" : "memory";
}

static int any_fdt_rc(void)
{
    int rc;
    __CPROVER_assume(rc <= 0);
    return rc;
}

int fdt_setprop(void *fdt, int nodeoffset, const char *name, const void *val, int len)
{
    in_dtb(fdt);
    __CPROVER_assert(__CPROVER_r_ok(name, 1) && __CPROVER_r_ok(val, len), "readable property");
    return any_fdt_rc();
}

int fdt_appendprop(void *fdt, int nodeoffset, const char *name, const void *val, int len)
{
    in_dtb(fdt);
    __CPROVER_assert(__CPROVER_r_ok(name, 1) && __CPROVER_r_ok(val, len), "readable property");
    return any_fdt_rc();
}

int fdt_delprop(void *fdt, int nodeoffset, const char *name)
{
    in_dtb(fdt);
    return any_fdt_rc();
}

int device_uuid(uuid_t uu)
{
    __CPROVER_havoc_object(uu);
    return PB_OK;
}

void uuid_unparse(const uuid_t uu, char *out)
{
    __CPROVER_assert(__CPROVER_w_ok(out, 37), "uuid string fits");
    out[36] = 0;
}

slc_t slc_read_status(void)
{
    slc_t slc;
    return slc;
}

size_t rot_no_of_keys(void)
{
    size_t n;
    __CPROVER_assume(n <= 2);
    return n;
}

int rot_read_key_status_by_idx(unsigned int index)
{
    int rc;
    return rc;
}

key_id_t rot_key_idx_to_id(unsigned int index)
{
    key_id_t id;
    return id;
}

static int patch_cb(void *fdt, int offset)
{
    in_dtb(fdt);
    int rc;
    return rc;
}

static const char *part_name(uuid_t part_uu)
{
    return "A";
}

/* The id, no id, or another one */
static bpak_id_t any_of(bpak_id_t id)
{
    int which;
    return which == 0 ? id : which == 1 ? 0 : 0x55u;
}

/* The part's u64 load address: the first load address entry for it, if it
 * holds a u64 (D-025) */
static bool load_addr_of(const struct bpak_header *hdr, bpak_id_t id, uint64_t *addr)
{
    for (unsigned int k = 0; k < BD_PARTS; k++) {
        if (hdr->meta[k].id == 0)
            return false;
        if (hdr->meta[k].id != BPAK_ID_PB_LOAD_ADDR || hdr->meta[k].part_id_ref != id)
            continue;
        if (hdr->meta[k].size < sizeof(*addr))
            return false;
        memcpy(addr, &hdr->metadata[8 * k], sizeof(*addr));
        return true;
    }
    return false;
}

/* The part with the id, before the first empty slot */
static const struct bpak_part_header *part_of(const struct bpak_header *hdr, bpak_id_t id)
{
    for (unsigned int i = 0; i < BD_PARTS; i++) {
        if (hdr->parts[i].id == 0)
            return NULL;
        if (hdr->parts[i].id == id)
            return &hdr->parts[i];
    }
    return NULL;
}

/* A header bpak_valid_header and boot_image_verify_parts accepted, with
 * part and entry i for ids[i] or another id, and BD_EXTRA_META more
 * entries after them (the harness havocs the header and fills those in
 * first) */
static void bd_header(struct bpak_header *hdr, const bpak_id_t ids[BD_PARTS])
{
    for (unsigned int i = 0; i < BD_PARTS; i++) {
        hdr->parts[i].id = any_of(ids[i]);
        hdr->meta[i].id = BPAK_ID_PB_LOAD_ADDR;
        hdr->meta[i].part_id_ref = any_of(ids[i]);
        hdr->meta[i].offset = 8 * i;
    }
    hdr->parts[BD_PARTS].id = 0;
    hdr->meta[BD_PARTS + BD_EXTRA_META].id = 0;
    __CPROVER_assume(bpak_valid_header(hdr) == BPAK_OK);

    /* What boot_image_verify_parts checked for every part in the image */
    for (unsigned int i = 0; i < BD_PARTS; i++) {
        struct bpak_part_header *p = &hdr->parts[i];
        uint64_t addr;
        if (p->id == 0)
            break;
        uint64_t size = (p->flags & BPAK_FLAG_TRANSPORT) ? p->transport_size
                                                         : p->size + p->pad_bytes;
        __CPROVER_assume(load_addr_of(hdr, p->id, &addr) && addr <= UINTPTR_MAX &&
                         size <= UINTPTR_MAX - addr);
    }
}

/* Where the harness expects the device tree, if the image has one */
static void bd_dtb(const struct bpak_header *hdr, bpak_id_t id)
{
    const struct bpak_part_header *dp = part_of(hdr, id);
    uint64_t daddr;

    /* The tree's total size, any value (a static starts at 0) */
    uint32_t size;
    tree_size = size;

    if (id && dp != NULL && load_addr_of(hdr, id, &daddr) && daddr <= UINTPTR_MAX) {
        dtb_known = true;
        dtb_addr_g = (uintptr_t)daddr;
        dtb_len_g = bpak_part_size((struct bpak_part_header *)dp);
    }
}

#endif /* PB_CBMC_BOOT_DRIVER_H */
