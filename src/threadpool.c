/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2016 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/threadpool.h"
#include "cel/allocator.h"
#include "cel/log.h"
#include "cel/atomic.h"

/* Debug defines */
#define Debug(args)    /*cel_log_debug args*/ 
#define Warning(args)  CEL_SETERRSTR(args)/*cel_log_warning args */
#define Err(args)    CEL_SETERRSTR(args)/*cel_log_err args*/ 

/* Global vars */
static CelAsyncQueue *unused_thread_queue = NULL;
static void *wakeup_thread_marker = (void *)&cel_threadpool_new;
static CelAtomic wakeup_thread_serial = 0;
static CelAtomic max_unused_threads = 0;
static CelAtomic num_unused_threads = 0;
static CelAtomic max_idle_time = 0;

/* Private functions */
static void cel_threadpool_start_thread(CelThreadPool *thread_pool);
static int cel_threadpool_thread_proxy(void *data);
static void cel_threadpool_wakeup_and_stop_all(CelThreadPool *thread_pool);
static void cel_threadpool_free_internal(CelThreadPool *thread_pool);
static CelThreadPool *cel_threadpool_wait_pool(void);
static void *cel_threadpool_wait_task(CelThreadPool *thread_pool);

int cel_threadpool_init(CelThreadPool *thread_pool, 
                        CelFunc work_func, void *user_data,
                        int max_threads, BOOL exclusive)
{
    if (work_func == NULL) 
        return -1;
    thread_pool->func = work_func;
    thread_pool->user_data = user_data;

    /* AsyncQueue initialize */
    cel_asyncqueue_init(&(thread_pool->task_queue), NULL);
    /* Thread cond initialize */
    cel_cond_init(&(thread_pool->cond), NULL);

    thread_pool->max_threads = (max_threads <= 0 ? -1 : max_threads);
    thread_pool->num_threads = 0;
    thread_pool->exclusive = exclusive;
    thread_pool->immediate = TRUE;
    thread_pool->waiting = TRUE;
    thread_pool->running = TRUE;
    /* Create unused thread queue */
    if (unused_thread_queue == NULL)
    {
        unused_thread_queue = cel_asyncqueue_new(NULL);
    }
    if (thread_pool->exclusive)
    {
        cel_asyncqueue_lock(&thread_pool->task_queue);
        while (thread_pool->num_threads < thread_pool->max_threads)
        {
            cel_threadpool_start_thread(thread_pool);
        }
        cel_asyncqueue_unlock(&thread_pool->task_queue);
    }
    Debug((_T("Threadpool %p init(max_threads:%d, exclusive:%d)."), 
        thread_pool, max_threads, exclusive));
    return 0;
}

void cel_threadpool_destroy(CelThreadPool *thread_pool, 
                            BOOL immediate, BOOL waiting)
{
    if (thread_pool == NULL) return;

    cel_asyncqueue_lock(&thread_pool->task_queue);
    thread_pool->running = FALSE;
    thread_pool->immediate = immediate;
    thread_pool->waiting = waiting;

    while (thread_pool->num_threads != 0)
    {
        /* 
         * If the pool is not running and no other thread is waiting for
         * this thread pool to finish and this is not the last thread of 
         * this pool and there are no tasks left in the queue, wakeup the
         * remaing threas.
         */
        if (cel_asyncqueue_get_size_unlocked(&thread_pool->task_queue) 
            == -thread_pool->num_threads)
        {
            cel_threadpool_wakeup_and_stop_all(thread_pool);
        }
        cel_cond_wait(&(thread_pool->cond), 
            cel_asyncqueue_get_mutex(&thread_pool->task_queue));
    }
    /* 
     * If the pool is not running and no other thread is waiting for this 
     * thread pool to finish and this is the last thread of this poll, 
     * free the pool 
     */
    cel_asyncqueue_unlock(&thread_pool->task_queue);
    cel_threadpool_free_internal(thread_pool);
    Debug((_T("Threadpool %p destroy"
        "(immediate:%d , waiting:%d , threads:%d)."), 
        thread_pool, immediate, waiting, thread_pool->num_threads));
}

CelThreadPool *cel_threadpool_new(CelFunc work_func, void *user_data,
                                  int max_threads, BOOL exclusive)
{
    CelThreadPool *thread_pool;

    /* Thread thread_pool memory alloc */
    if ((thread_pool = 
        (CelThreadPool *)cel_malloc(sizeof(CelThreadPool))) != NULL)
    {
        if (cel_threadpool_init(thread_pool, 
            work_func, user_data, max_threads, exclusive) == 0)
            return thread_pool;
        cel_free(thread_pool);
    }
    return NULL;
}

void cel_threadpool_free(CelThreadPool *thread_pool, 
                         BOOL immediate, BOOL waiting)
{
    cel_threadpool_destroy(thread_pool, immediate, waiting); 
    cel_free(thread_pool);
}

static void cel_threadpool_free_internal(CelThreadPool *thread_pool)
{
    if (thread_pool == NULL) return;

    cel_cond_destroy(&(thread_pool->cond));
    cel_asyncqueue_destroy(&thread_pool->task_queue);
}

static void cel_threadpool_wakeup_and_stop_all(CelThreadPool *thread_pool)
{
    int i;

    thread_pool->immediate = TRUE;
    for (i = 0; i < thread_pool->num_threads; i++)
    {
        cel_asyncqueue_push_unlocked(&thread_pool->task_queue, (void *)1);
    }
}

void cel_threadpool_push_task(CelThreadPool *thread_pool, void *data)
{
    if (thread_pool == NULL || !(thread_pool->running)) return;

    cel_asyncqueue_lock(&thread_pool->task_queue);
    if (cel_asyncqueue_get_size_unlocked(&thread_pool->task_queue) >= 0)
    {
        /* No thread is waiting in the queue */
        cel_threadpool_start_thread(thread_pool);
    }
    cel_asyncqueue_push_unlocked(&thread_pool->task_queue, data);
    cel_asyncqueue_unlock(&thread_pool->task_queue);
}

static void cel_threadpool_start_thread(CelThreadPool *thread_pool)
{
    BOOL success = FALSE;
    CelThread new_thread;

    if (thread_pool->num_threads >= thread_pool->max_threads 
        && thread_pool->max_threads != -1)
    {
        /* Enough threads are already running */
        return;
    }
    cel_asyncqueue_lock(unused_thread_queue);
    if (cel_asyncqueue_get_size_unlocked(unused_thread_queue) < 0)
    {
        cel_asyncqueue_push_unlocked(unused_thread_queue, thread_pool);
        success = TRUE;
    }
    cel_asyncqueue_unlock(unused_thread_queue);
    if (!success)
    {
        /* No thread was found, we have to start a new one */
        cel_thread_create(
            &new_thread, NULL, cel_threadpool_thread_proxy, thread_pool);
    }
    thread_pool->num_threads++;
}

static int cel_threadpool_thread_proxy(void *data)
{
    void *task_data;
    CelThreadPool *thread_pool = (CelThreadPool *)data;

    Debug((_T("New thread 0x%x started for pool %p."), 
        cel_thread_self(), thread_pool));
    cel_asyncqueue_lock(&thread_pool->task_queue);
    while (TRUE)
    {
        if ((task_data = cel_threadpool_wait_task(thread_pool)) != NULL)
        {
            /* An task date was received and the thread pool is active,
               so execute */
            if (thread_pool->running || !thread_pool->immediate)
            {
                cel_asyncqueue_unlock(&(thread_pool->task_queue));
                thread_pool->func(task_data, thread_pool->user_data);
                cel_asyncqueue_lock(&(thread_pool->task_queue));
            }
            continue;
        }
        thread_pool->num_threads--;
        if (!thread_pool->running)
        {
            /* Pool not runnning */
            if (thread_pool->num_threads == 0 
                || cel_asyncqueue_get_size_unlocked(
                &thread_pool->task_queue) == - thread_pool->num_threads)
            {
                cel_cond_broadcast(&(thread_pool->cond));
            }
        }

        cel_asyncqueue_unlock(&(thread_pool->task_queue));
        /* No task data was received, so this thread goes to the global pool */
        Debug((_T("Thread 0x%x leave pool %p for global pool."), 
            cel_thread_self(), thread_pool));
        if ((thread_pool = cel_threadpool_wait_pool()) == NULL)
        {
            Debug(("Thread 0x%x exit for global pool.", cel_thread_self()));
            break;
        }
        //printf("%p %p\r\n", thread_pool, &thread_pool->task_queue);
        cel_asyncqueue_lock(&(thread_pool->task_queue));
        Debug((_T("Thread 0x%x entering pool %p from global pool."), 
            cel_thread_self(), thread_pool));
    }
    cel_thread_exit(0);

    return 0;
}

static void *cel_threadpool_wait_task(CelThreadPool *thread_pool)
{
    void *task_data = NULL;
    static int waittime =  500 * 1000;

    if (thread_pool->running 
        || (!thread_pool->immediate 
        && cel_asyncqueue_get_size_unlocked(&thread_pool->task_queue) > 0))
    {
        /* This thread pool is still active. */
        if (thread_pool->num_threads > thread_pool->max_threads 
            && thread_pool->max_threads != -1)
        {
            return NULL;
        }
        else if (thread_pool->exclusive)
        {
            /* Exclusive threads stay attached to the pool. */
            task_data = 
                cel_asyncqueue_pop_unlocked(&(thread_pool->task_queue));
            Debug((_T("Thread 0x%x in exclusive pool %p waits for task")
                _T("(%d running, %d unprocessed)."), 
                cel_thread_self(), thread_pool, thread_pool->num_threads, 
                cel_asyncqueue_get_size_unlocked(&thread_pool->task_queue)));
            //printf("%p\r\n", task_data);
        }
        else
        {
            /* 
             * A thread will wait for new tasks for at most 500 milliseconds 
             * before going to the global pool.
             */
            Debug((_T("Thread 0x%x in pool %p waits for up to ")
                _T("a 1/2 second for task. %d running, %d unprocessed)."), 
                cel_thread_self(), thread_pool, thread_pool->num_threads, 
                cel_asyncqueue_get_size_unlocked(&thread_pool->task_queue)));
            //cel_set_timespec(waittime, 0, (1000 * 1000 * 500));
            task_data = cel_asyncqueue_timed_pop_unlocked(
                &(thread_pool->task_queue), waittime);
            //printf("**%p\r\n", task_data);
        }
    }
    return task_data;
}

static CelThreadPool *cel_threadpool_wait_pool(void)
{
    CelThreadPool *thread_pool;
    int waittime;
    BOOL have_relayed_thread_marker = FALSE;

    long local_max_unused_threads = cel_atomic_get(&max_unused_threads);
    long local_max_idle_time = cel_atomic_get(&max_idle_time);
    long local_wakeup_thread_serial;
    long last_wakeup_thread_serial = cel_atomic_get(&wakeup_thread_serial);

    cel_atomic_inc(&num_unused_threads);
    do
    {
        if (local_max_unused_threads != -1
            && cel_atomic_get(&num_unused_threads) > local_max_unused_threads)
        {
            /* If this is a superfluous thread, stop it. */
            thread_pool = NULL;
            break;
        }
        else if (local_max_idle_time > 0)
        {
            /* If a maximal idle time is given, wait for the given time. */
            Debug((_T("Thread 0x%x waiting in global pool "
                "for %d milliseconds ."), 
                cel_thread_self(), local_max_idle_time));
            waittime = local_max_idle_time * 1000;
            thread_pool = (CelThreadPool *)cel_asyncqueue_timed_pop(
                unused_thread_queue, waittime);
        }
        else
        {
            /* If no maximal idle time is given, wait indefinitely. */
            Debug((_T("Thread 0x%x waiting in global pool."), 
                cel_thread_self()));
            thread_pool = 
                (CelThreadPool *)cel_asyncqueue_pop(unused_thread_queue);
        }
        if (thread_pool == wakeup_thread_marker)
        {
            local_wakeup_thread_serial = cel_atomic_get(&wakeup_thread_serial);
            if (last_wakeup_thread_serial  == local_wakeup_thread_serial)
            {
                if (!have_relayed_thread_marker)
                {
                    cel_asyncqueue_push(
                        unused_thread_queue, wakeup_thread_marker);
                    have_relayed_thread_marker = TRUE;
                    usleep(100 *1000);
                }
            }
            else
            {
                local_max_unused_threads = cel_atomic_get(&max_unused_threads);
                local_max_idle_time = cel_atomic_get(&max_idle_time);
                last_wakeup_thread_serial = 
                    cel_atomic_get(&wakeup_thread_serial);

                have_relayed_thread_marker = FALSE;
            }
        }
    }while (thread_pool == wakeup_thread_marker);
    cel_atomic_dec(&num_unused_threads);

    return thread_pool;
}

int cel_threadpool_get_num_threads(CelThreadPool *thread_pool)
{
    int num_threads;

    if (thread_pool == NULL) return -1;

    cel_asyncqueue_lock(&thread_pool->task_queue);
    num_threads = thread_pool->num_threads;
    cel_asyncqueue_unlock(&thread_pool->task_queue);

    return num_threads;
}

void cel_threadpool_set_max_unused_threads(int max_threads)
{
    int i;

    max_threads = (max_threads < -1 ? -1 : max_threads);
    cel_atomic_set(&max_unused_threads, (long)max_threads);
    if ((i = num_unused_threads) > 0)
        cel_atomic_inc(&wakeup_thread_serial);

    cel_asyncqueue_lock(unused_thread_queue);
    while (i-- > 0)
    {
        cel_asyncqueue_push_unlocked(
            unused_thread_queue, wakeup_thread_marker);
    }
    cel_asyncqueue_unlock(unused_thread_queue);
}

int cel_threadpool_get_max_unused_threads(void)
{
    return cel_atomic_get(&max_unused_threads);
}

int cel_threadpool_get_num_unused_threads(void)
{
    return cel_atomic_get(&num_unused_threads);
}

void cel_threadpool_stop_unused_threads(void)
{
    int i, oldval = cel_threadpool_get_max_unused_threads();

    cel_threadpool_set_max_unused_threads(0);
    if ((i = num_unused_threads) > 0)
        cel_atomic_inc(&wakeup_thread_serial);

    cel_asyncqueue_lock(unused_thread_queue);
    while (i-- > 0)
    {
        cel_asyncqueue_push_unlocked(
            unused_thread_queue, wakeup_thread_marker);
    }
    cel_asyncqueue_unlock(unused_thread_queue);

    cel_threadpool_set_max_unused_threads(oldval);
}

void cel_threadpool_set_max_idle_time(int interval)
{
    int i;

    cel_atomic_set(&max_idle_time, (long)interval);
    if ((i = num_unused_threads) > 0)
        cel_atomic_inc(&wakeup_thread_serial);

    cel_asyncqueue_lock(unused_thread_queue);
    while (i-- > 0)
    {
        cel_asyncqueue_push_unlocked(
            unused_thread_queue, wakeup_thread_marker);
    }
    cel_asyncqueue_unlock(unused_thread_queue);
}

int cel_threadpool_get_max_idle_time(void)
{
    return cel_atomic_get(&max_idle_time);
}
