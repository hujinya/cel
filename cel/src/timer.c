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
#include "cel/timer.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

void cel_timeval_set(struct timeval *tv, 
                     const struct timeval *now, long milliseconds)
{
    CEL_TIMEVAL_NOW(now);
    tv->tv_sec = now->tv_sec + (milliseconds / 1000L);
    if ((tv->tv_usec = now->tv_usec 
        + ((milliseconds % 1000L) * 1000L)) > 1000000L)
    {
        tv->tv_sec += (tv->tv_usec / 1000000L);
        tv->tv_usec = (tv->tv_usec % 1000000L);
    }
    //_tprintf(_T("%ld - %ld %ld\n"), tv->tv_sec, tv->tv_usec, milliseconds);
}

int cel_timeval_compare(const struct timeval *tv2, const struct timeval *tv1)
{
    CEL_TIMEVAL_NOW(tv1);

    /*_tprintf(_T("%ld %ld - %ld %ld\n"), 
               tv2->tv_sec, tv2->tv_usec, tv1->tv_sec, tv1->tv_usec);*/
    if (tv2->tv_sec > tv1->tv_sec)
        return 1;
    else if (tv2->tv_sec < tv1->tv_sec)
        return -1;
    else if (tv2->tv_usec > tv1->tv_usec)
        return 1;
    else if (tv2->tv_usec < tv1->tv_usec)
        return -1;
    return 0;
}

long long cel_timeval_diffmicroseconds(const struct timeval *tv2, 
                                       const struct timeval *tv1)
{
    long long diff_microseconds;

    CEL_TIMEVAL_NOW(tv1);
    diff_microseconds = tv2->tv_usec - tv1->tv_usec;
    diff_microseconds += (tv2->tv_sec - tv1->tv_sec) * LL(1000000);

    return diff_microseconds;
}

long cel_timeval_diffmilliseconds(const struct timeval *tv2, 
                                  const struct timeval *tv1)
{
    long diff_microseconds, diff_milliseconds;

    CEL_TIMEVAL_NOW(tv1);
    diff_milliseconds = (tv2->tv_sec - tv1->tv_sec) * L(1000);
    diff_milliseconds += 
        (diff_microseconds = tv2->tv_usec - tv1->tv_usec) / L(1000);
    if (diff_microseconds % L(1000) != 0)
    {
        if (diff_milliseconds >= 0)
        {
            if (diff_microseconds > 0)
                diff_milliseconds += 1;
        }
        else if (diff_microseconds < 0)
            diff_milliseconds += -1;
    }
    /*
    _tprintf(_T("%ld %ld - %ld %ld\n"), 
        tv2->tv_sec, tv2->tv_usec, tv1->tv_sec, tv1->tv_usec);
        */

    return diff_milliseconds;
}

int cel_timer_init(CelTimer *timer, long milliseconds, int repeat,
                   CelTimerCallbackFunc call_back, void *user_data)
{
    timer->evt_type = CEL_EVENT_TIMER;
    timer->repeat = repeat;
    timer->timer_id = timer;
    timer->interval = milliseconds;
    timer->elapsed = 0;
    timer->call_back = call_back;
    timer->user_data = user_data;
    cel_timer_start(timer, NULL);

    return 0;
}

void cel_timer_destroy(CelTimer *timer)
{
    cel_timeval_clear(&(timer->expired));
    timer->repeat = 0;
    timer->timer_id = 0;
    timer->interval = 0;
    timer->elapsed = 0;
    timer->call_back = NULL;
    timer->user_data = NULL;
}

CelTimer *cel_timer_new(long microseconds, int repeat,
                        CelTimerCallbackFunc call_back, void *user_data)
{
    CelTimer *timer;

    if ((timer = (CelTimer *)cel_malloc(sizeof(CelTimer))) != NULL)
    {
        if (cel_timer_init(
            timer, microseconds, repeat, call_back, user_data) == 0)
            return timer;
        cel_free(timer);
    }

    return NULL;
}

void cel_timer_free(CelTimer *timer)
{
    //_tprintf(_T("Timer %p free.\r\n"), timer);
    cel_timer_destroy(timer); 
    cel_free(timer);
}
