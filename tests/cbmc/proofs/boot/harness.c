/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: boot flow (src/boot/boot.c)
 *
 * The central property of the bootloader: prepare, late_boot and jump only
 * run for a header that, in this boot_load, was authenticated, had its
 * parts checked, was loaded and hashed, and whose payload hash matched, in
 * that order and all for that same header. For BOOT_SOURCE_CUSTOM the
 * board's authenticate_image is trusted to do the same.
 * The harness boots twice in a row (any source each time, as command mode
 * can), then jumps, like every caller of boot_jump: only after boot_load
 * returned OK.
 * Assumptions: boot_jump is called only after boot_load returned OK (all
 * callers: boot(), cm_main.c).
 * Stubs: the image helpers record which header passed which step (their
 * own proofs are ih-*); bio; the board's boot driver callbacks.
 */

#include "boot/boot.c"
#include <common.h>

/* Which header passed each step of the current boot_load */
static const struct bpak_header *g_auth, *g_parts, *g_loaded, *g_verified;
static bool jumped;

static int any_rc(void)
{
    int rc;
    __CPROVER_assume(rc <= 0);
    return rc;
}

int boot_image_auth_header(struct bpak_header *hdr)
{
    int rc = any_rc();
    if (rc == PB_OK)
        g_auth = hdr;
    return rc;
}

int boot_image_verify_parts(struct bpak_header *hdr)
{
    int rc = any_rc();
    if (rc == PB_OK && g_auth == hdr)
        g_parts = hdr;
    return rc;
}

int boot_image_load_and_hash(struct bpak_header *hdr,
                             size_t load_chunk_size,
                             boot_read_cb_t read_f,
                             boot_result_cb_t result_f,
                             uint8_t *digest,
                             size_t digest_size)
{
    (void)load_chunk_size;
    (void)read_f;
    (void)result_f;
    __CPROVER_assert(__CPROVER_w_ok(digest, digest_size) && digest_size >= 64,
                     "a digest buffer for any hash");
    int rc = any_rc();
    if (rc == PB_OK && g_parts == hdr)
        g_loaded = hdr;
    return rc;
}

int boot_image_verify_payload(struct bpak_header *hdr, uint8_t *digest)
{
    (void)digest;
    int rc = any_rc();
    if (rc == PB_OK && g_loaded == hdr)
        g_verified = hdr;
    return rc;
}

/* bio */
bio_dev_t bio_get_part_by_uu(const uuid_t uu)
{
    (void)uu;
    bio_dev_t d;
    return d;
}

const unsigned char *bio_get_uu(bio_dev_t dev)
{
    (void)dev;
    static uuid_t uu;
    return uu;
}

/* For a valid device (the code only asks for one): flags and block counts
 * are not negative */
int bio_get_flags(bio_dev_t dev)
{
    (void)dev;
    int f;
    __CPROVER_assume(f >= 0 && f <= 0xffff);
    return f;
}

static int no_of_blocks;

int bio_get_no_of_blocks(bio_dev_t dev)
{
    (void)dev;
    return no_of_blocks;
}

ssize_t bio_block_size(bio_dev_t dev)
{
    (void)dev;
    return 512;
}

int bio_read(bio_dev_t dev, lba_t lba, size_t length, void *buf)
{
    (void)dev;
    /* bio's range check (the bio proof) */
    if ((uint64_t)lba + (length + 511) / 512 > (uint64_t)no_of_blocks)
        return -PB_ERR_PARAM;
    __CPROVER_assert(__CPROVER_w_ok(buf, length), "read buffer writable");
    __CPROVER_havoc_slice(buf, length);
    return any_rc();
}

void uuid_unparse(const uuid_t uu, char *out)
{
    (void)uu;
    out[0] = '\0';
}

/* The board's boot driver */
static struct bpak_header in_mem_header;

static int board_rc(void)
{
    return any_rc();
}

static bio_dev_t board_boot_device(void)
{
    bio_dev_t d;
    return d;
}

static int board_in_mem(struct bpak_header **hdr)
{
    *hdr = &in_mem_header;
    return any_rc();
}

static int board_custom(struct bpak_header **hdr)
{
    /* Trusted: the board authenticates and verifies the image itself */
    int rc = any_rc();
    if (rc == PB_OK)
        g_verified = *hdr;
    return rc;
}

static int board_prepare(struct bpak_header *hdr, uuid_t part_uu)
{
    (void)part_uu;
    __CPROVER_assert(hdr == g_verified && hdr == header_ptr,
                     "prepare gets the header verified in this boot");
    return any_rc();
}

static int board_late_boot(struct bpak_header *hdr, uuid_t part_uu)
{
    (void)part_uu;
    __CPROVER_assert(hdr == g_verified, "late_boot gets the header verified in this boot");
    return any_rc();
}

static void board_jump(void)
{
    __CPROVER_assert(g_verified != NULL && g_verified == header_ptr,
                     "jump only to a verified image");
    jumped = true;
}

static int read_cb_stub(int64_t offset, size_t length, void *buf)
{
    (void)offset;
    __CPROVER_havoc_slice(buf, length);
    return any_rc();
}

static int result_cb_stub(int result)
{
    (void)result;
    return any_rc();
}

static void one_boot_load(void)
{
    enum boot_source src;
    __CPROVER_assume(src > BOOT_SOURCE_INVALID && src < BOOT_SOURCE_END);
    boot_set_source(src);
    g_auth = g_parts = g_loaded = g_verified = NULL;
    int rc = boot_load(NULL);
    if (rc == PB_OK)
        __CPROVER_assert(g_verified != NULL, "boot_load OK only for a verified image");
}

void harness(void)
{
    int n;
    __CPROVER_assume(n >= 0);
    no_of_blocks = n;
    static struct boot_driver drv;
    bool early, prep, late;
    drv.default_boot_source = BOOT_SOURCE_BIO;
    drv.early_boot_cb = early ? board_rc : NULL;
    drv.get_boot_bio_device = board_boot_device;
    drv.get_in_mem_image = board_in_mem;
    drv.authenticate_image = board_custom;
    drv.prepare = prep ? board_prepare : NULL;
    drv.late_boot_cb = late ? board_late_boot : NULL;
    drv.jump = board_jump;
    __CPROVER_assume(boot_init(&drv) == PB_OK);
    boot_configure_load_cb(read_cb_stub, result_cb_stub);

    /* Command mode can try several boots before one works */
    one_boot_load();
    g_auth = g_parts = g_loaded = g_verified = NULL;
    enum boot_source src;
    __CPROVER_assume(src > BOOT_SOURCE_INVALID && src < BOOT_SOURCE_END);
    boot_set_source(src);
    int rc = boot_load(NULL);
    if (rc == PB_OK)
        (void)boot_jump();
    PB_COVER(jumped && src == BOOT_SOURCE_BIO);
    PB_COVER(jumped && src == BOOT_SOURCE_IN_MEM);
    PB_COVER(jumped && src == BOOT_SOURCE_CB);
}
