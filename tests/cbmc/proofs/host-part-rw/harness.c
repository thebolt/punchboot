/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: pb_api_partition_write and pb_api_partition_read
 * (tools/punchboot/api_partition.c), against a malicious device
 *
 * Properties, for any bytes the device sends, any transport, file and
 * malloc error: no undefined behaviour (a division by the device's number
 * of stream buffers included); both terminate; a successful write has read
 * the whole file (nothing is left out silently).
 * Assumptions: the device reports a chunk size of 0 or up to 8 KiB, at most
 * 2 partitions, each with first <= last, at most 4 blocks of up to 2 KiB and,
 * for a chunk size above 0, at most 3 chunks (this bounds the loops; each
 * pass is the same code). The file is at most its partition plus one byte
 * (larger ones are refused before the loop); a short read only at its end.
 * Stubs: the device (stubs/host_device.h), the file (read, write, lseek
 * below), bpak_valid_header (any file can be a bpak file; bpak-header
 * proves the function).
 */

#include <api.h>
#include <bpak/bpak.h>
#include <pb-tools/wire.h>
#include <stubs/host_device.h>
#include <unistd.h>

#define MAX_CHUNK   8192u
#define MAX_PARTS   2u
#define MAX_BLOCKS  4u
#define MAX_BLOCKSZ 2048u

/* The device: the first answer is the capabilities, the second the table
 * size, the third the table (both functions start with these) */
static unsigned int answers;
static uint32_t chunk;
static uint8_t no_of_entries;

static void device_answer(void *bfr, size_t sz)
{
    struct pb_result *result = bfr;

    answers++;
    if (answers == 1) {
        struct pb_result_device_caps *caps = (void *)result->response;
        chunk = caps->chunk_transfer_max_bytes;
        __CPROVER_assume(chunk <= MAX_CHUNK);
    } else if (answers == 2) {
        struct pb_result_part_table_read *tbl = (void *)result->response;
        no_of_entries = tbl->no_of_entries;
        __CPROVER_assume(no_of_entries <= MAX_PARTS);
    } else if (answers == 3 && no_of_entries > 0) {
        struct pb_result_part_table_entry *e = bfr;
        for (unsigned int i = 0; i < MAX_PARTS; i++) {
            if (i >= no_of_entries || (i + 1) * sizeof(*e) > sz)
                break;
            __CPROVER_assume(e[i].first_block <= e[i].last_block &&
                             e[i].last_block - e[i].first_block < MAX_BLOCKS);
            uint64_t blocks = e[i].last_block - e[i].first_block + 1;
            __CPROVER_assume(e[i].block_size <= MAX_BLOCKSZ);
            __CPROVER_assume(chunk == 0 || blocks * e[i].block_size <= 3 * (uint64_t)chunk);
        }
    }
}

/* The file */
static size_t file_size, file_pos;
static bool file_is_bpak;

off_t lseek(int fd, off_t offset, int whence)
{
    (void)fd;
    bool fail;
    if (fail)
        return -1;
    __CPROVER_assert(whence == SEEK_SET || whence == SEEK_END, "lseek to the start or end");
    __CPROVER_assert(offset == 0, "lseek offset 0");
    file_pos = whence == SEEK_SET ? 0 : file_size;
    return (off_t)file_pos;
}

ssize_t read(int fd, void *buf, size_t count)
{
    (void)fd;
    __CPROVER_assert(__CPROVER_w_ok(buf, count), "read into the caller's buffer");
    bool fail;
    if (fail)
        return -1;
    size_t n = file_size - file_pos < count ? file_size - file_pos : count;
    __CPROVER_havoc_slice(buf, n);
    file_pos += n;
    return (ssize_t)n;
}

ssize_t write(int fd, const void *buf, size_t count)
{
    (void)fd;
    __CPROVER_assert(__CPROVER_r_ok(buf, count), "write the caller's bytes");
    ssize_t n;
    __CPROVER_assume(n >= -1 && n <= (ssize_t)count);
    return n;
}

int bpak_valid_header(struct bpak_header *hdr)
{
    (void)hdr;
    return file_is_bpak ? BPAK_OK : -BPAK_BAD_MAGIC;
}

void harness(void)
{
    uint8_t uu[16];
    int fd;
    bool do_write;

    dev_answer = device_answer;
    /* Statics start at 0: give the file any size and kind */
    size_t size;
    bool is_bpak;
    __CPROVER_assume(size <= MAX_BLOCKS * MAX_BLOCKSZ + 1);
    file_size = size;
    file_is_bpak = is_bpak;
    file_pos = 0;

    if (do_write) {
        int rc = pb_api_partition_write(&dev_ctx, fd, uu);
        if (rc == PB_RESULT_OK)
            __CPROVER_assert(file_pos == file_size, "a successful write read the whole file");
        PB_COVER(rc == PB_RESULT_OK && file_is_bpak && file_size > sizeof(struct bpak_header));
        PB_COVER(rc == PB_RESULT_OK && !file_is_bpak && file_size > 0);
    } else {
        int rc = pb_api_partition_read(&dev_ctx, fd, uu);
        PB_COVER(rc == PB_RESULT_OK);
    }
}
