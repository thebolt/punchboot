/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * The firmware stdio.h plus fprintf to stderr, which the host tools use for
 * error messages. tests/cbmc/stubs/host_device.h gives fprintf a body.
 */

#ifndef TESTS_CBMC_HOST_STDIO_H
#define TESTS_CBMC_HOST_STDIO_H

#include_next <stdio.h>

typedef struct pb_cbmc_file FILE;
extern FILE *stderr;
int fprintf(FILE *stream, const char *fmt, ...);

#endif // TESTS_CBMC_HOST_STDIO_H
