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
#ifndef __CEL_ASYNCQUEUE_H__
#define __CEL_ASYNCQUEUE_H__

#include "cel/types.h"
#include "cel/thread.h"
#include "cel/queue.h"
#include "cel/list.h"
#include "cel/minheap.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (* CelAsyncQueueDestroyFunc) (void *async_queue);
typedef void (* CelAsyncQueuePushFunc) (void *async_queue, void *item);
typedef int (* CelAsyncQueueGetSizeFunc) (void *async_queue);
typedef BOOL (* CelAsyncQueueIsEmptyFunc) (void *async_queue);
typedef void * (* CelAsyncQueuePopFunc) (void *async_queue);

typedef enum _CelAsyncQueueType
{
    CEL_AQ_UNDEFINED,
    CEL_AQ_QUEUE,
    CEL_AQ_LIST,
    CEL_AQ_MINHEAP
}CelAsyncQueueType;

typedef struct _CelAsyncQueueClass
{
    CelAsyncQueueDestroyFunc destroy;
    CelAsyncQueuePushFunc push;
    CelAsyncQueueGetSizeFunc get_size;
    CelAsyncQueueIsEmptyFunc is_empty;
    CelAsyncQueuePopFunc pop;
}CelAsyncQueueClass;

typedef struct _CelAsyncQueue
{
    union{
        CelQueue dqueue;
        CelList list;
        CelMinHeap min_heap;
    }queue;
    unsigned int waiting_threads;
    CelMutex mutex;
    CelCond cond;
    CelAsyncQueueClass *kclass;
}CelAsyncQueue;

int cel_asyncqueue_init_full(CelAsyncQueue *async_queue, 
                             CelAsyncQueueType type,
                             CelCompareFunc comparator, CelFreeFunc free_func);
/* 
 * int cel_asyncqueue_init(CelAsyncQueue *async_queue, 
 *                         CelFreeFunc free_func); 
 */
#define cel_asyncqueue_init(async_queue, free_func) \
    cel_asyncqueue_init_full(async_queue, CEL_AQ_QUEUE, NULL, free_func)
void cel_asyncqueue_destroy(CelAsyncQueue *async_queue);

CelAsyncQueue *cel_asyncqueue_new_full(CelAsyncQueueType type, 
                                       CelCompareFunc comparator, 
                                       CelFreeFunc free_func);
/* CelAsyncQueue *cel_asyncqueue_new(CelFreeFunc free_func); */
#define cel_asyncqueue_new(free_func) \
    cel_asyncqueue_new_full(CEL_AQ_QUEUE, NULL, free_func)
void cel_asyncqueue_free(CelAsyncQueue *async_queue);

static __inline BOOL cel_asyncqueue_trylock(CelAsyncQueue *async_queue)
{
    return (cel_mutex_trylock(&(async_queue->mutex)) == -1 ? FALSE : TRUE);
}
static __inline void cel_asyncqueue_lock(CelAsyncQueue *async_queue)
{
    cel_mutex_lock(&(async_queue->mutex));
}
static __inline void cel_asyncqueue_unlock(CelAsyncQueue *async_queue)
{
    cel_mutex_unlock(&(async_queue->mutex));
}

void cel_asyncqueue_push(CelAsyncQueue *async_queue, void *item);
void cel_asyncqueue_push_unlocked(CelAsyncQueue *async_queue, void *item);

int cel_asyncqueue_get_size(CelAsyncQueue *async_queue);
static __inline 
int cel_asyncqueue_get_size_unlocked(CelAsyncQueue *async_queue)
{
    return (async_queue->kclass->get_size(&(async_queue->queue)) 
        - async_queue->waiting_threads);
}
static __inline CelMutex *cel_asyncqueue_get_mutex(CelAsyncQueue *async_queue)
{
    return &(async_queue->mutex);
}

void *cel_asyncqueue_pop(CelAsyncQueue *async_queue);
void *cel_asyncqueue_pop_unlocked(CelAsyncQueue *async_queue);
void *cel_asyncqueue_try_pop(CelAsyncQueue *async_queue);
void *cel_asyncqueue_try_pop_unlocked(CelAsyncQueue *async_queue);
void *cel_asyncqueue_timed_pop(CelAsyncQueue *async_queue, int milliseconds);
void *cel_asyncqueue_timed_pop_unlocked(CelAsyncQueue *async_queue, 
                                        int milliseconds);

#ifdef __cplusplus
}
#endif

#endif