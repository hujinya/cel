/**
 * CEL(C Extension Library)
 * Copyright (C)2008 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/timerheap.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

int cel_timerheap_init(CelTimerHeap *timer_heap, CelFreeFunc free_func)
{
    timer_heap->earliest = NULL;
    return cel_minheap_init(
        &(timer_heap->heap), (CelCompareFunc)cel_timer_compare, free_func);
}

void cel_timerheap_destroy(CelTimerHeap *timer_heap)
{
    cel_minheap_destroy(&(timer_heap->heap));
}

CelTimerHeap *cel_timerheap_new(CelFreeFunc free_func)
{
    CelTimerHeap *timer_heap;

    if ((timer_heap = 
        (CelTimerHeap *)cel_malloc(sizeof(CelTimerHeap))) != NULL)
    {
        if (cel_timerheap_init(timer_heap, free_func) == 0)
            return timer_heap;
        cel_free(timer_heap);
    }
    return NULL;
}

void cel_timerheap_free(CelTimerHeap *timer_heap)
{
    cel_timerheap_destroy(timer_heap); 
    cel_free(timer_heap);
}

BOOL cel_timerheap_push(CelTimerHeap *timer_heap, CelTimer *timer)
{
    cel_minheap_insert(&((timer_heap)->heap), timer);
    if (/*timer_heap->earliest == NULL
        || */(timer_heap->earliest 
        = (CelTimer *)cel_minheap_get_min(&(timer_heap->heap))) == timer)
        return TRUE;
    else
        return FALSE;
}

int cel_timerheap_cancel(CelTimerHeap *timer_heap, CelTimerId timer_id)
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

CelTimer *cel_timerheap_get_earliest(CelTimerHeap *timer_heap)
{
    /* This funciton is not thread-safe, not allow remove stopped timer */
    return (timer_heap->earliest != NULL 
        ? timer_heap->earliest 
        : (timer_heap->earliest = 
        (CelTimer *)cel_minheap_get_min(&(timer_heap->heap))));
}

long cel_timerheap_pop_timeout(CelTimerHeap *timer_heap, 
                               const CelTime *now)
{
    long diffmilliseconds;
    CelTimer *timer;

    CEL_TIME_NOW(now);
    if ((timer = cel_timerheap_get_earliest(timer_heap)) == NULL)
    {
        //_tprintf(_T("cel_timerheap_get_earliest null"));
        return -1;
    }
    diffmilliseconds = cel_timer_expired_interval(timer, now);
    //_tprintf(_T("diffmilliseconds = %ld \r\n"), diffmilliseconds);
    return (diffmilliseconds < 0 ? 0 : diffmilliseconds);
}

int cel_timerheap_pop_expired(CelTimerHeap *timer_heap, 
                              CelTimer **timers, int max_timers, 
                              const CelTime *now)
{
    int i = 0;
    CelTimer *timer;

    CEL_TIME_NOW(now);
    //puts("cel_timerheap_pop_expired start");
    while (i < max_timers
        && (timer = 
        (CelTimer *)cel_minheap_get_min(&(timer_heap->heap))) != NULL
        && cel_timer_is_expired(timer, now))
    {
        //printf("%x\r\n", timer);
        /* If elapsed != 0, this timer is stoped, remove it */
        timer_heap->earliest = NULL;
        if (timer->elapsed == 0)
            timers[i++] = (CelTimer *)cel_minheap_pop_min(&(timer_heap->heap));
        else
            cel_minheap_remove_min(&(timer_heap->heap));
        continue;
    }
    //puts("cel_timerheap_pop_expired end");

    return i;
}

int cel_timerheap_remove_expired(CelTimerHeap *timer_heap, 
                                 const CelTime *now)
{
    int i = 0;
    CelTimer *timer;

    CEL_TIME_NOW(now);
    while ((timer = 
        (CelTimer *)cel_minheap_get_min(&(timer_heap->heap))) != NULL)
    {
        if (cel_timer_is_expired(timer, now))
        {
            timer_heap->earliest = NULL;
            cel_minheap_remove_min(&(timer_heap->heap));
            i++;
            continue;
        }
        break;
    }

    return i;
}
