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
#ifndef __CEL_TIMERHEAP_H__
#define __CEL_TIMERHEAP_H__

#include "cel/minheap.h"
#include "cel/timer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelTimerHeap
{
    CelTimer *earliest;
    CelMinHeap heap;
}CelTimerHeap;

int cel_timerheap_init(CelTimerHeap *timer_heap, CelFreeFunc free_func);
void cel_timerheap_destroy(CelTimerHeap *timer_heap);
CelTimerHeap *cel_timerheap_new(CelFreeFunc free_func);
void cel_timerheap_free(CelTimerHeap *timer_heap);

BOOL cel_timerheap_push(CelTimerHeap *timer_heap, CelTimer *timer);

static __inline size_t cel_timerheap_get_size(CelTimerHeap *timer_heap)
{
    return cel_minheap_get_size(&((timer_heap)->heap));
}
static __inline BOOL cel_timerheap_is_empty(CelTimerHeap *timer_heap)
{
    return cel_minheap_is_empty(&((timer_heap)->heap));
}  
/* int cel_timerheap_get_capacity(CelTimerHeap *timer_heap)*/
#define cel_timerheap_get_capacity(timer_heap) \
    cel_minheap_get_capacity(&((timer_heap)->heap))
/* void cel_timerheap_set_reserve(CelTimerHeap *timer_heap, int num)*/
#define cel_timerheap_set_reserve(timer_heap, num) \
    cel_minheap_set_reserve(&((timer_heap)->heap), num)

CelTimer *cel_timerheap_get_earliest(CelTimerHeap *timer_heap);
int cel_timerheap_cancel(CelTimerHeap *timer_heap, CelTimerId timer_id);

long cel_timerheap_pop_timeout(CelTimerHeap *timer_heap, 
                               const struct timeval *now);
int cel_timerheap_pop_expired(CelTimerHeap *timer_heap, CelTimer **timers,
                              int max_timers, const struct timeval *now);
int cel_timerheap_remove_expired(CelTimerHeap *timer_heap,
                                 const struct timeval *now);

static __inline void cel_timerheap_clear(CelTimerHeap *timer_heap)
{
    cel_minheap_clear(&((timer_heap)->heap));
    timer_heap->earliest = NULL;
}


#ifdef __cplusplus
}
#endif

#endif
