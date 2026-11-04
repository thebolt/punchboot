/**
 * Punch BOOT
 *
 * Copyright (C) 2020 Jonas Blixt <jonpe960@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <pb/delay.h>
#include <pb/pb.h>
#include <pb/plat.h>
#include <string.h>

int pb_timeout_init_us(struct pb_timeout *timeout, uint32_t timeout_us)
{
    if ((timeout_us == 0) || (timeout_us == 0xffffffff))
        return -PB_ERR;

    memset(timeout, 0, sizeof(*timeout));
    timeout->ts_start = plat_get_us_tick();
    /* The tick wraps at 2^32, so does ts_end */
    timeout->ts_end = timeout->ts_start + timeout_us;
    timeout->rollover = (timeout->ts_end < timeout->ts_start);

    return PB_OK;
}

bool pb_timeout_has_expired(struct pb_timeout *timeout)
{
    uint32_t ts = plat_get_us_tick();

    /* Elapsed time modulo 2^32: right also when the tick wrapped between two
     * polls, as long as they are less than 2^32 us apart */
    return (uint32_t)(ts - timeout->ts_start) >= (uint32_t)(timeout->ts_end - timeout->ts_start);
}

void pb_delay_us(unsigned int us)
{
    struct pb_timeout to;

    if (pb_timeout_init_us(&to, us) == PB_OK) {
        while (pb_timeout_has_expired(&to) == false) {
            __asm__ volatile("nop");
        }
    }
}

void pb_delay_ms(uint32_t delay)
{
    pb_delay_us(delay * 1000);
}
