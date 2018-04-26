/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_NET_WEBLISTENER_H__
#define __CEL_NET_WEBLISTENER_H__

#include "cel/eventloop.h"
#include "cel/net/httplistener.h"
#include "cel/net/webclient.h"

typedef struct _CelWebListener
{
    CelHttpListener http_listener;
    CelWebContext *ctx;
    CelEventLoop *evt_loop;
}CelWebListener;

#ifdef __cplusplus
extern "C" {
#endif

int cel_weblistener_init(CelWebListener *listener, CelWebContext *ctx);
void cel_weblistener_destroy(CelWebListener *listener);

CelWebListener *cel_weblistener_new(CelWebContext *ctx);
void cel_weblistener_free(CelWebListener *listener);

#ifdef __cplusplus
}
#endif

#endif