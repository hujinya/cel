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
#ifndef __CEL_NET_TCPCLIENT_H__
#define __CEL_NET_TCPCLIENT_H__

#include "cel/types.h"
#include "cel/stream.h"
#include "cel/net/sslsocket.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelTcpClient CelTcpClient;

typedef void (* CelTcpConnectCallbackFunc)(
    CelTcpClient *client, CelAsyncResult *result);
typedef void (* CelTcpHandshakeCallbackFunc)(
    CelTcpClient *client, CelAsyncResult *result);
typedef void (* CelTcpRecvCallbackFunc) (
    CelTcpClient *client, CelStream *s, CelAsyncResult *result);
typedef void (* CelTcpSendCallbackFunc) (
    CelTcpClient *client, CelStream *s, CelAsyncResult *result);
typedef void (* CelTcpShutdownCallbackFunc)(
    CelTcpClient *client, CelAsyncResult *result);

typedef struct _CelTcpClientAsyncArgs
{
    CelAsyncBuf async_buf;
    CelStream *s;
    CelAsyncResult result;
    union {
        CelTcpConnectCallbackFunc connect_callback;
        CelTcpHandshakeCallbackFunc handshake_callback;
        CelTcpRecvCallbackFunc recv_callback;
        CelTcpSendCallbackFunc send_callback;
        CelTcpShutdownCallbackFunc shutdown_callback;
    };
    CelCoroutine *co;
}CelTcpClientAsyncArgs;

struct _CelTcpClient
{
    union{
        CelSocket sock;
        CelSslSocket ssl_sock;
    };
    CelSockAddr local_addr;
    CelSockAddr remote_addr;
    CelTcpClientAsyncArgs in, out;
    void *user_data;
    CelRefCounted ref_counted;
};

int cel_tcpclient_init_family(CelTcpClient *client, 
                              int family, CelSslContext *ssl_ctx);
int cel_tcpclient_init_addr(CelTcpClient *client, 
                            CelSockAddr *addr, CelSslContext *ssl_ctx);
/* String format - <ipv4|ipv6|unix>@<ipv4addr|[ipv6addr]>:<port> */
int cel_tcpclient_init_str(CelTcpClient *client, 
                           const TCHAR *str, CelSslContext *ssl_ctx);
int cel_tcpclient_init_socket(CelTcpClient *client, 
                              CelSocket *socket, CelSslContext *ssl_ctx);
void _cel_tcpclient_destroy_derefed(CelTcpClient *client);
void cel_tcpclient_destroy(CelTcpClient *client);

CelTcpClient *cel_tcpclient_new_family(int family, CelSslContext *ssl_ctx);
CelTcpClient *cel_tcpclient_new_addr(CelSockAddr *addr, 
                                     CelSslContext *ssl_ctx);
CelTcpClient *cel_tcpclient_new_str(const TCHAR *str, CelSslContext *ssl_ctx);
CelTcpClient *cel_tcpclient_new_socket(CelSocket *socket, 
                                       CelSslContext *ssl_ctx);
void _cel_tcpclient_free_derefed(CelTcpClient *client);
void cel_tcpclient_free(CelTcpClient *client);

#define cel_tcpclient_ref(client) \
    cel_refcounted_ref(&(client->ref_counted), client)
#define cel_tcpclient_deref(client) \
    cel_refcounted_deref(&(client->ref_counted), client)

#define cel_tcpclient_is_connected(client) \
    cel_socket_is_connected((CelSocket *)client)

void cel_tcpclient_set_ssl(CelTcpClient *client, BOOL use_ssl);

#define cel_tcpclient_set_keepalive(client, idle, interval, count) \
    cel_socket_set_keepalive((CelSocket *)client, idle, interval, count);

#define cel_tcpclient_set_linger(client, on, seconds ) \
    cel_socket_set_linger((CelSocket *)client, on, seconds)

#define cel_tcpclient_set_nodelay(client, on) \
    cel_socket_set_nodelay((CelSocket *)client, on)

#define cel_tcpclient_set_nonblock(client, nonblock) \
    cel_socket_set_nonblock((CelSocket *)client, nonblock)

#define cel_tcpclient_set_reuseaddr(client, reuseaddr) \
    cel_socket_set_reuseaddr((CelSocket *)client, reuseaddr)

#define cel_tcpclient_set_rcvbuffer(client, rcvbufsize) \
    cel_socket_set_rcvbuffer((CelSocket *)client, rcvbufsize)
#define cel_tcpclient_set_sndbuffer(client, sndbufsize) \
    cel_socket_set_sndbuffer((CelSocket *)client, sndbufsize)

#define cel_tcpclient_set_rcvtimeout(client, milliseconds) \
    cel_socket_set_rcvtimeout((CelSocket *)client, milliseconds)
#define cel_tcpclient_set_sndtimeout(client, milliseconds) \
    cel_socket_set_sndtimeout((CelSocket *)client, milliseconds)

#define cel_tcpclient_get_localaddr(client) \
    &(((CelTcpClient *)(client))->local_addr)
#define cel_tcpclient_get_localaddr_str(client) \
    cel_sockaddr_ntop(cel_tcpclient_get_localaddr(client))
#define cel_tcpclient_get_remoteaddr(client) \
    &(((CelTcpClient *)(client))->remote_addr)
#define cel_tcpclient_get_remoteaddr_str(client) \
    cel_sockaddr_ntop(cel_tcpclient_get_remoteaddr(client))

static __inline 
int cel_tcpclient_connect(CelTcpClient *client, CelSockAddr *remote_addr)
{
    return cel_socket_connect(&(client->sock), remote_addr);
}
static __inline
int cel_tcpclient_connect_host(CelTcpClient *client, 
                               const TCHAR *host, unsigned short port)
{
    return cel_socket_connect_host(&(client->sock), host, port);
}
static __inline 
int cel_tcpclient_handshake(CelTcpClient *client)
{
    return (client->ssl_sock.ssl != NULL 
        ? cel_sslsocket_handshake(&(client->ssl_sock)) : 0);
}
int cel_tcpclient_recv(CelTcpClient *client, CelStream *s);
int cel_tcpclient_send(CelTcpClient *client, CelStream *s);
static __inline 
int cel_tcpclient_shutdown(CelTcpClient *client)
{
    return (client->ssl_sock.ssl != NULL 
        ? cel_sslsocket_shutdown(&(client->ssl_sock)) 
        : cel_socket_shutdown(&(client->sock), 2));
}

int cel_tcpclient_async_connect(CelTcpClient *client, CelSockAddr *remote_addr,
                                CelTcpConnectCallbackFunc callback, 
                                CelCoroutine *co);
#define cel_tcpclient_async_cb_connect(client, remote_addr, callback) \
    cel_tcpclient_async_connect(client, remote_addr, callback, NULL)
#define cel_tcpclient_async_co_connect(client, remote_addr, co) \
    cel_tcpclient_async_connect(client, remote_addr, NULL, co)

int cel_tcpclient_async_connect_host(CelTcpClient *client, 
                                     const TCHAR *host, unsigned short port,
                                     CelTcpConnectCallbackFunc callback,
                                     CelCoroutine *co);
#define cel_tcpclient_async_cb_connect_host(client, host, port, callback) \
    cel_tcpclient_async_connect_host(client, host, port, callback, NULL)
#define cel_tcpclient_async_co_connect_host(client, host, port, co) \
    cel_tcpclient_async_connect_host(client, host, port, NULL, co)

int cel_tcpclient_async_handshake(CelTcpClient *client, 
                                  CelTcpHandshakeCallbackFunc callback, 
                                  CelCoroutine *co);
#define cel_tcpclient_async_cb_handshake(client, callback) \
    cel_tcpclient_async_handshake(client, callback, NULL)
#define cel_tcpclient_async_co_handshake(client, co) \
    cel_tcpclient_async_handshake(client, NULL, co)

int cel_tcpclient_async_send(CelTcpClient *client, CelStream *s, 
                             CelTcpSendCallbackFunc callback, CelCoroutine *co);
#define cel_tcpclient_async_cb_send(client, s, callback) \
    cel_tcpclient_async_send(client, s, callback, NULL)
#define cel_tcpclient_async_co_send(client, s, co) \
    cel_tcpclient_async_send(client, s, NULL, co)

int cel_tcpclient_async_recv(CelTcpClient *client, CelStream *s,
                             CelTcpRecvCallbackFunc callback, CelCoroutine *co);
#define cel_tcpclient_cb_async_recv(client, s, callback) \
    cel_tcpclient_async_recv(client, s, callback, NULL)
#define cel_tcpclient_co_async_recv(client, s, co) \
    cel_tcpclient_async_recv(client, s, NULL, co)

int cel_tcpclient_async_shutdown(CelTcpClient *client, 
                                 CelTcpShutdownCallbackFunc callback, 
                                 CelCoroutine *co);
#define cel_tcpclient_async_cb_shutdown(client, callback) \
    cel_tcpclient_async_shutdown(client, callback, NULL)
#define cel_tcpclient_async_co_shutdown(client, co) \
    cel_tcpclient_async_shutdown(client, NULL, co)


#ifdef __cplusplus
}
#endif

#endif