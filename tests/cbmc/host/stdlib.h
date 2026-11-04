/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * The firmware stdlib.h plus the allocator the host tools use. CBMC has the
 * bodies; with --malloc-may-fail --malloc-fail-null malloc can return NULL.
 */

#ifndef TESTS_CBMC_HOST_STDLIB_H
#define TESTS_CBMC_HOST_STDLIB_H

#include <stddef.h>
#include_next <stdlib.h>

void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void free(void *ptr);

#endif // TESTS_CBMC_HOST_STDLIB_H
