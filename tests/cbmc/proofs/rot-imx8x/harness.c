/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: i.MX8X key status and security life cycle
 * (src/plat/imx8x/rot_helpers.c, slc_helpers.c)
 *
 * Properties:
 *  - read_key_status is REVOKED exactly when the key's bit in the SECO
 *    revoke field (fuse 11, bits 8..11) is set, and passes fuse read errors on.
 *  - read_status is CONFIGURATION_LOCKED exactly when SECO reports the
 *    OEM closed life cycle (128); if SECO cannot be asked, the result is an
 *    error or LOCKED (fail closed: anything else opens command mode, D-011).
 *  - set_configuration_locked only moves the life cycle forward with the SRK
 *    fused, and fails without moving it when SECO cannot be asked.
 * Assumptions: param1 < 4 (four SRK slots).
 * Stubs: SECO calls return any error code and only write their outputs on
 * success; fuse reads and the SRK check return any value.
 */

#include <common.h>
#include <pb/pb.h>
#include <plat/imx8x/fusebox.h>
#include <plat/imx8x/imx8x.h>
#include <plat/imx8x/sci/svc/seco/api.h>

int imx8x_read_key_status(const struct rot_key *key);
slc_t imx8x_slc_read_status(void);
int imx8x_slc_set_configuration_locked(void);

static uint32_t revoke_fuse;
static int fuse_rc;
static sc_err_t chip_info_rc;
static uint16_t chip_lc;
static bool srk_fused;
static int forwarded;

int imx8x_fuse_read(uint32_t addr, uint32_t *result)
{
    __CPROVER_assert(addr == IMX8X_FUSE_REVOKE, "only the revoke fuse is read");
    if (fuse_rc == PB_OK)
        *result = revoke_fuse;
    return fuse_rc;
}

bool imx8x_is_srk_fused(void)
{
    return srk_fused;
}

sc_err_t
sc_seco_chip_info(sc_ipc_t ipc, uint16_t *lc, uint16_t *monotonic, uint32_t *uid_l, uint32_t *uid_h)
{
    (void)ipc;
    if (chip_info_rc == SC_ERR_NONE) {
        *lc = chip_lc;
        *monotonic = 0;
        *uid_l = 0;
        *uid_h = 0;
    }
    return chip_info_rc;
}

sc_err_t sc_seco_forward_lifecycle(sc_ipc_t ipc, uint32_t change)
{
    (void)ipc;
    (void)change;
    sc_err_t rc;
    if (rc == SC_ERR_NONE)
        forwarded++;
    return rc;
}

sc_err_t sc_seco_commit(sc_ipc_t ipc, uint32_t *info)
{
    (void)ipc;
    (void)info;
    sc_err_t rc;
    return rc;
}

void harness(void)
{
    /* Globals start as zero, give them any value */
    uint32_t any_fuse;
    int any_rc;
    sc_err_t any_info_rc;
    uint16_t any_lc;
    bool any_srk;
    __CPROVER_assume(any_rc <= 0);
    revoke_fuse = any_fuse;
    fuse_rc = any_rc;
    chip_info_rc = any_info_rc;
    chip_lc = any_lc;
    srk_fused = !!any_srk;

    struct rot_key key = { .name = "key", .id = 1 };
    __CPROVER_assume(key.param1 < 4);
    int status = imx8x_read_key_status(&key);
    PB_COVER(status == -PB_ERR_KEY_REVOKED);
    PB_COVER(status == PB_OK);
    if (fuse_rc != PB_OK)
        __CPROVER_assert(status == fuse_rc, "fuse read errors are passed on");
    else
        __CPROVER_assert((status == -PB_ERR_KEY_REVOKED) ==
                             !!(revoke_fuse & (1u << (8 + key.param1))),
                         "revoked exactly when the key's revoke bit is set");

    slc_t slc = imx8x_slc_read_status();
    PB_COVER(slc == SLC_CONFIGURATION_LOCKED);
    PB_COVER(slc == SLC_NOT_CONFIGURED);
    if (chip_info_rc == SC_ERR_NONE) {
        __CPROVER_assert((slc == SLC_CONFIGURATION_LOCKED) == (chip_lc == 128),
                         "locked exactly when SECO reports OEM closed");
    } else {
        __CPROVER_assert(slc < 0 || slc == SLC_CONFIGURATION_LOCKED,
                         "fail closed when SECO cannot be asked");
    }

    int rc = imx8x_slc_set_configuration_locked();
    PB_COVER(forwarded == 1);
    if (forwarded)
        __CPROVER_assert(srk_fused && rc == PB_OK, "life cycle forwarded only with the SRK fused");
    if (chip_info_rc != SC_ERR_NONE && srk_fused)
        __CPROVER_assert(rc < 0 && !forwarded, "no forward when SECO cannot be asked");
}
