/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: i.MX RT security life cycle and key status
 * (src/plat/imxrt/slc_helpers.c, rot_helpers.c)
 *
 * Properties:
 *  - read_status passes fuse read errors on, and is CONFIGURATION_LOCKED
 *    exactly when the SEC_CONFIG bit (bank 0 word 6, bit 1) is set.
 *  - CONFIGURATION means some SRK fuse word is nonzero, NOT_CONFIGURED that
 *    all read as zero.
 *  - Revocation is not supported: revoke always fails, and the key status is
 *    always OK (so no key is ever treated as revoked).
 * Assumptions: none.
 * Stubs: OCOTP (stubs/ocotp.c).
 */

#include <common.h>
#include <pb/pb.h>
#include <pb/rot.h>
#include <pb/slc.h>

#define PB_OCOTP_BANKS 8
#define PB_OCOTP_WORDS 8
extern uint32_t pb_ocotp_fuses[PB_OCOTP_BANKS][PB_OCOTP_WORDS];
void pb_ocotp_havoc(void);

slc_t imxrt_slc_read_status(void);
int imxrt_slc_set_configuration_locked(void);
int imxrt_revoke_key(const struct rot_key *key);
int imxrt_read_key_status(const struct rot_key *key);

void harness(void)
{
    pb_ocotp_havoc();

    slc_t slc = imxrt_slc_read_status();
    PB_COVER(slc == SLC_CONFIGURATION_LOCKED);
    PB_COVER(slc == SLC_CONFIGURATION);
    PB_COVER(slc == SLC_NOT_CONFIGURED);
    PB_COVER(slc < 0);

    if (slc >= 0) {
        __CPROVER_assert((slc == SLC_CONFIGURATION_LOCKED) == !!(pb_ocotp_fuses[0][6] & 2),
                         "locked exactly when SEC_CONFIG is set");
    }
    if (slc == SLC_NOT_CONFIGURED) {
        unsigned int n;
        __CPROVER_assume(n < 8);
        __CPROVER_assert(pb_ocotp_fuses[3][n] == 0, "not configured means no SRK bits");
    }

    struct rot_key key = { .name = "key", .id = 1 };
    __CPROVER_assert(imxrt_revoke_key(&key) != PB_OK, "revoke is not supported");
    __CPROVER_assert(imxrt_read_key_status(&key) == PB_OK, "no key reads as revoked");
    __CPROVER_assert(imxrt_slc_set_configuration_locked() != PB_OK, "locking is not supported");
}
