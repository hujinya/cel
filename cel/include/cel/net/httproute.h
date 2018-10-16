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

#include "cel/convert.h"
#include "cel/pattrie.h"
#include "cel/net/httpclient.h"

typedef CelPatTrieParams CelHttpRouteData;

typedef int (* CelHttpHandleFunc)(CelHttpClient *client,
                                  CelHttpRequest *req, CelHttpResponse *rsp, 
                                  CelHttpRouteData *rt_dt);

typedef struct _CelHttpRoute
{
    CelPatTrie root_tries[CEL_HTTPM_CONUT];
}CelHttpRoute;

#ifdef __cplusplus
extern "C" {
#endif

#define cel_httproutedata_init(rt_data) \
    cel_rbtree_init(rt_data, (CelCompareFunc)strcmp, NULL, cel_free)
#define cel_httproutedata_destroy(rt_data) cel_rbtree_destroy(rt_data)

#define cel_httproutedata_clear(rt_data) cel_rbtree_clear(rt_data)

static __inline 
char *cel_httproutedata_get(CelHttpRouteData *rt_data,
                            const char *key, char *value, size_t *size)
{
    char *_value;
    if ((_value = (char*)cel_rbtree_lookup(rt_data, key)) != NULL)
        return cel_strncpy(value, size, _value, strlen(_value));
    return NULL;
}
#define cel_httproutedata_get_string(rt_data, key, str, size) \
    cel_keystr((CelKeyGetFunc)cel_httproutedata_get, rt_data, key, str, size)
#define cel_httproutedata_get_bool(rt_data, key, b)\
    cel_keybool((CelKeyGetFunc)cel_httproutedata_get, rt_data, key, b)
#define cel_httproutedata_get_int(rt_data, key, i)\
    cel_keyint((CelKeyGetFunc)cel_httproutedata_get, rt_data, key, i)
#define cel_httproutedata_get_long(rt_data, key, l)\
    cel_keylong((CelKeyGetFunc)cel_httproutedata_get, rt_data, key, l)
#define cel_httproutedata_get_double(rt_data, key, d)\
    cel_keydouble((CelKeyGetFunc)cel_httproutedata_get, rt_data, key, d)

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
