/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: SRK fusing on i.MX6UL (src/plat/imx6ul/fusebox.c), reached from
 * SLC_SET_CONFIGURATION (jiffy).
 *
 * Properties, for any fuse contents and any read or write failures:
 *  1. A NULL table or one with an empty word is refused and burns nothing.
 *  2. An SRK word that is already fused to another value is never changed:
 *     then nothing is burnt and the result is an error.
 *  3. On PB_OK every SRK word holds the table's value.
 *  4. imx6ul_is_srk_fused is true only if an SRK word is not 0, and false
 *     when all of them are 0.
 * Stubs: the OCOTP driver (stubs/ocotp.c: one-time programmable, any read
 * or write may fail).
 */

#include <common.h>
#include <stdbool.h> /* fusebox.h uses bool without it */
#include <plat/imx6ul/fusebox.h>
#include <pb/pb.h>
#include <string.h>

#define SRK_BANK 3

extern uint32_t pb_ocotp_fuses[8][8];
void pb_ocotp_havoc(void);

void harness(void)
{
    struct imx6ul_srk_fuses srk;
    uint32_t before[8];
    bool null_table;
    bool check_fused;

    pb_ocotp_havoc();
    for (int n = 0; n < 8; n++)
        before[n] = pb_ocotp_fuses[SRK_BANK][n];

    bool empty = false;
    bool conflict = false;
    bool any_fused = false;
    for (int n = 0; n < 8; n++) {
        empty |= (srk.srk[n] == 0);
        conflict |= (before[n] != 0 && before[n] != srk.srk[n]);
        any_fused |= (before[n] != 0);
    }

    if (check_fused) {
        bool fused = imx6ul_is_srk_fused();
        /* 4 */
        __CPROVER_assert(!fused || any_fused, "fused only if an SRK word is set");
        PB_COVER(fused);
        PB_COVER(!fused && any_fused);
        return;
    }

    int rc = imx6ul_fuse_write_srk(null_table ? NULL : &srk);

    bool changed = false;
    for (int n = 0; n < 8; n++)
        changed |= (pb_ocotp_fuses[SRK_BANK][n] != before[n]);

    /* 1 */
    if (null_table || empty)
        __CPROVER_assert(rc == -PB_ERR_PARAM && !changed, "refused, nothing burnt");
    /* 2 */
    if (!null_table && conflict)
        __CPROVER_assert(rc != PB_OK && !changed, "a fused SRK is never changed");
    /* 3 */
    if (rc == PB_OK) {
        for (int n = 0; n < 8; n++)
            __CPROVER_assert(pb_ocotp_fuses[SRK_BANK][n] == srk.srk[n], "the SRK is fused");
    }
    PB_COVER(rc == PB_OK && !any_fused);
    PB_COVER(rc == PB_OK && any_fused);
    PB_COVER(rc == -PB_ERR_MEM);
    PB_COVER(rc == -PB_ERR_IO && changed);
}
