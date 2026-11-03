/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: block device layer, a disk and one partition from
 * bio_allocate_parent. The bio proof's harness with BIO_CHILD; see there.
 */

#define BIO_CHILD
#include "../bio/harness.c"
