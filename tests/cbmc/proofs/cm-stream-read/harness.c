/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: the PB_CMD_STREAM_READ_BUFFER handler (src/cm/cm_main.c). The cm-handlers harness
 * with CM_COMMAND PB_CMD_STREAM_READ_BUFFER; see there.
 */

#define CM_COMMAND PB_CMD_STREAM_READ_BUFFER
#include "../cm-handlers/harness.c"
