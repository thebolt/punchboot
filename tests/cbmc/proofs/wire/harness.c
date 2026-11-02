/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: wire helpers (src/wire.c)
 *
 * Properties: pb_wire_valid_command accepts exactly magic + command in
 * (PB_CMD_INVALID, PB_CMD_END); pb_wire_requires_auth is false only for
 * DEVICE_IDENTIFIER_READ and AUTHENTICATE (pins the list); the init helpers
 * never write outside the struct and reject sizes above the response/request
 * area.
 * Assumptions: none on the inputs (command/result structs are fully symbolic).
 * Stubs: CBMC's memset/memcpy models.
 */

#include <common.h>
#include <pb-tools/wire.h>

void harness(void)
{
    struct pb_command cmd;
    struct pb_result res;
    uint8_t data[PB_COMMAND_REQUEST_MAX_SIZE + 1];
    size_t size;
    int code;

    /* valid_command */
    bool valid = pb_wire_valid_command(&cmd);
    __CPROVER_assert(valid == (cmd.magic == PB_WIRE_MAGIC && cmd.command > PB_CMD_INVALID &&
                               cmd.command < PB_CMD_END),
                     "valid_command is magic and range");
    PB_COVER(valid);

    /* requires_auth */
    bool auth = pb_wire_requires_auth(&cmd);
    __CPROVER_assert(auth == !(cmd.command == PB_CMD_DEVICE_IDENTIFIER_READ ||
                               cmd.command == PB_CMD_AUTHENTICATE),
                     "only identifier read and authenticate are open");
    PB_COVER(!auth);

    /* init_command2 / init_result2 with any size */
    __CPROVER_assume(size <= sizeof(data));
    code = pb_wire_init_command2(&cmd, PB_CMD_AUTHENTICATE, data, size);
    __CPROVER_assert((code == PB_RESULT_OK) == (size <= PB_COMMAND_REQUEST_MAX_SIZE),
                     "command size check");
    __CPROVER_assert(cmd.magic == PB_WIRE_MAGIC, "command magic set");
    PB_COVER(code == PB_RESULT_OK && size > 0);
    PB_COVER(code != PB_RESULT_OK);

    code = pb_wire_init_result2(&res, PB_RESULT_OK, data, size);
    __CPROVER_assert((code == PB_RESULT_OK) == (size <= PB_RESULT_RESPONSE_MAX_SIZE),
                     "result size check");
    __CPROVER_assert(pb_wire_valid_result(&res), "result magic set");
    PB_COVER(code != PB_RESULT_OK);
}
