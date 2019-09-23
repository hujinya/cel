/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2019 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/asyncqueue.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

static CelAsyncQueueClass async_queue_kclass = 
{
    (CelAsyncQueueDestroyFunc)cel_queue_destroy,
    (CelAsyncQueuePushFunc)cel_queue_push_back,
    (CelAsyncQueueGetSizeFunc)cel_queue_get_size,
    (CelAsyncQueueIsEmptyFunc)cel_queue_is_empty,
    (CelAsyncQueuePopFunc)cel_queue_pop_front,
};

static CelAsyncQueueClass async_list_kclass = 
{
    (CelAsyncQueueDestroyFunc)cel_list_destroy,
    (CelAsyncQueuePushFunc)cel_list_push_back,
    (CelAsyncQueueGetSizeFunc)cel_list_get_size,
    (CelAsyncQueueIsEmptyFunc)cel_list_is_empty,
    (CelAsyncQueuePopFunc)cel_list_pop_front,
};

static CelAsyncQueueClass async_minheap_kclass = 
{
    (CelAsyncQueueDestroyFunc)cel_minheap_destroy,
    (CelAsyncQueuePushFunc)cel_minheap_insert,
    (CelAsyncQueueGetSizeFunc)cel_minheap_get_size,
    (CelAsyncQueueIsEmptyFunc)cel_minheap_is_empty,
    (CelAsyncQueuePopFunc)cel_minheap_pop_min,
};

static void *cel_asyncqueue_pop_intern_unlocked(CelAsyncQueue *async_queue, 
                                                BOOL is_try, int milliseconds);

int cel_asyncqueue_init_full(CelAsyncQueue *async_queue,
                             CelAsyncQueueType type,
                             CelCompareFunc comparator, CelFreeFunc free_func)
{
    switch (type)
    {
    case CEL_AQ_QUEUE:
        if (cel_queue_init((CelQueue *)async_queue, free_func) == -1)
            return -1;
        async_queue->kclass = &async_queue_kclass;
        break;
    case CEL_AQ_LIST:
        if (cel_list_init((CelList *)async_queue, free_func) == -1)
            return -1;
        async_queue->kclass = &async_list_kclass;
        break;
    case CEL_AQ_MINHEAP:
        if (cel_minheap_init(
            (CelMinHeap *)async_queue, comparator, free_func) == -1)
            return -1;
        async_queue->kclass = &async_minheap_kclass;
        break;
    default:
        CEL_SETERR((CEL_ERR_LIB, _T("Asyncqueue type %d undefined."), type));
        return -1;
    }
    cel_mutex_init(&(async_queue->mutex), NULL);
    cel_cond_init(&(async_queue->cond), NULL);
    async_queue->waiting_threads = 0;
    CEL_DEBUG(("Asyncqueue(%p)init successed.", async_queue));

    return 0;
}

void cel_asyncqueue_destroy(CelAsyncQueue *async_queue)
{
    cel_mutex_destroy(&(async_queue->mutex));
    cel_cond_destroy(&(async_queue->cond));
    async_queue->kclass->destroy(async_queue);
}

CelAsyncQueue *cel_asyncqueue_new_full(CelAsyncQueueType type, 
                                       CelCompareFunc comparator, 
                                       CelFreeFunc free_func)
{
    CelAsyncQueue *queue;

    if ((queue = (CelAsyncQueue *)cel_malloc(sizeof(CelAsyncQueue))) != NULL)
    {
        if (cel_asyncqueue_init_full(queue, type, comparator, free_func) == 0)
            return queue;
        cel_free(queue);
    }
    return NULL;
}

void cel_asyncqueue_free(CelAsyncQueue *async_queue)
{
    cel_asyncqueue_destroy(async_queue); 
    cel_free(async_queue);
}

void cel_asyncqueue_push(CelAsyncQueue *async_queue, void *item)
{
    cel_mutex_lock(&(async_queue->mutex));
    cel_asyncqueue_push_unlocked(async_queue, item);
    cel_mutex_unlock(&(async_queue->mutex));
}

void cel_asyncqueue_push_unlocked(CelAsyncQueue *async_queue, void *item)
{
    async_queue->kclass->push(async_queue, item);
    CEL_DEBUG((_T("Asyncqueue %p push item %p."), async_queue, item));
    if (async_queue->waiting_threads > 0)
    {
        cel_cond_signal(&(async_queue->cond));
        //puts("cond siganl ");
    }
}

static void *cel_asyncqueue_pop_intern_unlocked(CelAsyncQueue *async_queue, 
                                                BOOL is_try, int milliseconds)
{
    if (async_queue->kclass->is_empty(async_queue))
    {
        if (is_try || milliseconds == 0)
        {
            /* No item is available, return NULL */
            return NULL;
        }
        if (milliseconds == -1)
        {
            async_queue->waiting_threads++;
            while (async_queue->kclass->is_empty(async_queue))
            {
                //_tprintf(_T("pid %d wait\r\n"), (int)cel_thread_getid());
                cel_cond_wait(&(async_queue->cond), &(async_queue->mutex));
                //_tprintf(_T("pid %d wakeup\r\n"), (int)cel_thread_getid());
            }
            async_queue->waiting_threads--;
        }
        else
        {
            async_queue->waiting_threads++;
            while (async_queue->kclass->is_empty(async_queue))
            {
                /* If return 0, asyncqueue is not null */
                //_tprintf("milliseconds = %d\r\n", milliseconds);
                if (cel_cond_timedwait(&(async_queue->cond),
                    &(async_queue->mutex), milliseconds) != 0)
                    break;
            }
            async_queue->waiting_threads--;
            if (async_queue->kclass->is_empty(async_queue))
            {
                return NULL;
            }
        }
    }

    return async_queue->kclass->pop(async_queue);
}

void *cel_asyncqueue_pop(CelAsyncQueue *async_queue)
{
    void *pop_item = NULL;

    cel_mutex_lock(&(async_queue->mutex));
    pop_item = cel_asyncqueue_pop_intern_unlocked(async_queue, FALSE, -1);
    cel_mutex_unlock(&(async_queue->mutex));

    return pop_item;
}

void *cel_asyncqueue_pop_unlocked(CelAsyncQueue *async_queue)
{
    return cel_asyncqueue_pop_intern_unlocked(async_queue, FALSE, -1);
}

void *cel_asyncqueue_try_pop(CelAsyncQueue *async_queue)
{
    void *pop_item = NULL;

    cel_mutex_lock(&(async_queue->mutex));
    pop_item = cel_asyncqueue_pop_intern_unlocked(async_queue, TRUE, -1);
    cel_mutex_unlock(&(async_queue->mutex));

    return pop_item;
}

void *cel_asyncqueue_try_pop_unlocked(CelAsyncQueue *async_queue)
{
    return cel_asyncqueue_pop_intern_unlocked(async_queue, TRUE, -1);
}

void *cel_asyncqueue_timed_pop(CelAsyncQueue *async_queue, int milliseconds)
{
    void *pop_item = NULL;

    cel_mutex_lock(&(async_queue->mutex));
    pop_item = cel_asyncqueue_pop_intern_unlocked(
        async_queue, FALSE, milliseconds);
    cel_mutex_unlock(&(async_queue->mutex));

    return pop_item;
}

void *cel_asyncqueue_timed_pop_unlocked(CelAsyncQueue *async_queue, 
                                        int milliseconds)
{
    return cel_asyncqueue_pop_intern_unlocked(
        async_queue, FALSE, milliseconds);
}

int cel_asyncqueue_get_size(CelAsyncQueue *async_queue)
{
    int size;

    cel_mutex_lock(&(async_queue->mutex));
    size = async_queue->kclass->get_size(async_queue) 
        - async_queue->waiting_threads;
    cel_mutex_unlock(&(async_queue->mutex));

    return size;
}
