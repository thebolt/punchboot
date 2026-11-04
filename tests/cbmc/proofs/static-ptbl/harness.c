/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: static partition table (src/drivers/partition/static.c, the i.MX RT
 * NOR flash)
 *
 * Properties, for any table of up to NENT entries and any parent device:
 *  1. A NULL table or a parent that is not a device is refused, and no
 *     partition is made from a parent that is not a device.
 *  2. On PB_OK there is one partition per entry, in order, with the entry's
 *     UUID; it starts at the entry's first_lba, or right after the previous
 *     one when that is 0; it has exactly the entry's size in blocks.
 *  3. On PB_OK the partitions do not overlap.
 *  4. Every partition asked for lies inside the parent (bio_allocate_parent
 *     refuses the others, proven in bio-child).
 * Assumptions: a block size of 512 (as in bio: with any block size the
 * divisions do not finish); the table is board configuration (trusted, but
 * any values here).
 * Stubs: bio_block_size and bio_allocate_parent, by their contract in
 * src/bio.c (proven in bio, bio-child), recording each partition.
 */

#include <common.h>
#include <drivers/partition/static.h>
#include <pb/bio.h>
#include <pb/pb.h>

#define NENT 3

/* The parent device */
static bool parent_valid;
static lba_t parent_first;
static lba_t parent_last;
static size_t parent_bs;

/* The partitions made */
static int n_parts;
static lba_t part_first[NENT];
static lba_t part_last[NENT];
static const unsigned char *part_uu[NENT];

ssize_t bio_block_size(bio_dev_t dev)
{
    __CPROVER_assert(dev == 0, "block size of the parent");
    if (!parent_valid)
        return -PB_ERR_IO;
    return (ssize_t)parent_bs;
}

bio_dev_t bio_allocate_parent(bio_dev_t parent,
                              lba_t first_lba,
                              lba_t last_lba,
                              size_t block_size,
                              const uuid_t uu,
                              const char *description)
{
    bool pool_full;

    (void)description;
    if (parent != 0 || !parent_valid)
        return -PB_ERR_IO;
    /* 4 */
    if (first_lba < parent_first || last_lba > parent_last)
        return -PB_ERR_PARAM;
    if (pool_full)
        return -PB_ERR_MEM;
    if (block_size == 0 || first_lba > last_lba || last_lba >= INT32_MAX)
        return -PB_ERR_PARAM;

    __CPROVER_assert(block_size == parent_bs, "partitions use the parent's block size");
    __CPROVER_assert(n_parts < NENT, "at most one partition per entry");
    part_first[n_parts] = first_lba;
    part_last[n_parts] = last_lba;
    part_uu[n_parts] = uu;
    n_parts++;
    return n_parts;
}

void harness(void)
{
    static const unsigned char uus[NENT][16];
    struct static_part_table table[NENT + 1];
    int n_ent;
    bool null_table;
    bool valid;
    lba_t first, last;

    /* Statics start at 0: any values through locals */
    parent_valid = valid;
    parent_first = first;
    parent_last = last;
    parent_bs = 512;
    __CPROVER_assume(parent_first <= parent_last && parent_last < INT32_MAX);
    n_parts = 0;

    __CPROVER_assume(n_ent >= 0 && n_ent <= NENT);
    for (int i = 0; i < NENT + 1; i++) {
        table[i].uu = (i < n_ent) ? uus[i] : NULL;
        table[i].description = "part";
    }

    int rc = static_ptbl_init(0, null_table ? NULL : table);

    /* 1 */
    if (null_table)
        __CPROVER_assert(rc == -PB_ERR_PARAM, "NULL table refused");
    if (!parent_valid) {
        __CPROVER_assert(n_parts == 0, "nothing made from an invalid parent");
        __CPROVER_assert(null_table || n_ent == 0 || rc != PB_OK, "invalid parent refused");
    }

    if (rc == PB_OK && !null_table) {
        /* 2 */
        __CPROVER_assert(n_parts == n_ent, "one partition per entry");
        uint64_t next = 0;
        for (int i = 0; i < n_ent; i++) {
            uint64_t start = table[i].first_lba ? table[i].first_lba : next;
            __CPROVER_assert(part_uu[i] == table[i].uu, "the entry's UUID, in order");
            __CPROVER_assert(part_first[i] == start, "starts at first_lba or after the previous");
            __CPROVER_assert(((uint64_t)part_last[i] - part_first[i] + 1) * parent_bs ==
                                 table[i].size,
                             "exactly the entry's size");
            next = (uint64_t)part_last[i] + 1;
            /* 3 */
            for (int j = 0; j < i; j++)
                __CPROVER_assert(part_last[j] < part_first[i] || part_last[i] < part_first[j],
                                 "partitions do not overlap");
        }
        PB_COVER(n_ent == NENT);
        PB_COVER(n_ent == NENT && table[1].first_lba != 0);
    }
    PB_COVER(rc == -PB_ERR_ALIGN);
    PB_COVER(rc == -PB_ERR_PARAM && !null_table && parent_valid);
}
