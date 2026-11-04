/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * The POSIX file functions the host tools use (tools/punchboot), on top of
 * the firmware libc headers. Proofs that call them give them a body.
 */

#ifndef TESTS_CBMC_HOST_UNISTD_H
#define TESTS_CBMC_HOST_UNISTD_H

#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
off_t lseek(int fd, off_t offset, int whence);

#endif // TESTS_CBMC_HOST_UNISTD_H
