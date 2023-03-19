/**
 * Punch BOOT
 *
 * Copyright (C) 2020 Jonas Blixt <jonpe960@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdio.h>
#include <string.h>
#include <pb/pb.h>
#include <pb/arch.h>
#include <pb/plat.h>
#include <pb/timestamp.h>
#include <pb/storage.h>
#include <pb/command.h>
#include <pb/boot.h>

void pb_main(void)
{
    int rc;

    arch_init();
    rc = plat_early_init();

    if (rc != PB_OK)
        plat_reset();

    pb_timestamp_begin("Total");
    pb_timestamp_begin("MMU");
    rc = plat_mmu_init();

    if (rc != PB_OK)
        plat_reset();

    pb_timestamp_end();
    pb_timestamp_begin("Misc init");

    plat_console_init();

    printf("\n\rPB " PB_VERSION ", %s (%i)\n\r", plat_boot_reason_str(),
                                                 plat_boot_reason());

#ifdef CONFIG_ENABLE_WATCHDOG
    plat_wdog_init();
#endif

    rc = plat_crypto_init();

    if (rc != PB_OK)
    {
        LOG_ERR("Could not initialize crypto");
        goto run_command_mode;
    }

    pb_timestamp_end();
    pb_timestamp_begin("SLC");

    rc = plat_slc_init();

    if (rc != PB_OK)
    {
        LOG_ERR("Could not initialize SLC");
        goto run_command_mode;
    }

    pb_timestamp_end();
    pb_timestamp_begin("Boot init");

    rc = pb_boot_init();

    if (rc != PB_OK)
    {
        LOG_ERR("Could not load boot state");
        goto run_command_mode;
    }

    pb_timestamp_end();

    if (plat_force_command_mode())
    {
        LOG_INFO("Forced command mode");
        goto run_command_mode;
    }

    rc = pb_boot_load(PB_BOOT_SOURCE_BLOCK_DEV, NULL);

    if (rc != PB_OK)
    {
        LOG_ERR("Could not boot");
        goto run_command_mode;
    }

    pb_boot(PB_BOOT_MODE_NORMAL, false);

    LOG_INFO("Boot stopped, entering command mode");

run_command_mode:
    pb_command_run();
    plat_reset();
}
