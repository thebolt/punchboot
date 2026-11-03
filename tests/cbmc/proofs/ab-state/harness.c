/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: A/B boot state (src/boot/ab_state.c)
 *
 * The state partitions are not authenticated, so their contents are any
 * value.
 * Properties:
 *  - boot_ab_state_init: no undefined behaviour for any state on disk; after
 *    it the state in memory has the magic; defaults are installed only when
 *    neither copy is valid; a valid primary copy is kept as is.
 *  - boot_ab_state_get: returns sys A, sys B or an error. An unverified
 *    system is booted only while attempts remain (and the counter goes down
 *    by one, never wraps), or once by the speculative rollback.
 *  - The board registers are only accessed inside the array.
 * Assumptions: any state on disk, any CRC result (covers valid and corrupt
 * copies), either rollback mode.
 * Stubs: bio (state reads give any data, partitions found by UUID), crc32.
 */

#include "boot/ab_state.c"
#include <common.h>

#define DEV_A       1
#define DEV_B       2
#define DEV_PRIMARY 3
#define DEV_BACKUP  4

static const uuid_t uu_a = { 0xa };
static const uuid_t uu_b = { 0xb };
static const uuid_t uu_primary = { 0x1 };
static const uuid_t uu_backup = { 0x2 };

uint32_t crc32(uint32_t crc, const uint8_t *buf, uint32_t size)
{
    __CPROVER_assert(__CPROVER_r_ok(buf, size), "crc32: buffer readable");
    (void)crc;
    uint32_t any;
    return any;
}

bio_dev_t bio_get_part_by_uu(const uuid_t uu)
{
    if (uuid_compare(uu, uu_a) == 0)
        return DEV_A;
    if (uuid_compare(uu, uu_b) == 0)
        return DEV_B;
    if (uuid_compare(uu, uu_primary) == 0)
        return DEV_PRIMARY;
    if (uuid_compare(uu, uu_backup) == 0)
        return DEV_BACKUP;
    return -PB_ERR_NOT_FOUND;
}

int bio_clear_set_flags(bio_dev_t dev, uint16_t clear_flags, uint16_t set_flags)
{
    (void)dev;
    (void)clear_flags;
    (void)set_flags;
    return PB_OK;
}

int bio_read(bio_dev_t dev, lba_t lba, size_t length, void *buf)
{
    __CPROVER_assert(dev == DEV_PRIMARY || dev == DEV_BACKUP, "reads a state partition");
    __CPROVER_assert(lba == 0 && __CPROVER_w_ok(buf, length), "reads into the state");
    __CPROVER_havoc_slice(buf, length);
    int rc;
    __CPROVER_assume(rc <= 0);
    return rc;
}

int bio_write(bio_dev_t dev, lba_t lba, size_t length, const void *buf)
{
    __CPROVER_assert(dev == DEV_PRIMARY || dev == DEV_BACKUP, "writes a state partition");
    __CPROVER_assert(lba == 0 && __CPROVER_r_ok(buf, length), "writes the state");
    int rc;
    __CPROVER_assume(rc <= 0);
    return rc;
}

void harness(void)
{
    enum ab_rollback_mode mode;
    __CPROVER_assume(mode == AB_ROLLBACK_MODE_NORMAL || mode == AB_ROLLBACK_MODE_SPECULATIVE);
    static struct boot_ab_state_config c;
    c.primary_state_part_uu = uu_primary;
    c.backup_state_part_uu = uu_backup;
    c.sys_a_uu = uu_a;
    c.sys_b_uu = uu_b;
    c.rollback_mode = mode;

    (void)boot_ab_state_init(&c);
    __CPROVER_assert(boot_state.magic == PB_STATE_MAGIC, "a valid state after init");

    /* Any state the board could have committed before */
    struct pb_boot_state before = boot_state;
    bio_dev_t dev = boot_ab_state_get();

    __CPROVER_assert(dev == DEV_A || dev == DEV_B || dev < 0, "boots A, B or nothing");
    bool a = (dev == DEV_A);
    bool b = (dev == DEV_B);
    if (a && !(before.verified & PB_STATE_A_VERIFIED)) {
        bool attempt = (before.enable & PB_STATE_A_ENABLED) && before.remaining_boot_attempts > 0 &&
                       boot_state.remaining_boot_attempts == before.remaining_boot_attempts - 1;
        bool rollback = mode == AB_ROLLBACK_MODE_SPECULATIVE &&
                        (before.enable & PB_STATE_B_ENABLED) &&
                        boot_state.remaining_boot_attempts == 1;
        __CPROVER_assert(attempt || rollback, "unverified A only with an attempt left");
    }
    if (b && !(before.verified & PB_STATE_B_VERIFIED)) {
        bool attempt = !(before.enable & PB_STATE_A_ENABLED) &&
                       (before.enable & PB_STATE_B_ENABLED) && before.remaining_boot_attempts > 0 &&
                       boot_state.remaining_boot_attempts == before.remaining_boot_attempts - 1;
        bool rollback = mode == AB_ROLLBACK_MODE_SPECULATIVE &&
                        (before.enable & PB_STATE_A_ENABLED) &&
                        boot_state.remaining_boot_attempts == 1;
        __CPROVER_assert(attempt || rollback, "unverified B only with an attempt left");
    }
    PB_COVER(a && !(before.verified & PB_STATE_A_VERIFIED));
    PB_COVER(b && (before.verified & PB_STATE_B_VERIFIED));
    PB_COVER(dev < 0);

    unsigned int idx;
    uint32_t v;
    if (boot_ab_state_read_board_reg(idx, &v) == PB_OK)
        __CPROVER_assert(idx < PB_STATE_NO_OF_BOARD_REGS, "board register inside the array");
}
