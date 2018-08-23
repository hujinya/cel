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
#ifndef __CEL_POLL_H__
#define __CEL_POLL_H__

#include "cel/config.h"
#include "cel/coroutine.h"
#if defined(_CEL_UNIX)
#include "cel/_unix/_poll_epoll.h"
#elif defined(_CEL_WIN)
#include "cel/_win/_poll_iocp.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void (* CelAsyncCallbackFunc)(void *user_data, CelAsyncResult *ret);

CelPoll *cel_poll_new(int max_threads, int max_fileds);
void cel_poll_free(CelPoll *poll);

#ifdef __cplusplus
}
#endif

#endif
