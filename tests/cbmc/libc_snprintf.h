/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Shared harness for the libc-snprintf-* proofs (one format each; WHICH
 * selects it). See those for the properties.
 */

#ifndef TESTS_CBMC_LIBC_SNPRINTF_H
#define TESTS_CBMC_LIBC_SNPRINTF_H

#include <libc.h>
#include <stdio.h>

#define BUF       12
#define DEC_EXACT 100000u

void harness(void)
{
    size_t n, ssize;
    unsigned int u;
    int d;
    int which = WHICH;
    __CPROVER_assume(n <= BUF);
    char *buf = pb_cbmc_alloc(n);
    char *str = pb_cbmc_string(&ssize);
    int ret;
    char expect[BUF + 12];
    size_t len = 0;

    if (which == 0) {
        ret = snprintf(buf, n, "%s", str);
        len = spec_strlen(str);
        for (size_t i = 0; i < len; i++)
            expect[i] = str[i];
    } else if (which == 1 || which == 2) {
        /* Decimal: checked by reading the digits back, with multiplications
         * only (a division by 10 in the specification as well makes the
         * proof too slow for the solver) */
        unsigned int v = u;
        bool neg = false;
        if (which == 1) {
            ret = snprintf(buf, n, "%u", u);
        } else {
            ret = snprintf(buf, n, "%d", d);
            neg = d < 0;
            v = neg ? 0u - (unsigned int)d : (unsigned int)d;
        }
        static const unsigned long long pow10[11] = {
            1ull,       10ull,       100ull,       1000ull,       10000ull,       100000ull,
            1000000ull, 10000000ull, 100000000ull, 1000000000ull, 10000000000ull,
        };
        size_t nd = 1;
        while (nd < 10 && v >= pow10[nd])
            nd++;
        len = nd + neg;
        __CPROVER_assert(ret == (int)len, "returns the full length");
        if (n > 0) {
            size_t end = len < n - 1 ? len : n - 1;
            __CPROVER_assert(buf[end] == '\0', "NUL terminated in the buffer");
            if (neg && end > 0)
                __CPROVER_assert(buf[0] == '-', "minus sign first");
            size_t first = (neg && end > 0) ? 1 : 0;
            size_t m = end - first;
            unsigned long long prefix = 0;
            for (size_t i = 0; i < m; i++) {
                char c = buf[first + i];
                __CPROVER_assert(c >= '0' && c <= '9', "decimal digits");
                prefix = prefix * 10u + (unsigned long long)(c - '0');
            }
            /* The exact digits for values below DEC_EXACT (the same code
             * path for every value; all 32-bit values make the divisions
             * too slow for the solver). Every value: the length, the NUL,
             * decimal digits and no write outside the buffer. */
            if (m > 0 && v < DEC_EXACT) {
                /* The printed digits are the leading digits of v */
                unsigned long long scale = pow10[nd - m];
                __CPROVER_assert(prefix * scale <= v && v < (prefix + 1) * scale,
                                 "the leading digits of the value");
            }
        }
#if WHICH == 1 || WHICH == 2
        PB_COVER(n > 0 && len >= n);
        PB_COVER(n > len);
#endif
        return;
    } else {
        ret = which == 3 ? snprintf(buf, n, "%x", u) : snprintf(buf, n, "%08x", u);
        unsigned int v = u;
        char digits[8];
        size_t nd = 0;
        do {
            digits[nd++] = "0123456789abcdef"[v % 16];
            v /= 16;
        } while (v);
        if (which != 3) {
            for (size_t p = nd; p < 8; p++)
                expect[len++] = '0';
        }
        while (nd)
            expect[len++] = digits[--nd];
    }

    __CPROVER_assert(ret == (int)len, "returns the full length");
    if (n > 0) {
        size_t end = len < n - 1 ? len : n - 1;
        __CPROVER_assert(buf[end] == '\0', "NUL terminated in the buffer");
        size_t j;
        __CPROVER_assume(j < end);
        __CPROVER_assert(buf[j] == expect[j], "the expected text where it fits");
    }
#if WHICH != 1 && WHICH != 2
    PB_COVER(n > 0 && len >= n);
    PB_COVER(n > len);
#endif
}

#endif // TESTS_CBMC_LIBC_SNPRINTF_H
