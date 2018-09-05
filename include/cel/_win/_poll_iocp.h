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
#ifndef __CEL_POLL_IOCP_H__
#define __CEL_POLL_IOCP_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelPoll
{
    HANDLE CompletionPort;
}CelPoll;

typedef struct _CelChannel
{
    HANDLE handle;
    CelPoll *poll;
}CelChannel;

typedef struct _CelAsyncBuf
{
    ULONG len;          /**< WSABUF */
    __field_bcount(len)CHAR FAR *buf;
}CelAsyncBuf;

typedef struct _CelAsyncResult
{
    void *key;
    int error;
    long ret;
}CelAsyncResult;

#define CEL_EVENT_CHANNEL      0x10
#define CEL_EVENT_CHANNELIN    0x11
#define CEL_EVENT_CHANNELOUT   0x12

typedef struct _CelOverLapped
{
    BYTE evt_type;
    OVERLAPPED _ol;
    int (* handle_func) (void *ol);
    CelAsyncResult result;
    void (* async_callback) (void *ol);
}CelOverLapped;

int cel_poll_init(CelPoll *poll, int max_threads, int max_fileds);
void cel_poll_destroy(CelPoll *poll);

/* int cel_poll_add(CelPoll *poll, CelChannel *channel, void *key) */
#define cel_poll_add(poll, channel, key) \
    (CreateIoCompletionPort((channel)->handle, \
    (poll)->CompletionPort, (ULONG_PTR)key, 0) == NULL ? -1 : 0)
#define cel_poll_remove(poll, handle)
/* int cel_poll_post(CelPoll *poll, CelChannel *channel, CelOverLapped *ol); */
#define cel_poll_post(poll, handle, ol) (ol)->handle_func((ol))
static __inline int cel_poll_queued(CelPoll *poll, CelOverLapped *ol)
{
    return PostQueuedCompletionStatus(
        poll->CompletionPort, 0, 0, &(ol->_ol)) ? 0 : -1;
}

/* If return -1 and ol is null, eixt. */
int cel_poll_wait(CelPoll *poll, CelOverLapped **ol, int milliseconds);

#ifdef __cplusplus
}
#endif

#endif
