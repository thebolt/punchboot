/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: TZC-380 region setup (src/drivers/tzc/tzc380.c)
 *
 * Properties: for any configuration register and any arguments,
 * tzc380_configure_region programs a region only if the region number is
 * one of the controller's (not 0, the background region), the size is a
 * power of two from 32 KiB up to the address width, the base is aligned to
 * the size and the region lies inside the address space, and the
 * permissions are only the SP bits; it then writes exactly the base and an
 * attribute with size field log2(size) - 1 and the enable bit, into that
 * region's registers.
 * Assumptions: the controller's 4 KiB register window fits in the address
 * space.
 * Stubs: mmio (reads give any value, writes are recorded).
 */

#include <common.h>
#include <stdbool.h>
#include <stdint.h>

/* Replaces <pb/mmio.h> */
#define INCLUDE_PB_MMIO_H
static uint32_t config_reg;
static unsigned int nwrites;
static uintptr_t waddr[3];
static uint32_t wval[3];

static inline uint32_t mmio_read_32(uintptr_t addr)
{
    (void)addr;
    return config_reg;
}

static inline void mmio_write_32(uintptr_t addr, uint32_t value)
{
    __CPROVER_assert(nwrites < 3, "at most three register writes");
    waddr[nwrites] = addr;
    wval[nwrites] = value;
    nwrites++;
}

#include "drivers/tzc/tzc380.c"

void harness(void)
{
    uint32_t cfg;
    uintptr_t b;
    /* A controller base with room for its 4 KiB register window */
    __CPROVER_assume(b != 0 && b <= UINTPTR_MAX - 0x1000);
    config_reg = cfg;

    int rc = tzc380_init(b);
    if (rc != PB_OK) {
        __CPROVER_assert((cfg & 0xf) == 0, "init fails only without regions");
        return;
    }
    unsigned int nregions = (cfg & 0xf) + 1;
    unsigned int width = ((cfg >> 8) & 0x3f) + 1;

    unsigned int region;
    uint64_t rbase, size;
    uint32_t sp;
    nwrites = 0;
    rc = tzc380_configure_region(region, rbase, size, sp);
    PB_COVER(rc == PB_OK);
    PB_COVER(rc == PB_OK && width == 64);

    if (rc != PB_OK) {
        __CPROVER_assert(nwrites == 0, "nothing written on error");
        return;
    }
    __CPROVER_assert(region > 0 && region < nregions, "a region of the controller, not 0");
    __CPROVER_assert((sp & ~TZC380_SP_ALL) == 0, "only the SP bits");
    __CPROVER_assert(size != 0 && (size & (size - 1)) == 0, "a power of two");
    __CPROVER_assert(size >= 0x8000, "at least 32 KiB");
    __CPROVER_assert((rbase & (size - 1)) == 0, "aligned base");
    __CPROVER_assert(width == 64 || (rbase < (1ull << width) && size <= (1ull << width) &&
                                     rbase + size <= (1ull << width)),
                     "inside the address space");

    unsigned int log2 = 0;
    while ((1ull << log2) != size)
        log2++;
    __CPROVER_assert(nwrites == 3, "base low, base high, attributes");
    __CPROVER_assert(waddr[0] == b + 0x100 + 0x10 * region && wval[0] == (uint32_t)rbase,
                     "base low");
    __CPROVER_assert(waddr[1] == b + 0x104 + 0x10 * region && wval[1] == (uint32_t)(rbase >> 32),
                     "base high");
    __CPROVER_assert(waddr[2] == b + 0x108 + 0x10 * region &&
                         wval[2] == (sp | ((log2 - 1) << 1) | 1u),
                     "attributes: size log2(size) - 1, enabled");
}
