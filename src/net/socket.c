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
#include "cel/net/socket.h"
#include "cel/allocator.h"
#include "cel/log.h"
#include "cel/error.h"
#include "cel/convert.h"

/* Debug defines */
#define Debug(args)   /* cel_log_debug args */
#define Warning(args) CEL_SETERRSTR(args) /* cel_log_warning args */
#define Err(args)   CEL_SETERRSTR(args) /* cel_log_err args */

void _cel_socket_destroy_derefed(CelSocket *sock)
{
    SOCKET sock_fd;

    sock->is_connected = FALSE;
    if ((sock_fd = sock->fd) != INVALID_SOCKET)
    {
        //puts("xx");
        sock->fd = INVALID_SOCKET;
        closesocket(sock_fd);
    } 
}

void _cel_socket_free_derefed(CelSocket *sock)
{
    _cel_socket_destroy_derefed(sock);
    cel_free(sock);
}

int cel_socket_init(CelSocket *sock, int family, int socktype, int protocol)
{
    SOCKET fd;

    if ((fd =         
#ifdef _CEL_WIN
        WSASocket(family, socktype, protocol, NULL, 0, WSA_FLAG_OVERLAPPED)
#else
        socket(family, socktype, protocol)
#endif
    ) != INVALID_SOCKET)
    {
        memset(&(sock->channel), 0, sizeof(CelChannel));
        sock->fd = fd;
        sock->family = family;
        sock->socktype = socktype;
        sock->protocol = protocol;
        sock->is_connected = FALSE;
#ifdef _CEL_WIN
        sock->AcceptEx = NULL;
        sock->TransmitFile = NULL;
#endif
        cel_refcounted_init(&((sock)->ref_counted), (CelFreeFunc)_cel_socket_destroy_derefed);

        return 0;
    }
    return -1;
}

void cel_socket_destroy(CelSocket *sock)
{
    //printf("%ld %p\r\n", sock->ref_counted.cnt, sock->ref_counted.free_func);
    cel_refcounted_destroy(&((sock)->ref_counted), sock);
}

CelSocket *cel_socket_new(int family, int socktype, int protocol)
{
     CelSocket *sock;

    if ((sock = (CelSocket *)cel_malloc(sizeof(CelSocket))) != NULL)
    {
        if (cel_socket_init(sock, family, socktype, protocol) != -1)
        {
            cel_refcounted_init(&((sock)->ref_counted),
                (CelFreeFunc)_cel_socket_free_derefed);
            return sock;
        }
        cel_free(sock);
    }

    return NULL;
}

void cel_socket_free(CelSocket *sock)
{
    cel_refcounted_destroy(&((sock)->ref_counted), sock);
}

int cel_socket_bind_host(CelSocket *sock, 
                         const TCHAR *host, unsigned short port)
{
    ADDRINFOT *addr_info, *result, hints;
    TCHAR ports[CEL_NPLEN];

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = sock->family;
    hints.ai_socktype = sock->socktype;
    hints.ai_protocol = sock->protocol;
    if (GetAddrInfo(host, _itot(port, ports, 10), &hints, &addr_info) != 0)
    {
        Err((_T("GetAddrInfo():%s."), cel_geterrstr(cel_sys_geterrno())));
        return -1;
    }
    result = addr_info;
    while (addr_info != NULL)
    {
        if (cel_socket_bind(sock, (CelSockAddr *)addr_info->ai_addr) == 0)
            return 0;
        addr_info = addr_info->ai_next;
    }
    FreeAddrInfo(result);

    return -1;
}

int cel_socket_listen_host(CelSocket *sock, 
                           const TCHAR *host, unsigned short port, int backlog)
{
    ADDRINFOT *addr_info, *result, hints;
    TCHAR ports[CEL_NPLEN];

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = sock->family;
    hints.ai_socktype = sock->socktype;
    hints.ai_protocol = sock->protocol;
    if (GetAddrInfo(host, _itot(port, ports, 10), &hints, &addr_info) != 0)
    {
        Err((_T("GetAddrInfo():%s."), cel_geterrstr(cel_sys_geterrno())));
        return -1;
    }
    result = addr_info;
    while (addr_info != NULL)
    {
        if (cel_socket_listen(sock, 
            (CelSockAddr *)(addr_info->ai_addr), backlog) == 0)
            return 0;
        addr_info = addr_info->ai_next;
    }
    FreeAddrInfo(result);

    return -1;
}

int cel_socket_listen_str(CelSocket *sock, const TCHAR *str, int backlog)
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
        if (cel_socket_listen(
            sock, (CelSockAddr *)addr_info->ai_addr, backlog) == 0)
            return 0;
        addr_info = addr_info->ai_next;
    }
    FreeAddrInfo(result);

    return 0;
}

int cel_socket_connect_host(CelSocket *sock, 
                            const TCHAR *host, unsigned short port)
{
    ADDRINFOT *addr_info, *result, hints;
    TCHAR ports[CEL_NPLEN];

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = sock->family;
    hints.ai_socktype = sock->socktype;
    hints.ai_protocol = sock->protocol;
    if (GetAddrInfo(host, _itot(port, ports, 10), &hints, &addr_info) != 0)
    {
        Err((_T("GetAddrInfo():%s."), cel_geterrstr(cel_sys_geterrno())));
        return -1;
    }
    result = addr_info;
    while (addr_info != NULL)
    {
        if (cel_socket_connect(sock, (CelSockAddr *)addr_info->ai_addr) == 0)
            return 0;
        else if (cel_sys_geterrno() == EINPROGRESS)
            return -1;
        addr_info = addr_info->ai_next;
    }
    FreeAddrInfo(result);

    return -1;
}

int cel_socket_accept(CelSocket *sock, 
                      CelSocket *client_sock, CelSockAddr *addr)
{
    socklen_t len = sizeof(CelSockAddr);

    if ((client_sock->fd = 
        accept(sock->fd, (struct sockaddr *)addr, &len)) == INVALID_SOCKET)
        return -1;
    //_tprintf(_T("accept %d\r\n"), client_sock->fd);
    client_sock->family = sock->family;
    client_sock->socktype = sock->socktype;
    client_sock->protocol = sock->protocol;
    sock->is_connected = TRUE;
#ifdef _CEL_WIN
    client_sock->AcceptEx = NULL;
    client_sock->TransmitFile = NULL;
#endif
    cel_refcounted_init(&(client_sock->ref_counted),
        (CelFreeFunc)_cel_socket_destroy_derefed);

    return 0;
}

int cel_socket_async_accept(CelSocketAsyncAcceptArgs *args)
{
    CelChannel *channel = &(args->socket->channel);

    memset(&(args->ol), 0, sizeof(args->ol));
    args->ol.evt_type = CEL_EVENT_CHANNELIN;
    args->ol.handle_func = (int ( *)(void *))cel_socket_do_async_accept;
    return cel_poll_post(channel->poll, 
        channel->handle, (CelOverLapped *)args);
}

int cel_socket_async_connect(CelSocketAsyncConnectArgs *args)
{
    CelChannel *channel = &(args->socket->channel);

    memset(&(args->ol), 0, sizeof(args->ol));
    args->ol.evt_type = CEL_EVENT_CHANNELOUT;
    args->ol.handle_func = (int ( *)(void *))cel_socket_do_async_connect;
    return cel_poll_post(channel->poll, 
        channel->handle, (CelOverLapped *)args);
}

int cel_socket_async_connect_host(CelSocketAsyncConnectArgs *args)
{
    CelChannel *channel = &(args->socket->channel);
    ADDRINFOT *res, *result, hints;

    memset(&(args->ol), 0, sizeof(args->ol));
    args->ol.evt_type = CEL_EVENT_CHANNELOUT;
    args->ol.handle_func = (int ( *)(void *))cel_socket_do_async_connect;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family =  args->socket->family;
    hints.ai_socktype = args->socket->socktype;
    hints.ai_protocol = args->socket->protocol;
    if (GetAddrInfo(args->host, args->service, &hints, &res) != 0)
    {
        Err((_T("Get address information \"%s:%s\" failed.(%s)"), 
            args->host, args->service));
        return -1;
    }
    result = res;
    while (res != NULL)
    {
        memcpy(&(args->remote_addr), res->ai_addr, res->ai_addrlen); 
        if (cel_poll_post(channel->poll,
            channel->handle, (CelOverLapped *)args) == 0)
        {
            FreeAddrInfo(result);
            return 0;
        }
        res = res->ai_next;
    }
    FreeAddrInfo(result);
    return -1;
}

int cel_socket_async_send(CelSocketAsyncSendArgs *args)
{
    CelChannel *channel = &(args->socket->channel);

    memset(&(args->ol), 0, sizeof(args->ol));
    args->ol.evt_type = CEL_EVENT_CHANNELOUT;
    args->ol.handle_func = (int ( *)(void *))cel_socket_do_async_send;
    return cel_poll_post(channel->poll, 
        channel->handle, (CelOverLapped *)args);
}

int cel_socket_async_recv(CelSocketAsyncRecvArgs *args)
{
    CelChannel *channel = &(args->socket->channel);

    memset(&(args->ol), 0, sizeof(args->ol));
    args->ol.evt_type = CEL_EVENT_CHANNELIN;
    args->ol.handle_func = (int ( *)(void *))cel_socket_do_async_recv;
    return cel_poll_post(channel->poll, 
        channel->handle, (CelOverLapped *)args);
}

int cel_socket_async_sendto(CelSocketAsyncSendToArgs *args)
{
    CelChannel *channel = &(args->socket->channel);

    memset(&(args->ol), 0, sizeof(args->ol));
    args->ol.evt_type = CEL_EVENT_CHANNELOUT;
    args->ol.handle_func = (int ( *)(void *))cel_socket_do_async_sendto;
    return cel_poll_post(channel->poll, 
        channel->handle, (CelOverLapped *)args);
}

int cel_socket_async_recvfrom(CelSocketAsyncRecvFromArgs *args)
{
    CelChannel *channel = &(args->socket->channel);

    memset(&(args->ol), 0, sizeof(args->ol));
    args->ol.evt_type = CEL_EVENT_CHANNELIN;
    args->ol.handle_func = (int ( *)(void *))cel_socket_do_async_recvfrom;
    return cel_poll_post(channel->poll, 
        channel->handle, (CelOverLapped *)args);
}

int cel_socket_async_sendfile(CelSocketAsyncSendFileArgs *args)
{
    CelChannel *channel = &(args->socket->channel);

    memset(&(args->ol), 0, sizeof(args->ol));
    args->ol.evt_type = CEL_EVENT_CHANNELOUT;
    args->ol.handle_func = (int ( *)(void *))cel_socket_do_async_sendfile;
    return cel_poll_post(channel->poll, 
        channel->handle, (CelOverLapped *)args);
}
