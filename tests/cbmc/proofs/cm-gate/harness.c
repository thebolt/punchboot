/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: the command mode authentication gate (src/cm/cm_main.c)
 *
 * Property: a command that needs authentication (every command but
 * DEVICE_IDENTIFIER_READ and AUTHENTICATE, the wire proof pins the list),
 * sent while not authenticated, does nothing and is answered with
 * NOT_AUTHENTICATED, unless the life cycle is a valid unlocked state
 * (NOT_CONFIGURED, CONFIGURATION, or EOL by D-011). In particular a failed
 * SLC read (a negative slc) keeps the gate closed.
 * How: the inputs are restricted to that case, and every function without a
 * body (every board, bio, boot, slc, rot and crypto call a handler could
 * make) is generated to fail (assert-false). The transport may only send the
 * NOT_AUTHENTICATED result.
 * Assumptions: any command bytes, any slc value; not authenticated.
 * Stubs: the transport (cfg->tops); everything else is assert-false.
 */

#include "cm/cm_main.c"
#include <common.h>

static int t_write(const void *buf, size_t length)
{
    const struct pb_result *r = buf;
    __CPROVER_assert(length == sizeof(struct pb_result) && r->magic == PB_WIRE_MAGIC &&
                         r->result_code == -PB_RESULT_NOT_AUTHENTICATED,
                     "only NOT_AUTHENTICATED is sent");
    return PB_OK;
}

static int t_complete(void)
{
    return PB_OK;
}

static int t_read(void *buf, size_t length)
{
    (void)buf;
    (void)length;
    __CPROVER_assert(0, "nothing more is read");
    return -PB_ERR;
}

void harness(void)
{
    static const struct cm_config c = {
        .name = "cbmc",
        .tops = { .write = t_write, .read = t_read, .complete = t_complete },
    };
    cfg = &c;

    __CPROVER_havoc_object(&cmd);
    slc_t s;
    slc = s;
    authenticated = false;

    /* The case the gate must refuse */
    __CPROVER_assume(pb_wire_valid_command(&cmd) && pb_wire_requires_auth(&cmd));
    __CPROVER_assume(s != SLC_NOT_CONFIGURED && s != SLC_CONFIGURATION && s != SLC_EOL);
    PB_COVER(s < 0);
    PB_COVER(s == SLC_CONFIGURATION_LOCKED);

    (void)pb_command_parse();
}
