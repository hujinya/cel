/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_NET_WEBROUTER_H__
#define __CEL_NET_WEBROUTER_H__

#include "cel/net/httpclient.h"

typedef int (* CelWebHandleFunc)(CelHttpClient *client,
                                 CelHttpRequest *req, CelHttpResponse *rsp);

typedef struct _CelWebRoute
{
    char *path;
    CelWebHandleFunc handle_func[CEL_HTTPM_CONUT];
}CelWebRoute;

typedef struct _CelWebRouteContext
{
    int routes;
}CelWebRouteContext;

#ifdef __cplusplus
extern "C" {
#endif

int cel_webroutercontext_init(CelWebRouteContext *web_rt_ctx);
void cel_webroutercontext_destroy(CelWebRouteContext *web_rt_ctx);

CelWebRouteContext *cel_webroutercontext_new();
void cel_webroutercontext_free(CelWebRouteContext *web_rt_ctx);

int cel_webroutercontext_add(CelWebRouteContext *web_rt_ctx, 
                             CelHttpMethod method,
                             const char *path, CelWebHandleFunc handle_func);
int cel_webroutercontext_(CelWebRouteContext *web_rt_ctx, 
                          const char *path);

#ifdef __cplusplus
}
#endif

#endif
