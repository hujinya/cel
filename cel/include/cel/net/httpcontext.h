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
#ifndef __CEL_NET_HTTPCONTEXT_H__
#define __CEL_NET_HTTPCONTEXT_H__

#include "cel/net/httprequest.h"
#include "cel/net/httpresponse.h"
#include "cel/net/httpclient.h"
#include "cel/pattrie.h"

typedef enum _CelHttpRouteState
{
    CEL_HTTPROUTEST_BEFORE_ROUTER,
    CEL_HTTPROUTEST_BEFORE_EXEC,
    CEL_HTTPROUTEST_AFTER_EXEC,
    CEL_HTTPROUTEST_FINISH_ROUTER,
    CEL_HTTPROUTEST_END
}CelHttpRouteState;

typedef struct _CelHttpContext
{
	CelHttpClient *client;
    CelHttpRequest *req;
	CelHttpResponse *rsp;
	char user[CEL_UNLEN];
	void *user_data;
	CelHttpRouteState state;
	CelPatTrieParams params;
	CelListItem *current_filter;
}CelHttpContext;

#ifdef __cplusplus
extern "C" {
#endif

int cel_httpcontext_init(CelHttpContext *http_ctx, CelHttpClient *client,
						 CelHttpRequest *req, CelHttpResponse *rsp);
void cel_httpcontext_destroy(CelHttpContext *http_ctx);

CelHttpContext *cel_httpcontext_new(CelHttpClient *client,
									CelHttpRequest *req, CelHttpResponse *rsp);
void cel_httpcontext_free(CelHttpContext *http_ctx);

void cel_httpcontext_clear(CelHttpContext *http_ctx);

static __inline 
void cel_httpcontext_set_state(CelHttpContext *http_ctx, CelHttpRouteState state)
{
	http_ctx->state = state;
}

static __inline 
char *cel_httpcontext_get_param(CelHttpContext *http_ctx,
								const char *key, char *value, size_t *size)
{
    char *_value;
    if ((_value = (char*)cel_rbtree_lookup(&(http_ctx->params), key)) != NULL)
        return cel_strncpy(value, size, _value, strlen(_value));
    return NULL;
}
#define cel_httpcontext_get_param_string(http_ctx, key, str, size) \
    cel_keystr((CelKeyGetFunc)cel_httpcontext_get_param, http_ctx, key, str, size)
#define cel_httpcontext_get_param_bool(http_ctx, key, b)\
    cel_keybool((CelKeyGetFunc)cel_httpcontext_get_param, http_ctx, key, b)
#define cel_httpcontext_get_param_int(http_ctx, key, i)\
    cel_keyint((CelKeyGetFunc)cel_httpcontext_get_param, http_ctx, key, i)
#define cel_httpcontext_get_param_long(http_ctx, key, l)\
    cel_keylong((CelKeyGetFunc)cel_httpcontext_get_param, http_ctx, key, l)
#define cel_httpcontext_get_param_double(http_ctx, key, d)\
    cel_keydouble((CelKeyGetFunc)cel_httpcontext_get_param, http_ctx, key, d)

int cel_httpcontext_routing_again(CelHttpContext *http_ctx);

int cel_httpcontext_response_write(CelHttpContext *http_ctx, CelHttpStatusCode status,
								   int err_no, const char *msg);
int cel_httpcontext_response_tryfiles(CelHttpContext *http_ctx, 
									  const char *file_path, const char *uri_file_path,
									  long long first, long long last,
									  CelDateTime *if_modified_since,
									  char *if_none_match);
int cel_httpcontext_response_sendfile(CelHttpContext *http_ctx, const char *file_path, 
									  long long first, long long last,
									  CelDateTime *if_modified_since,
									  char *if_none_match);
int cel_httpcontext_response_redirect(CelHttpContext *http_ctx, const char *url);


#ifdef __cplusplus
}
#endif

#endif

