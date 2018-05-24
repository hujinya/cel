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
#ifndef __CEL_SSL_SOCKET_H__
#define __CEL_SSL_SOCKET_H__

#include "cel/net/socket.h"
#include "cel/net/ssl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelSslSocket
{
    CelSocket sock;
    BOOL use_ssl;
    CelSsl *ssl;
    CelBio *r_bio, *w_bio;
    CelAsyncBuf async_rbuf, async_wbuf;
    CelRefCounted ref_counted;
}CelSslSocket;

typedef struct _CelSslAsyncHandshakeArgs
{
    CelSocketAsyncArgs socket_args;
    void (* async_callback) (void *ol);
    CelSslSocket *ssl_sock;
    long result;
    int error;
}CelSslAsyncAcceptArgs, CelSslAsyncConnectArgs, CelSslAsyncHandshakeArgs;

typedef struct _CelSslAsyncSendArgs
{
    CelSocketAsyncArgs socket_args;
    void (* async_callback) (void *ol);
    CelSslSocket *ssl_sock;
    CelAsyncBuf *buffers;
    int buffer_count;
    long result;
    int error;
}CelSslAsyncSendArgs, CelSslAsyncRecvArgs;

typedef union _CelSslAsyncArgs
{
    CelOverLapped ol;
    CelSocketAsyncArgs socket_args;
    CelSslAsyncHandshakeArgs handshake_args;
    CelSslAsyncAcceptArgs accept_args;
    CelSslAsyncConnectArgs connect_args;
    CelSslAsyncRecvArgs recv_args;
    CelSslAsyncSendArgs send_args;
}CelSslAsyncArgs;

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
static __inline int cel_sslsocket_accept(CelSslSocket *ssl_sock)
{
    cel_ssl_set_endpoint(ssl_sock->ssl, CEL_SSLEP_SERVER);
    return cel_sslsocket_handshake(ssl_sock);
}
static __inline int cel_sslsocket_connect(CelSslSocket *ssl_sock)
{
    cel_ssl_set_endpoint(ssl_sock->ssl, CEL_SSLEP_CLIENT);
    return cel_sslsocket_handshake(ssl_sock);
}
int cel_sslsocket_send(CelSslSocket *ssl_sock, void *buf, size_t size);
int cel_sslsocket_recv(CelSslSocket *ssl_sock, void *buf, size_t size);

int cel_sslsocket_async_handshake(CelSslAsyncHandshakeArgs *args);
static __inline int cel_sslsocket_async_accept(CelSslAsyncAcceptArgs *args)
{
    cel_ssl_set_endpoint(args->ssl_sock->ssl, CEL_SSLEP_SERVER);
    return cel_sslsocket_async_handshake(args);
}
static __inline int cel_sslsocket_async_connect(CelSslAsyncConnectArgs *args)
{
    cel_ssl_set_endpoint(args->ssl_sock->ssl, CEL_SSLEP_CLIENT);
    return cel_sslsocket_async_handshake(args);
}
int cel_sslsocket_async_send(CelSslAsyncSendArgs *args);
int cel_sslsocket_async_recv(CelSslAsyncRecvArgs *args);

#ifdef __cplusplus
}
#endif

#endif
