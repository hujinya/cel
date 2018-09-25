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
#ifndef __CEL_NET_HTTPLISTENER_H__
#define __CEL_NET_HTTPLISTENER_H__

#include "cel/net/tcplistener.h"
#include "cel/net/httpclient.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelHttpListenerAsyncArgs CelHttpListenerAsyncArgs;
typedef struct _CelHttpListener CelHttpListener;
typedef int (* CelHttpAcceptCallbackFunc) (
    CelHttpListener *listener, CelHttpClient *new_client,  
    CelAsyncResult *result);

struct _CelHttpListenerAsyncArgs
{
    CelHttpAcceptCallbackFunc async_callback;
};

struct _CelHttpListener
{
    CelTcpListener tcp_listener;
    CelHttpListenerAsyncArgs *async_args;
    void *user_data;
    CelRefCounted ref_counted;
};

int cel_httplistener_init(CelHttpListener *listener, 
                          CelSockAddr *addr, CelSslContext *ssl_ctx);
int cel_httplistener_init_tcplistener(CelHttpListener *listener, 
                                      CelTcpListener *tcp_listener);
void cel_httplistener_destroy(CelHttpListener *listener);

CelHttpListener *cel_httplistener_new(CelSockAddr *addr, 
                                      CelSslContext *ssl_ctx);
CelHttpListener *cel_httplistener_new_tcplistener(CelTcpListener *tcp_listener);
void cel_httplistener_free(CelHttpListener *listener);

int cel_httplistener_start(CelHttpListener *listener, CelSockAddr *addr);

#define cel_httplistener_set_nonblock(listener, nonblock) \
    cel_socket_set_nonblock((CelSocket *)listener, nonblock)
#define cel_httplistener_set_reuseaddr(listener, reuseaddr) \
    cel_socket_set_reuseaddr((CelSocket *)listener, reuseaddr)

#define cel_httplistener_get_channel(listener) \
    &((listener)->tcp_listener.sock.channel)
#define cel_httplistener_get_localaddr(listener) \
    cel_tcplistener_get_localaddr(listener)

int cel_httplistener_accept(CelHttpListener *listener, CelHttpClient *new_client);
int cel_httplistener_async_accept(CelHttpListener *listener, 
                                  CelHttpClient *new_client,
                                  CelHttpAcceptCallbackFunc callback);
#define cel_httplistener_async_cb_accept(listener, new_client, callback) \
    cel_httplistener_async_accept(listener, new_client, callback)
#define cel_httplistener_async_co_accept(listener, new_client, co) \
    cel_httplistener_async_accept(listener, new_client, callback)

#ifdef __cplusplus
}
#endif

#endif
