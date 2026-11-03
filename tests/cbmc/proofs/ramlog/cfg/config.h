/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * The generated config.h with a 1 KiB RAM log instead of the configured
 * size: RAMLOG_SIZE is only used in % and MIN, and the configured sizes are
 * powers of two too. With 16 KiB the copies out of the ring are too big for
 * the solver. (config.h has no include guard, so this runs every time.)
 */

#undef CONFIG_RAMLOG_SIZE_KiB
#include_next <config.h>
#undef CONFIG_RAMLOG_SIZE_KiB
#define CONFIG_RAMLOG_SIZE_KiB 1
