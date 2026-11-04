/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: the boot sequence (src/main.c) and the self tests (src/self_test.c)
 *
 * Properties:
 *  1. Nothing boots unless plat_init and plat_board_init both returned OK;
 *     the self tests run after them and before the boot.
 *  2. When the boot returns (it failed), or an init failed, command mode
 *     runs, and then the board resets. Nothing runs after the reset.
 *  3. Every self test in the section runs once, in order, with the watchdog
 *     kicked before it, whatever it returns.
 * Assumptions: a firmware build with self tests (cfg/config.h, as jiffy), up
 * to NTESTS tests in the section. boot() and cm_run() return.
 * Stubs: arch, plat, ramlog, boot, cm_run: they record the order.
 */

#include <common.h>
#include <pb/pb.h>
#include <pb/self_test.h>

#define NTESTS 2

/* The .self_test_fn section: the linker symbols are the bounds of tests[] */
static int run_count[NTESTS];
static unsigned int kicks;
static unsigned int kicks_at_test[NTESTS];
static int test_rc[NTESTS];

static int test0(void)
{
    kicks_at_test[0] = kicks;
    run_count[0]++;
    return test_rc[0];
}

static int test1(void)
{
    __CPROVER_assert(run_count[0] == 1, "in order");
    kicks_at_test[1] = kicks;
    run_count[1]++;
    return test_rc[1];
}

static struct self_test tests[NTESTS] = { { "t0", test0 }, { "t1", test1 } };
static size_t ntests;

#undef IMPORT_SYM
#define IMPORT_SYM(type, sym, name) static type name;
#include "main.c"
#include "self_test.c"

/* The order of events */
enum { E_RAMLOG, E_ARCH, E_PLAT, E_BOARD, E_BOOT, E_CM, E_RESET, E_MAX };

static unsigned int step;
static unsigned int at[E_MAX];
static bool happened[E_MAX];

static void event(int e)
{
    __CPROVER_assert(!happened[E_RESET], "nothing after the reset");
    happened[e] = true;
    at[e] = ++step;
}

static int any_rc(void)
{
    int rc;
    __CPROVER_assume(rc <= 0);
    return rc;
}

static int plat_rc;
static int board_rc;

void ramlog_init(void)
{
    event(E_RAMLOG);
}

void arch_init(void)
{
    event(E_ARCH);
}

int plat_init(void)
{
    event(E_PLAT);
    plat_rc = any_rc();
    return plat_rc;
}

int plat_board_init(void)
{
    event(E_BOARD);
    board_rc = any_rc();
    return board_rc;
}

void plat_wdog_kick(void)
{
    kicks++;
}

int boot(uuid_t boot_part_override_uu)
{
    __CPROVER_assert(boot_part_override_uu == NULL, "no override at power on");
    event(E_BOOT);
    return any_rc();
}

int cm_run(void)
{
    event(E_CM);
    return any_rc();
}

void plat_reset(void)
{
    event(E_RESET);
}

void harness(void)
{
    /* Any number of tests in the section, returning anything */
    size_t n;
    __CPROVER_assume(n <= NTESTS);
    ntests = n;
    self_test_fn_start = &tests[0];
    self_test_fn_end = &tests[ntests];
    for (int i = 0; i < NTESTS; i++)
        test_rc[i] = any_rc() | (int)(i & 1); /* also a positive failure */

    main();

    __CPROVER_assert(happened[E_RAMLOG] && at[E_RAMLOG] == 1, "the RAM log first");
    if (happened[E_BOOT])
        __CPROVER_assert(plat_rc == PB_OK && board_rc == PB_OK && at[E_BOARD] < at[E_BOOT],
                         "boot only after both inits succeeded");
    if (plat_rc != PB_OK)
        __CPROVER_assert(!happened[E_BOARD] && !happened[E_BOOT],
                         "a failed plat_init: no board init, no boot");
    if (happened[E_BOARD] && board_rc != PB_OK)
        __CPROVER_assert(!happened[E_BOOT], "a failed board init: no boot");
    __CPROVER_assert(happened[E_CM] && happened[E_RESET] && at[E_CM] < at[E_RESET],
                     "command mode, then the reset");

    if (happened[E_BOOT]) {
        for (size_t i = 0; i < NTESTS; i++) {
            __CPROVER_assert(run_count[i] == (i < ntests ? 1 : 0), "each test once");
            if (i < ntests)
                __CPROVER_assert(kicks_at_test[i] == i + 1, "watchdog kicked before it");
        }
    } else {
        for (size_t i = 0; i < NTESTS; i++)
            __CPROVER_assert(run_count[i] == 0 || (plat_rc == PB_OK && board_rc == PB_OK),
                             "no self test after a failed init");
    }
    PB_COVER(happened[E_BOOT] && ntests == NTESTS && test_rc[0] != 0);
    PB_COVER(!happened[E_BOOT] && plat_rc != PB_OK);
    PB_COVER(!happened[E_BOOT] && plat_rc == PB_OK && board_rc != PB_OK);
}
