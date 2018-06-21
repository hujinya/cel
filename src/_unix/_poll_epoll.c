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
#include "cel/poll.h"
#ifdef _CEL_UNIX
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

/* Debug defines */
#define Debug(args)   /*cel_log_debug args */
#define Warning(args) CEL_SETERRSTR(args) /*cel_log_warning args */
#define Err(args)   CEL_SETERRSTR(args) /*cel_log_err args*/

int cel_eventchannel_init(CelChannel *evt_ch)
{
    evt_ch->handle = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    return 0;
}
void cel_eventchannel_destroy(CelChannel *evt_ch)
{
    close(evt_ch->handle);
}

static int _cel_eventchannel_read_async(CelEventReadAsyncArgs *args)
{
    //puts("event channel read");
    if (eventfd_read(args->evt_ch.handle, &(args->value)) == -1)
    {
        if ((args->ol.error = errno) == EAGAIN 
            || args->ol.error == EWOULDBLOCK)
            CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_WANTIN);
        else
            CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_ERROR);
        return -1;
    }
    CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_OK);
    args->ol.result = sizeof(eventfd_t);
    return 0;
}

int cel_eventchannel_read_async(CelEventReadAsyncArgs *args)
{
    CelChannel *channel = &(args->evt_ch);

    memset(&(args->ol), 0, sizeof(args->ol));
    args->ol.evt_type = CEL_EVENT_CHANNELIN;
    args->ol.handle_func = (int ( *)(void *))_cel_eventchannel_read_async;
    return cel_poll_post(channel->poll, channel->handle, (CelOverLapped *)args);
}

static void cel_eventchannel_do_read(CelEventReadAsyncArgs *args)
{
    //_tprintf(_T("event channel read %lld\r\n"), args->value);
    cel_eventchannel_read_async(args);
}

int cel_poll_init(CelPoll *poll, int max_threads, int max_fileds)
{
    if (max_fileds < 1024)
        max_fileds = 1024;
    if ((poll->epoll_datas = (struct _CelPollData **)
        cel_calloc(max_fileds, sizeof(CelPollData *))) != NULL)
    {
        if ((poll->epoll_fd = epoll_create(max_fileds)) > 0)
        {
            if (cel_asyncqueue_init(&(poll->async_queue), NULL) != -1)
            {
                poll->max_threads = max_threads;
                poll->max_fileds = max_fileds;
                poll->is_waited = FALSE;
                cel_mutex_init(&(poll->event_locker), 0);
                cel_mutex_init(&(poll->wait_locker), NULL);
                cel_eventchannel_init(&(poll->wakeup_ch));
                cel_poll_add(poll, &(poll->wakeup_ch), NULL);
                poll->wakeup_args.async_callback = 
                    (void ( *)(void *))cel_eventchannel_do_read;
                memcpy(&(poll->wakeup_args.evt_ch), 
                    &(poll->wakeup_ch), sizeof(CelChannel));
                cel_eventchannel_read_async(&(poll->wakeup_args));
                return 0;
            }
            close(poll->epoll_fd);
        }
        cel_free(poll->epoll_datas);
    }
    return -1;
}

void cel_poll_destroy(CelPoll *poll)
{
    close(poll->epoll_fd);
    cel_eventchannel_destroy(&(poll->wakeup_ch));
    while ((--poll->max_fileds) >= 0)
    {
        if (poll->epoll_datas[poll->max_fileds] != NULL)
        {
            cel_free(poll->epoll_datas[poll->max_fileds]);
            poll->epoll_datas[poll->max_fileds] = NULL;
        }
    }
    cel_asyncqueue_destroy(&poll->async_queue);
    cel_mutex_destroy(&(poll->event_locker));
    cel_mutex_destroy(&(poll->wait_locker));
    cel_free(poll->epoll_datas);
    cel_free(poll->events);
}

int cel_poll_add(CelPoll *poll, CelChannel *channel, void *key)
{
    int fileds = channel->handle;
    CelPollData *poll_data;
    struct epoll_event ctl_event;

    if (fileds <= 0 || fileds >= poll->max_fileds)
    {
        Err((_T("Poll add %d failed."), fileds));
        return -1;
    }
    if (poll->epoll_datas[fileds] == NULL
        && (poll->epoll_datas[fileds] = 
        (CelPollData *)cel_malloc(sizeof(CelPollData))) == NULL)
    {
        Err((_T("%s"), cel_geterrstr(cel_sys_geterrno())));
        return -1;
    }
    poll_data = poll->epoll_datas[fileds];
    poll_data->key = key;
    poll_data->events = 0;
    poll_data->out_ol = poll_data->in_ol = NULL;
    ctl_event.events = POLL_EVENTS;
    ctl_event.data.fd = fileds;
    if (epoll_ctl(poll->epoll_fd, EPOLL_CTL_ADD, fileds, &ctl_event) != 0)
    {
        Err((_T("epoll_ctl():%s"), cel_geterrstr(cel_sys_geterrno())));
        return -1;
    }
    //_tprintf(_T("ctl_event fd %d\r\n"), ctl_event.data.fd);
    channel->poll = poll;

    return 0;
}

static int cel_poll_handle(CelPoll *poll, 
                           CelOverLapped *ol, CelPollData *poll_data)
{
    /* Clear epoll events, call aio process function */
    switch (ol->evt_type)
    {
    case CEL_EVENT_CHANNELIN:
        POLLIN_CLR(poll_data->events);
        break;
    case CEL_EVENT_CHANNELOUT:
        POLLOUT_CLR(poll_data->events);
        break;
    default:
        Err((_T("Undfined event type \"%d\"."), ol->evt_type));
        return -1;
    }
    do
    {
        if (ol->handle_func(ol) == -1)
        {
            if (CEL_POLLSTATE_CHECK(ol, CEL_POLLSTATE_WANTIN))
            {
                cel_poll_lock(poll, &(poll->event_locker));
                if (POLLIN_CHK(poll_data->events) 
                    || POLLERROR_CHK(poll_data->events))
                {
                    ol->_ol.events = poll_data->events;
                    POLLIN_CLR(poll_data->events);
                    cel_poll_unlock(poll, &(poll->event_locker));
                    continue;
                }
                ol->evt_type = CEL_EVENT_CHANNELIN;
                poll_data->in_ol = ol;
                cel_poll_unlock(poll, &(poll->event_locker));
                return 0;
            }
            else if (CEL_POLLSTATE_CHECK(ol, CEL_POLLSTATE_WANTOUT))
            {
                cel_poll_lock(poll, &(poll->event_locker));
                if (POLLOUT_CHK(poll_data->events) 
                    || POLLERROR_CHK(poll_data->events))
                {
                    ol->_ol.events = poll_data->events;
                    POLLOUT_CLR(poll_data->events);
                    cel_poll_unlock(poll, &(poll->event_locker));
                    continue;
                }
                ol->evt_type = CEL_EVENT_CHANNELOUT;
                poll_data->out_ol = ol;
                cel_poll_unlock(poll, &(poll->event_locker));
                return 0;
            }
        }
        break;
    }while (1);

    return 1;
}

int cel_poll_post(CelPoll *poll, int fileds, CelOverLapped *ol)
{
    int ret;
    CelPollData *poll_data;

    if (fileds <=0 || fileds >= poll->max_fileds
        || (poll_data = poll->epoll_datas[fileds]) == NULL)
    {
        Err((_T("Poll post %d failed."), fileds));
        return -1;
    }
    ol->key = poll_data->key;
    ret = cel_poll_handle(poll, ol, poll_data);
    if (ret == -1)
        return -1;
    else if (ret == 1)
    {
        //_tprintf(_T("fileds %d, error %d\r\n"), fileds, ol->error);
        cel_poll_push(poll, ol);
        if (poll->max_threads <= 1 && poll->is_waited)
        {
            cel_eventchannel_write(&(poll->wakeup_ch), 1);
            //_putts(_T("eventchannel_write"));
        }
    }
    return 0;
}

static int cel_poll_do(CelPoll *poll, int milliseconds)
{
    int i, n_events;
    struct epoll_event *event;
    CelPollData *poll_data;
    CelOverLapped *ol;

    if ((n_events = epoll_wait(
        poll->epoll_fd, poll->events, EVENTS_MAX, milliseconds)) < 0 
        && errno != EINTR)
    {
        Err((_T("epoll_wait():%s"), cel_geterrstr(cel_sys_geterrno())));
        return -1;
    }
    /* _tprintf(_T("epoll wait %d, n_events %d, pid %d\r\n"), 
        milliseconds, n_events, cel_thread_getid()); */
    for (i = 0; i < n_events; i++)
    {
        event = &(poll->events[i]);
        if ((poll_data = poll->epoll_datas[event->data.fd]) == NULL)
        {
            Err((_T("Epoll data is null, file descriptor \"%d\"."), 
                event->data.fd));
            return -1;
        }
        //_tprintf("events 0x%x, fd %d\r\n", event->events, event->data.fd);
        if (POLLIN_CHK(event->events) || POLLERROR_CHK(event->events))
        {
            if ((ol = poll_data->in_ol) == NULL)
            {
                cel_poll_lock(poll, &(poll->event_locker));
                if ((ol = poll_data->in_ol) == NULL)
                    poll_data->events = event->events;
                cel_poll_unlock(poll, &(poll->event_locker));
            }
            if (ol != NULL)
            {
                //_tprintf("Read %d %p \r\n", event->data.fd, ol);
                ol->_ol.fileds = event->data.fd;
                ol->_ol.events = event->events;
                poll_data->in_ol = NULL;
                cel_poll_push(poll, ol);
            }
        }
        if (POLLOUT_CHK(event->events) || POLLERROR_CHK(event->events))
        {
            if ((ol = poll_data->out_ol) == NULL)
            {
                cel_poll_lock(poll, &(poll->event_locker));
                if ((ol = poll_data->out_ol) == NULL)
                    poll_data->events = event->events;
                cel_poll_unlock(poll, &(poll->event_locker));
            }
            if (ol != NULL)
            {
                //_tprintf("Write %d %p \r\n", event->data.fd, ol);
                ol->_ol.fileds = event->data.fd;
                ol->_ol.events = event->events;
                poll_data->out_ol = NULL;
                cel_poll_push(poll, ol);
            }
        }
    }
    return 0;
}

int cel_poll_wait(CelPoll *poll, CelOverLapped **ol, int milliseconds)
{
    int ret;
    CelPollData *poll_data;

    /*_tprintf("Asyncqueue size %d %d\r\n", 
        cel_asyncqueue_get_size(&(poll->async_queue)), cel_thread_getid()); */
    if (!(poll->is_waited)
        && cel_poll_trylock(poll, &(poll->wait_locker)) == 0)
    {
        poll->is_waited = TRUE;
        if ((*ol = (CelOverLapped *)cel_poll_try_pop(poll)) == NULL)
        {
            //_tprintf("poll wait %d\r\n", (int)cel_thread_getid());
            if ((ret = cel_poll_do(poll, milliseconds)) == -1)
            {
                cel_poll_unlock(poll, &(poll->wait_locker));
                return -1;
            }
            *ol = (CelOverLapped *)cel_poll_try_pop(poll);
            /*_tprintf(_T("poll return ol %p, milliseconds= %d, pid %d\r\n"), 
                *ol,  milliseconds, cel_thread_getid());*/
        }
        poll->is_waited = FALSE;
        cel_poll_unlock(poll, &(poll->wait_locker));
    }
    else
    {
        //puts("cel_asyncqueue_timed_pop");
        *ol = (CelOverLapped *)cel_asyncqueue_timed_pop(
            &(poll->async_queue), milliseconds);
        /*_tprintf(_T("poll return ol %p, milliseconds= %d, evt_type %d, pid %d\r\n"), 
                *ol,  milliseconds, cel_thread_getid());*/
    }
    if ((*ol) != NULL && CEL_CHECKFLAG((*ol)->evt_type, CEL_EVENT_CHANNEL))
    {
        //_tprintf(_T("fd %d state %d\r\n"), (*ol)->_ol.fileds, (*ol)->_ol.state);
        if (!CEL_POLLSTATE_ISCOMPLETED(*ol))
        {
            if ((poll_data = poll->epoll_datas[(*ol)->_ol.fileds]) == NULL)
            {
                Err((_T("Epoll data is null, file descriptor \"%d\"."), 
                    (*ol)->_ol.fileds));
                return -1;
            }
            if ((ret = cel_poll_handle(poll, *ol, poll_data)) == -1)
            {
                return -1;
            }
            else if (ret == 0)
            {
                //_tprintf(_T("ol %p, type %d = NULL\r\n"), *ol, (*ol)->evt_type);
                *ol = NULL;
            }
        }
    }
    return 0;
}

#endif
