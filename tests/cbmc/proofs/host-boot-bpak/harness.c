/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: pb_api_boot_bpak_image (tools/punchboot/api_boot.c), for any file
 * and a malicious device
 *
 * Properties, for any image bytes and size, any bytes the device sends and
 * any transport error: no undefined behaviour, no read outside the image
 * (every byte sent to the device is the host's, stubs/host_device.h), and
 * the transfer loop terminates.
 * Assumptions: the image is at most its 4 KiB header and 8 bytes; a header
 * has at most 2 parts and 1 metadata entry (this bounds the loops; the
 * header parser itself is proven for any header in bpak-header).
 * Stubs: the device (stubs/host_device.h).
 */

#include <api.h>
#include <bpak/bpak.h>
#include <stubs/host_device.h>

#define MAX_PAYLOAD 8
#define MAX_PARTS   2

void harness(void)
{
    size_t size;
    __CPROVER_assume(size <= sizeof(struct bpak_header) + MAX_PAYLOAD);
    uint8_t *image = pb_cbmc_alloc(size);

    if (size >= sizeof(struct bpak_header)) {
        struct bpak_header *h = (struct bpak_header *)image;
        __CPROVER_assume(h->parts[MAX_PARTS].id == 0 && h->meta[1].id == 0);
    }

    uint8_t uu[16];
    bool pretend, verbose;
    int rc = pb_api_boot_bpak_image(&dev_ctx, image, size, pretend ? uu : NULL, verbose);

    PB_COVER(rc == PB_RESULT_OK && size == sizeof(struct bpak_header) + MAX_PAYLOAD);
    PB_COVER(rc == -PB_RESULT_INVALID_ARGUMENT && size >= sizeof(struct bpak_header));
}
