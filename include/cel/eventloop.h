/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_EVENTLOOP_H__
#define __CEL_EVENTLOOP_H__

#include "cel/types.h"
#include "cel/atomic.h"
#include "cel/timerqueue.h"
#include "cel/poll.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (* CelEventRoutine) (void *evt_data);

//#define CEL_EVENT_TIEMR       0x01
//#define CEL_EVNET_CHANNEL_IN  0x11
//#define CEL_EVENT_CHANNEL_OUT 0x12
#define CEL_EVENT_QUEUED   0x8D
#define CEL_EVENT_WAKEUP   0x8E
#define CEL_EVENT_EXIT     0x8F

typedef struct _CelQueuedEvent
{
    BYTE evt_type;
    CelEventRoutine routine;
    void *evt_data;
}CelQueuedEvent;

typedef union _CelEventCtlBlock
{
    BYTE evt_type;
    CelTimer timer;
    CelOverLappedEntry ole;
    CelQueuedEvent queued;
}CelEventCtlBlock;

typedef struct _CelEventLoop
{
    BOOL is_running;
    int max_threads;
    volatile int wait_threads, wakeup_cnt, timer_wakeup;
    CelTimerQueue timer_queue;
    CelPoll poll;
}CelEventLoop;

int cel_eventloop_init(CelEventLoop *evt_loop, 
                       int max_threads, int max_fileds);
void cel_eventloop_destroy(CelEventLoop *evt_loop);

CelEventLoop *cel_eventloop_new( int max_threads, int max_fileds);
void cel_eventloop_free(CelEventLoop *evt_loop);

#define cel_eventloop_is_running(evt_loop) (evt_loop)->is_running
int cel_eventloop_do(CelEventLoop *evt_loop);
static __inline void cel_eventloop_run(CelEventLoop *evt_loop)
{
    while (cel_eventloop_is_running(evt_loop) 
        && cel_eventloop_do(evt_loop) != -1);
    //_tprintf(_T("Thread %d exit.%s\r\n"), (int)cel_thread_getid());
}
void cel_eventloop_wakeup(CelEventLoop *evt_loop);
void cel_eventloop_exit(CelEventLoop *evt_loop);

int cel_eventloop_queued_event(CelEventLoop *evt_loop,
                               CelEventRoutine routine, void *evt_data);

CelTimerId cel_eventloop_schedule_timer(CelEventLoop *evt_loop, 
                                        long milliseconds, int repeat, 
                                        CelTimerCallbackFunc call_back, 
                                        void *user_data);
/*
 * int cel_eventloop_cancel_timer(CelEventLoop *evt_loop, CelTimerId timer_id)
 */
#define cel_eventloop_cancel_timer(evt_loop, timer_id) \
    cel_timerqueue_cancel(&((evt_loop)->timer_queue), timer_id)
/* 
 * int cel_eventloop_xx_channel(CelEventLoop *evt_loop, 
 *                              CelChannel *channel, void *key)
 */
#define cel_eventloop_add_channel(evt_loop, channel, key) \
    cel_poll_add(&((evt_loop)->poll), channel, key)
#define cel_eventloop_remove_channel(evt_loop, channel) \
    cel_poll_remove_channel(&((evt_loop)->poll), channel)

#ifdef __cplusplus
}
#endif

#endif
