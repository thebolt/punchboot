/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: snprintf with "%u" (src/lib/libc/snprintf.c)
 *
 * Properties (C11 7.21.6.5, 7.21.6.1): for an unsigned int and any buffer size n,
 * no write outside buf[0, n); if n > 0 at most n - 1 characters and a null
 * character are written; the return value is the number of characters the
 * whole output has; the output is the text the conversion defines, where it
 * fits (the exact digits for values below 100000, the rest for all).
 * Assumptions: n <= BUF (12).
 * Stubs: the log stub (the unsupported specifier branch logs).
 */

#define WHICH 1
#include <libc_snprintf.h>
