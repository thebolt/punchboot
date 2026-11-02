/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: bpak_copyz_signature (src/lib/bpak.c)
 *
 * Runs on the unauthenticated header in boot_image_auth_header, before the
 * signature is checked.
 * Properties: no undefined behaviour for any header and any output buffer
 * size; OK only if 0 < signature_sz <= min(512, *size); on OK the signature is
 * copied, *size is its length, and the header's signature area and size are
 * zero (the header is hashed like that).
 * Assumptions: the output buffer really is *size bytes.
 * Stubs: string models.
 */

#include <bpak/bpak.h>
#include <common.h>

void harness(void)
{
    struct bpak_header hdr;
    size_t buf_size;
    __CPROVER_assume(buf_size <= 1024);
    uint8_t *sig = pb_cbmc_alloc(buf_size);
    size_t size = buf_size;
    uint16_t sz = hdr.signature_sz;
    unsigned int i;
    __CPROVER_assume(i < BPAK_SIGNATURE_MAX_BYTES);
    uint8_t byte = hdr.signature[i];

    int rc = bpak_copyz_signature(&hdr, sig, &size);

    PB_COVER(rc == BPAK_OK);
    PB_COVER(rc != BPAK_OK);

    if (rc == BPAK_OK) {
        __CPROVER_assert(sz > 0 && sz <= BPAK_SIGNATURE_MAX_BYTES && sz <= buf_size,
                         "OK only for a valid size");
        __CPROVER_assert(size == sz, "size is the signature length");
        __CPROVER_assert(hdr.signature[i] == 0, "signature area zeroed");
        __CPROVER_assert(hdr.signature_sz == 0, "signature size zeroed");
        if (i < sz)
            __CPROVER_assert(sig[i] == byte, "signature copied");
    } else {
        __CPROVER_assert(size == buf_size, "size unchanged on error");
        __CPROVER_assert(hdr.signature[i] == byte, "header unchanged on error");
    }
}
