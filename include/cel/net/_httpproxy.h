/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_NET_HTTPPROXY_H__
#define __CEL_NET_HTTPPROXY_H__

#include "cel/net/httplistener.h"

typedef struct _CelHttpProxyListener
{
    CelHttpListener listener;
}CelHttpProxyListener;

typedef struct _CelHttpProxySession
{
    CelHttpClient frontend;
    CelHttpClient backend;
    CelHttpRequest req;
    CelHttpResponse rsp;
}CelHttpProxySession;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif
