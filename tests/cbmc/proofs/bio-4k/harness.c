/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: block device layer with 4 KiB blocks. The bio proof's harness with
 * BIO_BS 4096; see there.
 */

#define BIO_BS 4096
#include "../bio/harness.c"
