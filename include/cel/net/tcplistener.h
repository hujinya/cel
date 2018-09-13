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
#ifndef __CEL_NET_TCPLISTENER_H__
#define __CEL_NET_TCPLISTENER_H__

#include "cel/net/tcpclient.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelTcpListenerAsyncArgs CelTcpListenerAsyncArgs;
typedef struct _CelTcpListener CelTcpListener;

typedef void (*CelTcpAcceptCallbackFunc) (
    CelTcpListener *listener, CelTcpClient *new_client, 
    CelAsyncResult *async_result);

struct _CelTcpListenerAsyncArgs
{
    CelAsyncResult result;
    CelTcpAcceptCallbackFunc async_callback;
    CelCoroutine *co;
};

struct _CelTcpListener
{
    CelSocket sock;
    CelSockAddr addr;
    CelSslContext *ssl_ctx;
    CelTcpListenerAsyncArgs *async_args;
    void *user_data;
    CelRefCounted ref_counted;
};

int cel_tcplistener_init(CelTcpListener *listener, int family, 
                         CelSslContext *ssl_ctx);
int cel_tcplistener_init_addr(CelTcpListener *listener,
                              CelSockAddr *addr, CelSslContext *ssl_ctx);
/* String format - <ipv4|ipv6|unix>@<ipv4addr|[ipv6addr]>:<port> */
int cel_tcplistener_init_str(CelTcpListener *listener,
                             const TCHAR *str, CelSslContext *ssl_ctx);
int cel_tcplistener_init_socket(CelTcpListener *listener,
                                CelSocket *socket, CelSslContext *ssl_ctx);
void cel_tcplistener_destroy(CelTcpListener *listener);

CelTcpListener *cel_tcplistener_new(int family, CelSslContext *ssl_ctx);
CelTcpListener *cel_tcplistener_new_addr(CelSockAddr *addr, 
                                         CelSslContext *ssl_ctx);
/* String format - <ipv4|ipv6|unix>@<ipv4addr|[ipv6addr]>:<port> */
CelTcpListener *cel_tcplistener_new_str(const TCHAR *str, 
                                        CelSslContext *ssl_ctx);
CelTcpListener *cel_tcplistener_new_socket(CelSocket *socket, 
                                           CelSslContext *ssl_ctx);
void cel_tcplistener_free(CelTcpListener *listener);

#define cel_tcplistener_set_nonblock(listener, nonblock) \
    cel_socket_set_nonblock((CelSocket *)listener, nonblock)
#define cel_tcplistener_set_reuseaddr(listener, reuseaddr) \
    cel_socket_set_reuseaddr((CelSocket *)listener, reuseaddr)

#define cel_tcplistener_get_localaddr(listener) \
    (&(((CelTcpListener *)(listener))->addr))
#define cel_tcplistener_get_localaddrs(listener) \
    cel_sockaddr_ntop(cel_tcplistener_get_localaddr(listener))

#define cel_tcplistener_start(listener, addr) \
    cel_socket_listen(((CelSocket *)listener), addr, 1024)

int cel_tcplistener_accept(CelTcpListener *listener, CelTcpClient *client);

int cel_tcplistener_async_accept(CelTcpListener *listener,
                                 CelTcpClient *new_client,
                                 CelTcpAcceptCallbackFunc callback, 
                                 CelCoroutine *co);
#define cel_tcplistener_async_cb_accept(listener, new_client, callback) \
    cel_tcplistener_async_accept(listener, new_client, callback, NULL)
#define cel_tcplistener_async_co_accept(listener, new_client, co) \
    cel_tcplistener_async_accept(listener, new_client, NULL, co)

#ifdef __cplusplus
}
#endif

#endif
