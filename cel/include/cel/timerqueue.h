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
#ifndef __CEL_TIMERQUEUE_H__
#define __CEL_TIMERQUEUE_H__

#include "cel/types.h"
#include "cel/timer.h"
#include "cel/thread.h"
#include "cel/timerwheel.h"
#include "cel/timerheap.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (* CelTimerQueueDestroyFunc) (void *timer_queue);
typedef BOOL (* CelTimerQueuePushFunc) (void *timer_queue, CelTimer *timer);
typedef int (* CelTimerQueueGetSizeFunc) (void *timer_queue);
typedef BOOL (* CelTimerQueueIsEmptyFunc) (void *timer_queue);
typedef CelTimer * (* CelTimerQueueGetEarliestFunc) (void *timer_queue);
typedef int (* CelTimerQueueCancelFunc) (void *timer_queue, 
                                         CelTimerId timer_id);
typedef long (* CelTimerQueuePopTimeoutlFunc) (void *timer_queue,
                                               const struct timeval *now);
typedef int (* CelTimerQueuePopExpiredFunc) (void *timer_queue, 
                                             CelTimer **timers, int max_timers,
                                             const struct timeval *now);
typedef int (* CelTimerQueueRemoveExpiredFunc) (void *timer_queue,
                                                const struct timeval *now);
typedef void (* CelTimerQueueClearFunc) (void *timer_queue);

typedef enum _CelTimerQueueType
{
    CEL_TQ_UNDEFINED,
    CEL_TQ_TIMERHEAP,
    CEL_TQ_TIMERWHEEL
}CelTimerQueueType;

typedef struct _CelTimerQueueClass
{
    CelTimerQueueDestroyFunc destroy;
    CelTimerQueuePushFunc push;
    CelTimerQueueGetSizeFunc get_size;
    CelTimerQueueIsEmptyFunc is_empty;
    CelTimerQueueGetEarliestFunc get_earliest;
    CelTimerQueuePopTimeoutlFunc pop_timeout;
    CelTimerQueueCancelFunc cancel;
    CelTimerQueuePopExpiredFunc pop_expired;
    CelTimerQueueRemoveExpiredFunc remove_expired;
    CelTimerQueueClearFunc clear;
}CelTimerQueueClass;

typedef struct _CelTimerQueue
{
    union{
        CelTimerHeap heap;
        CelTimerWheel wheel;
    }queue;
    CelMutex mutex;
    CelTimerQueueClass *kclass;
}CelTimerQueue;

int cel_timerqueue_init_full(CelTimerQueue *timer_queue, 
                             CelTimerQueueType type,
                             CelFreeFunc free_func);
#define cel_timerqueue_init(timer_queue, free_func) \
    cel_timerqueue_init_full(timer_queue, CEL_TQ_TIMERHEAP, free_func)
/* void cel_timerqueue_destroy(CelTimerQueue *timer_queue) */
#define cel_timerqueue_destroy(timer_queue) \
    (timer_queue)->kclass->destroy(timer_queue)

CelTimerQueue *cel_timerqueue_new_full(CelTimerQueueType type, 
                                       CelFreeFunc free_func);
#define cel_timerqueue_new(free_func) \
    cel_timerqueue_new_full(CEL_TQ_TIMERHEAP, free_func)
void cel_timerqueue_free(CelTimerQueue *timer_queue);

#define cel_timerqueue_trylock(_timer_queue) \
    cel_mutex_trylock(&((_timer_queue)->mutex))
#define cel_timerqueue_lock(_timer_queue) \
    cel_mutex_lock(&((_timer_queue)->mutex))
#define cel_timerqueue_unlock(_timer_queue) \
    cel_mutex_unlock(&((_timer_queue)->mutex))

/* int cel_timerqueue_get_size_unlocked(CelTimerQueue *timer_queue) */
#define cel_timerqueue_get_size_unlocked(timer_queue) \
    (timer_queue)->kclass->get_size(timer_queue)
#define cel_timerqueue_get_size(timer_queue) \
    cel_timerqueue_lock(timer_queue); \
    cel_timerqueue_get_size_unlocked(timer_queue); \
    cel_timerqueue_unlock(timer_queue)

/* BOOL cel_timerqueue_is_empty(CelTimerQueue *timer_queue) */
#define cel_timerqueue_is_empty(timer_queue) \
    (timer_queue)->kclass->is_empty(timer_queue)

#define cel_timerqueue_get_earliest_unlocked(timer_queue) \
    (timer_queue)->kclass->get_earliest(timer_queue)
static __inline 
CelTimer *cel_timerqueue_get_earliest(CelTimerQueue *timer_queue)
{
    CelTimer *timer;
    cel_timerqueue_lock(timer_queue); 
    timer = cel_timerqueue_get_earliest_unlocked(timer_queue); 
    cel_timerqueue_unlock(timer_queue);
    return timer;
}

/* 
 * BOOL cel_timerqueue_push_unlocked(CelTimerQueue *timer_queue, 
 *                                   CelTimer *timer) 
 */
#define cel_timerqueue_push_unlocked(timer_queue, timer) \
    (timer_queue)->kclass->push(timer_queue, timer)
static __inline BOOL cel_timerqueue_push(CelTimerQueue *timer_queue,
                                         CelTimer *timer)
{
    BOOL earliest_changed;
    cel_timerqueue_lock(timer_queue); 
    earliest_changed = cel_timerqueue_push_unlocked(timer_queue, timer); 
    cel_timerqueue_unlock(timer_queue);
    return earliest_changed;
}

/* 
 * int cel_timerqueue_cancel(CelTimerQueue *timer_queue, CelTimerId timer_id)
 */
#define cel_timerqueue_cancel(timer_queue, timer_id) \
    (timer_queue)->kclass->cancel(timer_queue, timer_id)

#define cel_timerqueue_pop_timeout_unlocked(timer_queue, now) \
    (timer_queue)->kclass->pop_timeout(timer_queue, now)
static __inline long cel_timerqueue_pop_timeout(CelTimerQueue *timer_queue,
                                                const struct timeval *now)
{
    long timeout;
    cel_timerqueue_lock(timer_queue); 
    timeout = cel_timerqueue_pop_timeout_unlocked(timer_queue, now); 
    cel_timerqueue_unlock(timer_queue);
    return timeout;
}

/* 
 * int cel_timerqueue_pop_expired_unlocked(CelTimerQueue *timer_queue, 
 *                                         CelTimer **timers, int max_timers,
 *                                         const struct timeval *now) 
 */
#define cel_timerqueue_pop_expired_unlocked(\
    timer_queue, timers, max_timers, now) \
    (timer_queue)->kclass->pop_expired(timer_queue, timers, max_timers, now)
static __inline int cel_timerqueue_pop_expired(CelTimerQueue *timer_queue, 
                                               CelTimer **timers, 
                                               int max_timers,
                                               const struct timeval *now) 
{
    int ret;
    cel_timerqueue_lock(timer_queue); 
    ret = cel_timerqueue_pop_expired_unlocked(
        timer_queue, timers, max_timers, now); 
    cel_timerqueue_unlock(timer_queue);
    return ret;
}
/* 
 * int cel_timerqueue_remove_expired_unlocked(CelTimerQueue *timer_queue, 
 *                                            const struct timeval *now) 
 */
#define cel_timerqueue_remove_expired_unlocked(timer_queue, now) \
    (timer_queue)->kclass->remove_expired(timer_queue, now)
static __inline int cel_timerqueue_remove_expired(CelTimerQueue *timer_queue, 
                                                  const struct timeval *now) 
{
    int ret;
    cel_timerqueue_lock(timer_queue); 
    cel_timerqueue_remove_expired_unlocked(timer_queue, now); 
    cel_timerqueue_unlock(timer_queue);
    return ret;
}

/* void cel_timerqueue_clear(CelTimerQueue *timer_queue) */
#define cel_timerqueue_clear(timer_queue) (timer_queue)->clear(timer_queue)


#ifdef __cplusplus
}
#endif
#endif
