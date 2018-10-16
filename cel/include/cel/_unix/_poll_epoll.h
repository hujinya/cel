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
#ifndef __CEL_POLL_EPOLL_H__
#define __CEL_POLL_EPOLL_H__

#include "cel/types.h"
#include "cel/thread.h"
#include "cel/asyncqueue.h"
#include <sys/epoll.h>
#include <sys/eventfd.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Since Linux 2.6.17 */
#ifndef EPOLLRDHUP
#define EPOLLRDHUP     0x2000
#endif
#define POLL_EVENTS   (EPOLLIN | EPOLLOUT| EPOLLHUP | EPOLLRDHUP| EPOLLET)

#define POLLIN_CHK(events)    (((events) & EPOLLIN) == EPOLLIN)
#define POLLOUT_CHK(events)   \
    (((events) & EPOLLOUT) == EPOLLOUT && ((events) & EPOLLHUP) != EPOLLHUP)
#define POLLERROR_CHK(events) (((events) & EPOLLERR) == EPOLLERR)
#define POLLCLOSE_CHK(events) \
    (((events) & (EPOLLIN | EPOLLRDHUP)) == (EPOLLIN | EPOLLRDHUP))

#define POLLIN_CLR(events) (events) &= (~EPOLLIN)
#define POLLOUT_CLR(events) (events) &= (~EPOLLOUT)

typedef struct _CelChannel
{
    int handle;
    struct _CelPoll *poll;
}CelChannel;

typedef struct _CelAsyncBuf
{
    void *buf;          /**< struct iovec */
    size_t len;
}CelAsyncBuf;

#define CEL_EVENT_CHANNEL      0x10
#define CEL_EVENT_CHANNELIN    0x11
#define CEL_EVENT_CHANNELOUT   0x12

typedef struct _CelAsyncResult
{
    void *key;
    int error;
    long ret;
}CelAsyncResult;

typedef struct _CelOverLapped
{
    BYTE evt_type;
    struct {
        int fileds, events, state;
    }_ol;
    int (* handle_func) (void *ol);
    CelAsyncResult result;
    void (* async_callback) (void *ol);
}CelOverLapped;

typedef struct _CelEventReadAsyncArgs
{
    CelOverLapped ol;
    CelChannel *evt_ch;
    eventfd_t value;
}CelEventReadAsyncArgs;

int cel_eventchannel_init(CelChannel *evt_ch);
void cel_eventchannel_destroy(CelChannel *evt_ch);
/* int cel_eventchannel_write(CelChannel *evt_ch, eventfd_t value); */
#define cel_eventchannel_write(evt_ch, value) \
    eventfd_write((evt_ch)->handle, value)
int cel_eventchannel_read_async(CelEventReadAsyncArgs *args);

#define CEL_POLLSTATE_WANTIN     0x01
#define CEL_POLLSTATE_WANTOUT    0x02
#define CEL_POLLSTATE_COMPLETED  0x10
#define CEL_POLLSTATE_ERROR      0x11
#define CEL_POLLSTATE_OK         0x12
#define CEL_POLLSTATE_SET(ol, _state) (ol)->_ol.state = _state
#define CEL_POLLSTATE_CHECK(ol, _state) (ol)->_ol.state == _state
#define CEL_POLLSTATE_ISCOMPLETED(ol) \
    (((ol)->_ol.state & CEL_POLLSTATE_COMPLETED) == CEL_POLLSTATE_COMPLETED)

typedef struct _CelPollData
{
    int events;
    void *key;
    struct _CelOverLapped *in_ol, *out_ol;
}CelPollData;

#define EVENTS_MAX 256

typedef struct _CelPoll
{
    int epoll_fd;
    int max_threads, max_fileds;
    BOOL is_waited;
    CelMutex wait_locker;
    CelMutex event_locker;
    CelPollData *epoll_datas;
    struct epoll_event *events;
    CelAsyncQueue async_queue;

    CelChannel wakeup_ch;
    CelEventReadAsyncArgs wakeup_args;
}CelPoll;

int cel_poll_init(CelPoll *poll, int max_threads, int max_fileds);
void cel_poll_destroy(CelPoll *poll);

static __inline void cel_poll_lock(CelPoll *poll, CelMutex *locker)
{
    if (poll->max_threads > 1)
        cel_mutex_lock(locker);
}

static __inline int cel_poll_trylock(CelPoll *poll, CelMutex *locker)
{
    if (poll->max_threads > 1)
        return cel_mutex_trylock(locker);
    return 0;
}

static __inline void cel_poll_unlock(CelPoll *poll, CelMutex *locker)
{
    if (poll->max_threads > 1)
        cel_mutex_unlock(locker);
}

static __inline void cel_poll_push(CelPoll *poll, void *task)
{
    /*if (poll->max_threads > 1)*/
        cel_asyncqueue_push(&(poll->async_queue), task);
    /*else
        cel_asyncqueue_push_unlocked(&(poll->async_queue), task);*/
}

static __inline void *cel_poll_try_pop(CelPoll *poll)
{
    /*if (poll->max_threads > 1)*/
        return cel_asyncqueue_try_pop(&(poll->async_queue));
    /*else
        return cel_asyncqueue_try_pop_unlocked(&(poll->async_queue));*/
}

int cel_poll_add(CelPoll *poll, CelChannel *channel, void *key);
int cel_poll_post(CelPoll *poll, int fileds, CelOverLapped *ol);
static __inline void cel_poll_queued(CelPoll *poll, CelOverLapped *ol)
{
    cel_poll_push(poll, ol);
    cel_eventchannel_write(&(poll->wakeup_ch), 1);
    //_putts(_T("eventchannel_write"));
}
int cel_poll_wait(CelPoll *poll, CelOverLapped **ol, int milliseconds);

#ifdef __cplusplus
}
#endif

#endif
