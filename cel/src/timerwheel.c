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
#include "cel/timerwheel.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

static int cel_timerwheel_queue_init(CelQueue **tv, int max_index, 
                                     CelFreeFunc free_func)
{
    int i = 0;

    while (i < max_index)
    {
        if ((tv[i++] = cel_queue_new(free_func)) == NULL)
        {
            i--;
            while ((--i) > 0)
                cel_queue_free(tv[i]);
            return -1;
        }
    }
    return 0;
}

static void cel_timerwheel_queue_destroy(CelQueue **tv, int max_index)
{
    int i = 0;

    while (i < max_index)
    {
        cel_queue_free(tv[i]);
        tv[i++] = NULL;
    }
}

int cel_timerwheel_init(CelTimerWheel *timer_wheel, CelFreeFunc free_func)
{
    if (cel_timerwheel_queue_init(
        timer_wheel->tv1, CEL_TIMEWHEEL_BASE_SPOKE_SIZE, free_func) != -1)
    {
        if (cel_timerwheel_queue_init(
            timer_wheel->tv2, CEL_TIMEWHEEL_SPOKE_SIZE, free_func) != -1)
        {
            if (cel_timerwheel_queue_init(
                timer_wheel->tv3, CEL_TIMEWHEEL_SPOKE_SIZE, free_func) != -1)
            {
                if (cel_timerwheel_queue_init(
                    timer_wheel->tv4,
                    CEL_TIMEWHEEL_SPOKE_SIZE, free_func) != -1)
                {
                    if (cel_timerwheel_queue_init(
                        timer_wheel->tv5, 
                        CEL_TIMEWHEEL_SPOKE_SIZE, free_func) != -1)
                    {
                        timer_wheel->tv1_spoke = 0;
                        timer_wheel->tv2_spoke = 0;
                        timer_wheel->tv3_spoke = 0;
                        timer_wheel->tv4_spoke = 0;
                        timer_wheel->tv5_spoke = 0;
                        timer_wheel->size = 0;
                        timer_wheel->now_expires = 0;
                        timer_wheel->base_expires = 0;
                        timer_wheel->earliest = NULL;
                        gettimeofday(&(timer_wheel->base_timeval), NULL);
                        return 0;
                    }
                    cel_timerwheel_queue_destroy(
                        timer_wheel->tv4, CEL_TIMEWHEEL_SPOKE_SIZE);
                }
                cel_timerwheel_queue_destroy(
                    timer_wheel->tv3, CEL_TIMEWHEEL_SPOKE_SIZE);
            }
            cel_timerwheel_queue_destroy(timer_wheel->tv2, 
                CEL_TIMEWHEEL_SPOKE_SIZE);
        }
        cel_timerwheel_queue_destroy(timer_wheel->tv1, 
            CEL_TIMEWHEEL_BASE_SPOKE_SIZE);
    }
    return -1;
}

void cel_timerwheel_destroy(CelTimerWheel *timer_wheel)
{
    cel_timerwheel_queue_destroy(timer_wheel->tv1, 
        CEL_TIMEWHEEL_BASE_SPOKE_SIZE);
    cel_timerwheel_queue_destroy(timer_wheel->tv2, 
        CEL_TIMEWHEEL_SPOKE_SIZE);
    cel_timerwheel_queue_destroy(timer_wheel->tv3, 
        CEL_TIMEWHEEL_SPOKE_SIZE);
    cel_timerwheel_queue_destroy(timer_wheel->tv4, 
        CEL_TIMEWHEEL_SPOKE_SIZE);
    cel_timerwheel_queue_destroy(timer_wheel->tv5, 
        CEL_TIMEWHEEL_SPOKE_SIZE);
}

CelTimerWheel *cel_timerwheel_new(CelFreeFunc free_func)
{
    CelTimerWheel *timer_wheel;

    if ((timer_wheel = (CelTimerWheel *)
        cel_malloc(sizeof(CelTimerWheel))) != NULL)
    {
        if (cel_timerwheel_init(timer_wheel, free_func) == 0)
            return timer_wheel;
        cel_free(timer_wheel);
    }
    return NULL;
}

void cel_timerwheel_free(CelTimerWheel *timer_wheel)
{
    cel_timerwheel_destroy(timer_wheel); 
    cel_free(timer_wheel);
}

static long cel_timerwheel_expries(CelTimerWheel *timer_wheel,
                                   const struct timeval *tv)
{
    long expries;

    if (timer_wheel->base_expires > 0)
    {
        //printf("Timer wheel base_expires %ld\r\n", 
        //    timer_wheel->base_expires);
        if ((expries = (timer_wheel->base_expires 
            * CEL_TIMEWHEEL_BASE_SPOKE_RES)) > 1000000L)
        {
            timer_wheel->base_timeval.tv_sec += (expries / 1000000L);
            expries %= 1000000L;
        }
        timer_wheel->base_timeval.tv_usec += expries;
        timer_wheel->base_timeval.tv_sec += 
            (timer_wheel->base_timeval.tv_usec / 1000000L);
        timer_wheel->base_timeval.tv_usec %= 1000000L;
        timer_wheel->base_expires = 0;
    }
   /* printf("%ld-%ld %ld-%ld\r\n", 
        tv->tv_sec, tv->tv_usec,
        timer_wheel->base_timeval.tv_sec, timer_wheel->base_timeval.tv_usec);*/

    return (cel_timeval_diffmilliseconds(tv, &timer_wheel->base_timeval)
        / CEL_TIMEWHEEL_BASE_SPOKE_RES);
}

static void cel_timerwheel_add_timer(CelTimerWheel *timer_wheel, 
                                     CelTimer *timer)
{
    int idx;
    long expries = cel_timerwheel_expries(timer_wheel, &(timer->expired));
    
    //printf("timer expires %ld \r\n", expries);
    if (expries < 0)
    {
        /*
         * Can happen if you add a timer with now_expires == jiffies,
         * or you set a timer to go off in the past
         */
        idx = timer_wheel->tv1_spoke;
        cel_queue_push_back(timer_wheel->tv1[idx], timer);
        CEL_DEBUG((_T("Add timer(%ld)to timerwheel, 1 - %d."), expries, idx));
    }
    else if (expries < CEL_TIMEWHEEL_BASE_SPOKE_SIZE)
    {
        idx = (expries & CEL_TIMEWHEEL_BASE_SPOKE_MASK);
        if ((idx = idx + timer_wheel->tv1_spoke) 
            >= CEL_TIMEWHEEL_BASE_SPOKE_SIZE)
            idx -= CEL_TIMEWHEEL_BASE_SPOKE_SIZE;
        cel_queue_push_back(timer_wheel->tv1[idx], timer);
        CEL_DEBUG((_T("Add timer(%ld)to timerwheel, 1 - %d."), expries, idx));
    }
    else if (expries < 
        (1 << (CEL_TIMEWHEEL_BASE_SPOKE_BITS + CEL_TIMEWHEEL_SPOKE_BITS)))
    {
        idx = ((expries >> CEL_TIMEWHEEL_BASE_SPOKE_BITS) 
            & CEL_TIMEWHEEL_SPOKE_MASK);
        //printf("idx %d\r\n", idx);
        if ((idx = idx + timer_wheel->tv2_spoke) >= CEL_TIMEWHEEL_SPOKE_SIZE)
            idx -= CEL_TIMEWHEEL_SPOKE_SIZE;
        cel_queue_push_back(timer_wheel->tv2[idx], timer);
        CEL_DEBUG((_T("Add timer(%ld)to timerwheel, 2 - %d."), expries, idx));
    }
    else if (expries < 
        (1 << (CEL_TIMEWHEEL_BASE_SPOKE_BITS + 2 * CEL_TIMEWHEEL_SPOKE_BITS)))
    {
        idx = ((expries 
            >> (CEL_TIMEWHEEL_BASE_SPOKE_BITS + CEL_TIMEWHEEL_SPOKE_BITS))
            & CEL_TIMEWHEEL_SPOKE_MASK);
        if ((idx = idx + timer_wheel->tv3_spoke) >= CEL_TIMEWHEEL_SPOKE_SIZE)
            idx -= CEL_TIMEWHEEL_SPOKE_SIZE;
        cel_queue_push_back(timer_wheel->tv3[idx], timer);
        CEL_DEBUG((_T("Add timer(%ld)to timerwheel, 3 - %d."), expries, idx));
    }
    else if (expries < 
        (1 << (CEL_TIMEWHEEL_BASE_SPOKE_BITS + 3 * CEL_TIMEWHEEL_SPOKE_BITS)))
    {
        idx = ((expries 
            >> (CEL_TIMEWHEEL_BASE_SPOKE_BITS + 2 * CEL_TIMEWHEEL_SPOKE_BITS))
            & CEL_TIMEWHEEL_SPOKE_MASK);
        if ((idx = idx + timer_wheel->tv4_spoke) >= CEL_TIMEWHEEL_SPOKE_SIZE)
            idx -= CEL_TIMEWHEEL_SPOKE_SIZE;
        cel_queue_push_back(timer_wheel->tv4[idx], timer);
        CEL_DEBUG((_T("Add timer(%ld)to timerwheel, 4 - %d."), expries, idx));
    }
    else
    {
        /* 
         * If the timeout is larger than 0xffffffff on 64-bit
         * architectures then we use the maximum timeout.
         */
        if (expries > 0xffffffffL)
            expries = 0xffffffffL;
        idx = ((expries 
            >> (CEL_TIMEWHEEL_BASE_SPOKE_BITS + 3 * CEL_TIMEWHEEL_SPOKE_BITS))
            & CEL_TIMEWHEEL_SPOKE_MASK);
        if ((idx = idx + timer_wheel->tv5_spoke) >= CEL_TIMEWHEEL_SPOKE_SIZE)
            idx -= CEL_TIMEWHEEL_SPOKE_SIZE;
        cel_queue_push_back(timer_wheel->tv5[idx], timer);
        CEL_DEBUG((_T("Add timer(%ld)to timerwheel, 5 - %d."), expries, idx));
    }
}

BOOL cel_timerwheel_push(CelTimerWheel *timer_wheel, CelTimer *timer)
{
    cel_timerwheel_add_timer(timer_wheel, timer);
    timer_wheel->size++;
    if (timer_wheel->earliest == NULL)
        return TRUE;
    else if (cel_timer_compare(timer_wheel->earliest, timer) > 0)
    {
        timer_wheel->earliest = timer;
        return TRUE;
    }
    return FALSE;
}

CelTimer *cel_timerwheel_get_earliest(CelTimerWheel *timer_wheel)
{
    int i;
    CelTimer *timer;

    if (timer_wheel->earliest != NULL)
        return timer_wheel->earliest;
    if (timer_wheel->size <= 0)
        return NULL;
    i = timer_wheel->tv1_spoke;
    do
    {
        if ((timer = (CelTimer *)
            cel_queue_get_front(timer_wheel->tv1[i])) != NULL)
            return timer;
        if ((++i) >= CEL_TIMEWHEEL_BASE_SPOKE_SIZE) 
            i = 0;
    }while (i != timer_wheel->tv1_spoke);
    i = timer_wheel->tv2_spoke;
    do
    {
        if ((timer = (CelTimer *)
            cel_queue_get_front(timer_wheel->tv2[i])) != NULL)
            return timer;
        if ((++i) >= CEL_TIMEWHEEL_SPOKE_SIZE) i = 0;
    }while (i != timer_wheel->tv2_spoke);
    i = timer_wheel->tv3_spoke;
    do
    {
        if ((timer = (CelTimer *)
            cel_queue_get_front(timer_wheel->tv3[i])) != NULL)
            return timer;
        if ((++i) >= CEL_TIMEWHEEL_BASE_SPOKE_SIZE) i = 0;
    }while (i != timer_wheel->tv3_spoke);
    i = timer_wheel->tv4_spoke;
    do
    {
        if ((timer = (CelTimer *)
            cel_queue_get_front(timer_wheel->tv4[i])) != NULL)
            return timer;
        if ((++i) >= CEL_TIMEWHEEL_BASE_SPOKE_SIZE) i = 0;
    }while (i != timer_wheel->tv4_spoke);
    i = timer_wheel->tv5_spoke;
    do
    {
        if ((timer = (CelTimer *)
            cel_queue_get_front(timer_wheel->tv5[i])) != NULL)
            return timer;
        if ((++i) >= CEL_TIMEWHEEL_BASE_SPOKE_SIZE) i = 0;
    }while (i != timer_wheel->tv5_spoke);

    return NULL;
}

int cel_timerwheel_cancel(CelTimerWheel *timer_wheel, CelTimerId timer_id)
{
    CelTimer *timer = (CelTimer *)timer_id;

    if (timer->timer_id != timer_id) 
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Timer(%p) id %p invaild."), timer, timer->timer_id));
        return -1;
    }
    cel_timer_stop(timer, NULL);
    return 0;
}

long cel_timerwheel_pop_timeout(CelTimerWheel *timer_wheel,
                                const struct timeval *now)
{
    long diffmilliseconds;
    CelTimer *timer;

    CEL_TIMEVAL_NOW(now);
    if ((timer = cel_timerwheel_get_earliest(timer_wheel)) == NULL)
        return -1;
    return ((diffmilliseconds = cel_timer_expired_interval(timer, now)) < 0)
        ? 0 : diffmilliseconds;
}

static int cel_timerwheel_pop_expired_intern(CelTimerWheel *timer_wheel, 
                                             CelTimer **timers,
                                             int max_timers, 
                                             const struct timeval *now,
                                             BOOL is_remove)
{
    int i = 0;
    CelTimer *timer;

    CEL_TIMEVAL_NOW(now);
    timer_wheel->now_expires += cel_timerwheel_expries(timer_wheel, now);
    //printf("now expires %ld\r\n", timer_wheel->now_expires);
    do 
    {
        while ((timer = (CelTimer *)cel_queue_get_front(
            (timer_wheel->tv1[timer_wheel->tv1_spoke]))) != NULL)
        {
            timer_wheel->size--;
            timer_wheel->earliest = NULL;
            if (!is_remove)
            {
                if (timer->elapsed == 0)
                {
                    timers[i] = (CelTimer *)cel_queue_pop_front(
                        (timer_wheel->tv1[timer_wheel->tv1_spoke]));
                    CEL_DEBUG((_T("Timer expired.")));
                    if ((i++) >= max_timers)
                        return i;
                }
                else
                {
                    cel_queue_remove_front(
                        (timer_wheel->tv1[timer_wheel->tv1_spoke]));
                }
            }
            else
            {
                cel_queue_remove_front(
                    (timer_wheel->tv1[timer_wheel->tv1_spoke]));
            }
        }
        if (timer_wheel->now_expires <= 0) 
            break;
        timer_wheel->now_expires--;
        timer_wheel->base_expires++;
        /*printf("Timer wheel base time:%ld:%ld\r\n", 
            timer_wheel->base_timeval.tv_sec, 
            timer_wheel->base_timeval.tv_usec);*/
        if ((++timer_wheel->tv1_spoke) >= CEL_TIMEWHEEL_BASE_SPOKE_SIZE)
        {
            timer_wheel->tv1_spoke = 0;
            if ((++timer_wheel->tv2_spoke) >= CEL_TIMEWHEEL_SPOKE_SIZE)
            {
                timer_wheel->tv2_spoke = 0;
                if ((++timer_wheel->tv3_spoke) >= CEL_TIMEWHEEL_SPOKE_SIZE)
                {
                    timer_wheel->tv3_spoke = 0;
                    if ((++timer_wheel->tv4_spoke) >= CEL_TIMEWHEEL_SPOKE_SIZE)
                    {
                        timer_wheel->tv4_spoke = 0;
                        if ((++timer_wheel->tv5_spoke) 
                            >= CEL_TIMEWHEEL_SPOKE_SIZE)
                        {
                            timer_wheel->tv5_spoke = 0; 
                        }
                        while ((timer = (CelTimer *)cel_queue_pop_front(
                            (timer_wheel->tv5[timer_wheel->tv5_spoke]))) != NULL)
                        {
                            cel_timerwheel_add_timer(timer_wheel, timer); 
                        }
                    }
                    while ((timer = (CelTimer *)cel_queue_pop_front(
                        (timer_wheel->tv4[timer_wheel->tv4_spoke]))) != NULL)
                    {
                        cel_timerwheel_add_timer(timer_wheel, timer); 
                    }
                }
                while ((timer = (CelTimer *)cel_queue_pop_front(
                    (timer_wheel->tv3[timer_wheel->tv3_spoke]))) != NULL)
                {
                    CEL_DEBUG((_T("Timer cascade, 3 - %d."), 
                        timer_wheel->tv3_spoke));
                    cel_timerwheel_add_timer(timer_wheel, timer); 
                }
            }
            while ((timer = (CelTimer *)cel_queue_pop_front(
                (timer_wheel->tv2[timer_wheel->tv2_spoke]))) != NULL)
            {
                CEL_DEBUG((_T("Timer cascade, 2 - %d."), timer_wheel->tv2_spoke));
                cel_timerwheel_add_timer(timer_wheel, timer);
            }
        }
    }while (TRUE);

    return i;
}

int cel_timerwheel_pop_expired(CelTimerWheel *timer_wheel, CelTimer **timers, 
                               int max_timers, const struct timeval *now)
{
    return cel_timerwheel_pop_expired_intern(
        timer_wheel, timers, max_timers, now, FALSE);
}

int cel_timerwheel_remove_expired(CelTimerWheel *timer_wheel, 
                                  const struct timeval *now)
{
    return cel_timerwheel_pop_expired_intern(timer_wheel, NULL, 0, now, TRUE);
}

void cel_timerwheel_clear(CelTimerWheel *timer_wheel)
{
    int i;

    i = 0;
    while (i < CEL_TIMEWHEEL_BASE_SPOKE_SIZE)
        cel_queue_clear((timer_wheel->tv1[i++]));
    i = 0;
    while (i < CEL_TIMEWHEEL_SPOKE_SIZE)
        cel_queue_clear((timer_wheel->tv2[i++]));
    i = 0;
    while (i < CEL_TIMEWHEEL_SPOKE_SIZE)
        cel_queue_clear((timer_wheel->tv3[i++]));
    i = 0;
    while (i < CEL_TIMEWHEEL_SPOKE_SIZE)
        cel_queue_clear((timer_wheel->tv4[i++]));
    i = 0;
    while (i < CEL_TIMEWHEEL_SPOKE_SIZE)
        cel_queue_clear((timer_wheel->tv5[i++]));
    timer_wheel->tv1_spoke = 0;
    timer_wheel->tv2_spoke = 0;
    timer_wheel->tv3_spoke = 0;
    timer_wheel->tv4_spoke = 0;
    timer_wheel->tv5_spoke = 0;
    timer_wheel->now_expires = 0;
    timer_wheel->base_expires = 0;
    timer_wheel->size = 0;
    timer_wheel->earliest = NULL;
    gettimeofday(&(timer_wheel->base_timeval), NULL);
}
