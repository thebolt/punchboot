/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Proof: console output and the RAM log replay (src/console.c)
 *
 * Messages logged before console_init() only go to the RAM log. When
 * console_init() gets a UART, it writes them out, except the ones that were
 * muted (console_uart_mute, levels above LOGLEVEL).
 *
 * Properties:
 *  1. The replay reads the RAM log only inside what it holds, into its own
 *     buffer, and terminates.
 *  2. It writes out exactly the bytes still in the RAM log that were not
 *     logged inside a completed mute (unmuted before console_init), each once
 *     and in order.
 *  3. After console_init, console_putc reaches the UART exactly when not
 *     muted, and always the RAM log.
 * How: the RAM log stub checks every read against the completed mutes (as
 * ranges of byte positions) and the previous read, and the harness counts
 * the bytes the UART gets.
 * Assumptions: up to STEPS steps before console_init, each logs up to
 * MAX_WRITE bytes or mutes or unmutes; any RAM log size up to MAX_RING and any
 * start position (a log kept over a reset), with no wrap of the 32-bit byte
 * count (4 GiB of log). Fewer mutes than EARLY_MUTED_MAX (32).
 * Stubs: ramlog_* (a ring of the given size with any contents), the UART
 * putc, plat_boot_reason*, printf (no output).
 */

#include <common.h>

#include "console.c"

#define STEPS     5
#define MAX_WRITE 3
#define MAX_RING  8
#define MAX_MUTES ((STEPS + 1) / 2)

/* The RAM log: byte positions [head - length, head) */
static uint32_t head;
static uint32_t written; /* since the start of the log */
static size_t ring;
/* Completed mutes before console_init, [start, end) */
static uint32_t muted_start[MAX_MUTES];
static uint32_t muted_end[MAX_MUTES];
static unsigned int mutes;
static uint32_t next_read; /* the replay reads each byte once, in order */
static unsigned int read_bytes;
static unsigned int uart_bytes;
static unsigned int ramlog_bytes;

uint32_t ramlog_head(void)
{
    return head;
}

size_t ramlog_length(void)
{
    return written < ring ? written : ring;
}

size_t ramlog_read(size_t offset, void *buf, size_t length)
{
    size_t len = ramlog_length();
    uint32_t oldest = head - (uint32_t)len;

    __CPROVER_assert(__CPROVER_w_ok(buf, length), "read into a valid buffer");
    __CPROVER_assert(offset < len, "read inside the log");
    if (offset >= len)
        return 0;

    size_t n = len - offset < length ? len - offset : length;
    uint32_t from = oldest + (uint32_t)offset;
    uint32_t to = from + (uint32_t)n;

    __CPROVER_assert(from >= next_read, "each byte once, in order");
    for (unsigned int i = 0; i < MAX_MUTES; i++) {
        if (i < mutes)
            __CPROVER_assert(to <= muted_start[i] || from >= muted_end[i],
                             "no muted byte replayed");
    }
    next_read = to;
    read_bytes += n;
    __CPROVER_havoc_slice(buf, n);
    return n;
}

void ramlog_putc(char c)
{
    (void)c;
    ramlog_bytes++;
}

static void uart_putc(uintptr_t b, char c)
{
    (void)b;
    (void)c;
    uart_bytes++;
}

int printf(const char *fmt, ...)
{
    (void)fmt;
    return 0;
}

int plat_boot_reason(void)
{
    return 0;
}

const char *plat_boot_reason_str(void)
{
    return "";
}

static const struct console_ops uart_ops = { .putc = uart_putc };

/* Bytes of [from, to) inside [start, end) */
static uint32_t overlap(uint32_t from, uint32_t to, uint32_t start, uint32_t end)
{
    uint32_t lo = from > start ? from : start;
    uint32_t hi = to < end ? to : end;

    return hi > lo ? hi - lo : 0;
}

void harness(void)
{
    uint32_t head0;
    uint32_t written0;
    size_t ring_size;
    bool muted = false;
    uint32_t mute_start = 0;

    __CPROVER_assume(ring_size >= 1 && ring_size <= MAX_RING);
    __CPROVER_assume(written0 <= MAX_RING && written0 <= head0);
    __CPROVER_assume(head0 <= UINT32_MAX - STEPS * MAX_WRITE);
    ring = ring_size;
    /* Bytes from before (an earlier boot) can be in the log, never muted */
    head = head0;
    written = written0;

    for (int s = 0; s < STEPS; s++) {
        unsigned int what;
        if (what == 0) {
            uint32_t n;
            __CPROVER_assume(n <= MAX_WRITE);
            for (uint32_t i = 0; i < n; i++)
                console_putc('x');
            head += n;
            written += n;
        } else {
            bool m = (what == 1);
            console_uart_mute(m);
            if (m && !muted) {
                mute_start = head;
            } else if (!m && muted) {
                muted_start[mutes] = mute_start;
                muted_end[mutes] = head;
                mutes++;
            }
            muted = m;
        }
    }
    __CPROVER_assert(uart_bytes == 0, "nothing to the UART before console_init");

    /* A mute still open at console_init is not a completed mute: replayed */
    bool muted_at_init = muted;
    uint32_t oldest = head - (uint32_t)ramlog_length();
    next_read = oldest;
    console_init(0, &uart_ops);

    /* Each once, in order, none muted (checked by the stub), and as many as
     * the unmuted bytes still in the log: exactly those */
    uint32_t expected = head - oldest;
    for (unsigned int i = 0; i < MAX_MUTES; i++) {
        if (i < mutes)
            expected -= overlap(oldest, head, muted_start[i], muted_end[i]);
    }
    __CPROVER_assert(read_bytes == expected, "every unmuted byte in the log replayed");
    __CPROVER_assert(uart_bytes == read_bytes, "every byte read goes to the UART");

    PB_COVER(mutes == 1 && uart_bytes > 0 && expected < head - oldest);
    PB_COVER(mutes == 2 && uart_bytes > 0);
    PB_COVER(mutes == 1 && muted_at_init && uart_bytes > 0);
    PB_COVER(written > ring && muted_start[0] < oldest && mutes == 1 && uart_bytes > 0);

    /* 3. After init */
    unsigned int before_uart = uart_bytes;
    unsigned int before_ram = ramlog_bytes;
    console_putc('y');
    __CPROVER_assert(ramlog_bytes == before_ram + 1, "always to the RAM log");
    __CPROVER_assert(uart_bytes == before_uart + (muted_at_init ? 0 : 1),
                     "to the UART exactly when not muted");
    console_uart_mute(!muted_at_init);
    console_putc('z');
    __CPROVER_assert(uart_bytes == before_uart + 1, "mute after init");
}
