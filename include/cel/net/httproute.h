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
#ifndef __CEL_NET_HTTPROUTE_H__
#define __CEL_NET_HTTPROUTE_H__

#include "cel/net/httpclient.h"
#include "cel/rbtree.h"

typedef struct _CelHttpRouteData CelHttpRouteData;

typedef int (* CelHttpHandleFunc)(CelHttpClient *client,
                                  CelHttpRequest *req, CelHttpResponse *rsp, 
                                  CelHttpRouteData *rt_dt);

typedef enum _CelHttpRouteEntityType
{
    CEL_HTTPROUTEENTITY_ROOT,
    CEL_HTTPROUTEENTITY_KEY,
    CEL_HTTPROUTEENTITY_PARAM,
    CEL_HTTPROUTEENTITY_REGEXP
}CelHttpRouteEntityType;

typedef struct _CelHttpRouteEntity
{
    char *key;
    CelHttpRouteEntityType type;
    CelRbTree *next_key_entity;
    struct _CelHttpRouteEntity *next_param_entity;
    CelHttpHandleFunc handle_func;
}CelHttpRouteEntity;

struct _CelHttpRouteData
{
    int n_params;
    struct {
        const char *key;
        const char *value;
        size_t value_size;
    }params[64];
};

typedef struct _CelHttpRoute
{
    CelHttpRouteEntity root_entities[CEL_HTTPM_CONUT];
}CelHttpRoute;

#ifdef __cplusplus
extern "C" {
#endif

int cel_httproutedata_init(CelHttpRouteData *rt_data);
void cel_httproutedata_destroy(CelHttpRouteData *rt_data);
char *cel_httproutedata_get(CelHttpRouteData *rt_data,
                            const char *key, char *value, size_t *size);

int cel_httproute_init(CelHttpRoute *route);
void cel_httproute_destroy(CelHttpRoute *route);

CelHttpRoute *cel_httproute_new(void);
void cel_httproute_free(CelHttpRoute *route);

int cel_httproute_add(CelHttpRoute *route, 
                      CelHttpMethod method, const char *path, 
                      CelHttpHandleFunc handle_func);
#define cel_httproute_get_add(route, path, handle_func) \
    cel_httproute_add(route, CEL_HTTPM_GET, path, handle_func)
#define cel_httproute_post_add(route, path, handle_func) \
    cel_httproute_add(route, CEL_HTTPM_POST, path, handle_func)
#define cel_httproute_delete_add(route, path, handle_func) \
    cel_httproute_add(route, CEL_HTTPM_DELETE, path, handle_func)
#define cel_httproute_put_add(route, path, handle_func) \
    cel_httproute_add(route, CEL_HTTPM_PUT, path, handle_func)

int cel_httproute_remove(CelHttpRoute *route, 
                         CelHttpMethod method, const char *path);
#define cel_httproute_get_remove(route, path) \
    cel_httproute_remove(route, CEL_HTTPM_GET, path)
#define cel_httproute_post_remove(route, path) \
    cel_httproute_remove(route, CEL_HTTPM_POST, path)
#define cel_httproute_delete_remove(route, path) \
    cel_httproute_remove(route, CEL_HTTPM_DELETE, path)
#define cel_httproute_put_remove(route, path) \
    cel_httproute_remove(route, CEL_HTTPM_PUT, path)

CelHttpHandleFunc cel_httproute_handler(CelHttpRoute *route,
                                        CelHttpMethod method, 
                                        const char *path, 
                                        CelHttpRouteData *rt_data);



#ifdef __cplusplus
}
#endif

#endif
