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
#include "cel/eventloop.h"
#include "cel/allocator.h"
#include "cel/error.h"
//#define _CEL_ASSERT
#include "cel/log.h"

static CelQueuedEvent s_wakeupevent = { CEL_EVENT_WAKEUP, NULL, NULL };
static CelQueuedEvent s_exitevent = { CEL_EVENT_EXIT, NULL, NULL };

int cel_eventloop_init(CelEventLoop *evt_loop, int max_threads, int max_fileds)
{
    if (cel_timerqueue_init(
        &(evt_loop->timer_queue), (CelFreeFunc)cel_timer_free) == 0)
    {
        if (cel_poll_init(&(evt_loop->poll), max_threads, max_fileds) == 0)
        {
            evt_loop->is_running = TRUE;
            evt_loop->max_threads = max_threads;
            evt_loop->wait_threads = 0;
            evt_loop->timer_wakeup = 0;
            evt_loop->wakeup_cnt = 0;
            return 0;
        }
        cel_timerqueue_destroy(&(evt_loop->timer_queue));
    }
    return -1;
}

void cel_eventloop_destroy(CelEventLoop *evt_loop)
{
    cel_poll_destroy(&(evt_loop->poll));
    cel_timerqueue_destroy(&(evt_loop->timer_queue));
}

CelEventLoop *cel_eventloop_new(int max_threads, int max_fileds)
{
    CelEventLoop *evt_loop;

    if ((evt_loop = (CelEventLoop *)cel_malloc(sizeof(CelEventLoop))) != NULL)
    {
        if (cel_eventloop_init(evt_loop, max_threads, max_fileds) == 0)
            return evt_loop;
        cel_free(evt_loop);
    }
    return NULL;
}

void cel_eventloop_free(CelEventLoop *evt_loop)
{
    cel_eventloop_destroy(evt_loop); 
    cel_free(evt_loop);
}

void cel_eventloop_wakeup(CelEventLoop *evt_loop)
{
    cel_poll_queued(&(evt_loop->poll), (CelOverLapped *)(&s_wakeupevent));
}

void cel_eventloop_exit(CelEventLoop *evt_loop)
{
    evt_loop->is_running = FALSE;
    cel_poll_queued(&(evt_loop->poll), (CelOverLapped *)(&s_exitevent));
}

int cel_eventloop_queued_event(CelEventLoop *evt_loop, 
                               CelEventRoutine routine, void *evt_data)
{
    CelQueuedEvent *queued_evt;

    if ((queued_evt = 
        (CelQueuedEvent *)cel_malloc(sizeof(CelQueuedEvent))) == NULL)
        return -1;
    queued_evt->evt_type = CEL_EVENT_QUEUED;
    queued_evt->routine = routine;
    queued_evt->evt_data = evt_data;
    cel_poll_queued(&(evt_loop->poll), (CelOverLapped *)queued_evt);

    return 0;
}

CelTimerId cel_eventloop_schedule_timer(CelEventLoop *evt_loop, 
                                        long milliseconds, int repeat, 
                                        CelTimerCallbackFunc call_back, 
                                        void *user_data)
{
    CelTimer *timer;
    
    if ((timer = 
        cel_timer_new(milliseconds, repeat, call_back, user_data)) == NULL)
        return 0;
    cel_timer_start(timer, NULL);
    if (cel_timerqueue_push(&(evt_loop->timer_queue), timer))
    {
        /* Earliest changed */
        //cel_atomic_store(&(evt_loop->timer_wakeup), 1);
        evt_loop->timer_wakeup++;
        cel_eventloop_wakeup(evt_loop);
    }
    return timer->timer_id;
}

static int cel_eventloop_handle(CelEventLoop *evt_loop, CelEventCtlBlock *ecb)
{
    //_tprintf(_T("event %d, ecb %p, pid %d\r\n"),
    //    ecb->evt_type, ecb, (int)cel_thread_getid());
    switch (ecb->evt_type)
    {
    case CEL_EVENT_CHANNELIN:
    case CEL_EVENT_CHANNELOUT:
        if (ecb->ol.async_callback == NULL)
        {
            CEL_SETERR((CEL_ERR_LIB, _T("Event loop channel event error")));
            return -1;
        }
        ecb->ol.async_callback(&(ecb->ol));
        break;
    case CEL_EVENT_TIMER:
        /* Callback */
        if (ecb->timer.call_back != NULL)
            ecb->timer.call_back(ecb->timer.user_data);
        if (ecb->timer.repeat == 1 && ecb->timer.elapsed == 0)
        {
            cel_timer_start(&(ecb->timer), NULL);
            /* Earliest changed */
            if (cel_timerqueue_push(&(evt_loop->timer_queue), &(ecb->timer)))
            {
                evt_loop->timer_wakeup++;
                if (evt_loop->wait_threads > 0)
                    cel_eventloop_wakeup(evt_loop);
            }
            //_tprintf(_T("timer restart\r\n"));
            return 0;
        }
        /* This timer has stoped */
        cel_timer_free(&(ecb->timer));
        break;
    case CEL_EVENT_QUEUED:
        if (ecb->queued.routine != NULL)
            ecb->queued.routine(ecb->queued.evt_data);
        cel_free(ecb);
        break;
    case CEL_EVENT_WAKEUP:
        break;
    case CEL_EVENT_EXIT:
        evt_loop->is_running = FALSE;
        cel_poll_queued(&(evt_loop->poll), (CelOverLapped *)(&s_exitevent));
        break;
    default:
		CEL_SETERR((CEL_ERR_LIB, _T("Event %p type %d undefined."), ecb, ecb->evt_type));
        CEL_ASSERT(TRUE);
        break;
    }
    return 0;
}

int cel_eventloop_do_work(CelEventLoop *evt_loop)
{
    CelAtomic timer_wakeup;
    long timeout;
    CelOverLapped *ol = NULL;
    CelTimer *expireds;
   
    /*CEL_DEBUG((_T("wakeup_cnt = %d, %d, pid %d"), 
        evt_loop->wakeup_cnt, evt_loop->timer_wakeup, (int)cel_thread_getid()));*/
    if (evt_loop->wakeup_cnt != evt_loop->timer_wakeup)
    {
        evt_loop->wakeup_cnt = evt_loop->timer_wakeup;
        timer_wakeup = evt_loop->timer_wakeup;
        if ((timeout = 
            cel_timerqueue_pop_timeout(&(evt_loop->timer_queue), NULL)) != 0)
        {
           /*_tprintf(_T("wait_timeout start %ld, pid %d, ticks %ld\r\n"), 
                timeout, (int)cel_thread_getid(), cel_getticks());*/
            if (cel_poll_wait(&(evt_loop->poll), &ol, timeout) == -1)
            {
                cel_eventloop_exit(evt_loop);
                return 0;
            }
            /*
            _tprintf(_T("wait_timeout end %ld, pid %d, ol %p, ticks %ld\r\n"), 
                timeout, (int)cel_thread_getid(), ol, cel_getticks());
              */
            if (ol != NULL)
            {
                if (timer_wakeup == evt_loop->timer_wakeup)
                {
                    cel_atomic_increment(&(evt_loop->timer_wakeup), 1);
                    if (evt_loop->wait_threads > 0)
                        cel_eventloop_wakeup(evt_loop);
                }
                cel_eventloop_handle(evt_loop, (CelEventCtlBlock *)ol);
                return 0;
            }
        }
        if (cel_timerqueue_pop_expired(
            &(evt_loop->timer_queue), &expireds, 1, NULL) > 0)
        {
            if (timer_wakeup == evt_loop->timer_wakeup)
            {
                cel_atomic_increment(&(evt_loop->timer_wakeup), 1);
                if (evt_loop->wait_threads > 0)
                    cel_eventloop_wakeup(evt_loop);
            }
            cel_eventloop_handle(evt_loop, (CelEventCtlBlock *)expireds);
        }
        if (timer_wakeup == evt_loop->timer_wakeup)
            cel_atomic_increment(&(evt_loop->timer_wakeup), 1);
    }
    else
    {
        //_tprintf(_T("wait_timeout -1, pid %d\r\n"), (int)cel_thread_getid());
        cel_atomic_increment(&(evt_loop->wait_threads), 1);
        if (cel_poll_wait(&(evt_loop->poll), &ol, -1) == -1)
        {
            cel_eventloop_exit(evt_loop);
            return 0;
        }
        cel_atomic_increment(&(evt_loop->wait_threads), -1);
        //_tprintf(_T("wait_timeout -1 end, pid %d\r\n"), 
        //    (int)cel_thread_getid());
        if (ol != NULL)
            cel_eventloop_handle(evt_loop, (CelEventCtlBlock *)ol);
    }

    return 0;
}

int cel_eventloop_do_work2(CelEventLoop *evt_loop, CelCoroutineScheduler *schd)
{
    //int wakeup_cnt, n_expireds;
    //long timeout;
    CelTimer *expireds[2];
    CelOverLapped *ol = NULL;
    CelCoroutineEntity *ceo;

    cel_timerqueue_pop_expired(&(evt_loop->timer_queue), expireds, 2, NULL);
    while (cel_poll_wait(&(evt_loop->poll), &ol, 0))
        ;
    while ((ceo = cel_coroutinescheduler_readies_pop(schd)) != NULL)
        cel_coroutineentity_resume(ceo);

    return 0;
}
