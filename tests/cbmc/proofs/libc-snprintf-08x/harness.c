/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: snprintf with "%08x" (src/lib/libc/snprintf.c)
 *
 * Properties (C11 7.21.6.5, 7.21.6.1): for an unsigned int, zero padded to 8 digits and any buffer
 * size n, no write outside buf[0, n); if n > 0 at most n - 1 characters and a null character are
 * written; the return value is the number of characters the whole output has; the output is the
 * text the conversion defines, where it fits. Assumptions: n <= BUF (12); strings up to LIBC_MAX
 * bytes. Stubs: the log stub (the unsupported specifier branch logs).
 */

#define WHICH 4
#include <libc_snprintf.h>
