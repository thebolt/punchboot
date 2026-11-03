/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: RAM log (src/ramlog.c)
 *
 * The log is in .no_init RAM, so after a reset it holds anything.
 * Properties:
 *  - Before ramlog_init nothing is written and reads return 0.
 *  - ramlog_putc writes inside the data array, at head % size, and counts
 *    the head up by one.
 *  - ramlog_read, for any head, offset and length: copies at most length
 *    and at most what is in the log after offset, never reads or writes
 *    outside the log and the buffer, and byte j is the one at position
 *    offset + j counted from the oldest byte in the ring.
 * Assumptions: any log contents and head; the buffer is length (<= 64) bytes;
 * a 1 KiB log (see below).
 * Stubs: none (RAMLOG_SYNC is off in the host config).
 */

#include <common.h>
/* cfg/config.h makes the log 1 KiB */
#include "ramlog.c"
_Static_assert((RAMLOG_SIZE & (RAMLOG_SIZE - 1)) == 0, "a power of two, like the real log");

void harness(void)
{
    /* .no_init: any contents */
    __CPROVER_havoc_object(&ramlog);
    uint32_t head = ramlog.hdr.head;

    char c;
    ramlog_putc(c);
    __CPROVER_assert(ramlog.hdr.head == head, "nothing written before init");
    __CPROVER_assert(ramlog_read(0, NULL, 0) == 0 && ramlog_length() == 0,
                     "nothing to read before init");

    ramlog_init();
    __CPROVER_havoc_slice(ramlog.data, sizeof(ramlog.data));
    uint32_t h;
    ramlog.hdr.head = h;

    ramlog_putc(c);
    __CPROVER_assert(ramlog.hdr.head == h + 1, "the head counts up");
    __CPROVER_assert(ramlog.data[h % RAMLOG_SIZE] == c, "written at head % size");

    size_t offset, length;
    __CPROVER_assume(length <= 64);
    char *buf = pb_cbmc_alloc(length);
    size_t avail = ramlog_length();
    size_t n = ramlog_read(offset, buf, length);

    __CPROVER_assert(n <= length, "at most length");
    __CPROVER_assert(offset >= avail ? n == 0 : n == MIN(length, avail - offset),
                     "what is in the log after offset");
    size_t j;
    __CPROVER_assume(j < n);
    size_t oldest = (ramlog.hdr.head - avail) % RAMLOG_SIZE;
    __CPROVER_assert(buf[j] == ramlog.data[(oldest + offset + j) % RAMLOG_SIZE],
                     "the byte at offset + j from the oldest");
    PB_COVER(n > 0 && (oldest + offset) % RAMLOG_SIZE + n > RAMLOG_SIZE);
    PB_COVER(n > 0 && avail < RAMLOG_SIZE);
}
