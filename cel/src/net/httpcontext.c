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
#include "cel/net/httpcontext.h"

int cel_httpcontext_init(CelHttpContext *http_ctx,
						 CelHttpClient *client,
						 CelHttpRequest *req, CelHttpResponse *rsp)
{
	http_ctx->client = client;
	http_ctx->req = req;
	http_ctx->rsp = rsp;
	http_ctx->state = 0;
	http_ctx->current_filter = NULL;
	return cel_rbtree_init(&(http_ctx->params), (CelCompareFunc)strcmp, NULL, cel_free);
}
void cel_httpcontext_destroy(CelHttpContext *http_ctx)
{
	http_ctx->client = NULL;
	http_ctx->req = NULL;
	http_ctx->rsp = NULL;
	http_ctx->state = 0;
	http_ctx->current_filter = NULL;
	cel_rbtree_destroy(&(http_ctx->params));
}
void cel_httpcontext_clear(CelHttpContext *http_ctx)
{
	http_ctx->client = NULL;
	http_ctx->req = NULL;
	http_ctx->rsp = NULL;
	http_ctx->state = 0;
	http_ctx->current_filter = NULL;
	cel_rbtree_clear(&(http_ctx->params));
}

#include "cel/net/httpwebclient.h"

int cel_httpcontext_routing_again(CelHttpContext *http_ctx)
{
	return cel_httpwebclient_routing((CelHttpWebClient *)(http_ctx->client));
}

int cel_httpcontext_response_write(CelHttpContext *http_ctx,
								   CelHttpStatusCode status,
								   int err_no, const char *msg)
{
	return cel_httpwebclient_response_write(
		(CelHttpWebClient *)(http_ctx->client), status, err_no, msg);
}

int cel_httpcontext_response_sendfile(CelHttpContext *http_ctx, 
									  const char *file_path, 
									  long long first, long long last,
									  CelDateTime *if_modified_since,
									  char *if_none_match)
{
	return cel_httpresponse_send_file(http_ctx->rsp, 
		file_path, first, last, if_modified_since, if_none_match);
}

int cel_httpcontext_response_redirect(CelHttpContext *http_ctx, const char *url)
{
	return cel_httpresponse_send_redirect(http_ctx->rsp, url);
}
