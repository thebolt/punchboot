/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: the boot sequence (src/main.c)
 *
 * Properties:
 *  1. Nothing boots unless plat_init and plat_board_init both returned OK.
 *  2. When the boot returns (it failed), or an init failed, command mode
 *     runs, and then the board resets. Nothing runs after the reset.
 * Assumptions: a firmware build (cfg/config.h, as jiffy). boot() and cm_run()
 * return.
 * Stubs: arch, plat, ramlog, boot, cm_run: they record the order.
 */

#include <common.h>
#include <pb/pb.h>

#include "main.c"

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

    PB_COVER(happened[E_BOOT]);
    PB_COVER(!happened[E_BOOT] && plat_rc != PB_OK);
    PB_COVER(!happened[E_BOOT] && plat_rc == PB_OK && board_rc != PB_OK);
}
