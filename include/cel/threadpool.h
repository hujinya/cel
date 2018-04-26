/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_THREADPOOL_H__
#define __CEL_THREADPOOL_H__

#include "cel/types.h"
#include "cel/asyncqueue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelThreadPool
{
    BOOL exclusive, immediate, waiting, running;
    int max_threads;
    int num_threads;
    CelFunc func;
    void *user_data;
    CelCond cond;
    struct _CelAsyncQueue task_queue;
}CelThreadPool;

int cel_threadpool_init(CelThreadPool *thread_pool, CelFunc work_func,
                        void *user_data, int max_threads, BOOL exclusive);
void cel_threadpool_destroy(CelThreadPool *thread_pool, 
                            BOOL immediate, BOOL waiting);
/*
 * \param exclusive: should this thread pool be exclusive
 */
CelThreadPool *cel_threadpool_new(CelFunc work_func, void *user_data,
                                  int max_threads, BOOL exclusive);
/* 
 * void cel_threadpool_free(CelThreadPool *thread_pool, 
 *                          BOOL immediate, BOOL waiting)
 *
 * \param immediate: should pool shut down immediately
 * \param waiting: should the function wait for all tasks to be finished 
 */
void cel_threadpool_free(CelThreadPool *thread_pool,
                         BOOL immediate, BOOL waiting);

void cel_threadpool_push_task(CelThreadPool *thread_pool, void *data);

static __inline int cel_threadpool_get_unprocessed(CelThreadPool *thread_pool)
{
    return (thread_pool == NULL ? \
        -1 : cel_asyncqueue_get_size(&thread_pool->task_queue));
}
int cel_threadpool_get_num_threads(CelThreadPool *thread_pool);
static __inline int cel_threadpool_get_max_threads(CelThreadPool *thread_pool)
{
    return (thread_pool == NULL ? -1 : thread_pool->max_threads);
}

/*
 * These functions are global functions.
 */
void cel_threadpool_set_max_unused_threads(int max_unused_threads);
int  cel_threadpool_get_max_unused_threads();

void cel_threadpool_set_max_idle_time(int interval);
int  cel_threadpool_get_max_idle_time(void);

void cel_threadpool_stop_unused_threads(void);

#ifdef __cplusplus
}
#endif

#endif
