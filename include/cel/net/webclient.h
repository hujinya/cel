/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_NET_WEBCLIENT_H__
#define __CEL_NET_WEBCLIENT_H__

#include "cel/net/httpclient.h"
#include "cel/net/webroute.h"
#include "cel/net/websession.h"

typedef struct _CelWebContext
{
    CelSslContext *ssl_ctx;
    CelWebRouteContext *route_ctx;
    CelWebSessionContext *seesion_ctx;
    CelRefCounted ref_counted;
}CelWebContext;

typedef struct _CelWebClient
{
    CelHttpClient http_client;
    CelWebContext *ctx;
    CelWebSeesion *session;
    CelHttpRequest *req;
    CelHttpResponse *rsp;
}CelWebClient;

#ifdef __cplusplus
extern "C" {
#endif

int cel_webclient_init(CelWebClient *web_clt, CelWebContext *ctx);
void cel_webclient_destroy(CelWebClient *web_clt);

CelWebClient *cel_webclient_new(CelWebContext *ctx);
void cel_webclient_free(CelWebClient *web_clt);

int cel_webclient_handle_request(CelWebClient *web_clt);

#ifdef __cplusplus
}
#endif

#endif
