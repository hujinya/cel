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
#ifndef __CEL_SSL_SOCKET_H__
#define __CEL_SSL_SOCKET_H__

#include "cel/net/socket.h"
#include "cel/net/ssl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelSslSocket CelSslSocket;
typedef void (*CelSslSocketAcceptCallbackFunc)(
    CelSslSocket *ssl_sock, CelAsyncResult *result);
typedef void (*CelSslSocketConnectCallbackFunc)(
    CelSslSocket *ssl_sock, CelAsyncResult *result);
typedef void (*CelSslSocketHandshakeCallbackFunc)(
    CelSslSocket *ssl_sock, CelAsyncResult *result);
typedef void (*CelSslSocketSendCallbackFunc)(
    CelSslSocket *ssl_sock, CelAsyncBuf *buffers, int count,
    CelAsyncResult *result);
typedef void (*CelSslSocketRecvCallbackFunc)(
    CelSslSocket *ssl_sock, CelAsyncBuf *buffers, int count,
    CelAsyncResult *result);
typedef void (*CelSslSocketShutdownCallbackFunc)(
    CelSslSocket *ssl_sock, CelAsyncResult *result);

typedef struct _CelSslAsyncArgs
{
    int want;
    CelAsyncBuf sock_buf;
    CelAsyncBuf *ssl_buf;
    CelAsyncResult result;
    union {
        CelSslSocketAcceptCallbackFunc accept_callback;
        CelSslSocketConnectCallbackFunc connect_callback;
        CelSslSocketHandshakeCallbackFunc handshake_callback;
        CelSslSocketSendCallbackFunc send_callback;
        CelSslSocketRecvCallbackFunc recv_callback;
        CelSslSocketShutdownCallbackFunc shutdown_callback;
    };
}CelSslAsyncArgs;

struct _CelSslSocket{
    CelSocket sock;
    BOOL use_ssl;
    CelSsl *ssl;
    CelBio *r_bio, *w_bio;
    CelSslAsyncArgs in, out;
    CelRefCounted ref_counted;
};

int cel_sslsocket_init(CelSslSocket *ssl_sock, 
                       CelSocket *sock, CelSslContext *ssl_ctx);
void _cel_sslsocket_destroy_derefed(CelSslSocket *ssl_sock);
void cel_sslsocket_destroy(CelSslSocket *ssl_sock);

CelSslSocket *cel_sslsocket_new(CelSocket *sock, CelSslContext *ssl_ctx);
void _cel_sslsocket_free_derefed(CelSslSocket *ssl_sock);
void cel_sslsocket_free(CelSslSocket *ssl_sock);

#define cel_sslsocket_ref(ssl_sock) \
    (CelSslSocket *)cel_refcounted_ref(&(ssl_sock->ref_counted), ssl_sock)
#define cel_sslsocket_deref(ssl_sock) \
    cel_refcounted_deref(&(ssl_sock->ref_counted), ssl_sock)

int cel_sslsocket_handshake(CelSslSocket *ssl_sock);
static __inline 
int cel_sslsocket_accept(CelSslSocket *ssl_sock)
{
    cel_ssl_set_endpoint(ssl_sock->ssl, CEL_SSLEP_SERVER);
    return cel_sslsocket_handshake(ssl_sock);
}
static __inline 
int cel_sslsocket_connect(CelSslSocket *ssl_sock)
{
    cel_ssl_set_endpoint(ssl_sock->ssl, CEL_SSLEP_CLIENT);
    return cel_sslsocket_handshake(ssl_sock);
}
int cel_sslsocket_send(CelSslSocket *ssl_sock, 
                       CelAsyncBuf *buffers, int count);
int cel_sslsocket_recv(CelSslSocket *ssl_sock, 
                       CelAsyncBuf *buffers, int count);
int cel_sslsocket_shutdown(CelSslSocket *ssl_sock, int how);


int cel_sslsocket_async_handshake(CelSslSocket *ssl_sock,
                                  CelSslSocketHandshakeCallbackFunc callback);

static __inline 
int cel_sslsocket_async_accept(CelSslSocket *ssl_sock,
                               CelSslSocketAcceptCallbackFunc callback)
{
    cel_ssl_set_endpoint(ssl_sock->ssl, CEL_SSLEP_SERVER);
    return cel_sslsocket_async_handshake(ssl_sock, callback);
}

static __inline 
int cel_sslsocket_async_connect(CelSslSocket *ssl_sock,
                                CelSslSocketConnectCallbackFunc callback)
{
    cel_ssl_set_endpoint(ssl_sock->ssl, CEL_SSLEP_CLIENT);
    return cel_sslsocket_async_handshake(ssl_sock, callback);
}

int cel_sslsocket_async_send(CelSslSocket *ssl_sock,
                             CelAsyncBuf *buffers, int count,
                             CelSslSocketSendCallbackFunc callback);

int cel_sslsocket_async_recv(CelSslSocket *ssl_sock, 
                             CelAsyncBuf *buffers, int count,
                             CelSslSocketSendCallbackFunc ca);

int cel_sslsocket_async_shutdown(CelSslSocket *ssl_sock, 
                                 CelSslSocketShutdownCallbackFunc ca);

#ifdef __cplusplus
}
#endif

#endif
