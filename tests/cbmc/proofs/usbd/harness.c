/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: USB enumeration and transfers (src/drivers/usb/usbd.c, the string
 * descriptors of src/drivers/usb/pb_dev_cls.c)
 *
 * The host sends setup packets before anything is authenticated, during
 * enumeration (usbd_connect) and in the middle of a transfer
 * (usbd_xfer_complete).
 * Properties, for any setup packets:
 *  1. Every EP0 answer is read from inside its descriptor, and is at most
 *     wLength bytes (F-175).
 *  2. SET_CONFIGURATION passes the interface's endpoints, all inside the
 *     descriptors.
 *  3. A setup packet in the middle of a transfer cancels it and restarts it
 *     with the same endpoint, buffer and length.
 *  4. The serial number string is only made from a UUID device_uuid wrote,
 *     or zeros if it failed (F-176).
 *  5. usbd_connect is OK only after a successful SET_CONFIGURATION.
 * Assumptions: the descriptors have the layout and lengths of pb_dev_cls.c
 * (wTotalLength covers the configuration, the interface and its 2
 * endpoints); the HAL sets every op, as all three UDC drivers do.
 * Stubs: the HAL (any result, setup packets with any bytes), device_uuid
 * (any error, or a UUID), uuid_clear, uuid_unparse (36 characters).
 */

#include <common.h>
#include <drivers/usb/usbd.h>
#include <pb/errors.h>
#include <string.h>
#include <uuid.h>

/* pb_dev_cls.c's descriptors end in an initialized flexible array member,
 * which CBMC gives no storage; the harness uses a copy with a fixed number of
 * endpoints and the same layout. Only get_string_descriptor is used from it. */
#include "drivers/usb/pb_dev_cls.c"

#define NEPS 2

struct descs {
    struct usb_language_descriptor language;
    struct usb_qualifier_descriptor qualifier;
    struct usb_device_descriptor device;
    struct usb_configuration_descriptor config;
    struct usb_interface_descriptor interface;
    struct usb_endpoint_descriptor eps[NEPS];
} __attribute__((packed));

_Static_assert(sizeof(struct descs) ==
                   sizeof(struct usbd_descriptors) + NEPS * sizeof(struct usb_endpoint_descriptor),
               "same layout");

static struct descs descs;

/* The device UUID */
static bool uu_written;

int device_uuid(uuid_t uu)
{
    int rc;

    __CPROVER_assume(rc <= 0);
    uu_written = false;
    if (rc == PB_OK) {
        __CPROVER_havoc_slice(uu, 16);
        uu_written = true;
    }
    return rc;
}

void uuid_clear(uuid_t uu)
{
    memset(uu, 0, 16);
    uu_written = true;
}

void uuid_unparse(const uuid_t uu, char *out)
{
    (void)uu;
    __CPROVER_assert(uu_written, "the serial number is a UUID device_uuid wrote, or zeros");
    __CPROVER_assert(__CPROVER_w_ok(out, 37), "unparse: room for 37");
    memset(out, 'a', 36);
    out[36] = 0;
}

/* The HAL */
static usb_ep_t xfer_ep;
static void *xfer_buf;
static size_t xfer_len;
static bool in_setup;
static uint16_t setup_wlength;
static bool cancelled;
static bool configured_ok;
/* The user's transfer, and its restart after a cancel */
static usb_ep_t user_ep;
static void *user_buf;
static size_t user_len;
static bool restarted;

static int any_rc(void)
{
    int rc;
    __CPROVER_assume(rc <= 0);
    return rc;
}

static int hal_init(void)
{
    return any_rc();
}

static int hal_xfer_start(usb_ep_t ep, void *buf, size_t length)
{
    if (ep == USB_EP0_IN && in_setup) {
        __CPROVER_assert(__CPROVER_r_ok(buf, length), "EP0 answer inside its descriptor");
        __CPROVER_assert(length <= setup_wlength, "EP0 answer at most wLength");
    }
    if (cancelled && ep != USB_EP0_IN && ep != USB_EP0_OUT)
        restarted = (ep == user_ep && buf == user_buf && length == user_len);
    xfer_ep = ep;
    xfer_buf = buf;
    xfer_len = length;
    return any_rc();
}

static int hal_xfer_complete(usb_ep_t ep)
{
    if (cancelled && ep != USB_EP0_IN && ep != USB_EP0_OUT)
        __CPROVER_assert(restarted, "the same transfer restarted before waiting on it");
    int rc;
    /* Not AGAIN forever: wait_for_completion loops on it */
    __CPROVER_assume(rc <= 0 && rc != -PB_ERR_AGAIN);
    return rc;
}

static void hal_xfer_cancel(usb_ep_t ep)
{
    __CPROVER_assert(ep == xfer_ep, "cancel the current transfer");
    cancelled = true;
}

static int hal_poll_setup_pkt(struct usb_setup_packet *pkt)
{
    int rc = any_rc();

    if (rc == PB_OK) {
        __CPROVER_havoc_object(pkt);
        setup_wlength = pkt->wLength;
    }
    return rc;
}

static int hal_set_configuration(const struct usb_endpoint_descriptor *eps, size_t no_of_eps)
{
    __CPROVER_assert(no_of_eps <= NEPS && __CPROVER_r_ok(eps, no_of_eps * sizeof(*eps)),
                     "the interface's endpoints, inside the descriptors");
    int rc = any_rc();
    configured_ok = (rc == PB_OK);
    return rc;
}

static int hal_set_address(uint16_t addr)
{
    (void)addr;
    return any_rc();
}

static int hal_ep0_xfer_zlp(usb_ep_t ep)
{
    (void)ep;
    return any_rc();
}

static const struct usbd_hal_ops hal = {
    .init = hal_init,
    .xfer_start = hal_xfer_start,
    .xfer_complete = hal_xfer_complete,
    .xfer_cancel = hal_xfer_cancel,
    .poll_setup_pkt = hal_poll_setup_pkt,
    .set_configuration = hal_set_configuration,
    .set_address = hal_set_address,
    .ep0_xfer_zlp = hal_ep0_xfer_zlp,
};

void harness(void)
{
    /* Any descriptor contents, with pb_dev_cls.c's lengths and endpoint count */
    __CPROVER_havoc_object(&descs);
    __CPROVER_assume(descs.config.wTotalLength ==
                     sizeof(descs.config) + sizeof(descs.interface) + NEPS * sizeof(descs.eps[0]));
    __CPROVER_assume(descs.interface.bLength == sizeof(descs.interface));
    __CPROVER_assume(descs.interface.bNumEndpoints == NEPS);
    static struct usbd_cls_config cls;
    cls.desc = (const struct usbd_descriptors *)&descs;
    cls.get_string_descriptor = get_string_descriptor;

    int init_rc = usbd_init_hal_ops(&hal);
    __CPROVER_assert(init_rc == PB_OK, "every op set");
    init_rc = usbd_init_cls(&cls);
    __CPROVER_assert(init_rc == PB_OK, "class set");

    /* Enumeration */
    in_setup = true;
    int rc = usbd_connect();
    in_setup = false;
    if (rc == PB_OK)
        __CPROVER_assert(configured_ok, "connected only after SET_CONFIGURATION");
    PB_COVER(rc == PB_OK);
    PB_COVER(rc == -PB_ERR_AGAIN && setup_wlength > 0);

    /* A transfer, with a setup packet in the middle of it */
    uint8_t data[8];
    usb_ep_t ep;
    size_t len;
    __CPROVER_assume(ep == USB_EP1_IN || ep == USB_EP2_OUT);
    __CPROVER_assume(len <= sizeof(data));
    user_ep = ep;
    user_buf = data;
    user_len = len;
    (void)usbd_xfer_start(ep, data, len);
    cancelled = false;
    in_setup = true;
    rc = usbd_xfer_complete();
    in_setup = false;
    PB_COVER(cancelled && restarted);
}
