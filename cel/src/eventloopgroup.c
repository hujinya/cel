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
#include "cel/eventloopgroup.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/multithread.h"
#include "cel/log.h"

CelEventLoopThreadId *_cel_eventloopthread_id()
{
    CelEventLoopThreadId *ptr;

    if ((ptr = (CelEventLoopThreadId *)
        cel_multithread_get_keyvalue(CEL_MT_KEY_EVENTLOOPTHREADID)) == NULL)
    {
		if ((ptr = (CelEventLoopThreadId *)
			_cel_sys_malloc(sizeof(CelEventLoopThreadId))) != NULL)
		{
			ptr->i = 0;
			if (cel_multithread_set_keyvalue(
				CEL_MT_KEY_EVENTLOOPTHREADID, ptr) != -1
				&& cel_multithread_set_keydestructor(
				CEL_MT_KEY_EVENTLOOPTHREADID, _cel_sys_free) != -1)
				return ptr;
			_cel_sys_free(ptr);
		}
        return NULL;
    }
    return ptr;
}

static int cel_eventloopgroup_start(CelEventLoopThread *evt_loop_thread)
{
    CelEventLoopThreadId *thread_id = _cel_eventloopthread_id();

    thread_id->i = evt_loop_thread->i;
    //printf("thread id %d\r\n", thread_id->i);
    cel_eventloop_run(evt_loop_thread->evt_loop);
    CEL_DEBUG((_T("Event loop thread %d exit.(%s)"),
		(int)cel_thread_getid(), cel_geterrstr()));
    cel_thread_exit(0);

    return 0;
}

int cel_eventloopgroup_init(CelEventLoopGroup *group, 
                            int max_fileds,
                            int n_threads, BOOL is_shared)
{
    int i, n_cpus;
    CelCpuMask mask;
    CelEventLoopThread *evt_loop_thread;

    n_cpus = (int)cel_getnprocs();
    if (n_threads <= 0)
        n_threads = n_cpus;
    if (n_threads > CEL_THDNUM)
        n_threads = n_threads / 2;
    else if (n_threads < 4)
        n_threads = 4;

    if (is_shared
        && (group->evt_loop = cel_eventloop_new(n_threads, max_fileds)) == NULL)
        return -1;
    group->is_shared = is_shared;
    
    if ((group->evt_loop_threads = (CelEventLoopThread *)
        cel_calloc(1, sizeof(CelEventLoopThread) * n_threads)) != NULL)
    {
        group->n_threads = n_threads;
        for (i  = 0; i < group->n_threads; i++)
        {
            evt_loop_thread = &(group->evt_loop_threads[i]);
            evt_loop_thread->i = i;
            if (is_shared)
                evt_loop_thread->evt_loop = group->evt_loop;
            else
                evt_loop_thread->evt_loop = cel_eventloop_new(1, max_fileds);
            cel_thread_create(&(evt_loop_thread->thread), NULL, 
                cel_eventloopgroup_start, evt_loop_thread);
            cel_setcpumask(&mask, i % n_cpus);
            cel_thread_setaffinity(&(evt_loop_thread->thread), &mask);
        }
    }

    return 0;
}

void cel_eventloopgroup_destroy(CelEventLoopGroup *group)
{
    int i;
    void *ret_val;
    CelEventLoopThread *evt_loop_threads;

    for (i = 0; i < group->n_threads; i++)
    {
        evt_loop_threads = &(group->evt_loop_threads[i]);
        cel_thread_join(evt_loop_threads->thread, &ret_val);
        if (!(group->is_shared))
            cel_eventloop_free(evt_loop_threads->evt_loop);
    }
    cel_free(group->evt_loop_threads);
    if (group->is_shared)
        cel_eventloop_free(group->evt_loop);
}

CelEventLoopGroup *cel_eventloopgroup_new(int max_fileds,
                                          int n_threads, BOOL is_shared)
{
    CelEventLoopGroup *group;

    if ((group = (CelEventLoopGroup *)
        cel_malloc(sizeof(CelEventLoopGroup))) != NULL)
    {
        if (cel_eventloopgroup_init(group, 
            max_fileds, n_threads, is_shared) == 0)
            return group;
        cel_free(group);
    }
    return NULL;
}

void cel_eventloopgroup_free(CelEventLoopGroup *group)
{
    cel_eventloopgroup_destroy(group); 
    cel_free(group);
}
