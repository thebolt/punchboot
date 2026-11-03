/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: the PB_CMD_PART_VERIFY handler (src/cm/cm_main.c). The cm-handlers harness
 * with CM_COMMAND PB_CMD_PART_VERIFY; see there.
 */

#define CM_COMMAND PB_CMD_PART_VERIFY
/* Up to 4 buffers to verify: the loop does the same for every buffer */
#define CM_ASSUME \
    __CPROVER_assume(((struct pb_command_verify_part *)cmd.request)->size <= 4 * CM_BUF_SIZE)
#include "../cm-handlers/harness.c"
