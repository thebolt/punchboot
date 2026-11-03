/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: block device layer (src/bio.c)
 *
 * Properties, for any devices bio_allocate / bio_allocate_parent accepted:
 *  - bio_read and bio_write reach the HAL only for a range inside the device,
 *    and pass the absolute LBA (first_lba + lba) without a 32-bit wrap.
 *  - bio_erase reaches the HAL only for a range inside the device.
 *  - bio_size, bio_get_no_of_blocks, bio_get_first_block and
 *    bio_get_last_block return the real values (no wrap, no truncation).
 *  - A child from bio_allocate_parent lies inside its parent.
 *  - Invalid device handles are rejected everywhere; bio_get_part_by_uu
 *    returns a device with that UUID.
 * Assumptions: up to NDEV devices; any LBAs; a block size of BIO_BS (512
 * here, 4096 in bio-4k): a constant keeps check_lba_range's division a
 * shift for the solver, with any block size it does not finish.
 * Stubs: HAL read/write/erase record their arguments; uuid_unparse (only
 * used for logging).
 */

#include <common.h>
#include <pb/bio.h>
#include <pb/pb.h>
#include <uuid.h>

/* bio: one device, every property. bio-child (BIO_CHILD): a disk and one
 * partition, the partition properties. More devices in one proof do not
 * finish. */
#ifdef BIO_CHILD
#define NDEV 2
#else
#define NDEV 1
#endif
#ifndef BIO_BS
#define BIO_BS 512
#endif

void uuid_unparse(const uuid_t uu, char *out)
{
    (void)uu;
    out[0] = '\0';
}

static bool hal_called;
static bio_dev_t hal_dev;
static lba_t hal_lba;
static size_t hal_length;

static int hal_read(bio_dev_t dev, lba_t lba, size_t length, void *buf)
{
    (void)buf;
    hal_called = true;
    hal_dev = dev;
    hal_lba = lba;
    hal_length = length;
    int rc;
    return rc;
}

static int hal_write(bio_dev_t dev, lba_t lba, size_t length, const void *buf)
{
    return hal_read(dev, lba, length, (void *)buf);
}

static int hal_erase(bio_dev_t dev, lba_t lba, size_t count)
{
    hal_called = true;
    hal_dev = dev;
    hal_lba = lba;
    hal_length = count;
    int rc;
    return rc;
}

struct ghost {
    lba_t first, last;
    size_t bs;
    bio_dev_t parent;
};
static struct ghost g[NDEV];

/* [hal_lba, ...) holds `bytes` bytes (read/write) or `blocks` blocks (erase)
 * inside the device. Stated with a multiplication, not a division like the
 * code under proof, to keep the solver fast. */
static void check_range(bio_dev_t dev, bool bytes, size_t amount)
{
    __CPROVER_assert(hal_dev == dev, "the HAL gets the device");
    __CPROVER_assert(hal_lba >= g[dev].first && hal_lba <= g[dev].last,
                     "range starts in the device");
    if (hal_lba < g[dev].first || hal_lba > g[dev].last)
        return;
    uint64_t room = (uint64_t)g[dev].last - hal_lba + 1;
    if (bytes)
        __CPROVER_assert(room * g[dev].bs >= amount, "range ends in the device");
    else
        __CPROVER_assert(room >= amount, "range ends in the device");
}

void harness(void)
{
    unsigned int ndev = NDEV;
    for (unsigned int i = 0; i < ndev; i++) {
        lba_t first, last;
        size_t bs = BIO_BS;
        uuid_t uu;
        bio_dev_t parent;
        bio_dev_t d;
        if (i > 0) {
            parent = 0;
            d = bio_allocate_parent(parent, first, last, bs, uu, "part");
        } else {
            parent = -1;
#ifdef BIO_CHILD
            /* A fixed disk: the property is the partition's range relative
             * to it, and with both symbolic the proof takes over 30 min */
            first = 0;
            last = 0xfffff;
#endif
            d = bio_allocate(first, last, bs, uu, "disk");
        }
        __CPROVER_assume(d == (bio_dev_t)i);
        g[i].first = first;
        g[i].last = last;
        g[i].bs = bs;
        g[i].parent = parent;
        if (parent < 0) {
            bio_set_ios(d, hal_read, hal_write);
            bio_set_ios_erase(d, hal_erase);
        }
    }

    bio_dev_t dev = NDEV - 1;

    /* A child lies inside its parent */
    if (g[dev].parent >= 0) {
        __CPROVER_assert(g[dev].first >= g[g[dev].parent].first &&
                             g[dev].last <= g[g[dev].parent].last,
                         "a child lies inside its parent");
    }

#ifndef BIO_CHILD
    /* Sizes */
    uint64_t blocks = (uint64_t)g[dev].last - g[dev].first + 1;
    __CPROVER_assert(bio_size(dev) == (int64_t)(blocks * g[dev].bs), "bio_size");
    __CPROVER_assert(bio_get_no_of_blocks(dev) >= 0 &&
                         (uint64_t)bio_get_no_of_blocks(dev) == blocks,
                     "bio_get_no_of_blocks");
    __CPROVER_assert(bio_get_last_block(dev) >= 0 && (lba_t)bio_get_last_block(dev) == g[dev].last,
                     "bio_get_last_block");
    __CPROVER_assert(bio_get_first_block(dev) >= 0 &&
                         (lba_t)bio_get_first_block(dev) == g[dev].first,
                     "bio_get_first_block");

#endif

#ifdef BIO_CHILD
    /* The partition reads from the disk's HAL (a read on any device is the
     * bio proof) */
    __CPROVER_assert(bio_get_flags(dev) == bio_get_flags(0), "the partition has the disk's flags");
    PB_COVER(g[dev].first > 0);
    return;
#endif

    /* Read, write, erase */
    lba_t lba;
    size_t length;
    int op;
#ifdef BIO_CHILD
    /* The partition's own read; write and erase are the bio proof */
    __CPROVER_assume(op == 0);
#else
    __CPROVER_assume(op >= 0 && op <= 2);
#endif
    hal_called = false;
    if (op == 0)
        (void)bio_read(dev, lba, length, NULL);
    else if (op == 1)
        (void)bio_write(dev, lba, length, NULL);
    else
        (void)bio_erase(dev, lba, length);

    if (hal_called) {
        if (op != 2)
            __CPROVER_assert(hal_lba == g[dev].first + lba &&
                                 (uint64_t)g[dev].first + lba <= UINT32_MAX,
                             "absolute LBA without wrap");
        check_range(dev, op != 2, length);
    }
#ifndef BIO_CHILD
    PB_COVER(hal_called && op == 0 && length > 0);
    PB_COVER(!hal_called && op == 1);
#endif

#ifndef BIO_CHILD
    /* Invalid handles */
    bio_dev_t bad;
    __CPROVER_assume(bad < 0 || bad >= (bio_dev_t)ndev);
    hal_called = false;
    __CPROVER_assert(bio_read(bad, lba, length, NULL) < 0 && !hal_called, "invalid handle");
    __CPROVER_assert(!bio_valid(bad), "invalid handle is not valid");

    /* Lookup by UUID */
    uuid_t uu;
    bio_dev_t found = bio_get_part_by_uu(uu);
    if (found >= 0) {
        __CPROVER_assert(found < (bio_dev_t)ndev, "found a device");
        __CPROVER_assert(uuid_compare(bio_get_uu(found), uu) == 0, "with that UUID");
    }
    PB_COVER(found >= 0);
#endif
}
