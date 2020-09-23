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
#include "cel/net/httpserve.h"
//#define _CEL_DEBUG
#include "cel/log.h"
#include "cel/error.h"
#include "cel/allocator.h"

int cel_httpserve_accept(CelHttpServe *listener, CelHttpContext *http_ctx);
int cel_httpserve_post_accept(CelHttpServe *listener);

int cel_httpserve_init(CelHttpServe *listener, 
					   CelSockAddr *addr, CelSslContext *ssl_ctx, 
					   CelHttpServeContext *serve_ctx)
{
	if (cel_httplistener_init(&(listener->http_listener), addr, ssl_ctx) == 0)
	{
		listener->serve_ctx = serve_ctx;
		return 0;
	}
	return -1;
}

void cel_httpserve_destroy(CelHttpServe *listener)
{
	cel_httplistener_destroy(&(listener->http_listener));
}

int cel_httpserve_start(CelHttpServe *listener, CelSockAddr *addr)
{
	return cel_httplistener_start(&(listener->http_listener), addr);
}

static void cel_httpserve_do_accept(CelHttpServe *listener, 
									CelHttpClient *http_client, CelAsyncResult *result)
{
	CelHttpContext *new_ctx;

	if (result->ret == -1)
	{
		CEL_SETERR((CEL_ERR_LIB, _T("Http serve %s do accept %d.(%s)"), 
			cel_httpserve_get_localaddrs(listener), 
			result->ret, cel_geterrstr()));
		cel_httpserve_post_accept(listener);
		return ;
	}
	if ((new_ctx = cel_httpcontext_new(http_client, listener->serve_ctx)) == NULL)
	{
		CEL_SETERR((CEL_ERR_LIB, _T("Http context new return null")));
		cel_httpclient_destroy(http_client);
		cel_httpserve_post_accept(listener);
		return ;
	}
	cel_httpserve_post_accept(listener);
	if (cel_httpclient_set_nonblock(&(new_ctx->http_client), 1) == 0)
	{
		if (listener->is_run_group)
		{
			if (cel_eventloopgroup_add_channel(listener->evt_loop_group, -1,
				cel_httpclient_get_channel(&(new_ctx->http_client)), NULL) == 0)
			{
				CEL_DEBUG((_T("Http context %s succeed."), 
					cel_httpclient_get_remoteaddr_str(&(new_ctx->http_client))));
				cel_httpcontext_routing(new_ctx);
				return ;
			}
		}
		else 
		{
			if (cel_eventloop_add_channel(listener->evt_loop, 
				cel_httpclient_get_channel(&(new_ctx->http_client)), NULL) == 0)
			{
				CEL_DEBUG((_T("Http context %s init succeed."), 
					cel_httpclient_get_remoteaddr_str(&(new_ctx->http_client))));
				cel_httpcontext_routing(new_ctx);
				return ;
			} 
		}
	}
	CEL_SETERR((CEL_ERR_LIB, _T("Http context %s init failed(%s)."), 
		cel_httpclient_get_remoteaddr_str(&(new_ctx->http_client)), 
		cel_geterrstr()));
	cel_httpcontext_free(new_ctx);
}

int cel_httpserve_post_accept(CelHttpServe *listener)
{
	CelHttpServeAsyncArgs *args;

	if (listener->async_args == NULL
		&& (listener->async_args = 
		(CelHttpServeAsyncArgs *)cel_calloc(1, sizeof(CelHttpServeAsyncArgs))) == NULL)
		return -1;
	args = listener->async_args;
	memset(&(args->client), 0, sizeof(args->client));
	return cel_httplistener_async_accept(&(listener->http_listener),
		&(args->client), 
		(CelHttpAcceptCallbackFunc)cel_httpserve_do_accept);
}

int cel_httpserve_run(CelHttpServe *listener, CelEventLoop *evt_loop)
{
	if (cel_httplistener_set_nonblock(listener, 1) != -1
		&& cel_eventloop_add_channel(evt_loop,
		cel_httpserve_get_channel(listener), NULL) != -1
		&& cel_httpserve_post_accept(listener) != -1)
	{
		listener->is_run_group = FALSE;
		listener->evt_loop = evt_loop;
		return 0;
	}
	return -1;
}

int cel_httpserve_run_group(CelHttpServe *listener, CelEventLoopGroup *evt_loop_grp)
{
	if (cel_httplistener_set_nonblock(listener, 1) != -1
		&& cel_eventloopgroup_add_channel(evt_loop_grp, -1,
		cel_httpserve_get_channel(listener), NULL) != -1
		&& cel_httpserve_post_accept(listener) != -1)
	{
		listener->is_run_group = TRUE;
		listener->evt_loop_group = evt_loop_grp;
		return 0;
	}
	return -1;
}
