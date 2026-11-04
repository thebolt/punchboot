/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * The firmware errno.h has the error numbers only; the host tools read errno.
 */

#ifndef TESTS_CBMC_HOST_ERRNO_H
#define TESTS_CBMC_HOST_ERRNO_H

#include_next <errno.h>

extern int errno;

#endif // TESTS_CBMC_HOST_ERRNO_H
