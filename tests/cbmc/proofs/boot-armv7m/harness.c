/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: the bare-metal Cortex-M boot driver (src/boot/armv7m_baremetal.c),
 * which no board uses today. ARMv7-M only, so 32 bit only (words).
 *
 * Properties:
 *  1. init accepts exactly a config with a base address, an image of at
 *     least a header that does not wrap the address space, and a boot part
 *     id.
 *  2. prepare returns OK only when the header has a part with the boot part
 *     id, and then the entry point is that part's first byte: the base plus
 *     the 64-bit sum of the earlier part sizes, without truncation.
 *  3. The boot part lies inside the image region before the header
 *     (image_size bytes at the base, the header in its last 4096 bytes), and
 *     the entry point does not wrap the address space (D-025).
 *  4. jump goes to that entry point.
 * Assumptions: a header bpak_valid_header accepts (boot verified it), with
 * up to NP parts (any ids, sizes, flags) and then an empty slot, and no
 * metadata; any config.
 * Stubs: arch_disable_mmu, arch_jump (records the address).
 */

#include <common.h>

#include "../../../../src/boot/armv7m_baremetal.c"

#define NP 3

/* A part's size in the image, as bpak.c counts it (static there) */
static uint64_t part_size64(const struct bpak_part_header *p)
{
    if (p->flags & BPAK_FLAG_TRANSPORT)
        return p->transport_size;
    return p->size + p->pad_bytes;
}

static bool jumped;
static uintptr_t jump_to;

void arch_disable_mmu(void)
{
}

void arch_jump(void *addr, void *p0, void *p1, void *p2, void *p3)
{
    (void)p0;
    (void)p1;
    (void)p2;
    (void)p3;
    jumped = true;
    jump_to = (uintptr_t)addr;
}

void harness(void)
{
    struct boot_driver_armv7m_baremetal_config c;
    struct bpak_header *hdr = pb_cbmc_alloc(sizeof(*hdr));
    uuid_t uu;

    /* 1 */
    int rc = boot_driver_armv7m_baremetal_init(&c);
    __CPROVER_assert((rc == PB_OK) ==
                         (c.image_base_address != 0 && c.image_size >= sizeof(struct bpak_header) &&
                          c.image_size <= UINTPTR_MAX - c.image_base_address &&
                          c.boot_partition_id != 0),
                     "init accepts exactly a usable config");
    if (rc != PB_OK)
        return;

    hdr->parts[NP].id = 0;
    hdr->meta[0].id = 0;
    __CPROVER_assume(bpak_valid_header(hdr) == BPAK_OK);

    /* The boot part and its offset in the data, by the 64-bit sum */
    int boot_idx = -1;
    uint64_t offset = 0;
    uint64_t boot_size = 0;
    for (int i = 0; i < NP && hdr->parts[i].id != 0; i++) {
        if (hdr->parts[i].id == c.boot_partition_id) {
            boot_idx = i;
            boot_size = part_size64(&hdr->parts[i]);
            break;
        }
        offset += part_size64(&hdr->parts[i]);
    }

    /* 2, 3 */
    rc = boot_driver_armv7m_baremetal_prepare(hdr, uu);
    if (rc == PB_OK) {
        __CPROVER_assert(boot_idx >= 0, "OK only with a boot part");
        __CPROVER_assert(offset + boot_size <= c.image_size - sizeof(struct bpak_header),
                         "the boot part is inside the image");
        __CPROVER_assert((uint64_t)c.image_base_address + offset <= UINTPTR_MAX,
                         "the entry point does not wrap");
        __CPROVER_assert(jump_part_offset == offset, "the entry point is the boot part");

        /* 4 */
        boot_driver_armv7m_baremetal_jump();
        __CPROVER_assert(jumped && jump_to == c.image_base_address + offset,
                         "jumps to the entry point");
        PB_COVER(boot_idx == 2);
    } else {
        __CPROVER_assert(!jumped, "no jump");
    }
    PB_COVER(rc == PB_OK && boot_idx == 0);
    PB_COVER(rc != PB_OK && boot_idx >= 0);
    PB_COVER(rc != PB_OK && boot_idx < 0);
}
