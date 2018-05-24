/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com) 
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation; either version 2 
 * of the License, or (at your option) any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 */
#ifndef __CEL_TIMERWHEEL_H__
#define __CEL_TIMERWHEEL_H__

#include "cel/timer.h"
#include "cel/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CEL_TIMEWHEEL_BASE_SPOKE_BITS   8
#define CEL_TIMEWHEEL_SPOKE_BITS        6
#define CEL_TIMEWHEEL_BASE_SPOKE_SIZE  (1 << CEL_TIMEWHEEL_BASE_SPOKE_BITS)
#define CEL_TIMEWHEEL_SPOKE_SIZE       (1 << CEL_TIMEWHEEL_SPOKE_BITS)
#define CEL_TIMEWHEEL_BASE_SPOKE_MASK  (CEL_TIMEWHEEL_BASE_SPOKE_SIZE - 1)
#define CEL_TIMEWHEEL_SPOKE_MASK       (CEL_TIMEWHEEL_SPOKE_SIZE - 1)
#define CEL_TIMEWHEEL_BASE_SPOKE_RES    10 /* unit:milliseconds */

typedef struct _CelTimerWheel
{
    int tv1_spoke, tv2_spoke, tv3_spoke, tv4_spoke, tv5_spoke;
    int size;
    long now_expires, base_expires;
    CelTimer *earliest;
    struct timeval base_timeval;
    CelQueue *tv1[CEL_TIMEWHEEL_BASE_SPOKE_SIZE];
    CelQueue *tv2[CEL_TIMEWHEEL_SPOKE_SIZE];
    CelQueue *tv3[CEL_TIMEWHEEL_SPOKE_SIZE]; 
    CelQueue *tv4[CEL_TIMEWHEEL_SPOKE_SIZE];
    CelQueue *tv5[CEL_TIMEWHEEL_SPOKE_SIZE];
}CelTimerWheel;

int cel_timerwheel_init(CelTimerWheel *timer_wheel, CelFreeFunc free_func);
void cel_timerwheel_destroy(CelTimerWheel *timer_wheel);
CelTimerWheel *cel_timerwheel_new(CelFreeFunc free_func);
void cel_timerwheel_free(CelTimerWheel *timer_wheel);

BOOL cel_timerwheel_push(CelTimerWheel *timer_wheel, CelTimer *timer);

static __inline int cel_timerwheel_get_size(CelTimerWheel *timer_wheel)
{
    return timer_wheel->size;
}
static __inline BOOL cel_timerwheel_is_empty(CelTimerWheel *timer_wheel)
{
    return (timer_wheel->size <= 0 ? TRUE : FALSE);
}

CelTimer *cel_timerwheel_get_earliest(CelTimerWheel *timer_wheel);
int cel_timerwheel_cancel(CelTimerWheel *timer_wheel, CelTimerId timer_id);

long cel_timerwheel_pop_timeout(CelTimerWheel *timer_wheel, 
                                const struct timeval *now);
int cel_timerwheel_pop_expired(CelTimerWheel *timer_wheel, CelTimer **timers,
                               int max_timers, const struct timeval *now);
int cel_timerwheel_remove_expired(CelTimerWheel *timer_wheel, 
                                  const struct timeval *now);

void cel_timerwheel_clear(CelTimerWheel *timer_wheel);

#ifdef __cplusplus
}
#endif

#endif
