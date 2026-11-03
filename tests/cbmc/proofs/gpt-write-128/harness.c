/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: gpt_write_tbl with 128 entries, as gpt_install_partition_table
 * writes it. The gpt-write proof's harness with GPT_WRITE_PARTS 128.
 */

#define GPT_WRITE_PARTS 128
#include "../gpt-write/harness.c"
