/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: GPT parsing when the primary table is corrupt (src/drivers/partition/gpt.c).
 * The gpt proof's harness with GPT_MODE 1; see there.
 */

#define GPT_MODE 1
#include "../gpt/harness.c"
