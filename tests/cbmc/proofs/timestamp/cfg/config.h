/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * The generated config.h with timestamping on (jiffy, imxrt1060evk) and room
 * for 2 timestamps instead of 64: the array is only indexed by the count, so
 * a smaller one has the same paths. (config.h has no include guard, so this
 * runs every time.)
 */

#undef CONFIG_ENABLE_TIMESTAMPING
#undef CONFIG_NO_OF_TIMESTAMPS
#include_next <config.h>
#undef CONFIG_ENABLE_TIMESTAMPING
#undef CONFIG_NO_OF_TIMESTAMPS
#define CONFIG_ENABLE_TIMESTAMPING 1
#define CONFIG_NO_OF_TIMESTAMPS    2
