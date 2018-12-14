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
#ifndef __CEL_NET_HTTPFILTER_H__
#define __CEL_NET_HTTPFILTER_H__

#include "cel/list.h"
#include "cel/net/httprequest.h"
#include "cel/net/httpresponse.h"
#include "cel/net/httpclient.h"

typedef struct _CelHttpFilter CelHttpFilter;

/* return 0 = continue;-1 = error;1 = break */
typedef int (* CelHttpFilterHanlderFunc)(
    CelHttpFilter *filter, CelHttpClient *client, 
    CelHttpRequest *req, CelHttpResponse *rsp);

struct _CelHttpFilter
{
    CelListItem _item;
    CelHttpFilterHanlderFunc handler;
};

#ifdef __cplusplus
extern "C" {
#endif

int cel_httpfilter_init(CelHttpFilter *filter);
void cel_httpfilter_destroy(CelHttpFilter *filter);

/** 
 * Cross-origin resource sharing
 * beego/plugins/cors/cors.go 
 */
typedef struct _CelHttpFilterAllowCors
{
    CelHttpFilter _filter;
    char *allow_origins;
    BOOL is_allow_credentials;
    char allow_methods;
    char allow_headers[CEL_HTTPHDR_COUNT/8 + 1];
    char expose_headers[CEL_HTTPHDR_COUNT/8 + 1];
    long max_age;
}CelHttpFilterAllowCors;

/* 
 * allow_origins : '*'
 * allow_methods : 'GET, HEAD, POST, OPTIONS, PUT, PATCH, DELETE'
 * expose_headers : NULL
 * allow_headers : '*', allow all headers
 * is_allow_credentials : False
 * max_age : None
 */
int cel_httpfilter_allowcors_init(CelHttpFilterAllowCors *cors,
                                  const char *allow_origins, 
                                  BOOL is_allow_credentials,
                                  const char *allow_methods, 
                                  const char *allow_headers, 
                                  const char *expose_headers,
                                  long max_age);
void cel_httpfilter_allowcors_destroy(CelHttpFilterAllowCors *cors);

#ifdef __cplusplus
}
#endif

#endif
