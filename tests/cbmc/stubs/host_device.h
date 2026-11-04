/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * A malicious device for the host API proofs (tools/punchboot): every read
 * from the transport fills the buffer with any bytes or fails, every write
 * succeeds or fails. The transport itself (usb.c, socket.c) is not proven;
 * the contract is that read fills exactly sz bytes or returns an error.
 *
 * Checks that the API only hands the transport buffers it owns, and counts
 * the transfers so a harness can bound them.
 */

#ifndef TESTS_CBMC_STUBS_HOST_DEVICE_H
#define TESTS_CBMC_STUBS_HOST_DEVICE_H

#include <api.h>
#include <common.h>
#include <pb-tools/error.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

FILE *stderr;
int errno;

/* Error messages are not checked: they print only what the code has */
int fprintf(FILE *stream, const char *fmt, ...)
{
    (void)stream;
    (void)fmt;
    int n;
    return n;
}

static unsigned int dev_transfers;

/* Called with each successful read after it is filled, so a harness can
 * bound what the device sends (and must say so in its assumptions) */
static void (*dev_answer)(void *bfr, size_t sz);

static int dev_error(void)
{
    int rc;
    __CPROVER_assume(rc < 0);
    return rc;
}

static int dev_read(struct pb_context *ctx, void *bfr, size_t sz)
{
    (void)ctx;
    dev_transfers++;
    __CPROVER_assert(__CPROVER_w_ok(bfr, sz), "the device only writes the caller's buffer");
    bool ok;
    if (!ok)
        return dev_error();
    __CPROVER_havoc_slice(bfr, sz);
    if (dev_answer)
        dev_answer(bfr, sz);
    return PB_RESULT_OK;
}

static int dev_write(struct pb_context *ctx, const void *bfr, size_t sz)
{
    (void)ctx;
    dev_transfers++;
    __CPROVER_assert(__CPROVER_r_ok(bfr, sz), "the host only sends its own bytes");
    bool ok;
    return ok ? PB_RESULT_OK : dev_error();
}

/* The debug callback formats nothing: the arguments are only read */
static int dev_debug(struct pb_context *ctx, int level, const char *fmt, ...)
{
    (void)ctx;
    (void)level;
    (void)fmt;
    return PB_RESULT_OK;
}

static struct pb_context dev_ctx = {
    .connected = true,
    .write = dev_write,
    .read = dev_read,
    .d = dev_debug,
};

#endif // TESTS_CBMC_STUBS_HOST_DEVICE_H
