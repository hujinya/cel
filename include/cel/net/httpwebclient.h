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
#ifndef __CEL_NET_WEBCLIENT_H__
#define __CEL_NET_WEBCLIENT_H__

#include "cel/net/httpclient.h"
#include "cel/net/httpwebroute.h"
#include "cel/net/httpwebsession.h"

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
