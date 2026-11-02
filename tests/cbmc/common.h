/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Included by every CBMC harness.
 */

#ifndef TESTS_CBMC_COMMON_H
#define TESTS_CBMC_COMMON_H

#include <stddef.h>
#include <stdint.h>

/* The proof is only meaningful if the word size is the one we asked for */
_Static_assert(sizeof(void *) * 8 == PB_CBMC_WORD, "pointer width");
_Static_assert(sizeof(size_t) * 8 == PB_CBMC_WORD, "size_t width");

/* Reachability goals for the vacuity check (make cover). A cover goal that is
 * UNSATISFIED means the assumptions or stubs cut the path away, so the proof
 * says nothing about it. Only compiled in for the cover run. */
#ifdef PB_CBMC_COVER
#define PB_COVER(cond) __CPROVER_cover(cond)
#else
#define PB_COVER(cond) ((void)0)
#endif

#endif // TESTS_CBMC_COMMON_H
