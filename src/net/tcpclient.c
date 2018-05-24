/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2016 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

/* Debug defines */
#define Debug(args)   /* cel_log_debug args */
#define Warning(args) CEL_SETERRSTR(args)/* cel_log_warning args */
#define Err(args)   CEL_SETERRSTR(args)/* cel_log_err args */

void _cel_tcpclient_destroy_derefed(CelTcpClient *client)
{
    //puts("_cel_tcpclient_destroy_derefed");
    if (client->in != NULL)
    {
        cel_free(client->in);
        client->in = NULL;
    }
    if (client->out != NULL)
    {
        cel_free(client->out);
        client->out = NULL;
    }
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
            client->in = client->out = NULL;
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
        Err((_T("GetAddrInfo():%s."), gai_strerror(ret)));
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

void cel_tcpclient_do_connect(CelTcpClientAsyncArgs *args)
{
    CelAsyncResult _result;
    CelTcpClient *client = args->client;

    _result.ret = args->connect_args.ol.result;
    _result.error = args->connect_args.ol.error;
    if (_result.ret == 0)
    {
        if (client->ssl_sock.use_ssl)
            cel_ssl_set_endpoint(client->ssl_sock.ssl, CEL_SSLEP_CLIENT);
#ifdef _CEL_WIN
        cel_socket_update_connectcontext(&(client->sock));
#endif
        cel_socket_get_localaddr(&(client->sock), &(client->local_addr));
        cel_socket_get_remoteaddr(&(client->sock), &(client->remote_addr));
    }
    args->connect_callback(client, &_result);
}

int cel_tcpclient_async_connect(CelTcpClient *client, CelSockAddr *remote_addr,
                                CelTcpConnectCallbackFunc async_callback)
{
    CelTcpClientAsyncArgs *args;

    if (client->out == NULL
        && (client->out = (CelTcpClientAsyncArgs *)
        cel_calloc(1, sizeof(CelTcpClientAsyncArgs))) == NULL)
        return -1;
    args = client->out;
    args->connect_callback = async_callback;
    args->client = client;
    args->connect_args.socket = &(client->sock);
    args->connect_args.async_callback = (void (*) (void *))cel_tcpclient_do_connect;
    memcpy(&(args->connect_args.remote_addr), remote_addr, sizeof(CelSockAddr));
    return cel_socket_async_connect(&(args->connect_args));
}

int cel_tcpclient_async_connect_host(CelTcpClient *client, 
                                     const TCHAR *host, unsigned short port,
                                     CelTcpConnectCallbackFunc async_callback)
{
    TCHAR p_str[CEL_NPLEN];
    CelTcpClientAsyncArgs *args;

    if (client->out == NULL
        && (client->out = (CelTcpClientAsyncArgs *)
        cel_calloc(1, sizeof(CelTcpClientAsyncArgs))) == NULL)
        return -1;
    args = client->out;
    args->connect_callback = async_callback;
    args->client = client;
    args->connect_args.socket = &(client->sock);
    args->connect_args.async_callback = (void (*) (void *))cel_tcpclient_do_connect;
    _tcsncpy(args->connect_args.host, host, CEL_HNLEN);
    _tcsncpy(args->connect_args.service, _itot(port, p_str, 10), CEL_NPLEN); 
    return cel_socket_async_connect_host(&(args->connect_args));
}

void cel_tcpclient_do_handshake(CelTcpClientAsyncArgs *args)
{
    CelAsyncResult _result;

    _result.ret = args->ssl_handshake_args.result;
    _result.error = args->ssl_handshake_args.error;
    args->handshake_callback(args->client, &_result);
}

int cel_tcpclient_async_handshake(CelTcpClient *client, 
                                  CelTcpHandshakeCallbackFunc async_callback)
{
    CelTcpClientAsyncArgs *args;
    CelAsyncResult _result;

    if (client->in == NULL
        && (client->in = (CelTcpClientAsyncArgs *)
        cel_calloc(1, sizeof(CelTcpClientAsyncArgs))) == NULL)
        return -1;
    args = client->in;
    if (client->ssl_sock.use_ssl)
    {
        args->handshake_callback = async_callback;
        args->client = client;
        args->ssl_handshake_args.ssl_sock = &(client->ssl_sock);
        args->ssl_handshake_args.async_callback = 
            (void (*) (void *))cel_tcpclient_do_handshake;
        return cel_sslsocket_async_handshake(&(args->ssl_handshake_args));
    }
    else
    {
        _result.error = 0;
        _result.ret = 1;
        async_callback(client, &_result);
    }
    return 0;
}

void cel_tcpclient_do_send(CelTcpClientAsyncArgs *args)
{
    CelAsyncResult _result;

    if (args->client->ssl_sock.use_ssl)
    {
        _result.ret = args->ssl_send_args.result;
        _result.error = args->ssl_send_args.error;
        //_tprintf("tcp ssl client send %ld\r\n", _result.ret);
    }
    else
    {
        _result.ret = args->send_args.ol.result;
        _result.error = args->send_args.ol.error;
        //_tprintf("tcp client send %ld\r\n", _result.ret);
    }
    if (_result.ret > 0)
        cel_stream_seek(args->s, _result.ret);
    
    args->send_callback(args->client, args->s, &_result);
}

int cel_tcpclient_async_send(CelTcpClient *client, CelStream *s, 
                             CelTcpSendCallbackFunc async_callback)
{
    CelTcpClientAsyncArgs *args;

    if (client->out == NULL
        && (client->out = (CelTcpClientAsyncArgs *)
        cel_calloc(1, sizeof(CelTcpClientAsyncArgs))) == NULL)
        return -1;
    args = client->out;
    /*_tprintf(_T("tcpclient %d send buf %p, len %ld\r\n"), 
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
    args->client = client;
    args->send_callback = async_callback;
    if (client->ssl_sock.use_ssl)
    {
        args->ssl_send_args.ssl_sock = &(client->ssl_sock);
        args->ssl_send_args.async_callback = 
            (void (*) (void *))cel_tcpclient_do_send;
        args->ssl_send_args.buffers = &(args->async_buf);
        args->ssl_send_args.buffer_count = 1;
        return cel_sslsocket_async_send(&(args->ssl_send_args));
    }
    else
    {
        args->send_args.socket = &(client->sock);
        args->send_args.async_callback = (void (*) (void *))cel_tcpclient_do_send;
        args->send_args.buffers = &(args->async_buf);
        args->send_args.buffer_count = 1;
        return cel_socket_async_send(&(args->send_args));
    }
}

void cel_tcpclient_do_recv(CelTcpClientAsyncArgs *args)
{
    CelAsyncResult _result;

    if (args->client->ssl_sock.use_ssl)
    {
        _result.ret = args->ssl_recv_args.result;
        _result.error = args->ssl_recv_args.error;
    }
    else
    {
        _result.ret = args->recv_args.ol.result;
        _result.error = args->recv_args.ol.error;
    }
    if (_result.ret > 0)
    {
        cel_stream_seek(args->s, _result.ret);
        cel_stream_seal_length(args->s);
    }
    args->recv_callback(args->client, args->s, &_result);
}

int cel_tcpclient_async_recv(CelTcpClient *client, CelStream *s,
                             CelTcpRecvCallbackFunc async_callback)
{
    CelTcpClientAsyncArgs *args;

    if (client->in == NULL
        && (client->in = (CelTcpClientAsyncArgs *)
        cel_calloc(1, sizeof(CelTcpClientAsyncArgs))) == NULL)
        return -1;
    args = client->in;
    if ((args->async_buf.buf = cel_stream_get_pointer(s))== NULL
        || (args->async_buf.len = cel_stream_get_remaining_capacity(s)) <= 0)
    {
        _tprintf(_T("tcpclient %d recv buf %p, len %ld\r\n"), 
            client->sock.fd, 
            cel_stream_get_pointer(s), cel_stream_get_remaining_capacity(s));
        return -1;
    }
    args->s = s;
    args->client = client;
    args->send_callback = async_callback;
    if (client->ssl_sock.ssl != NULL)
    {
        args->ssl_recv_args.ssl_sock = &(client->ssl_sock);
        args->ssl_recv_args.async_callback = 
            (void (*) (void *))cel_tcpclient_do_recv;
        args->ssl_recv_args.buffers = &(args->async_buf);
        args->ssl_recv_args.buffer_count = 1;
        return cel_sslsocket_async_recv(&(args->ssl_recv_args));
    }
    else
    {
        args->recv_args.socket = &(client->sock);
        args->recv_args.async_callback = 
            (void (*) (void *))cel_tcpclient_do_recv;
        args->recv_args.buffers = &(args->async_buf);
        args->recv_args.buffer_count = 1;
        return cel_socket_async_recv(&(args->recv_args));
    }
}
