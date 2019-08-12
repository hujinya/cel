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
#include "cel/timerqueue.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"


static CelTimerQueueClass timer_heap_kclass = 
{
    (CelTimerQueueDestroyFunc)cel_timerheap_destroy,
    (CelTimerQueuePushFunc)cel_timerheap_push,
    (CelTimerQueueGetSizeFunc)cel_timerheap_get_size,
    (CelTimerQueueIsEmptyFunc)cel_timerheap_is_empty,
    (CelTimerQueueGetEarliestFunc)cel_timerheap_get_earliest,
    (CelTimerQueuePopTimeoutlFunc)cel_timerheap_pop_timeout,
    (CelTimerQueueCancelFunc)cel_timerheap_cancel,
    (CelTimerQueuePopExpiredFunc)cel_timerheap_pop_expired,
    (CelTimerQueueRemoveExpiredFunc)cel_timerheap_remove_expired,
    (CelTimerQueueClearFunc)cel_timerheap_clear,
};

static CelTimerQueueClass timer_wheel_kclass = 
{
    (CelTimerQueueDestroyFunc)cel_timerwheel_destroy,
    (CelTimerQueuePushFunc)cel_timerwheel_push,
    (CelTimerQueueGetSizeFunc)cel_timerwheel_get_size,
    (CelTimerQueueIsEmptyFunc)cel_timerwheel_is_empty,
    (CelTimerQueueGetEarliestFunc)cel_timerwheel_get_earliest,
    (CelTimerQueuePopTimeoutlFunc)cel_timerwheel_pop_timeout,
    (CelTimerQueueCancelFunc)cel_timerwheel_cancel,
    (CelTimerQueuePopExpiredFunc)cel_timerwheel_pop_expired,
    (CelTimerQueueRemoveExpiredFunc)cel_timerwheel_remove_expired,
    (CelTimerQueueClearFunc)cel_timerwheel_clear
};

int cel_timerqueue_init_full(CelTimerQueue *timer_queue, 
                             CelTimerQueueType type, 
                             CelFreeFunc free_func)
{
    int ret;

    if (cel_mutex_init(&(timer_queue->mutex), NULL) == 0)
    {
        switch (type)
        {
        case CEL_TQ_TIMERHEAP:
             timer_queue->kclass = &timer_heap_kclass;
             ret = cel_timerheap_init((CelTimerHeap *)timer_queue, free_func);
            break;
        case CEL_TQ_TIMERWHEEL:
            timer_queue->kclass = &timer_wheel_kclass;
            ret = cel_timerwheel_init((CelTimerWheel *)timer_queue, free_func);
            break;
        default:
            ret = -1;
            CEL_SETERR((CEL_ERR_LIB, _T("Timerqueue type %d undefined."), type));
            break;
        }
        if (ret != -1)
            return 0;
        cel_mutex_destroy(&(timer_queue->mutex));
    }
    return -1;
}

CelTimerQueue *cel_timerqueue_new_full(CelTimerQueueType type, 
                                       CelFreeFunc free_func)
{
    CelTimerQueue *timer_queue;

    if ((timer_queue = (CelTimerQueue *)
        cel_malloc(sizeof(CelTimerQueue))) != NULL)
    {
        if (cel_timerqueue_init_full(timer_queue, type, free_func) == 0)
            return timer_queue;
        cel_free(timer_queue);
    }
    return NULL;
}

void cel_timerqueue_free(CelTimerQueue *timer_queue)
{
    cel_timerqueue_destroy(timer_queue);
    cel_free(timer_queue);
}
