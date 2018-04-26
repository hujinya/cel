/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
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
