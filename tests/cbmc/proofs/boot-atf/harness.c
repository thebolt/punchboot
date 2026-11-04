/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: boot_driver_atf_prepare (src/boot/atf.c)
 *
 * Runs on a signed header after boot_image_verify_parts, aarch64 only (64
 * bit). Properties, for any such header and any board config: no undefined
 * behaviour; the ATF, BL33, BL32, ramdisk and dtb addresses are their u64
 * pb-load-addr entries (D-025), the BL33 and BL32 sizes their part sizes;
 * the OP-TEE arch entry is read only if it holds a u32; libfdt is only
 * called on the dtb load address, after the header check on a part that
 * holds the header, and only with a tree whose total size is inside the dtb
 * part (F-065).
 * Assumptions: bpak_valid_header and boot_image_verify_parts accepted the
 * header (for a part in it: its load address is a u64 and the part fits
 * above it, ih-verify-parts and ih-verify-parts-read); 4 parts and load
 * address entries as in stubs/boot_driver.h, then an OP-TEE arch entry of
 * any size or none.
 * Stubs: libfdt, slc, rot, device_uuid, uuid_unparse (stubs/boot_driver.h).
 */

#include <bpak/bpak.h>
#include <libfdt.h>

/* The tree's total size comes from the header the driver checked */
static uint32_t cbmc_fdt_totalsize(const void *fdt);
#undef fdt_totalsize
#define fdt_totalsize(fdt) cbmc_fdt_totalsize(fdt)

#include "boot/atf.c"

#define BD_PARTS      4
#define BD_EXTRA_META 1
#include <stubs/boot_driver.h>

#define ID_ATF   0x11u
#define ID_BL33  0x22u
#define ID_DTB   0x33u
#define ID_OTHER 0x44u

void harness(void)
{
    static struct bpak_header hdr;
    static const bpak_id_t ids[BD_PARTS] = { ID_ATF, ID_BL33, ID_DTB, ID_OTHER };
    __CPROVER_havoc_object(&hdr);
    /* An OP-TEE arch entry after the load addresses, or none */
    bool optee;
    hdr.meta[BD_PARTS].id = optee ? BPAK_ID_OPTEE_ARCH : 0;
    hdr.meta[BD_PARTS].offset = 8 * BD_PARTS;
    bd_header(&hdr, ids);

    /* BL32 and the ramdisk are optional, and either can be the fourth part */
    static struct boot_driver_atf_config config;
    config.atf_bpak_id = ID_ATF;
    config.bl33_bpak_id = ID_BL33;
    config.bl32_bpak_id = any_of(ID_OTHER);
    config.ramdisk_bpak_id = any_of(ID_OTHER);
    config.dtb_bpak_id = any_of(ID_DTB);
    bool patch, resolve;
    config.dtb_patch_cb = patch ? patch_cb : NULL;
    config.resolve_part_name = resolve ? part_name : NULL;
    __CPROVER_assume(boot_driver_atf_init(&config) == PB_OK);

    bd_dtb(&hdr, config.dtb_bpak_id);

    uuid_t boot_part_uu;
    int rc = boot_driver_atf_prepare(&hdr, boot_part_uu);
    PB_COVER(rc == PB_OK && config.dtb_bpak_id != 0 && config.bl32_bpak_id != 0 &&
             config.ramdisk_bpak_id != 0);
    PB_COVER(rc == PB_OK && config.dtb_bpak_id == 0);

    if (rc != PB_OK)
        return;

    uint64_t addr;
    __CPROVER_assert(load_addr_of(&hdr, ID_ATF, &addr) && addr == jump_addr,
                     "the ATF address is its u64 load address entry");
    __CPROVER_assert(load_addr_of(&hdr, ID_BL33, &addr) && addr == atf_bl33_info.image_base &&
                         addr == atf_bl33_ep.pc,
                     "the BL33 address is its u64 load address entry");
    __CPROVER_assert(atf_bl33_info.image_size ==
                         bpak_part_size((struct bpak_part_header *)part_of(&hdr, ID_BL33)),
                     "the BL33 size is its part size");
    if (config.bl32_bpak_id) {
        __CPROVER_assert(load_addr_of(&hdr, config.bl32_bpak_id, &addr) &&
                             addr == atf_bl32_info.image_base && addr == atf_bl32_ep.pc,
                         "the BL32 address is its u64 load address entry");
        __CPROVER_assert(
            atf_bl32_info.image_size ==
                bpak_part_size((struct bpak_part_header *)part_of(&hdr, config.bl32_bpak_id)),
            "the BL32 size is its part size");
    }
    if (config.ramdisk_bpak_id)
        __CPROVER_assert(load_addr_of(&hdr, config.ramdisk_bpak_id, &addr) && addr == ramdisk_addr,
                         "the ramdisk address is its u64 load address entry");
    if (config.dtb_bpak_id)
        __CPROVER_assert(dtb_known && dtb_addr_g == atf_bl33_ep.args.arg0,
                         "the dtb address is its u64 load address entry");
}
