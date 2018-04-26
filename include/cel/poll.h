/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_POLL_H__
#define __CEL_POLL_H__

#include "cel/config.h"
#include "cel/_asyncio.h"
#if defined(_CEL_UNIX)
#include "cel/_unix/_poll_epoll.h"
#elif defined(_CEL_WIN)
#include "cel/_win/_poll_iocp.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelAsyncResult
{
    int error;
    long ret;
}CelAsyncResult;

typedef struct _CelOverLappedEntry
{
    CelOverLapped ol;
    void (* async_callback) (struct _CelOverLappedEntry *ole);
}CelOverLappedEntry;

typedef void (* CelAsyncCallbackFunc)(void *user_data, CelAsyncResult *ret);

CelPoll *cel_poll_new(int max_threads, int max_fileds);
void cel_poll_free(CelPoll *poll);

#ifdef __cplusplus
}
#endif

#endif
