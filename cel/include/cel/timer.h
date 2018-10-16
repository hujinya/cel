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
#ifndef __CEL_TIMER_H__
#define __CEL_TIMER_H__

#include "cel/types.h"
#include "cel/datetime.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CEL_EVENT_TIMER    0x01

typedef void * CelTimerId;
typedef void (* CelTimerCallbackFunc) (void *user_data);

typedef struct _CelTimer
{
    BYTE evt_type;
    int repeat;
    long interval, elapsed;
    CelTimerId timer_id;
    struct timeval expired;
    CelTimerCallbackFunc call_back;
    void *user_data;
}CelTimer;

#define CEL_TIMEVAL_NOW(now) \
    struct timeval ltime; \
    if (now == NULL) { gettimeofday(&ltime, NULL); now = &ltime;}
void cel_timeval_set(struct timeval *tv, 
                     const struct timeval *now, long milliseconds);
/* void cel_timeval_clear(struct timeval *tv); */
#define cel_timeval_clear(tv) (tv)->tv_sec = 0; (tv)->tv_usec = 0
int cel_timeval_compare(const struct timeval *tv2, const struct timeval *tv1);
/* BOOL cel_timeval_is_expired(struct timeval *tv, struct timeval *now); */
#define cel_timeval_is_expired(tv, now) (cel_timeval_compare((tv), now) <= 0)
long long cel_timeval_diffmicroseconds(const struct timeval *tv2, 
                                       const struct timeval *tv1);
long cel_timeval_diffmilliseconds(const struct timeval *tv2, 
                                  const struct timeval *tv1);

int cel_timer_init(CelTimer *timer, long milliseconds, int repeat,
                   CelTimerCallbackFunc call_back, void *user_data);
void cel_timer_destroy(CelTimer *timer);
CelTimer *cel_timer_new(long milliseconds, int repeat,
                        CelTimerCallbackFunc call_back, void *user_data);
void cel_timer_free(CelTimer *timer);

static __inline void cel_timer_start(CelTimer *timer, struct timeval *now)
{
    cel_timeval_set(&((timer)->expired), now, ((timer)->elapsed > 0 
        ? ((timer)->interval - (timer)->elapsed) : (timer)->interval));
    (timer)->elapsed = 0;
}
static __inline void cel_timer_stop(CelTimer *timer, struct timeval *now)
{ 
    if (((timer)->elapsed 
        = cel_timeval_diffmilliseconds(&((timer)->expired), now)) <= 0) 
        (timer)->elapsed = -1; 
}
static __inline void cel_timer_restart(CelTimer *timer, struct timeval *now)
{
    cel_timeval_set(&((timer)->expired), now, (timer)->interval); 
    (timer)->elapsed = 0;
}

static __inline int cel_timer_compare(const CelTimer *timer2, 
                                      const CelTimer *timer1)
{
    return cel_timeval_compare(&(timer2->expired), &(timer1->expired));
}
/* 
 * long cel_timer_expired_interval(const CelTimer *timer, 
 *                                 const struct timeval *now);
 */
#define cel_timer_expired_interval(timer, now) \
    cel_timeval_diffmilliseconds(&((timer)->expired), now)
/* 
 * BOOL cel_timer_is_expired(const CelTimer *timer, const struct timeval *now);
 */
#define cel_timer_is_expired(timer, now) \
    (cel_timeval_compare(&((timer)->expired), now) <= 0)

#ifdef __cplusplus
}
#endif

#endif
