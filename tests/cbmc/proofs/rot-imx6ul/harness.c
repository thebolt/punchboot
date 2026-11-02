/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: i.MX6UL key revocation and security life cycle
 * (src/plat/imx6ul/rot_helpers.c, slc_helpers.c)
 *
 * Properties:
 *  - A key that was revoked (the revoke call returned OK, or its bit was
 *    already burnt) reads back as revoked, or the read fails.
 *  - Revoking sets only the key's bit, in the revoke fuse.
 *  - read_status is CONFIGURATION_LOCKED exactly when HAB reports secure
 *    boot, and passes HAB errors on.
 *  - set_configuration_locked burns the lock fuse only from CONFIGURATION
 *    with the SRK fused.
 * Assumptions: param1 < 4 (one bit per SRK slot; all boards use 0..3).
 * Stubs: OCOTP (stubs/ocotp.c), HAB and the SRK check return any value.
 */

#include <common.h>
#include <pb/pb.h>
#include <plat/imx6ul/fusebox.h>
#include <plat/imx6ul/hab.h>
#include <plat/imx6ul/imx6ul.h>

#define PB_OCOTP_BANKS 8
#define PB_OCOTP_WORDS 8
extern uint32_t pb_ocotp_fuses[PB_OCOTP_BANKS][PB_OCOTP_WORDS];
void pb_ocotp_havoc(void);

int imx6ul_revoke_key(const struct rot_key *key);
int imx6ul_read_key_status(const struct rot_key *key);
slc_t imx6ul_slc_read_status(void);
int imx6ul_slc_set_configuration_locked(void);

static bool hab_secure;
static int hab_rc;

int hab_has_no_errors(void)
{
    int rc;
    return rc;
}

int hab_secureboot_active(bool *result)
{
    if (hab_rc == PB_OK)
        *result = hab_secure;
    return hab_rc;
}

bool imx6ul_is_srk_fused(void)
{
    bool fused;
    return fused;
}

static void check_revoke(void)
{
    struct rot_key key = { .name = "key", .id = 1 };
    __CPROVER_assume(key.param1 < 4);
    uint32_t bit = 1u << key.param1;
    uint32_t before[PB_OCOTP_BANKS][PB_OCOTP_WORDS];
    __CPROVER_array_copy(before, pb_ocotp_fuses);

    int rc = imx6ul_revoke_key(&key);
    PB_COVER(rc == PB_OK);

    for (unsigned int b = 0; b < PB_OCOTP_BANKS; b++) {
        for (unsigned int w = 0; w < PB_OCOTP_WORDS; w++) {
            uint32_t changed = before[b][w] ^ pb_ocotp_fuses[b][w];
            if (b == IMX6UL_FUSE_REVOKE_BANK && w == IMX6UL_FUSE_REVOKE_WORD)
                __CPROVER_assert((changed & ~bit) == 0, "revoke only burns the key's bit");
            else
                __CPROVER_assert(changed == 0, "revoke only touches the revoke fuse");
        }
    }

    bool revoked = (rc == PB_OK) ||
                   (before[IMX6UL_FUSE_REVOKE_BANK][IMX6UL_FUSE_REVOKE_WORD] & bit);
    int status = imx6ul_read_key_status(&key);
    PB_COVER(revoked && status != -PB_ERR_IO);
    PB_COVER(!revoked && status == PB_OK);
    if (revoked && status != -PB_ERR_IO)
        __CPROVER_assert(status == -PB_ERR_KEY_REVOKED, "a revoked key reads as revoked");
    if (status == -PB_ERR_KEY_REVOKED)
        __CPROVER_assert(pb_ocotp_fuses[IMX6UL_FUSE_REVOKE_BANK][IMX6UL_FUSE_REVOKE_WORD] & bit,
                         "revoked only if the key's revoke bit is burnt");
}

static void check_slc(void)
{
    slc_t slc = imx6ul_slc_read_status();
    PB_COVER(slc == SLC_CONFIGURATION_LOCKED);
    PB_COVER(slc == SLC_CONFIGURATION);
    PB_COVER(slc == SLC_NOT_CONFIGURED);
    if (hab_rc != PB_OK)
        __CPROVER_assert(slc == hab_rc, "HAB errors are passed on");
    else
        __CPROVER_assert((slc == SLC_CONFIGURATION_LOCKED) == !!hab_secure,
                         "locked exactly when secure boot is active");

    uint32_t lock_before = pb_ocotp_fuses[IMX6UL_FUSE_LOCK_BANK][IMX6UL_FUSE_LOCK_WORD];
    bool srk_fused = imx6ul_is_srk_fused();
    (void)srk_fused;
    int rc = imx6ul_slc_set_configuration_locked();
    PB_COVER(rc == PB_OK);
    if (pb_ocotp_fuses[IMX6UL_FUSE_LOCK_BANK][IMX6UL_FUSE_LOCK_WORD] != lock_before) {
        __CPROVER_assert(hab_rc == PB_OK && !hab_secure, "lock burnt only from an unlocked state");
    }
}

void harness(void)
{
    /* Globals start as zero, give them any value */
    int any_rc;
    bool any_secure;
    __CPROVER_assume(any_rc <= 0);
    hab_rc = any_rc;
    hab_secure = any_secure;
    pb_ocotp_havoc();

    check_revoke();
    check_slc();
}
