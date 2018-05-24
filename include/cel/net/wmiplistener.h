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
#ifndef __CEL_NET_WMIPLISTENER_H__
#define __CEL_NET_WMIPLISTENER_H__

#include "cel/eventloopgroup.h"
#include "cel/net/httplistener.h"
#include "cel/net/wmipclient.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelWmipListenerAsyncArgs CelWmipListenerAsyncArgs;
typedef struct _CelWmipListener CelWmipListener;
//typedef int (*CelWmipListenerAcceptCallbackFunc) (
//    CelWmipListener *listener, CelWmipClient *new_client,  
//    CelAsyncResult *result);

struct _CelWmipListenerAsyncArgs
{
    CelHttpClient client;
    //CelWmipListenerAcceptCallbackFunc async_callback;
};

struct _CelWmipListener
{
    CelHttpListener http_listener;
    CelWmipContext *wmip_ctx;
    BOOL is_run_group;
    union {
        CelEventLoop *evt_loop;
        CelEventLoopGroup *evt_loop_group;
    };
    CelWmipListenerAsyncArgs *async_args;
    CelRefCounted ref_counted;
};

int cel_wmiplistener_init(CelWmipListener *listener, 
                          CelSockAddr *addr, CelSslContext *ssl_ctx, 
                          CelWmipContext *wmip_ctx);
void cel_wmiplistener_destroy(CelWmipListener *listener);

CelWmipListener *cel_wmiplistener_new(CelSockAddr *addr, 
                                      CelWmipContext *wmip_ctx);
void cel_wmiplistener_free(CelWmipListener *listener);

int cel_wmiplistener_start(CelWmipListener *listener, CelSockAddr *addr);

#define cel_wmiplistener_set_nonblock(listener, nonblock) \
    cel_socket_set_nonblock((CelSocket *)listener, nonblock)
#define cel_wmiplistener_set_reuseaddr(listener, reuseaddr) \
    cel_socket_set_reuseaddr((CelSocket *)listener, reuseaddr)

#define cel_wmiplistener_get_channel(listener) \
    &((listener)->http_listener.tcp_listener.sock.channel)

#define cel_wmiplistener_get_localaddr(listener) \
    cel_httplistener_get_localaddr(listener)
#define cel_wmiplistener_get_localaddrs(listener) \
    cel_sockaddr_ntop(cel_wmiplistener_get_localaddr(listener))

int cel_wmiplistener_accept(CelWmipListener *listener, CelWmipClient *client);
int cel_wmiplistener_post_accept(CelWmipListener *listener);
int cel_wmiplistener_run(CelWmipListener *listener, CelEventLoop *evt_loop);
int cel_wmiplistener_run_group(CelWmipListener *listener, CelEventLoopGroup *evt_loop_grp);

#ifdef __cplusplus
}
#endif

#endif
