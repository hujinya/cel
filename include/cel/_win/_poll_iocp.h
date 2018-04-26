/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_POLL_IOCP_H__
#define __CEL_POLL_IOCP_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelChannel
{
    HANDLE handle;
}CelChannel;

typedef struct _CelAsyncBuf
{
    ULONG len;          /**< WSABUF */
    __field_bcount(len)CHAR FAR *buf;
}CelAsyncBuf;

#define CEL_EVENT_CHANNEL      0x10
#define CEL_EVENT_CHANNELIN    0x11
#define CEL_EVENT_CHANNELOUT   0x12

typedef struct _CelOverLapped
{
    BYTE evt_type;
    OVERLAPPED _ol;
    int (* handle_func) (void *ol);
    void *key;
    long result;
    int error;
}CelOverLapped;

typedef struct _CelPoll
{
    HANDLE CompletionPort;
}CelPoll;

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
