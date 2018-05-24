/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2016 Hu Jinya(hu_jinya@163.com) 
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
#ifndef __CEL_ASYNCIO_H__
#define __CEL_ASYNCIO_H__

#include "cel/types.h"

typedef enum _CelAsyncIoOp
{
    CEL_ASYNCIO_OP_ACCEPT,
    CEL_ASYNCIO_OP_READ,
    CEL_ASYNCIO_OP_RECVFROM,
    CEL_ASYNCIO_OP_CONNECT,
    CEL_ASYNCIO_OP_WRITE,
    CEL_ASYNCIO_OP_SENDTO,
    CEL_ASYNCIO_OP_SENDFILE
}CelAsyncIoOp;

//typedef struct _CelOverLapped
//{
//    ;
//}CelOverLapped;

typedef struct _CelAsyncIoResult
{
    int error;
    void *key;
    long result;
}CelAsyncIoResult;

typedef struct _CelAsyncIo CelAsyncIo;

typedef int (* CelAsyncIoDoAcceptFunc) (void *async_args);
typedef int (* CelAsyncIoDoConnectFunc) (void *async_args);
typedef int (* CelAsyncIoDoReadFunc) (void *async_args);
typedef int (* CelAsyncIoDoWriteFunc) (void *async_args);
typedef int (* CelAsyncIoDoSendToFunc) (void *async_args);
typedef int (* CelAsyncIoDoRecvFromFunc) (void *async_args);
typedef int (* CelAsyncIoDoSendFileFunc) (void *async_args);

typedef union _CelAsyncIoClass
{
    struct 
    {
        CelAsyncIoDoAcceptFunc do_accept;
        CelAsyncIoDoReadFunc do_read;
        CelAsyncIoDoRecvFromFunc do_recvfrom;
    };
    struct {
        CelAsyncIoDoConnectFunc do_connect;
        CelAsyncIoDoWriteFunc do_write;
        CelAsyncIoDoSendToFunc do_sendto;
        CelAsyncIoDoSendFileFunc do_sendfile;
    };
}CelAsyncIoClass;

struct _CelAsyncIo
{
    HANDLE handle;
    void *poll;
    int events;
    void *key;
    CelAsyncIoClass *kclass;
};

#ifdef __cplusplus
extern "C" {
#endif

int cel_asyncio_init(CelAsyncIo *async_io);
void cel_asyncio_destroy(CelAsyncIo *async_io);

#ifdef __cplusplus
}
#endif

#endif
