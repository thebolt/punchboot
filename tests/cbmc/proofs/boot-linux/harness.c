/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: boot_driver_linux_prepare (src/boot/linux.c)
 *
 * Runs on a signed header after boot_image_verify_parts. Properties, for any
 * such header and any board config: no undefined behaviour; the kernel,
 * ramdisk and dtb addresses are their u64 pb-load-addr entries (D-025);
 * libfdt is only called on the dtb load address, after the header check on
 * a part that holds the header, and only with a tree whose total size is
 * inside the dtb part (F-065); the cache clean covers the patched tree.
 * Assumptions: bpak_valid_header and boot_image_verify_parts accepted the
 * header (for a part in it: its load address is a u64 and the part fits
 * above it, ih-verify-parts and ih-verify-parts-read); 3 parts and load
 * address entries as in stubs/boot_driver.h.
 * Stubs: libfdt, slc, rot, device_uuid, uuid_unparse (stubs/boot_driver.h),
 * the cache clean.
 */

#include <bpak/bpak.h>
#include <libfdt.h>

/* The tree's total size comes from the header the driver checked */
static uint32_t cbmc_fdt_totalsize(const void *fdt);
#undef fdt_totalsize
#define fdt_totalsize(fdt) cbmc_fdt_totalsize(fdt)

#include "boot/linux.c"

#define BD_PARTS      3
#define BD_EXTRA_META 0
#include <pb/arch.h>
#include <stubs/boot_driver.h>

#define ID_KERNEL  0x11u
#define ID_RAMDISK 0x22u
#define ID_DTB     0x33u

static bool cleaned;

void arch_clean_cache_range(uintptr_t start, size_t len)
{
    __CPROVER_assert(start == dtb_addr_g && len >= tree_size, "the patched tree is cleaned");
    cleaned = true;
}

void harness(void)
{
    static struct bpak_header hdr;
    static const bpak_id_t ids[BD_PARTS] = { ID_KERNEL, ID_RAMDISK, ID_DTB };
    __CPROVER_havoc_object(&hdr);
    bd_header(&hdr, ids);

    static struct boot_driver_linux_config config;
    config.image_bpak_id = ID_KERNEL;
    config.ramdisk_bpak_id = any_of(ID_RAMDISK);
    config.dtb_bpak_id = any_of(ID_DTB);
    bool patch, resolve;
    config.dtb_patch_cb = patch ? patch_cb : NULL;
    config.resolve_part_name = resolve ? part_name : NULL;
    __CPROVER_assume(boot_driver_linux_init(&config) == PB_OK);

    bd_dtb(&hdr, config.dtb_bpak_id);

    uuid_t boot_part_uu;
    int rc = boot_driver_linux_prepare(&hdr, boot_part_uu);
    PB_COVER(rc == PB_OK && config.dtb_bpak_id != 0 && config.ramdisk_bpak_id != 0);
    PB_COVER(rc == PB_OK && config.dtb_bpak_id == 0);

    if (rc != PB_OK)
        return;

    uint64_t kaddr;
    __CPROVER_assert(load_addr_of(&hdr, ID_KERNEL, &kaddr) && kaddr == jump_addr,
                     "the kernel address is its u64 load address entry");
    if (config.ramdisk_bpak_id) {
        uint64_t raddr;
        __CPROVER_assert(load_addr_of(&hdr, config.ramdisk_bpak_id, &raddr) &&
                             raddr == ramdisk_addr,
                         "the ramdisk address is its u64 load address entry");
    }
    if (config.dtb_bpak_id) {
        __CPROVER_assert(dtb_known && dtb_addr_g == dtb_addr,
                         "the dtb address is its u64 load address entry");
        __CPROVER_assert(cleaned, "the patched tree is cleaned");
    }
}
