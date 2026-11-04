/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: key revocation and locking over a sequence of operations on
 * i.MX6UL (src/plat/imx6ul/rot_helpers.c, slc_helpers.c)
 *
 * rot-imx6ul proves each operation on its own. This proof runs any sequence
 * of them on the same fuses, the way a host can send revoke, lock and status
 * commands one after the other.
 * Properties, for every step:
 *  - Once a key is revoked (a revoke returned OK, or its status read as
 *    revoked), its status never reads OK again.
 *  - Once the lock fuse is burnt it stays burnt.
 * Assumptions: STEPS operations, each a revoke, a key status read or a lock,
 * on any of the 4 SRK slots; any fuse contents at the start.
 * Stubs: OCOTP (stubs/ocotp.c, fuses only gain bits), HAB and the SRK check
 * return any value.
 */

#include <common.h>
#include <pb/pb.h>
#include <plat/imx6ul/fusebox.h>
#include <plat/imx6ul/hab.h>
#include <plat/imx6ul/imx6ul.h>

#define STEPS          3
#define SLOTS          4
/* The bit imx6ul_slc_set_configuration_locked burns (SEC_CONFIG[1]) */
#define LOCK_BIT       0x02u

#define PB_OCOTP_BANKS 8
#define PB_OCOTP_WORDS 8
extern uint32_t pb_ocotp_fuses[PB_OCOTP_BANKS][PB_OCOTP_WORDS];
void pb_ocotp_havoc(void);

int imx6ul_revoke_key(const struct rot_key *key);
int imx6ul_read_key_status(const struct rot_key *key);
int imx6ul_slc_set_configuration_locked(void);

int hab_has_no_errors(void)
{
    int rc;
    return rc;
}

int hab_secureboot_active(bool *result)
{
    int rc;
    bool secure;

    __CPROVER_assume(rc <= 0);
    if (rc == PB_OK)
        *result = secure;
    return rc;
}

bool imx6ul_is_srk_fused(void)
{
    bool fused;
    return fused;
}

void harness(void)
{
    bool revoked[SLOTS] = { false };
    bool locked = false;

    pb_ocotp_havoc();

    for (int s = 0; s < STEPS; s++) {
        unsigned int op;
        struct rot_key key = { .name = "key", .id = 1 };
        __CPROVER_assume(key.param1 < SLOTS);
        unsigned int slot = key.param1;

        if (op == 0) {
            if (imx6ul_revoke_key(&key) == PB_OK)
                revoked[slot] = true;
        } else if (op == 1) {
            int status = imx6ul_read_key_status(&key);
            if (revoked[slot])
                __CPROVER_assert(status != PB_OK, "a revoked key never reads OK again");
            if (status == -PB_ERR_KEY_REVOKED)
                revoked[slot] = true;
            PB_COVER(s == STEPS - 1 && revoked[slot] && status == -PB_ERR_KEY_REVOKED);
        } else {
            (void)imx6ul_slc_set_configuration_locked();
        }

        uint32_t lock = pb_ocotp_fuses[IMX6UL_FUSE_LOCK_BANK][IMX6UL_FUSE_LOCK_WORD];
        if (locked)
            __CPROVER_assert(lock & LOCK_BIT, "the lock fuse stays burnt");
        locked = (lock & LOCK_BIT) != 0;
        PB_COVER(s > 0 && op == 2 && locked);
    }
}
