/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * The generated config.h as a firmware build with self tests (jiffy): not
 * the host architecture, so main() is the entry point. (config.h has no
 * include guard, so this runs every time.)
 */

#undef CONFIG_ARCH_HOST
#undef CONFIG_SELF_TEST
#include_next <config.h>
#undef CONFIG_ARCH_HOST
#undef CONFIG_SELF_TEST
#define CONFIG_SELF_TEST 1
