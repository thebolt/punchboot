/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: GPT parsing (src/drivers/partition/gpt.c)
 *
 * gpt_ptbl_init runs at board init on the disk contents, before any image is
 * verified, so the tables are attacker controlled.
 * Properties, for any disk contents and any CRC results:
 *  - no access outside the tables or the device, for both the normal path
 *    and the recovery of one table from the other;
 *  - at most 128 partitions are created (F-056);
 *  - every partition lies inside the disk, and partitions do not overlap.
 * Assumptions: the disk has 67..INT32_MAX blocks of 512 bytes (the tables
 * take 66, bio_get_last_block returns an int); a table has at most
 * GPT_PARTS (3) entries, fewer in use when an entry has first_lba 0 (any
 * count for the header check, which proves more than 128 are rejected). The entry loop does the
 * same for every entry, so 3 cover two partitions overlapping and the end of the array. Stubs: bio
 * (reads give any data, writes and allocations are recorded), crc32 (picks the path, see GPT_MODE),
 * plat_get_us_tick. gpt_write_tbl and memcpy are abstracted (see the Makefile); gpt_write_tbl is
 * the gpt-write proof. The CRC itself is the crc proof.
 */

#include <common.h>
/* Included to reach the static tables (see bio_read below) */
#include "drivers/partition/gpt.c"
#include <pb/bio.h>
#include <pb/crc.h>
#include <pb/pb.h>
#include <pb/plat.h>

#define DISK      0
#define MAX_PARTS 128
#define GPT_PARTS 3

static lba_t disk_last;
static unsigned int nparts;
static lba_t part_first[MAX_PARTS + 1];
static lba_t part_last[MAX_PARTS + 1];
static bio_call_t install_cb;

/* The CRC decides which table is valid. GPT_MODE picks the path through
 * gpt_ptbl_init, one proof each (all paths in one proof do not fit in
 * memory):
 *   0: both tables valid (the normal boot), proof gpt
 *   1: the primary table is corrupt, recovered from the backup
 *      (gpt-recover-primary)
 *   2: the backup table is corrupt, recovered from the primary
 *      (gpt-recover-backup)
 * A corrupt table has a bad header CRC or a bad entry array CRC (bad_hdr). */
#ifndef GPT_MODE
#define GPT_MODE 0
#endif
static bool bad_hdr;
/* The CRCs as read from the disk (the code zeroes hdr_crc to compute it) */
static uint32_t primary_hdr_crc, primary_array_crc, backup_hdr_crc, backup_array_crc;

uint32_t crc32(uint32_t crc, const uint8_t *buf, uint32_t size)
{
    __CPROVER_assert(__CPROVER_r_ok(buf, size), "crc32: buffer readable");
    (void)crc;
    uint32_t want = 0;
    bool bad = false;
    bool table = true;
    if (buf == (const uint8_t *)&primary.hdr) {
        want = primary_hdr_crc;
        bad = (GPT_MODE == 1) && bad_hdr;
    } else if (buf == (const uint8_t *)primary.part) {
        want = primary_array_crc;
        bad = (GPT_MODE == 1) && !bad_hdr;
    } else if (buf == (const uint8_t *)&backup.hdr) {
        want = backup_hdr_crc;
        bad = (GPT_MODE == 2) && bad_hdr;
    } else if (buf == (const uint8_t *)backup.part) {
        want = backup_array_crc;
        bad = (GPT_MODE == 2) && !bad_hdr;
    } else {
        table = false;
    }
    /* Exactly the CRC that makes the table valid, or one that never does: a
     * constant comparison lets CBMC drop the other paths before solving */
    if (table)
        return bad ? ((want ^ ~0u) ^ 1u) : (want ^ ~0u);
    uint32_t any;
    return any;
}

unsigned int plat_get_us_tick(void)
{
    unsigned int t;
    return t;
}

int bio_read(bio_dev_t dev, lba_t lba, size_t length, void *buf)
{
    __CPROVER_assert(dev == DISK, "reads the disk");
    __CPROVER_assert(__CPROVER_w_ok(buf, length), "read buffer writable");
    __CPROVER_assert((uint64_t)lba + (length + 511) / 512 - 1 <= disk_last, "read inside the disk");
    /* Any header and any GPT_PARTS entries. CBMC keeps the tables as one bit
     * vector each (about 135 kbit), so only the bytes the code reads are
     * symbolic, and no_of_parts is assigned (a constant CBMC can propagate,
     * an assumption it cannot): with no_of_parts == GPT_PARTS nothing reads
     * the other entries. Fewer entries in use are covered because the entry
     * loop stops at the first entry with first_lba 0; other counts are
     * check_header() below. */
    if (buf == &primary) {
        __CPROVER_havoc_slice(&primary.hdr, sizeof(primary.hdr));
        __CPROVER_havoc_slice(primary.part, GPT_PARTS * sizeof(primary.part[0]));
        primary.hdr.no_of_parts = GPT_PARTS;
        primary_hdr_crc = primary.hdr.hdr_crc;
        primary_array_crc = primary.hdr.part_array_crc;
    } else if (buf == &backup) {
        __CPROVER_havoc_slice(&backup.hdr, sizeof(backup.hdr));
        __CPROVER_havoc_slice(backup.part, GPT_PARTS * sizeof(backup.part[0]));
        backup.hdr.no_of_parts = GPT_PARTS;
        backup_hdr_crc = backup.hdr.hdr_crc;
        backup_array_crc = backup.hdr.part_array_crc;
    } else {
        __CPROVER_havoc_object(buf);
    }
    int rc;
    __CPROVER_assume(rc <= 0);
    return rc;
}

int bio_write(bio_dev_t dev, lba_t lba, size_t length, const void *buf)
{
    __CPROVER_assert(dev == DISK, "writes the disk");
    __CPROVER_assert(__CPROVER_r_ok(buf, length), "write buffer readable");
    __CPROVER_assert((uint64_t)lba + (length + 511) / 512 - 1 <= disk_last,
                     "write inside the disk");
    int rc;
    __CPROVER_assume(rc <= 0);
    return rc;
}

int bio_get_last_block(bio_dev_t dev)
{
    (void)dev;
    return (int)disk_last;
}

ssize_t bio_block_size(bio_dev_t dev)
{
    (void)dev;
    return 512;
}

int bio_set_install_partition_cb(bio_dev_t dev, bio_call_t cb)
{
    (void)dev;
    install_cb = cb;
    return PB_OK;
}

bio_dev_t bio_allocate_parent(bio_dev_t parent,
                              lba_t first_lba,
                              lba_t last_lba,
                              size_t block_size,
                              const uuid_t uu,
                              const char *description)
{
    (void)uu;
    __CPROVER_assert(parent == DISK && block_size == 512, "a partition of the disk");
    __CPROVER_assert(__CPROVER_r_ok(description, 37) && description[36] == '\0',
                     "a terminated description");
    __CPROVER_assert(nparts < MAX_PARTS, "at most 128 partitions");
    part_first[nparts] = first_lba;
    part_last[nparts] = last_lba;
    nparts++;
    bio_dev_t d;
    __CPROVER_assume(d > 0 || d < 0);
    return d;
}

#ifndef GPT_WRITE_PROOF
/* Any header: OK only for at most 128 entries of the right size (F-056) */
static void check_header(void)
{
    struct gpt_header hdr;
    if (gpt_has_valid_header(&hdr) == PB_OK)
        __CPROVER_assert(hdr.no_of_parts <= 128 && hdr.part_entry_sz == sizeof(struct gpt_part_hdr),
                         "the partition array fits the tables");
    PB_COVER(hdr.no_of_parts > 128);
}

void harness(void)
{
    bool any_bad_hdr;
    bad_hdr = any_bad_hdr;
    check_header();

    lba_t last;
    /* The tables need 1 + 1 + 32 blocks at the start, 32 at the end */
    __CPROVER_assume(last >= 66 && last <= INT32_MAX);
    disk_last = last;

    int rc = gpt_ptbl_init(DISK, NULL, 0);
    PB_COVER(rc == PB_OK && nparts > 1);

    /* Partitions inside the disk and not overlapping */
    unsigned int i, j;
    __CPROVER_assume(i < nparts && j < nparts && i != j);
    __CPROVER_assert(part_first[i] <= part_last[i] && part_last[i] <= disk_last,
                     "a partition lies inside the disk");
    __CPROVER_assert(part_last[i] < part_first[j] || part_last[j] < part_first[i],
                     "partitions do not overlap");
}
#endif // GPT_WRITE_PROOF
