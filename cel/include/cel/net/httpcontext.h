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

#include "cel/net/httpclient.h"
#include "cel/net/httpresponse.h"
#include "cel/net/httproute.h"

#define CEL_HTTPCONTEXT_CONTENT_TYPE         "application/json; charset=utf-8"
#define CEL_HTTPCONTEXT_CONTENT_TYPE_LEN     (sizeof(CEL_HTTPCONTEXT_CONTENT_TYPE) - 1)
#define CEL_HTTPCONTEXT_SUCCESSED_MSG        "{\"error\":0, \"message\":\"Successed\"}"
#define CEL_HTTPCONTEXT_SUCCESSED_MSG_LEN    (sizeof(CEL_HTTPCONTEXT_SUCCESSED_MSG) - 1)

typedef CelHttpContext *(* CelHttpContextNewFunc)(size_t size, int fd);
typedef void (* CelHttpContextFreeFunc)(CelHttpContext *http_ctx);

typedef struct _CelHttpServeContext
{
    char server[CEL_SNLEN];
    CelHttpRoute route;
    CelHttpContextNewFunc new_func;
    CelHttpContextFreeFunc free_func;
}CelHttpServeContext;

struct _CelHttpContext
{
	CelHttpClient http_client;
    CelHttpRequest req;
	CelHttpResponse rsp;
	// Context data
	CelHttpServeContext *serve_ctx;
	CelTime req_start_dt, rsp_finish_dt;
	CelHttpRouteState state;
	CelPatTrieParams params;
	CelListItem *current_filter;
	CelVString err_msg;
	// Token user
	char user[CEL_UNLEN];
	void *user_data;
};

#ifdef __cplusplus
extern "C" {
#endif

CelHttpContext *cel_httpcontext_new(CelHttpClient *client,
									CelHttpServeContext *serve_ctx);
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

static __inline long cel_httpcontext_request_time(CelHttpContext *http_ctx)
{
	return cel_time_diffmilliseconds(&(http_ctx->rsp_finish_dt),
		&(http_ctx->req_start_dt));
}

static __inline TCHAR *cel_httpcontxt_get_errstr(CelHttpContext *http_ctx)
{
	return cel_vstring_str(&(http_ctx->err_msg));
}

int cel_httpcontext_routing(CelHttpContext *http_ctx);
#define cel_httpcontext_routing_again cel_httpcontext_routing

int cel_httpcontext_response_write(CelHttpContext *http_ctx, CelHttpStatusCode status,
								   int err_no, const char *msg);
static __inline 
int cel_httpcontext_response_tryfiles(CelHttpContext *http_ctx, 
									  const char *file_path, const char *uri_file_path,
									  long long first, long long last,
									  CelTime *if_modified_since, 
									  char *if_none_match)
{
	return cel_httpresponse_send_tryfile(&(http_ctx->rsp), 
		file_path, uri_file_path, first, last, if_modified_since, if_none_match);
}

static __inline 
int cel_httpcontext_response_sendfile(CelHttpContext *http_ctx, 
									  const char *file_path, 
									  long long first, long long last,
									  CelTime *if_modified_since,
									  char *if_none_match)
{
	return cel_httpresponse_send_file(&(http_ctx->rsp), 
		file_path, first, last, if_modified_since, if_none_match);
}

static __inline 
int cel_httpcontext_response_redirect(CelHttpContext *http_ctx, const char *url)
{
	return cel_httpresponse_send_redirect(&(http_ctx->rsp), url);
}


#ifdef __cplusplus
}
#endif

#endif

