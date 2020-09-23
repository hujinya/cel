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
#ifndef __CEL_NET_HTTPSERVE_H__
#define __CEL_NET_HTTPSERVE_H__

#include "cel/eventloopgroup.h"
#include "cel/net/httplistener.h"
#include "cel/net/httpcontext.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelHttpServeAsyncArgs CelHttpServeAsyncArgs;
typedef struct _CelHttpServe CelHttpServe;

struct _CelHttpServeAsyncArgs
{
    CelHttpClient client;
};

struct _CelHttpServe
{
    CelHttpListener http_listener;
    CelHttpServeContext *serve_ctx;
    BOOL is_run_group;
    union {
        CelEventLoop *evt_loop;
        CelEventLoopGroup *evt_loop_group;
    };
    CelHttpServeAsyncArgs *async_args;
    CelRefCounted ref_counted;
};

int cel_httpserve_init(CelHttpServe *serve, 
					   CelSockAddr *addr, CelSslContext *ssl_ctx, 
					   CelHttpServeContext *serve_ctx);
void cel_httpserve_destroy(CelHttpServe *listener);

CelHttpServe *cel_httpserve_new(CelSockAddr *addr, 
								CelHttpServeContext *serve_ctx);
void cel_httpserve_free(CelHttpServe *serve);

int cel_httpserve_start(CelHttpServe *serve, CelSockAddr *addr);

#define cel_httpserve_set_nonblock(serve, nonblock) \
	cel_socket_set_nonblock((CelSocket *)serve, nonblock)
#define cel_httpserve_set_reuseaddr(serve, reuseaddr) \
	cel_socket_set_reuseaddr((CelSocket *)serve, reuseaddr)

#define cel_httpserve_get_channel(serve) \
	&((serve)->http_listener.tcp_listener.sock.channel)

#define cel_httpserve_get_localaddr(serve) \
	cel_httplistener_get_localaddr(serve)
#define cel_httpserve_get_localaddrs(serve) \
	cel_sockaddr_ntop(cel_httpserve_get_localaddr(serve))

int cel_httpserve_run(CelHttpServe *serve, CelEventLoop *evt_loop);
int cel_httpserve_run_group(CelHttpServe *serve, CelEventLoopGroup *evt_loop_grp);

#ifdef __cplusplus
}
#endif

#endif
