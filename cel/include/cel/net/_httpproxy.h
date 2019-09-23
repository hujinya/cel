/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2019 Hu Jinya(hu_jinya@163.com) 
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
#ifndef __CEL_NET_HTTPPROXY_H__
#define __CEL_NET_HTTPPROXY_H__

#include "cel/net/httplistener.h"

typedef struct _CelHttpProxy
{
    CelHttpListener listener;
}CelHttpProxy;

typedef struct _CelHttpFrontend
{
    CelHttpClient client;
}CelHttpFrontend;

typedef struct _CelHttpBackend
{
    CelHttpClient client;
}CelHttpBackend;

typedef struct _HttpMsg
{
    CelStream s;
    CelHttpRequest req;
    CelHttpResponse rsp;
}HttpMsg;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif
