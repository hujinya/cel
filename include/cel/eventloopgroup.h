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
#ifndef __CEL_EVENTLOOPGROUP_H__
#define __CEL_EVENTLOOPGROUP_H__

#include "cel/eventloop.h"

typedef struct _CelEventLoopThreadId
{
    int i;
}CelEventLoopThreadId;

typedef struct _CelEventLoopThread
{
    int i;
    CelEventLoop *evt_loop;
    CelThread thread;
}CelEventLoopThread;

typedef struct _CelEventLoopGroup
{
    BOOL is_shared;
    CelEventLoop *evt_loop;
    int n_threads;
    CelEventLoopThread *evt_loop_threads;
}CelEventLoopGroup;

#ifdef __cplusplus
extern "C" {
#endif

CelEventLoopThreadId *_cel_eventloopthread_id();

int cel_eventloopgroup_init(CelEventLoopGroup *group, 
                            int n_threads, BOOL is_shared);
void cel_eventloopgroup_destroy(CelEventLoopGroup *group);

CelEventLoopGroup *cel_eventloopgroup_new(int n_threads, BOOL is_shared);
void cel_eventloopgroup_free(CelEventLoopGroup *group);

static __inline void cel_eventloopgroup_exit(CelEventLoopGroup *group)
{
    int i;

    for (i = 0; i < group->n_threads; i++)
        cel_eventloop_exit(group->evt_loop_threads[i].evt_loop);
}

static  __inline 
int cel_eventloopgroup_get_threads_num(CelEventLoopGroup *group)
{
    return group->n_threads;
}

static __inline int cel_eventloopgroup_get_id()
{
    return _cel_eventloopthread_id()->i;
}

static __inline 
int cel_eventloopgroup_queued_event(CelEventLoopGroup *group,
                                    CelEventRoutine routine, void *evt_data)
{
    return cel_eventloop_queued_event(
        group->evt_loop_threads[_cel_eventloopthread_id()->i].evt_loop,
        routine, evt_data);
}

static __inline 
CelTimerId cel_eventloopgroup_schedule_timer(CelEventLoopGroup *group,
                                             long milliseconds, int repeat, 
                                             CelTimerCallbackFunc call_back, 
                                             void *user_data)
{
    return cel_eventloop_schedule_timer(
        group->evt_loop_threads[_cel_eventloopthread_id()->i].evt_loop,
        milliseconds, repeat, call_back, user_data);
}

static __inline 
int cel_eventloopgroup_add_channel(CelEventLoopGroup *group,  int id,
                                   CelChannel *channel, void *key)
{
    if (id < 0 || id >= group->n_threads)
        id = ((int)channel->handle) % group->n_threads;
    return cel_eventloop_add_channel(group->evt_loop_threads[id].evt_loop,
        channel, key);
}

#endif
