/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: security life cycle dispatch (src/slc.c)
 *
 * Properties: every slc_* call returns -PB_ERR_NOT_SUPPORTED without a config
 * or without the board callback it needs, and never calls a NULL callback.
 * Assumptions: none; the board callbacks return any value.
 * Stubs: board slc callbacks.
 */

#include <common.h>
#include <pb/pb.h>
#include <pb/slc.h>

/* Board slc callbacks */
static int slc_calls;

static slc_t board_read_status(void)
{
    slc_t s;
    slc_calls++;
    return s;
}

static int board_set(void)
{
    int rc;
    slc_calls++;
    return rc;
}

static void check_slc(void)
{
    struct slc_config cfg;
    bool use_cfg, has_read, has_conf, has_lock, has_eol;
    cfg.read_status = has_read ? board_read_status : NULL;
    cfg.set_configuration = has_conf ? board_set : NULL;
    cfg.set_configuration_locked = has_lock ? board_set : NULL;
    cfg.set_eol = has_eol ? board_set : NULL;
    slc_init(use_cfg ? &cfg : NULL);

    slc_calls = 0;
    slc_t s = slc_read_status();
    if (!use_cfg || !has_read)
        __CPROVER_assert(s == -PB_ERR_NOT_SUPPORTED && slc_calls == 0, "read_status: no callback");
    int rc = slc_set_configuration();
    if (!use_cfg || !has_conf)
        __CPROVER_assert(rc == -PB_ERR_NOT_SUPPORTED, "set_configuration: no callback");
    rc = slc_set_configuration_locked();
    if (!use_cfg || !has_lock)
        __CPROVER_assert(rc == -PB_ERR_NOT_SUPPORTED, "set_configuration_locked: no callback");
    rc = slc_set_eol();
    if (!use_cfg || !has_eol)
        __CPROVER_assert(rc == -PB_ERR_NOT_SUPPORTED, "set_eol: no callback");
    PB_COVER(use_cfg && has_read && slc_calls > 0);
}

void harness(void)
{
    check_slc();
}
