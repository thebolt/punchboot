/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * No-op console output for the LOG_* macros, linked into every proof except
 * the libc proofs. The macro arguments are still evaluated by the code under
 * proof, only the output is dropped.
 */

#include <stdbool.h>

int printf(const char *fmt, ...)
{
    (void)fmt;
    return 0;
}

void console_uart_mute(bool mute)
{
    (void)mute;
}
