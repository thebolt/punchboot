/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: gpt_write_tbl (src/drivers/partition/gpt.c)
 *
 * Writes the protective MBR, the primary and the backup table, when a table
 * is recovered or a partition table is installed.
 * Properties: every write lies inside the disk, every CRC reads inside the
 * tables, for a table with GPT_WRITE_PARTS entries (3 here, as a valid
 * table recovered from the disk can have; 128 in gpt-write-128, as
 * installed) and any header.
 * Assumptions: the disk has 67..INT32_MAX blocks of 512 bytes; the primary
 * table is at LBA 1 (as gpt_ptbl_init sets it before writing).
 * Stubs: the gpt proof's (see ../gpt/harness.c); memcpy only checks its
 * preconditions, both tables have any contents and the same no_of_parts.
 */

#define GPT_WRITE_PROOF
#ifndef GPT_WRITE_PARTS
#define GPT_WRITE_PARTS 3
#endif
#include "../gpt/harness.c"

void harness(void)
{
    lba_t last;
    /* A constant count keeps the copies constant sized */
    uint32_t n = GPT_WRITE_PARTS;
    __CPROVER_assume(last >= 66 && last <= INT32_MAX);
    disk_last = last;

    /* Any headers. The entries do not decide where anything is written (the
     * CRC stub ignores them), so they stay zero to keep the tables small. */
    __CPROVER_havoc_slice(&primary.hdr, sizeof(primary.hdr));
    __CPROVER_havoc_slice(&backup.hdr, sizeof(backup.hdr));
    primary.hdr.no_of_parts = n;
    backup.hdr.no_of_parts = n;
    primary.hdr.current_lba = 1;

    int rc = gpt_write_tbl(DISK);
    PB_COVER(rc == PB_OK);
}
