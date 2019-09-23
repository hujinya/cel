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
#ifndef __CEL_NET_HTTPFILTER_H__
#define __CEL_NET_HTTPFILTER_H__

#include "cel/list.h"
#include "cel/net/httpcontext.h"

typedef struct _CelHttpFilter CelHttpFilter;

/* return 0 = continue;-1 = error;1 = break; */
typedef int (* CelHttpFilterHandlerFunc)(CelHttpContext *http_ctx);

struct _CelHttpFilter
{
    CelListItem _item;
    CelHttpFilterHandlerFunc handler;
};

#ifdef __cplusplus
extern "C" {
#endif

int cel_httpfilter_init(CelHttpFilter *filter,
						CelHttpFilterHandlerFunc handler);
void cel_httpfilter_destroy(CelHttpFilter *filter);

/** 
 * Cross-origin resource sharing
 * beego/plugins/cors/cors.go 
 */
typedef struct _CelHttpFilterAllowCors
{
    CelHttpFilter _filter;
    CelVStringA allow_origins;
    BOOL is_allow_credentials;
    CelVStringA allow_methods;
	CelVStringA allow_headers;
	CelVStringA expose_headers;
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
int cel_httpfilter_allowcors_init(CelHttpFilterAllowCors *cors_filter,
                                  const char *allow_origins, 
                                  BOOL is_allow_credentials,
                                  const char *allow_methods, 
                                  const char *allow_headers, 
                                  const char *expose_headers,
                                  long max_age);
void cel_httpfilter_allowcors_destroy(CelHttpFilterAllowCors *cors_filter);

typedef struct _CelHttpFilterStatic
{
	CelHttpFilter _filter;
	char path[CEL_PATHLEN];
}CelHttpFilterStatic;

int cel_httpfilter_static_init(CelHttpFilterStatic *static_filter, 
							   const char *path);
void cel_httpfilter_static_destroy(CelHttpFilterStatic *static_filter);

typedef struct _CelHttpFilterLog
{
	CelHttpFilter _filter;
}CelHttpFilterLog;

int cel_httpfilter_log_init(CelHttpFilterLog *log_filter, 
							   const char *path);
void cel_httpfilter_log_destroy(CelHttpFilterLog *log_filter);

#ifdef __cplusplus
}
#endif

#endif
