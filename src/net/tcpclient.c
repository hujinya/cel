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
#include "cel/net/tcpclient.h"
#undef _CEL_DEBUG
//#define _CEL_DEBUG
#include "cel/log.h"
#include "cel/error.h"
#include "cel/allocator.h"

void _cel_tcpclient_destroy_derefed(CelTcpClient *client)
{
    //puts("_cel_tcpclient_destroy_derefed");
    if (client->ssl_sock.ssl != NULL)
        cel_sslsocket_destroy(&(client->ssl_sock));
    else
        cel_socket_destroy(&(client->sock));
}

void _cel_tcpclient_free_derefed(CelTcpClient *client)
{
    _cel_tcpclient_destroy_derefed(client);
    cel_free(client);
}

int cel_tcpclient_init_family(CelTcpClient *client, int family, 
                              CelSslContext *ssl_ctx)
{
    if (cel_socket_init(&(client->sock), 
        family, SOCK_STREAM, IPPROTO_TCP) == 0)
    {
        client->ssl_sock.use_ssl = FALSE;
        client->ssl_sock.ssl = NULL;
        if (cel_socket_set_reuseaddr(&(client->sock), 1) == 0
            && (ssl_ctx == NULL
            || cel_sslsocket_init(&(client->ssl_sock),
            &(client->sock), ssl_ctx) == 0))
        {
            cel_refcounted_init(&(client->ref_counted),
                (CelFreeFunc)_cel_tcpclient_destroy_derefed);
            return 0;
        }
        cel_socket_destroy(&(client->sock));
    }
    return -1;
}

int cel_tcpclient_init_addr(CelTcpClient *client, 
                            CelSockAddr *addr, CelSslContext *ssl_ctx)
{
    if (cel_socket_init(&(client->sock), 
        addr->sa_family, SOCK_STREAM, IPPROTO_TCP) == 0)
    {
        client->ssl_sock.use_ssl = FALSE;
        client->ssl_sock.ssl = NULL;
        if (cel_socket_set_reuseaddr(&(client->sock), 1) == 0
            && cel_socket_connect(&(client->sock), addr) == 0
            && (ssl_ctx == NULL
            || cel_sslsocket_init(&(client->ssl_sock), 
            &(client->sock), ssl_ctx) == 0))
        {
            memcpy(&(client->remote_addr),
                addr, cel_sockaddr_get_len(addr));
            cel_refcounted_init(&(client->ref_counted),
                (CelFreeFunc)_cel_tcpclient_destroy_derefed);
            return 0;
        }
        cel_socket_destroy(&(client->sock));
    }
    return -1;
}

int cel_tcpclient_init_str(CelTcpClient *client, 
                           const TCHAR *str, CelSslContext *ssl_ctx)
{
    int ret;
    TCHAR *node, *service;
    ADDRINFOT *addr_info, *result, hints;
    TCHAR addrs[CEL_ADDRLEN];

    _tcsncpy(addrs, str, CEL_ADDRLEN - 1);
    addrs[CEL_ADDRLEN - 1] = _T('\0');

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    if (cel_sockaddr_str_split(addrs, &hints.ai_family, &node, &service) != 0
        || (ret = GetAddrInfo(node, service, &hints, &addr_info)) != 0)
    {
        CEL_ERR((_T("GetAddrInfo():%s."), gai_strerror(ret)));
        return -1;
    }
    result = addr_info;
    while (addr_info != NULL)
    {
        if (cel_tcpclient_init_addr(
            client, (CelSockAddr *)addr_info->ai_addr, ssl_ctx) == 0)
            return 0;
        addr_info = addr_info->ai_next;
    }
    FreeAddrInfo(result);

    return 0;
}

void cel_tcpclient_destroy(CelTcpClient *client)
{
    cel_refcounted_destroy(&(client->ref_counted), client);
}

CelTcpClient *cel_tcpclient_new_family(int family, CelSslContext *ssl_ctx)
{
    CelTcpClient *client;

    if ((client = (CelTcpClient *)cel_malloc(sizeof(CelTcpClient))) != NULL)
    {
        if (cel_tcpclient_init_family(client, family, ssl_ctx) != -1)
        {
            cel_refcounted_init(&(client->ref_counted),
                (CelFreeFunc)_cel_tcpclient_free_derefed);
            return client;
        }
        cel_free(client);
    }

    return NULL;
}

CelTcpClient *cel_tcpclient_new_addr(CelSockAddr *addr, CelSslContext *ssl_ctx)
{
    CelTcpClient *client;

    if ((client = (CelTcpClient *)cel_malloc(sizeof(CelTcpClient))) != NULL)
    {
        if (cel_tcpclient_init_addr(client, addr, ssl_ctx) != -1)
        {
            cel_refcounted_init(
                &(client->ref_counted), (CelFreeFunc)_cel_tcpclient_free_derefed);
            return client;
        }
        cel_free(client);
    }

    return NULL;
}

CelTcpClient *cel_tcpclient_new_str(const TCHAR *str, CelSslContext *ssl_ctx)
{
    CelTcpClient *client;

    if ((client = (CelTcpClient *)cel_malloc(sizeof(CelTcpClient))) != NULL)
    {
        if (cel_tcpclient_init_str(client, str, ssl_ctx) != -1)
        {
            cel_refcounted_init(
                &(client->ref_counted), (CelFreeFunc)_cel_tcpclient_free_derefed);
            return client;
        }
        cel_free(client);
    }

    return NULL;
}

void cel_tcpclient_free(CelTcpClient *client)
{
    cel_refcounted_destroy(&(client->ref_counted), client);
}

void cel_tcpclient_set_ssl(CelTcpClient *client, BOOL use_ssl)
{
    CelSslContext *ssl_ctx;

    if (use_ssl
        && client->ssl_sock.ssl == NULL)
    {
        //puts("ssl_ctx default");
        ssl_ctx = cel_sslcontext_new(cel_sslcontext_method(_T("SSLv23")));
        cel_sslsocket_init(&(client->ssl_sock), &(client->sock), ssl_ctx);
        cel_sslcontext_free(ssl_ctx);
    }
    client->ssl_sock.use_ssl = use_ssl;
}

int cel_tcpclient_connect(CelTcpClient *client, 
                          CelSockAddr *remote_addr, CelCoroutine *co)
{
    return 0;
}

int cel_tcpclient_connect_host(CelTcpClient *client, 
                               const TCHAR *host, unsigned short port, 
                               CelCoroutine *co)
{
    return 0;
}

void cel_tcpclient_do_connect(CelTcpClient *client, CelAsyncResult *result)
{
    CelTcpClientAsyncArgs *args = &(client->out);

    if (result->ret == 0)
    {
        if (client->ssl_sock.use_ssl)
            cel_ssl_set_endpoint(client->ssl_sock.ssl, CEL_SSLEP_CLIENT);
        cel_socket_get_localaddr(&(client->sock), &(client->local_addr));
        cel_socket_get_remoteaddr(&(client->sock), &(client->remote_addr));
    }
    args->connect_callback(client, result);
}

int cel_tcpclient_async_connect(CelTcpClient *client, CelSockAddr *remote_addr,
                                CelTcpConnectCallbackFunc async_callback)
{
    CelTcpClientAsyncArgs *args;

    args = &(client->out);
    args->connect_callback = async_callback;
    return cel_socket_async_connect(
        &(client->sock), remote_addr, 
        (CelSocketConnectCallbackFunc)cel_tcpclient_do_connect);
}

int cel_tcpclient_async_connect_host(CelTcpClient *client, 
                                     const TCHAR *host, unsigned short port,
                                     CelTcpConnectCallbackFunc async_callback)
{
    CelTcpClientAsyncArgs *args;

    args = &(client->out);
    args->connect_callback = async_callback;
    return cel_socket_async_connect_host(
        &(client->sock), host, port, 
        (CelSocketConnectCallbackFunc)cel_tcpclient_do_connect);
}

void cel_tcpclient_do_handshake(CelTcpClient *client, CelAsyncResult *result)
{
    CelTcpClientAsyncArgs *args = &(client->in);
    args->handshake_callback(client, result);
}

int cel_tcpclient_async_handshake(CelTcpClient *client, 
                                  CelTcpHandshakeCallbackFunc async_callback)
{
    CelTcpClientAsyncArgs *args;
    CelAsyncResult _result;

    args = &(client->in);
    args->handshake_callback = async_callback;
    if (client->ssl_sock.use_ssl)
    {
        //puts("tcpclient ssl handshake");
        return cel_sslsocket_async_handshake(&(client->ssl_sock), 
            (CelSslSocketHandshakeCallbackFunc)cel_tcpclient_do_handshake);
    }
    else
    {
        //puts("tcpclient no ssl");
        _result.error = 0;
        _result.ret = 1;
        async_callback(client, &_result);
    }
    return 0;
}

int cel_tcpclient_send(CelTcpClient *client, CelStream *s, CelCoroutine *co)
{
    return 0;
}

void cel_tcpclient_do_send(CelTcpClient *client, 
                           CelAsyncBuf *buffers, int count, 
                           CelAsyncResult *result)
{
    CelTcpClientAsyncArgs *args = &(client->out);

    //printf("send ret = %d\r\n", result->ret);
    if (result->ret > 0)
        cel_stream_seek(args->s, result->ret);
    
    args->send_callback(client, args->s, result);
}

int cel_tcpclient_async_send(CelTcpClient *client, CelStream *s, 
                             CelTcpSendCallbackFunc async_callback)
{
    CelTcpClientAsyncArgs *args;

    args = &(client->out);
    /*_tprintf(_T("tcpclient %d post send buf %p, len %ld\r\n"), 
            client->sock.fd, 
            cel_stream_get_pointer(s), cel_stream_get_length(s));*/
    if ((args->async_buf.buf = cel_stream_get_pointer(s))== NULL
        || (args->async_buf.len = cel_stream_get_length(s)) <= 0)
    {
        _tprintf(_T("tcpclient %d send buf %p, len %ld\r\n"), 
            client->sock.fd, 
            cel_stream_get_pointer(s), cel_stream_get_length(s));
        return -1;
    }
    args->s = s;
    args->send_callback = async_callback;
    if (client->ssl_sock.use_ssl)
        return cel_sslsocket_async_send(
        &(client->ssl_sock), &(args->async_buf), 1, 
        (CelSslSocketSendCallbackFunc)cel_tcpclient_do_send);
    else
        return cel_socket_async_send(
        &(client->sock), &(args->async_buf), 1, 
        (CelSocketSendCallbackFunc)cel_tcpclient_do_send);
}

int cel_tcpclient_recv(CelTcpClient *client, CelStream *s, CelCoroutine *co)
{
    return 0;
}

void cel_tcpclient_do_recv(CelTcpClient *client, 
                           CelAsyncBuf *buffers, int count, 
                           CelAsyncResult *result)
{
    CelTcpClientAsyncArgs *args = &(client->in);

    if (result->ret > 0)
    {
        cel_stream_seek(args->s, result->ret);
        cel_stream_seal_length(args->s);
    }
    //puts(buffers->buf);
    /*printf("recv ret = %d, buffers %p s %p\r\n",
        result->ret, buffers->buf, args->s->buffer);*/
    args->recv_callback(client, args->s, result);
}

int cel_tcpclient_async_recv(CelTcpClient *client, CelStream *s,
                             CelTcpRecvCallbackFunc async_callback)
{
    CelTcpClientAsyncArgs *args;

    args = &(client->in);
    /*_tprintf(_T("tcpclient %d post recv buf %p, len %ld\r\n"), 
            client->sock.fd, 
            cel_stream_get_pointer(s), cel_stream_get_remaining_capacity(s));*/
    if ((args->async_buf.buf = cel_stream_get_pointer(s))== NULL
        || (args->async_buf.len = cel_stream_get_remaining_capacity(s)) <= 0)
    {
        _tprintf(_T("tcpclient %d recv buf %p, len %ld\r\n"), 
            client->sock.fd, 
            cel_stream_get_pointer(s), cel_stream_get_remaining_capacity(s));
        return -1;
    }
    args->s = s;
    args->recv_callback = async_callback;
    if (client->ssl_sock.ssl != NULL)
        return cel_sslsocket_async_recv(
            &(client->ssl_sock), &(args->async_buf), 1, 
            (CelSslSocketRecvCallbackFunc)cel_tcpclient_do_recv);
    else
        return cel_socket_async_recv(
            &(client->sock), &(args->async_buf), 1, 
            (CelSocketRecvCallbackFunc)cel_tcpclient_do_recv);
}
