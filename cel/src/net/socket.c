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

void _cel_socket_destroy_derefed(CelSocket *sock)
{
    SOCKET sock_fd;

    sock->state = CEL_SOCKET_INIT;
    if (sock->in != NULL)
    {
        cel_free(sock->in);
        sock->in = NULL;
    }
    if (sock->out != NULL)
    {
        cel_free(sock->out);
        sock->out = NULL;
    }
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
        sock->state = CEL_SOCKET_INIT;
#ifdef _CEL_WIN
        sock->AcceptEx = NULL;
        sock->TransmitFile = NULL;
#endif
        sock->in = sock->out = NULL;
        cel_refcounted_init(&((sock)->ref_counted),
            (CelFreeFunc)_cel_socket_destroy_derefed);

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
        CEL_ERR((_T("GetAddrInfo():%s."), cel_geterrstr(cel_sys_geterrno())));
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
        CEL_ERR((_T("GetAddrInfo():%s."), cel_geterrstr(cel_sys_geterrno())));
        return -1;
    }
    result = addr_info;
    while (addr_info != NULL)
    {
        if (cel_socket_listen(sock, 
            (CelSockAddr *)(addr_info->ai_addr), backlog) == 0)
        {
            return 0;
        }
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
        CEL_ERR((_T("GetAddrInfo():%s."), gai_strerror(ret)));
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

int cel_socket_accept(CelSocket *sock, 
                      CelSocket *client_sock, CelSockAddr *addr)
{
    socklen_t len = sizeof(CelSockAddr);

    if ((client_sock->fd = 
        accept(sock->fd, (struct sockaddr *)addr, &len)) == INVALID_SOCKET)
        return -1;
    //_tprintf(_T("accept %d %s\r\n"), client_sock->fd, cel_sockaddr_ntop(addr));
    client_sock->family = sock->family;
    client_sock->socktype = sock->socktype;
    client_sock->protocol = sock->protocol;
    sock->state = TRUE;
#ifdef _CEL_WIN
    client_sock->AcceptEx = NULL;
    client_sock->TransmitFile = NULL;
#endif
    client_sock->in = client_sock->out = NULL;
    cel_refcounted_init(&(client_sock->ref_counted),
        (CelFreeFunc)_cel_socket_destroy_derefed);
    return 0;
}

void cel_socket_do_accept(CelSocketAsyncAcceptArgs *args)
{
#ifdef _CEL_WIN
    if (args->_ol.result.ret == 0)
    {
        cel_socket_update_acceptcontext(
            args->new_socket, args->socket);
        cel_socket_get_remoteaddr(args->new_socket, args->addr);
        /*memcpy(args->addr, 
            args->addr_buf + ACCEPTEX_LOCALADDRESS_LEN, sizeof(CelSockAddr));*/
    }
#endif
    if (args->async_callback != NULL)
        args->async_callback(
        args->socket, args->new_socket, &(args->_ol.result));
    else
        cel_coroutine_resume(args->co);
}

int cel_socket_async_accept(CelSocket *sock,
                            CelSocket *new_sock, CelSockAddr *addr,
                            CelSocketAcceptCallbackFunc callback,
                            CelCoroutine *co)
{
    CelSocketAsyncArgs *args;

    if (sock->in == NULL
        && (sock->in = (CelSocketAsyncArgs *)
        cel_calloc(1, sizeof(CelSocketAsyncArgs))) == NULL)
        return -1;
    args = sock->in;
    memset(&(args->_ol), 0, sizeof(args->_ol));
    args->_ol.evt_type = CEL_EVENT_CHANNELIN;
    args->_ol.handle_func = (int ( *)(void *))cel_socket_do_async_accept;
    args->_ol.async_callback = (void (*)(void *))cel_socket_do_accept;

    args->accept_args.async_callback = callback;
    args->accept_args.co = co;
    args->accept_args.socket = sock;
    args->accept_args.new_socket = new_sock;
    args->accept_args.addr = addr;
    
    if ((args->accept_args.result.ret = cel_poll_post(
        sock->channel.poll, sock->channel.handle, (CelOverLapped *)args)) == -1)
    {
        CEL_ERR((_T("cel_socket_async_accept poll post failed")));
        return -1;
    }
   if (args->accept_args.co != NULL)
        cel_coroutine_yield(args->accept_args.co);
    return 0;
}

int cel_socket_connect(CelSocket *sock, CelSockAddr *remote_addr)
{

    if (connect((sock)->fd, 
        &((remote_addr)->addr), cel_sockaddr_get_len(remote_addr)) == 0)
    {
        sock->state = TRUE;
        return 0;
    }
    return -1;
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
        CEL_ERR((_T("GetAddrInfo():%s."), cel_geterrstr(cel_sys_geterrno())));
        return -1;
    }
    result = addr_info;
    while (addr_info != NULL)
    {
        if (cel_socket_connect(
            sock, (CelSockAddr *)addr_info->ai_addr) == 0)
            return 0;
        else if (cel_sys_geterrno() == EINPROGRESS)
            return -1;
        addr_info = addr_info->ai_next;
    }
    FreeAddrInfo(result);

    return -1;
}

void cel_socket_do_connect(CelSocketAsyncConnectArgs *args)
{
#ifdef _CEL_WIN
    if (args->_ol.result.ret == 0)
        cel_socket_update_connectcontext(args->socket);
#endif
    memcpy(&(args->result), &(args->_ol.result), sizeof(CelAsyncResult));
    if (args->async_callback != NULL)
        args->async_callback(args->socket, &(args->_ol.result));
    else
        cel_coroutine_resume(args->co);
}

int cel_socket_async_connect(CelSocket *sock, CelSockAddr *addr, 
                             CelSocketConnectCallbackFunc callback,
                             CelCoroutine *co)
{
    CelSocketAsyncArgs *args;

    if (sock->out == NULL
        && (sock->out = (CelSocketAsyncArgs *)
        cel_calloc(1, sizeof(CelSocketAsyncArgs))) == NULL)
    {
        CEL_ERR((_T("cel_socket_async_connect calloc failed")));
        return -1;
    }
    args = sock->out;
    memset(&(args->_ol), 0, sizeof(args->_ol));
    args->_ol.evt_type = CEL_EVENT_CHANNELOUT;
    args->_ol.handle_func = (int ( *)(void *))cel_socket_do_async_connect;
    args->_ol.async_callback = (void (*)(void *))cel_socket_do_connect;

    args->connect_args.async_callback = callback;
    args->connect_args.co = co;
    args->connect_args.socket = sock;
    memcpy(&(args->connect_args.remote_addr), addr, sizeof(CelSockAddr));
    if ((args->connect_args.result.ret = cel_poll_post(
        sock->channel.poll, sock->channel.handle, (CelOverLapped *)args)) == -1)
    {
        CEL_ERR((_T("cel_socket_async_connect poll post failed")));
        return -1;
    }
    if (args->connect_args.co != NULL)
        cel_coroutine_yield(args->connect_args.co);
    return 0;
}

int cel_socket_async_connect_host(CelSocket *sock, 
                                  const TCHAR *host, unsigned short port,
                                  CelSocketConnectCallbackFunc callback,
                                  CelCoroutine *co)
{
    ADDRINFOT *res, *result, hints;
    TCHAR ports[CEL_NPLEN];
    CelSocketAsyncArgs *args;

    if (sock->out == NULL
        && (sock->out = (CelSocketAsyncArgs *)
        cel_calloc(1, sizeof(CelSocketAsyncArgs))) == NULL)
        return -1;
    args = sock->out;
    memset(&(args->_ol), 0, sizeof(args->_ol));
    args->_ol.evt_type = CEL_EVENT_CHANNELOUT;
    args->_ol.handle_func = (int ( *)(void *))cel_socket_do_async_connect;
    args->_ol.async_callback = (void (*)(void *))cel_socket_do_connect;

    args->connect_args.async_callback = callback;
    args->connect_args.co = co;
    args->connect_args.socket = sock;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family =  sock->family;
    hints.ai_socktype = sock->socktype;
    hints.ai_protocol = sock->protocol;
    if (GetAddrInfo(host, _itot(port, ports, 10), &hints, &res) != 0)
    {
        CEL_ERR((_T("Get address information \"%s:%d\" failed.(%s)"), 
            host, port));
        return -1;
    }
    result = res;
    while (res != NULL)
    {
        memcpy(&(args->connect_args.remote_addr), res->ai_addr, res->ai_addrlen);
        //printf("channel handle %d\r\n", sock->channel.handle);
        if ((args->connect_args.result.ret = cel_poll_post(
            sock->channel.poll, sock->channel.handle, (CelOverLapped *)args)) == 0)
        {
            FreeAddrInfo(result);
            if (args->connect_args.co != NULL)
                cel_coroutine_yield(args->connect_args.co);
            return args->connect_args.result.ret;
        }
        res = res->ai_next;
    }
    FreeAddrInfo(result);
    return -1;
}

int cel_socket_send(CelSocket *sock, CelAsyncBuf *buffers, int buffer_count)
{
    return send(sock->fd, buffers->buf, buffers->len, 0);
}

void cel_socket_do_send(CelSocketAsyncSendArgs *args)
{
    memcpy(&(args->result), &(args->_ol.result), sizeof(CelAsyncResult));
    if (args->async_callback != NULL)
        args->async_callback(args->socket, 
        args->buffers, args->buffer_count, &(args->_ol.result));
    else
        cel_coroutine_resume(args->co);
}

int cel_socket_async_send(CelSocket *sock, 
                          CelAsyncBuf *buffers, int buffer_count, 
                          CelSocketSendCallbackFunc callback,
                          CelCoroutine *co)
{
    CelSocketAsyncArgs *args;

    if (sock->out == NULL
        && (sock->out = (CelSocketAsyncArgs *)
        cel_calloc(1, sizeof(CelSocketAsyncArgs))) == NULL)
    {
        CEL_ERR((_T("cel_socket_async_send calloc failed")));
        return -1;
    }
    args = sock->out;
    memset(&(args->_ol), 0, sizeof(args->_ol));
    args->_ol.evt_type = CEL_EVENT_CHANNELOUT;
    args->_ol.handle_func = (int ( *)(void *))cel_socket_do_async_send;
    args->_ol.async_callback = (void (*)(void *))cel_socket_do_send;

    args->send_args.async_callback = callback;
    args->send_args.co = co;
    args->send_args.socket = sock;
    args->send_args.buffers = buffers;
    args->send_args.buffer_count = buffer_count;
    CEL_DEBUG((_T("cel_socket_async_send")));
    if ((args->send_args.result.ret = cel_poll_post(
        sock->channel.poll, sock->channel.handle, (CelOverLapped *)args)) == -1)
    {
        CEL_ERR((_T("cel_socket_async_send poll post failed")));
        return -1;
    }
    if (args->send_args.co != NULL)
        cel_coroutine_yield(args->send_args.co);
    return 0;
}

int cel_socket_recv(CelSocket *sock, CelAsyncBuf *buffers, int buffer_count)
{
    return recv(sock->fd, buffers->buf, buffers->len, 0);
}

void cel_socket_do_recv(CelSocketAsyncRecvArgs *args)
{
    memcpy(&(args->result), &(args->_ol.result), sizeof(CelAsyncResult));
    if (args->async_callback != NULL)
        args->async_callback(args->socket, args->buffers, args->buffer_count, 
        &(args->_ol.result));
    else
        cel_coroutine_resume(args->co);
}

int cel_socket_async_recv(CelSocket *sock, 
                          CelAsyncBuf *buffers, int buffer_count, 
                          CelSocketRecvCallbackFunc callback,
                          CelCoroutine *co)
{
    CelSocketAsyncArgs *args;

    if (sock->in == NULL
        && (sock->in = (CelSocketAsyncArgs *)
        cel_calloc(1, sizeof(CelSocketAsyncArgs))) == NULL)
    {
        CEL_ERR((_T("cel_socket_async_recv calloc failed")));
        return -1;
    }
    args = sock->in;
    memset(&(args->_ol), 0, sizeof(args->_ol));
    args->_ol.evt_type = CEL_EVENT_CHANNELIN;
    args->_ol.handle_func = (int ( *)(void *))cel_socket_do_async_recv;
    args->_ol.async_callback = (void (*)(void *))cel_socket_do_recv;

    args->recv_args.async_callback = callback;
    args->recv_args.co = co;
    args->recv_args.socket = sock;
    args->recv_args.buffers = buffers;
    args->recv_args.buffer_count = buffer_count;
    CEL_DEBUG((_T("cel_socket_async_recv")));
    if ((args->recv_args.result.ret = cel_poll_post(
        sock->channel.poll, sock->channel.handle, (CelOverLapped *)args)) == -1)
    {
        CEL_ERR((_T("cel_socket_async_recv poll post failed")));
        return -1;
    }
    if (args->recv_args.co != NULL)
        cel_coroutine_yield(args->recv_args.co);
    return 0;
}


int cel_socket_sendto(CelSocket *sock, 
                      CelAsyncBuf *buffers, int buffer_count, 
                      CelSockAddr *to)
{
    return sendto(sock->fd, buffers->buf, buffers->len, 0,
        (struct sockaddr *)to, cel_sockaddr_get_len(to));
}

void cel_socket_do_sendto(CelSocketAsyncSendToArgs *args)
{
    memcpy(&(args->result), &(args->_ol.result), sizeof(CelAsyncResult));
    if (args->async_callback != NULL)
        args->async_callback(args->socket, args->buffers, args->buffer_count, 
        args->addr, &(args->_ol.result));
    else
        cel_coroutine_resume(args->co);
}

int cel_socket_async_sendto(CelSocket *sock, 
                            CelAsyncBuf *buffers, int buffer_count, 
                            CelSockAddr *to, 
                            CelSocketSendToCallbackFunc callback,
                            CelCoroutine *co)
{
    CelSocketAsyncArgs *args;

    if (sock->out == NULL
        && (sock->out = (CelSocketAsyncArgs *)
        cel_calloc(1, sizeof(CelSocketAsyncArgs))) == NULL)
    {
        CEL_ERR((_T("cel_socket_async_sendto calloc failed")));
        return -1;
    }
    args = sock->out;
    memset(&(args->_ol), 0, sizeof(args->_ol));
    args->_ol.evt_type = CEL_EVENT_CHANNELOUT;
    args->_ol.handle_func = (int ( *)(void *))cel_socket_do_async_sendto;
    args->_ol.async_callback = (void (*)(void *))cel_socket_do_sendto;

    args->sendto_args.async_callback = callback;
    args->sendto_args.co = co;
    args->sendto_args.socket = sock;
    args->sendto_args.buffers = buffers;
    args->sendto_args.buffer_count = buffer_count;
    args->sendto_args.addr = to;
    if ((args->send_args.result.ret = cel_poll_post(
        sock->channel.poll, sock->channel.handle, (CelOverLapped *)args)) == -1)
    {
        CEL_ERR((_T("cel_socket_async_sendto poll post failed")));
        return -1;
    }
    if (args->sendto_args.co != NULL)
        cel_coroutine_yield(args->sendto_args.co);
    return 0;
}

int cel_socket_recvfrom(CelSocket *sock, CelAsyncBuf *buffers, int buffer_count, 
                        CelSockAddr *from)
{
    socklen_t len = sizeof(CelSockAddr);

    return recvfrom(sock->fd, 
        buffers->buf, buffers->len, 0, (struct sockaddr *)from, &len);
}

void cel_socket_do_recvfrom(CelSocketAsyncRecvFromArgs *args)
{
    memcpy(&(args->result), &(args->_ol.result), sizeof(CelAsyncResult));
    if (args->async_callback != NULL)
        args->async_callback(args->socket, args->buffers, args->buffer_count, 
        args->addr, &(args->_ol.result));
    else
        cel_coroutine_resume(args->co);
}

int cel_socket_async_recvfrom(CelSocket *sock, 
                              CelAsyncBuf *buffers, int buffer_count, 
                              CelSockAddr *from, 
                              CelSocketRecvFromCallbackFunc callback,
                              CelCoroutine *co)
{
    CelSocketAsyncArgs *args;

    if (sock->in == NULL
        && (sock->in = (CelSocketAsyncArgs *)
        cel_calloc(1, sizeof(CelSocketAsyncArgs))) == NULL)
    {
        CEL_ERR((_T("cel_socket_async_recvfrom calloc failed")));
        return -1;
    }
    args = sock->in;
    memset(&(args->_ol), 0, sizeof(args->_ol));
    args->_ol.evt_type = CEL_EVENT_CHANNELIN;
    args->_ol.handle_func = (int ( *)(void *))cel_socket_do_async_recvfrom;
    args->_ol.async_callback = (void (*)(void *))cel_socket_do_recvfrom;

    args->recvfrom_args.async_callback = callback;
    args->recvfrom_args.co = co;
    args->recvfrom_args.socket = sock;
    args->recvfrom_args.buffers = buffers;
    args->recvfrom_args.buffer_count = buffer_count;
    args->recvfrom_args.addr = from;
    if ((args->recvfrom_args.result.ret = cel_poll_post(
        sock->channel.poll, sock->channel.handle, (CelOverLapped *)args)) == -1)
    {
        CEL_ERR((_T("cel_socket_async_recvfrom poll post failed")));
        return -1;
    }
    if (args->recvfrom_args.co != NULL)
        cel_coroutine_yield(args->recvfrom_args.co);
    return 0;
}

void cel_socket_do_sendfile(CelSocketAsyncSendFileArgs *args)
{
    memcpy(&(args->result), &(args->_ol.result), sizeof(CelAsyncResult));
    if (args->async_callback != NULL)
        args->async_callback(args->socket, &(args->_ol.result));
    else
        cel_coroutine_resume(args->co);
}

int cel_socket_async_sendfile(CelSocket *sock, 
                              const TCHAR *path, 
                              long long first, long long last, 
                              CelSocketSendFileCallbackFunc callback,
                              CelCoroutine *co)
{
    CelSocketAsyncArgs *args;

    if (sock->out == NULL
        && (sock->out = (CelSocketAsyncArgs *)
        cel_calloc(1, sizeof(CelSocketAsyncArgs))) == NULL)
    {
        CEL_ERR((_T("cel_socket_async_sendfile calloc failed")));
        return -1;
    }
    args = sock->out;
    memset(&(args->_ol), 0, sizeof(args->_ol));
    args->_ol.evt_type = CEL_EVENT_CHANNELOUT;
    args->_ol.handle_func = (int ( *)(void *))cel_socket_do_async_sendfile;
    args->_ol.async_callback = (void (*)(void *))cel_socket_do_sendfile;

    args->sendfile_args.async_callback = callback;
    args->sendfile_args.co = co;
    args->sendfile_args.socket = sock;
    //args->sendfile_args. = NULL;
    //args->sendfile_args. = 0;
    //args->sendfile_args. = ;
    if ((args->sendfile_args.result.ret = cel_poll_post(
        sock->channel.poll, sock->channel.handle, (CelOverLapped *)args)) == -1)
    {
        CEL_ERR((_T("cel_socket_async_sendfile poll post failed")));
        return -1;
    }
    if (args->sendfile_args.co != NULL)
        cel_coroutine_yield(args->sendfile_args.co);
    return 0;
}
