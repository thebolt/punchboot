/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Stub of the i.MX OCOTP fuse driver (src/drivers/fuse/imx_ocotp.c) for the
 * platform rot/slc helpers.
 *
 * Contract: fuses are one-time programmable, so a write can only set bits
 * (the result is the OR of the old value and the written one). Every read and
 * write can fail with an error code, and a failed read leaves *value alone. A
 * failed write may or may not have burnt the bits.
 * pb_ocotp_havoc() gives the fuse array any contents; call it first in the
 * harness (globals start as zero). Read pb_ocotp_fuses to state properties.
 */

#include <drivers/fuse/imx_ocotp.h>
#include <pb/errors.h>
#include <stdbool.h>

#define PB_OCOTP_BANKS 8
#define PB_OCOTP_WORDS 8

uint32_t pb_ocotp_fuses[PB_OCOTP_BANKS][PB_OCOTP_WORDS];

void pb_ocotp_havoc(void)
{
    uint32_t any[PB_OCOTP_BANKS][PB_OCOTP_WORDS];
    __CPROVER_array_copy(pb_ocotp_fuses, any);
}

int imx_ocotp_read(uint32_t bank, uint32_t row, uint32_t *value)
{
    __CPROVER_assert(bank < PB_OCOTP_BANKS && row < PB_OCOTP_WORDS, "ocotp: fuse exists");
    if (bank >= PB_OCOTP_BANKS || row >= PB_OCOTP_WORDS)
        return -PB_ERR_PARAM;

    bool fail;
    if (fail)
        return -PB_ERR_IO;

    *value = pb_ocotp_fuses[bank][row];
    return PB_OK;
}

int imx_ocotp_write(uint32_t bank, uint32_t row, uint32_t value)
{
    __CPROVER_assert(bank < PB_OCOTP_BANKS && row < PB_OCOTP_WORDS, "ocotp: fuse exists");
    if (bank >= PB_OCOTP_BANKS || row >= PB_OCOTP_WORDS)
        return -PB_ERR_PARAM;

    bool fail;
    bool burnt;
    if (!fail || burnt)
        pb_ocotp_fuses[bank][row] |= value;

    return fail ? -PB_ERR_IO : PB_OK;
}
