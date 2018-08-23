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
#include "cel/net/tcplistener.h"
#include "cel/allocator.h"
#include "cel/error.h"
#undef _CEL_DEBUG
//#define _CEL_DEBUG
#include "cel/log.h"

int cel_tcplistener_init(CelTcpListener *listener, int family, CelSslContext *ssl_ctx)
{
    if ((cel_socket_init(&(listener->sock), family, SOCK_STREAM, IPPROTO_TCP)) == 0)
    {
        if (cel_socket_set_reuseaddr(&(listener->sock), 1) == 0)
        {
            listener->async_args = NULL;
            listener->ssl_ctx = ssl_ctx;
            return 0;
        }
        cel_socket_destroy(&(listener->sock));
    }
    return -1;
}

int cel_tcplistener_init_addr(CelTcpListener *listener,
                              CelSockAddr *addr, CelSslContext *ssl_ctx)
{
    if (cel_tcplistener_init(listener, addr->sa_family, ssl_ctx) == 0)
    {
        if (cel_socket_listen(&(listener->sock), addr, 1024) == 0)
        {
            memcpy(&(listener->addr), addr, cel_sockaddr_get_len(addr));
            return 0;
        }
        cel_socket_destroy(&(listener->sock));
    }
    return -1;
}

int cel_tcplistener_init_str(CelTcpListener *listener,
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
        if (cel_tcplistener_init_addr(
            listener, (CelSockAddr *)addr_info->ai_addr, ssl_ctx) == 0)
            return 0;
        addr_info = addr_info->ai_next;
    }
    FreeAddrInfo(result);

    return -1;
}

void cel_tcplistener_destroy(CelTcpListener *listener)
{
    if (listener->async_args != NULL)
    {
        cel_free(listener->async_args);
        listener->async_args = NULL;
    }
    cel_socket_destroy(&(listener->sock));
}

CelTcpListener *cel_tcplistener_new(int family, CelSslContext *ssl_ctx)
{
    CelTcpListener *listener;

    if ((listener = (CelTcpListener *)cel_malloc(sizeof(CelTcpListener))) != NULL)
    {
        if (cel_tcplistener_init(listener, family, ssl_ctx) != -1)
            return listener;
        cel_free(listener);
    }

    return NULL;
}

CelTcpListener *cel_tcplistener_new_addr(CelSockAddr *addr, CelSslContext *ssl_ctx)
{
    CelTcpListener *listener;

    if ((listener = (CelTcpListener *)cel_malloc(sizeof(CelTcpListener))) != NULL)
    {
        if (cel_tcplistener_init_addr(listener, addr, ssl_ctx) != -1)
            return listener;
        cel_free(listener);
    }

    return NULL;
}

CelTcpListener *cel_tcplistener_new_str(const TCHAR *str, CelSslContext *ssl_ctx)
{
    CelTcpListener *listener;

    if ((listener = (CelTcpListener *)cel_malloc(sizeof(CelTcpListener))) != NULL)
    {
        if (cel_tcplistener_init_str(listener, str, ssl_ctx) != -1)
            return listener;
        cel_free(listener);
    }

    return NULL;
}

void cel_tcplistener_free(CelTcpListener *listener)
{
    cel_tcplistener_destroy(listener);
    cel_free(listener);
}

int cel_tcplistener_accept(CelTcpListener *listener, CelTcpClient *client)
{
    if (cel_socket_accept(
        (CelSocket *)listener, (CelSocket *)client, &((client)->remote_addr)) == 0)
    {
        if (listener->ssl_ctx == NULL
            || cel_sslsocket_init(
            &(client->ssl_sock), &(client->sock), listener->ssl_ctx) == 0)
        {
            memcpy(&(client->local_addr), &(listener->addr), sizeof(CelSockAddr));
            cel_refcounted_init(&(client->ref_counted), 
                (CelFreeFunc)_cel_tcpclient_destroy_derefed);
            return 0;
        }
        cel_socket_destroy(&(client->sock));
    }
    return -1;
}

static void cel_tcplistener_do_accept(CelTcpListener *listener, 
                                      CelTcpClient *new_client, 
                                      CelAsyncResult *result)
{
    CelTcpListenerAsyncArgs *args = listener->async_args;
    CelAsyncResult _result;

    _result.ret = result->ret;
    _result.error = result->error;
    if (result->ret == 0)
    {
        if (listener->ssl_ctx == NULL
            || cel_sslsocket_init(
            &(new_client->ssl_sock), &(new_client->sock), listener->ssl_ctx) == 0)
        {
            if (new_client->ssl_sock.ssl != NULL)
                cel_ssl_set_endpoint(new_client->ssl_sock.ssl, CEL_SSLEP_SERVER);
            memcpy(&(new_client->local_addr), &(listener->addr), sizeof(CelSockAddr));
            cel_socket_get_localaddr(&(new_client->sock), &(new_client->local_addr));
            cel_socket_get_remoteaddr(&(new_client->sock), &(new_client->remote_addr));
            cel_refcounted_init(&(new_client->ref_counted), 
                (CelFreeFunc)_cel_tcpclient_destroy_derefed);
        }
        else
        {
            _result.ret = -1;
            _result.error = cel_sys_geterrno();
            cel_socket_destroy(&(new_client->sock));
        }
    }
    args->async_callback(listener, new_client, &_result);
}

int cel_tcplistener_async_accept(CelTcpListener *listener, CelTcpClient *new_client,
                                 CelTcpAcceptCallbackFunc async_callback)
{
    CelTcpListenerAsyncArgs *args;

    if (listener->async_args == NULL
        && (listener->async_args = (CelTcpListenerAsyncArgs *)
        cel_calloc(1, sizeof(CelTcpListenerAsyncArgs))) == NULL)
        return -1;
    args = listener->async_args;
    args->async_callback = async_callback;
    return cel_socket_async_accept(
        &(listener->sock), &(new_client->sock), 
        (CelSocketAcceptCallbackFunc)cel_tcplistener_do_accept);
}
