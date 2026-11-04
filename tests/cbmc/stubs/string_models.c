/**
 * Punch BOOT
 *
 * Copyright (C) 2026 Actia Nordic AB, Marten Svanfeldt <marten.svanfeldt@actia.se>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * CBMC models of the libc string functions, linked into every proof except
 * the libc proofs (which verify the real src/lib/libc implementations):
 * memcpy, memmove and memset in one step each, memcmp, strlen, strnlen, strcmp and
 * strncpy as plain loops.
 *
 * They assert the C standard preconditions (the buffers are valid for n bytes,
 * memcpy does not overlap) and then have the standard effect, using CBMC's
 * array primitives so a copy of n bytes is one step, not n loop iterations.
 * CBMC's own library models are not used because they are preprocessed with
 * the host libc headers, which are not available for 32 bit builds.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static int pb_cbmc_overlap(const void *a, const void *b, size_t n)
{
    if (n == 0 || !__CPROVER_same_object(a, b))
        return 0;

    size_t oa = __CPROVER_POINTER_OFFSET(a);
    size_t ob = __CPROVER_POINTER_OFFSET(b);

    return (oa < ob) ? (ob - oa < n) : (oa - ob < n);
}

void *memcpy(void *dst, const void *src, size_t n)
{
    __CPROVER_assert(__CPROVER_w_ok(dst, n), "memcpy: destination valid for n bytes");
    __CPROVER_assert(__CPROVER_r_ok(src, n), "memcpy: source valid for n bytes");
    __CPROVER_assert(!pb_cbmc_overlap(dst, src, n), "memcpy: no overlap");

#ifdef PB_CBMC_MEMCPY_CHECK_ONLY
    /* Copies of more than PB_CBMC_MEMCPY_CHECK_ONLY bytes only have their
     * preconditions checked. For the gpt proof, where those are copies
     * between entry arrays that both hold any value, so the destination
     * already covers whatever the copy would put there. */
    if (n > PB_CBMC_MEMCPY_CHECK_ONLY)
        return dst;
#endif
    if (n > 0) {
        char tmp[n];
        __CPROVER_array_replace(tmp, (const char *)src);
        __CPROVER_array_replace((char *)dst, tmp);
    }
    return dst;
}

void *memmove(void *dst, const void *src, size_t n)
{
    __CPROVER_assert(__CPROVER_w_ok(dst, n), "memmove: destination valid for n bytes");
    __CPROVER_assert(__CPROVER_r_ok(src, n), "memmove: source valid for n bytes");

    if (n > 0) {
        char tmp[n];
        __CPROVER_array_replace(tmp, (const char *)src);
        __CPROVER_array_replace((char *)dst, tmp);
    }
    return dst;
}

void *memset(void *dst, int val, size_t n)
{
    __CPROVER_assert(__CPROVER_w_ok(dst, n), "memset: destination valid for n bytes");

    if (n > 0) {
        char tmp[n];
        __CPROVER_array_set(tmp, (unsigned char)val);
        __CPROVER_array_replace((char *)dst, tmp);
    }
    return dst;
}

int memcmp(const void *a, const void *b, size_t n)
{
    __CPROVER_assert(__CPROVER_r_ok(a, n), "memcmp: first buffer valid for n bytes");
    __CPROVER_assert(__CPROVER_r_ok(b, n), "memcmp: second buffer valid for n bytes");

    const unsigned char *pa = a;
    const unsigned char *pb = b;

    /* The result only matters as <0, 0, >0; the loop is unwound by the
     * proof's bound for memcmp (CBMC_EXTRA --unwindset memcmp.0:N) */
    for (size_t i = 0; i < n; i++) {
        if (pa[i] != pb[i])
            return (int)pa[i] - (int)pb[i];
    }
    return 0;
}

size_t strlen(const char *s)
{
    size_t n = 0;

    /* Each read is checked by the pointer checks; a string that is not
     * terminated inside its object fails them */
    while (s[n] != '\0')
        n++;
    return n;
}

size_t strnlen(const char *s, size_t maxlen)
{
    size_t n = 0;

    /* POSIX: the length, but at most maxlen; reads no more than maxlen */
    while (n < maxlen && s[n] != '\0')
        n++;
    return n;
}

int strcmp(const char *a, const char *b)
{
    size_t i = 0;

    /* C11 7.24.4.2: compare as unsigned char up to the first difference or
     * the null character */
    while (a[i] != '\0' && a[i] == b[i])
        i++;
    return (unsigned char)a[i] - (unsigned char)b[i];
}

char *strncpy(char *dst, const char *src, size_t n)
{
    size_t i;

    /* C11 7.24.2.4: copy up to n characters, stop at the null character,
     * pad the rest of dst[0, n) with null characters */
    for (i = 0; i < n && src[i] != '\0'; i++)
        dst[i] = src[i];
    for (; i < n; i++)
        dst[i] = '\0';
    return dst;
}
