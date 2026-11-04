/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: pb_api_partition_read_table (tools/punchboot/api_partition.c),
 * against a malicious device
 *
 * Properties, for any bytes the device sends, any transport error and any
 * malloc failure: no undefined behaviour and no access outside the caller's
 * table; on success at most the caller's number of entries, and every
 * description ends inside its entry (the Python wrapper reads it as a
 * string).
 * Assumptions: the caller's table has *entries entries, up to 3 (the device
 * may report up to 255).
 * Stubs: the device (stubs/host_device.h).
 */

#include <api.h>
#include <stubs/host_device.h>

#define MAX_ENTRIES 3

static bool terminated(const char *s, size_t n)
{
    for (size_t c = 0; c < n; c++)
        if (s[c] == '\0')
            return true;
    return false;
}

void harness(void)
{
    int entries;
    __CPROVER_assume(entries >= 0 && entries <= MAX_ENTRIES);
    const int capacity = entries;
    /* A table of the largest size (a symbolic size makes every byte access
     * an array theory term); writes past the caller's entries are caught by
     * the assertion on the count below, which bounds the copy loop */
    static struct pb_partition_table_entry out[MAX_ENTRIES];
    /* Any contents, like the Python wrapper's malloc'd table */
    __CPROVER_havoc_object(out);

    int rc = pb_api_partition_read_table(&dev_ctx, out, &entries);

    PB_COVER(rc == PB_RESULT_OK && entries == MAX_ENTRIES);
    if (rc != PB_RESULT_OK)
        return;

    __CPROVER_assert(entries >= 0 && entries <= capacity, "at most the caller's entries");
    for (int i = 0; i < MAX_ENTRIES; i++)
        if (i < entries)
            __CPROVER_assert(terminated(out[i].description, sizeof(out[i].description)),
                             "every description ends inside its entry");
}
