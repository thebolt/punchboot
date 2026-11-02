/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: root of trust dispatch (src/rot.c)
 *
 * Properties:
 *  - rot_init accepts a key map only if it matches the keystore (count and
 *    ids, in order).
 *  - rot_read_key_status and rot_revoke_key act on the key with the given
 *    id, or return KEY_NOT_FOUND.
 *  - rot_get_dsa_key returns a key only if the board reported its status as
 *    OK, and returns that key's data and kind.
 *  - rot_no_of_keys and rot_key_idx_to_id return no error codes disguised as
 *    a count or a key id.
 * Assumptions: keystore with NKEYS keys (fixed ids); the board key map has up
 * to NKEYS entries with any ids. The board callbacks return any value.
 * Stubs: board rot callbacks, keystore.
 */

#include <bpak/bpak.h>
#include <bpak/keystore.h>
#include <common.h>
#include <pb/pb.h>
#include <pb/rot.h>
#include <stddef.h>

#define NKEYS       3

/* The keystore compiled into the firmware. CBMC gives an initialized
 * flexible array member no storage, so rot.c reaches keystore_pb through a
 * pointer to a fixed size copy with the same layout. Every access in rot.c is
 * unchanged. */
#define keystore_pb (*pb_cbmc_keystore)
#include "rot.c"
#undef keystore_pb

struct key {
    enum bpak_key_kind kind;
    uint16_t size;
    uint32_t id;
    uint8_t rz[4];
    uint8_t data[4];
};
static struct key key_a = { .kind = BPAK_KEY_PUB_PRIME256v1, .size = 4, .id = 0xa };
static struct key key_b = { .kind = BPAK_KEY_PUB_SECP384r1, .size = 4, .id = 0xb };
static struct key key_c = { .kind = BPAK_KEY_PUB_SECP521r1, .size = 4, .id = 0xc };

static const struct {
    uint32_t id;
    uint8_t no_of_keys;
    bool verified;
    struct key *keys[NKEYS];
} keystore = {
    .id = 1,
    .no_of_keys = NKEYS,
    .keys = { &key_a, &key_b, &key_c },
};

_Static_assert(offsetof(struct bpak_keystore, keys) == offsetof(__typeof__(keystore), keys),
               "keystore layout");
_Static_assert(offsetof(struct bpak_key, data) == offsetof(struct key, data), "key layout");
const struct bpak_keystore *pb_cbmc_keystore = (const struct bpak_keystore *)&keystore;

/* Board rot callbacks; record the key they were called with */
static const struct rot_key *status_key;
static int status_rc;
static const struct rot_key *revoked_key;

static int board_read_key_status(const struct rot_key *key)
{
    status_key = key;
    return status_rc;
}

static int board_revoke_key(const struct rot_key *key)
{
    int rc;
    revoked_key = key;
    return rc;
}

static void check_rot(void)
{
    /* A rot_config with NKEYS key map entries (a fixed size copy of the
     * layout, see the keystore) */
    static struct {
        int (*revoke_key)(const struct rot_key *key);
        int (*read_key_status)(const struct rot_key *key);
        size_t key_map_length;
        struct rot_key key_map[NKEYS];
    } board;

    _Static_assert(offsetof(struct rot_config, key_map) == offsetof(__typeof__(board), key_map),
                   "rot_config layout");
    const struct rot_config *cfg = (const struct rot_config *)&board;
    struct rot_key *map = board.key_map;
    size_t len;
    bool has_status, has_revoke;
    __CPROVER_assume(len <= NKEYS);
    board.key_map_length = len;
    for (unsigned int i = 0; i < NKEYS; i++) {
        key_id_t id;
        map[i].id = id;
        map[i].name = "key";
    }
    board.read_key_status = has_status ? board_read_key_status : NULL;
    board.revoke_key = has_revoke ? board_revoke_key : NULL;

    __CPROVER_bool matches = (len == NKEYS);
    for (unsigned int i = 0; i < NKEYS; i++) {
        if (i < len && map[i].id != keystore.keys[i]->id)
            matches = 0;
    }

    /* Before rot_init: no config */
    __CPROVER_assert(rot_read_key_status(0xa) == -PB_ERR_NOT_SUPPORTED, "no config: status");
    __CPROVER_assert(rot_no_of_keys() == 0, "no config: no keys");

    int rc = rot_init(cfg);
    __CPROVER_assert((rc == PB_OK) == matches, "rot_init accepts exactly a matching key map");
    PB_COVER(rc == PB_OK);
    if (rc != PB_OK)
        return;

    int any_rc;
    status_rc = any_rc;
    key_id_t id;
    status_key = NULL;
    rc = rot_read_key_status(id);
    if (has_status && status_key != NULL) {
        __CPROVER_assert(status_key->id == id && rc == status_rc, "status of the requested key");
    } else if (has_status) {
        __CPROVER_assert(rc == -PB_ERR_KEY_NOT_FOUND, "unknown key");
    }

    revoked_key = NULL;
    rc = rot_revoke_key(id);
    if (revoked_key != NULL)
        __CPROVER_assert(revoked_key->id == id, "revokes the requested key");
    PB_COVER(revoked_key != NULL);

    dsa_t kind;
    const uint8_t *der = NULL;
    size_t der_len;
    status_key = NULL;
    rc = rot_get_dsa_key(id, &kind, &der, &der_len);
    PB_COVER(rc == PB_OK);
    if (rc == PB_OK) {
        __CPROVER_assert(status_key != NULL && status_key->id == id && status_rc == PB_OK,
                         "a key is used only if the board says it is OK");
        const struct key *k = (id == 0xa) ? &key_a : (id == 0xb) ? &key_b : &key_c;
        __CPROVER_assert(id >= 0xa && id <= 0xc, "the id is in the keystore");
        __CPROVER_assert(__CPROVER_same_object(der, k) && der_len == k->size,
                         "the keystore key for the id");
        __CPROVER_assert((id == 0xa && kind == DSA_EC_SECP256r1) ||
                             (id == 0xb && kind == DSA_EC_SECP384r1) ||
                             (id == 0xc && kind == DSA_EC_SECP521r1),
                         "the key kind for the id");
    }

    unsigned int idx;
    key_id_t kid = rot_key_idx_to_id(idx);
    if (idx < len)
        __CPROVER_assert(kid == map[idx].id, "key id by index");
}

void harness(void)
{
    check_rot();
}
