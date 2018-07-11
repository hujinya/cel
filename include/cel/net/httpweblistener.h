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
#ifndef __CEL_NET_HTTPWEBLISTENER_H__
#define __CEL_NET_HTTPWEBLISTENER_H__

#include "cel/eventloopgroup.h"
#include "cel/net/httplistener.h"
#include "cel/net/httpwebclient.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelHttpWebListenerAsyncArgs CelHttpWebListenerAsyncArgs;
typedef struct _CelHttpWebListener CelHttpWebListener;

struct _CelHttpWebListenerAsyncArgs
{
    CelHttpClient client;
};

struct _CelHttpWebListener
{
    CelHttpListener http_listener;
    CelHttpWebContext *web_ctx;
    BOOL is_run_group;
    union {
        CelEventLoop *evt_loop;
        CelEventLoopGroup *evt_loop_group;
    };
    CelHttpWebListenerAsyncArgs *async_args;
    CelRefCounted ref_counted;
};

int cel_httpweblistener_init(CelHttpWebListener *listener, 
                             CelSockAddr *addr, CelSslContext *ssl_ctx, 
                             CelHttpWebContext *web_ctx);
void cel_httpweblistener_destroy(CelHttpWebListener *listener);

CelHttpWebListener *cel_httpweblistener_new(CelSockAddr *addr, 
                                            CelHttpWebContext *web_ctx);
void cel_httpweblistener_free(CelHttpWebListener *listener);

int cel_httpweblistener_start(CelHttpWebListener *listener, CelSockAddr *addr);

#define cel_httpweblistener_set_nonblock(listener, nonblock) \
    cel_socket_set_nonblock((CelSocket *)listener, nonblock)
#define cel_httpweblistener_set_reuseaddr(listener, reuseaddr) \
    cel_socket_set_reuseaddr((CelSocket *)listener, reuseaddr)

#define cel_httpweblistener_get_channel(listener) \
    &((listener)->http_listener.tcp_listener.sock.channel)

#define cel_httpweblistener_get_localaddr(listener) \
    cel_httplistener_get_localaddr(listener)
#define cel_httpweblistener_get_localaddrs(listener) \
    cel_sockaddr_ntop(cel_httpweblistener_get_localaddr(listener))

int cel_httpweblistener_run(CelHttpWebListener *listener, 
                            CelEventLoop *evt_loop);
int cel_httpweblistener_run_group(CelHttpWebListener *listener, 
                                  CelEventLoopGroup *evt_loop_grp);

#ifdef __cplusplus
}
#endif

#endif
