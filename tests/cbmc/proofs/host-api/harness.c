/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: the host API calls that send one command and parse its result
 * (tools/punchboot/api_{device,misc,slc,authentication,boot,partition,
 * stream,board}.c), against a malicious device
 *
 * Properties, for any bytes the device sends and any transport error: no
 * undefined behaviour and no access outside the caller's buffers; the
 * transport only gets buffers the host owns; the strings the API returns
 * (bootloader version, boot status, board status) end inside the caller's
 * buffer; the sizes it returns (board command, log read) are at most the
 * caller's buffer.
 * Assumptions: the caller's buffers are as large as the sizes it passes;
 * output buffers up to 512 bytes, board command requests up to 1024.
 * Stubs: the device (stubs/host_device.h). pb_api_boot_bpak,
 * pb_api_partition_{read_table,read,write} have their own proofs.
 */

#include <api.h>
#include <string.h>
#include <stubs/host_device.h>

#define MAX_BUF 512

static void *buf(size_t *n, size_t min)
{
    size_t sz;
    __CPROVER_assume(sz >= min && sz <= MAX_BUF);
    *n = sz;
    return pb_cbmc_alloc(sz);
}

/* The Python wrapper turns these buffers into str with strlen */
static bool is_string(const char *s, size_t n)
{
    for (size_t i = 0; i < n; i++)
        if (s[i] == '\0')
            return true;
    return false;
}

void harness(void)
{
    struct pb_context *ctx = &dev_ctx;
    uint8_t uu[16], sha[32];
    size_t n;
    int rc;
    unsigned int which;

    switch (which) {
    case 0:
        rc = pb_api_device_reset(ctx);
        break;
    case 1: {
        size_t n2;
        uint8_t *device_uu = buf(&n, 0);
        char *board_id = buf(&n2, 0);
        rc = pb_api_device_read_identifier(ctx, device_uu, n, board_id, n2);
    } break;
    case 2: {
        struct pb_device_capabilities caps;
        rc = pb_api_device_read_caps(ctx, &caps);
    } break;
    case 3: {
        char *version = buf(&n, 0);
        rc = pb_api_bootloader_version(ctx, version, n);
        if (rc == PB_RESULT_OK)
            __CPROVER_assert(is_string(version, n), "the version string ends inside the buffer");
        PB_COVER(rc == PB_RESULT_OK);
    } break;
    case 4: {
        uint8_t slc;
        uint8_t *active = pb_cbmc_alloc(64);
        uint8_t *revoked = pb_cbmc_alloc(64);
        bool keys;
        rc = pb_api_slc_read(ctx, &slc, keys ? active : NULL, keys ? revoked : NULL);
    } break;
    case 5:
        rc = pb_api_slc_set_configuration(ctx);
        break;
    case 6:
        rc = pb_api_slc_set_configuration_lock(ctx);
        break;
    case 7:
        rc = pb_api_slc_set_end_of_life(ctx);
        break;
    case 8: {
        uint32_t key_id;
        rc = pb_api_slc_revoke_key(ctx, key_id);
    } break;
    case 9: {
        uint8_t *data = buf(&n, 0);
        uint32_t key_id;
        bool password;
        rc = password ? pb_api_authenticate_password(ctx, data, n)
                      : pb_api_authenticate_key(ctx, key_id, data, n);
    } break;
    case 10: {
        char *password = buf(&n, 0);
        rc = pb_api_auth_set_password(ctx, password, n);
    } break;
    case 11: {
        bool any;
        rc = pb_api_boot_activate(ctx, any ? uu : NULL);
    } break;
    case 12: {
        bool verbose;
        rc = pb_api_boot_part(ctx, uu, verbose);
    } break;
    case 13: {
        char *status = buf(&n, 1);
        rc = pb_api_boot_status(ctx, uu, status, n);
        if (rc == PB_RESULT_OK)
            __CPROVER_assert(is_string(status, n), "the boot status ends inside the buffer");
        PB_COVER(rc == PB_RESULT_OK);
    } break;
    case 14: {
        uint8_t variant;
        rc = pb_api_partition_install_table(ctx, uu, variant);
    } break;
    case 15: {
        uint32_t size;
        bool bpak;
        rc = pb_api_partition_verify(ctx, uu, sha, size, bpak);
    } break;
    case 16: {
        struct bpak_header *header = pb_cbmc_alloc(sizeof(*header));
        rc = pb_api_partition_read_bpak(ctx, uu, header);
    } break;
    case 17: {
        uint32_t start, count;
        rc = pb_api_partition_erase(ctx, uu, start, count);
    } break;
    case 18:
        rc = pb_api_stream_init(ctx, uu);
        break;
    case 19: {
        uint8_t id;
        void *data = buf(&n, 0);
        rc = pb_api_stream_prepare_buffer(ctx, id, data, n);
    } break;
    case 20: {
        uint8_t id;
        uint64_t offset;
        uint32_t size;
        rc = pb_api_stream_write_buffer(ctx, id, offset, size);
    } break;
    case 21: {
        uint8_t id;
        uint64_t offset;
        void *data = buf(&n, 0);
        rc = pb_api_stream_read_buffer(ctx, id, offset, n, data);
    } break;
    case 22:
        rc = pb_api_stream_finalize(ctx);
        break;
    case 23: {
        char *status = buf(&n, 1);
        rc = pb_api_board_status(ctx, status, n);
        if (rc == PB_RESULT_OK)
            __CPROVER_assert(is_string(status, n), "the board status ends inside the buffer");
        PB_COVER(rc == PB_RESULT_OK);
    } break;
    case 24: {
        size_t req_n;
        uint8_t *request = pb_cbmc_alloc(1024);
        __CPROVER_assume(req_n <= 1024);
        void *response = buf(&n, 0);
        size_t response_size = n;
        uint32_t id;
        rc = pb_api_board_command(ctx, id, request, req_n, response, &response_size);
        if (rc == PB_RESULT_OK)
            __CPROVER_assert(response_size <= n, "the response fits the buffer");
        PB_COVER(rc == PB_RESULT_OK && response_size > 0);
    } break;
    case 25: {
        uint32_t position, head;
        void *log = buf(&n, 0);
        size_t size = n;
        rc = pb_api_log_read(ctx, &position, log, &size, &head);
        if (rc == PB_RESULT_OK)
            __CPROVER_assert(size <= n, "the log data fits the buffer");
        PB_COVER(rc == PB_RESULT_OK && size > 0);
    } break;
    default:
        return;
    }
    PB_COVER(rc == PB_RESULT_OK);
    PB_COVER(rc != PB_RESULT_OK);
}
