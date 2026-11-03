/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: the command mode handlers (src/cm/cm_main.c)
 *
 * After authentication (or on an open device) every command runs on host
 * data. Properties, for any command bytes, any data the host sends and any
 * results of the functions the handlers call: no undefined behaviour, and
 * every transport read and write is inside a valid buffer (in particular
 * the stream buffers and the result).
 * Assumptions: authenticated; CM_COMMAND picks the command (one proof per
 * command, cm-*: all commands in one proof do not fit in memory).
 * Stubs: the transport (cfg->tops, cfg->status, cfg->command), reads check
 * the buffer and the buffers hold any contents from the start;
 * stubs/cm_env.c (bio, rot, ramlog, slc); every other
 * function without a body returns any value and gives any value to what its
 * pointer arguments point at (goto-instrument havoc).
 */

#include "cm/cm_main.c"
#include <common.h>

static int t_write(const void *buf, size_t length)
{
    __CPROVER_assert(__CPROVER_r_ok(buf, length), "transport write: buffer readable");
    int rc;
    __CPROVER_assume(rc <= 0);
    return rc;
}

static int t_read(void *buf, size_t length)
{
    __CPROVER_assert(__CPROVER_w_ok(buf, length), "transport read: buffer writable");
    /* Not written: the buffers hold any contents from the start (a symbolic
     * sized write into them is too big for the solver) */
    int rc;
    __CPROVER_assume(rc <= 0);
    return rc;
}

static int t_complete(void)
{
    int rc;
    __CPROVER_assume(rc <= 0 && rc != -PB_ERR_AGAIN);
    return rc;
}

static int board_status(uint8_t *buf, size_t *size)
{
    __CPROVER_assert(__CPROVER_w_ok(buf, *size), "board status: buffer writable");
    size_t n;
    *size = n;
    int rc;
    __CPROVER_assume(rc <= 0);
    return rc;
}

static int board_command(uint32_t command,
                         uint8_t *bfr,
                         size_t size,
                         uint8_t *response_bfr,
                         size_t *response_size)
{
    (void)command;
    __CPROVER_assert(__CPROVER_r_ok(bfr, size), "board command: request readable");
    __CPROVER_assert(__CPROVER_w_ok(response_bfr, *response_size),
                     "board command: response writable");
    size_t n;
    *response_size = n;
    int rc;
    __CPROVER_assume(rc <= 0);
    return rc;
}

void harness(void)
{
    static const struct cm_config c = {
        .name = "cbmc",
        .tops = { .write = t_write, .read = t_read, .complete = t_complete },
        .status = board_status,
        .command = board_command,
    };
    cfg = &c;

    __CPROVER_havoc_object(&cmd);
    /* The data a transport or bio read would put in the buffers */
    __CPROVER_havoc_object(&buffer);
    /* Assigned, not assumed: a constant lets CBMC drop the other handlers */
    cmd.magic = PB_WIRE_MAGIC;
    cmd.command = CM_COMMAND;
#ifdef CM_ASSUME
    /* The command's own bounds on host sizes (see its harness) */
    CM_ASSUME;
#endif
    slc_t s;
    slc = s;
    authenticated = true;
    /* Any state from earlier commands */
    bool written, failed;
    bio_dev_t dev;
    stream_written = written;
    stream_failed = failed;
    block_dev = dev;

    (void)pb_command_parse();
}
