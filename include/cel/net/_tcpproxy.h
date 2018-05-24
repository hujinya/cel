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
#ifndef __CEL_NET_TCPPROXY_H__
#define __CEL_NET_TCPPROXY_H__

#include "cel/net/tcplistener.h"

typedef struct _CelTcpProxyListener
{
    CelTcpListener listener;
}CelTcpProxyListener;

typedef struct _CelTcpProxySession
{
    CelTcpClient frontend;
    CelTcpClient backend;
    CelStream rs;
    CelStream ss;
}CelTcpProxySession;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif
