/**
 * CEL(C Extension Library)
 * Copyright (C)2008 Hu Jinya(hu_jinya@163.com) 
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
#ifndef __CEL_NET_HTTPROUTE_H__
#define __CEL_NET_HTTPROUTE_H__

#include "cel/convert.h"
#include "cel/pattrie.h"
#include "cel/net/httprequest.h"
#include "cel/net/httpfilter.h"

typedef int (* CelHttpRouteHandleFunc)(CelHttpContext *http_ctx);

typedef struct _CelHttpRouteData CelHttpRouteData;
typedef int (* CelHttpRouteEachFunc)(CelHttpRouteData *rt_data, void *user_data);

typedef enum _CelHttpRouteState
{
	CEL_HTTPROUTEST_RECEVIE_REQUEST,
    CEL_HTTPROUTEST_BEFORE_ROUTER,
    CEL_HTTPROUTEST_BEFORE_EXEC,
    CEL_HTTPROUTEST_AFTER_EXEC,
    CEL_HTTPROUTEST_FINISH_ROUTER,
	CEL_HTTPROUTEST_END
}CelHttpRouteState;

struct _CelHttpRouteData
{
	CelHttpMethod method;
	char *path;
	CelHttpRouteHandleFunc handle_func;
	CelHttpRouteEachFunc _each_func;
	void *_user_data;
};

typedef struct _CelHttpRoute
{
	CelVStringA prefix;
    CelPatTrie root_tries[CEL_HTTPM_CONUT];
    //BOOL policy_on;
    //CelList policies;
    BOOL filter_on, logger_on;
    CelList filters[CEL_HTTPROUTEST_END];
	CelHttpFilterHandlerFunc logger_filter;
}CelHttpRoute;

#ifdef __cplusplus
extern "C" {
#endif

int cel_httproute_init(CelHttpRoute *route, const char *prefix);
void cel_httproute_destroy(CelHttpRoute *route);

CelHttpRoute *cel_httproute_new(const char *prefix);
void cel_httproute_free(CelHttpRoute *route);

int cel_httproute_add(CelHttpRoute *route, 
                      CelHttpMethod method, const char *path, 
                      CelHttpRouteHandleFunc handle_func);
#define cel_httproute_get_add(route, path, handle_func) \
    cel_httproute_add(route, CEL_HTTPM_GET, path, handle_func)
#define cel_httproute_post_add(route, path, handle_func) \
    cel_httproute_add(route, CEL_HTTPM_POST, path, handle_func)
#define cel_httproute_delete_add(route, path, handle_func) \
    cel_httproute_add(route, CEL_HTTPM_DELETE, path, handle_func)
#define cel_httproute_put_add(route, path, handle_func) \
    cel_httproute_add(route, CEL_HTTPM_PUT, path, handle_func)
#define cel_httproute_patch_add(route, path, handle_func) \
    cel_httproute_add(route, CEL_HTTPM_PATCH, path, handle_func)

//#define cel_httproute_static_add(route, path);

int cel_httproute_remove(CelHttpRoute *route, CelHttpMethod method, const char *path);
#define cel_httproute_get_remove(route, path) \
    cel_httproute_remove(route, CEL_HTTPM_GET, path)
#define cel_httproute_post_remove(route, path) \
    cel_httproute_remove(route, CEL_HTTPM_POST, path)
#define cel_httproute_delete_remove(route, path) \
    cel_httproute_remove(route, CEL_HTTPM_DELETE, path)
#define cel_httproute_put_remove(route, path) \
    cel_httproute_remove(route, CEL_HTTPM_PUT, path)
#define cel_httproute_patch_remove(route, path) \
    cel_httproute_remove(route, CEL_HTTPM_PATCH, path)

int cel_httproute_foreach(CelHttpRoute *route,
						  CelHttpRouteEachFunc each_func, void *user_data);

//#define cel_httproute_static_remove(route, path);

int cel_httproute_logger_filter_set(CelHttpRoute *route,
									CelHttpFilterHandlerFunc handle);
int cel_httproute_filter_insert(CelHttpRoute *route,
								CelHttpRouteState state, CelHttpFilter *filter);

int cel_httproute_routing(CelHttpRoute *route, CelHttpContext *http_ctx);

#ifdef __cplusplus
}
#endif

#endif
