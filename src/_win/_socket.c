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
#ifdef _CEL_WIN
#include <Mstcpip.h>
#include "cel/log.h"
#include "cel/error.h"
#include "cel/allocator.h"

/* Debug defines */
#define Debug(args)   cel_log_debug args
#define Warning(args) /*CEL_SETERRSTR(args)*/ cel_log_warning args 
#define Err(args)  /* CEL_SETERRSTR(args)*/ cel_log_err args


/* https://msdn.microsoft.com/en-us/library/dd877220 */
int os_socket_set_keepalive(CelSocket *sock, 
                            int on, int idle, int interval, int count)
{
    struct tcp_keepalive keepalive_in;
    DWORD bytes;

    keepalive_in.onoff = on;
    keepalive_in.keepalivetime = idle * 1000; /* milliseconds */
    keepalive_in.keepaliveinterval = interval * 1000; /* milliseconds */
    if (WSAIoctl(sock->fd, SIO_KEEPALIVE_VALS, 
        &keepalive_in, sizeof(keepalive_in), NULL, 0, &bytes, NULL, NULL) != 0)
    {
        return -1;
    }
    return 0;
}

int os_socket_do_async_accept(CelSocketAsyncAcceptArgs *args)
{
    DWORD dwBytes;
    GUID GuidAcceptEx = WSAID_ACCEPTEX;

    if (args->socket->AcceptEx == NULL
        && WSAIoctl(args->socket->fd, 
        SIO_GET_EXTENSION_FUNCTION_POINTER, 
        &GuidAcceptEx, sizeof(GuidAcceptEx), 
        &args->socket->AcceptEx, sizeof(LPFN_ACCEPTEX), 
        &dwBytes, NULL, NULL) == SOCKET_ERROR)
    {
        //puts("WSAIoctl error");
        return -1;
    }
    if (cel_socket_init(&(args->accept_socket), 
        args->socket->family, 
        args->socket->socktype, 
        args->socket->protocol) == -1)
    {
        //puts("cel_socket_init error");
        return -1;
    }
    /*Debug((_T("acceptex fd %d, accept_fd %d, buf size %d\r\n"),
        args->socket->fd, args->accept_socket.fd,
        (DWORD)(args->buffer_size - ACCEPTEX_RECEIVEDATA_OFFSET)));*/
    if (!(args->socket->AcceptEx(args->socket->fd, args->accept_socket.fd, 
        args->buffer, (DWORD)(args->buffer_size - ACCEPTEX_RECEIVEDATA_OFFSET),
        ACCEPTEX_LOCALADDRESS_LEN, ACCEPTEX_REMOTEADDRESS_LEN, 
        &(args->ol.result), &(args->ol._ol))) 
        && WSAGetLastError() != WSA_IO_PENDING)
    {
        //Err((_T("AcceptEx error")));
        return -1;
    }
    args->socket->is_connected = TRUE;

    return 0;
}

int os_socket_do_async_connect(CelSocketAsyncConnectArgs *args)
{
    DWORD dwBytes;
    GUID GuidConnectEx = WSAID_CONNECTEX;
    LPFN_CONNECTEX ConnectEx;
    CelSockAddr addr;

    if (WSAIoctl(args->socket->fd, 
        SIO_GET_EXTENSION_FUNCTION_POINTER, 
        &GuidConnectEx, sizeof(GuidConnectEx), 
        &ConnectEx, sizeof(LPFN_CONNECTEX), &dwBytes, NULL, NULL)
        == SOCKET_ERROR)
    {
        //_tprintf(_T("WSAIoctl error %d\r\n"), WSAGetLastError());
        return -1;
    }
    memset(&addr, 0, sizeof(CelSockAddr));
    addr.sa_family = args->remote_addr.sa_family;
    if (cel_socket_bind(args->socket, &addr) != 0
        || !ConnectEx(args->socket->fd, 
        (struct sockaddr *)(&args->remote_addr), 
        cel_sockaddr_get_len(&args->remote_addr),
        args->buffer, (DWORD)args->buffer_size, 
        &(args->ol.result), &(args->ol._ol))
        && WSAGetLastError() != WSA_IO_PENDING)
    {
        //_tprintf(_T("WSAIoctl error %d\r\n"), WSAGetLastError());
        return -1;
    }
    args->socket->is_connected = TRUE;

    return 0;
}

int os_socket_do_async_send(CelSocketAsyncSendArgs *args)
{
    int iRet, iErr = 0;

    if ((iRet = WSASend(args->socket->fd, 
        (LPWSABUF)args->buffers, args->buffer_count,
        NULL, 0, &(args->ol._ol), NULL)) == -1
        && (iErr = WSAGetLastError()) != WSA_IO_PENDING)
    {
        //_tprintf(_T("WSAGetLastError() %d\r\n"), iErr);
        return -1;
    }
    //_tprintf(_T("s = %d, errno %d\r\n"), iRet,  iErr);

    return 0;
}

int os_socket_do_async_recv(CelSocketAsyncRecvArgs *args)
{
    DWORD flags = 0;
    if (WSARecv(args->socket->fd, 
        (LPWSABUF)args->buffers, args->buffer_count,
        NULL, &flags, &(args->ol._ol), NULL) == SOCKET_ERROR
        && WSAGetLastError() != WSA_IO_PENDING)
        return -1;
    return 0;
}

int os_socket_do_async_sendto(CelSocketAsyncSendToArgs *args)
{
    if (WSASendTo(args->socket->fd, 
        (LPWSABUF)args->buffers, args->buffer_count, NULL, 0, 
        (struct sockaddr *)(args->addr), cel_sockaddr_get_len(args->addr),
        &(args->ol._ol), NULL) == -1
        && WSAGetLastError() == WSA_IO_PENDING)
        return -1;
    return 0;
}

int os_socket_do_async_recvfrom(CelSocketAsyncRecvFromArgs *args)
{
    socklen_t len = sizeof(CelSockAddr);
    if (WSARecvFrom(args->socket->fd, 
        (LPWSABUF)args->buffers, args->buffer_count, NULL, 0, 
        (struct sockaddr *)(&args->addr), &len, &(args->ol._ol), NULL) == -1
        && WSAGetLastError() != WSA_IO_PENDING)
        return -1;

    return 0;
}

int os_socket_do_async_sendfile(CelSocketAsyncSendFileArgs *args)
{
    DWORD dwBytes;
    GUID GuidTransmitFile = WSAID_TRANSMITFILE;

    if (args->socket->TransmitFile == NULL
        && WSAIoctl(args->socket->fd, 
        SIO_GET_EXTENSION_FUNCTION_POINTER, 
        &GuidTransmitFile, sizeof(GuidTransmitFile), 
        &args->socket->TransmitFile, sizeof(LPFN_TRANSMITFILE), 
        &dwBytes, NULL, NULL) == SOCKET_ERROR)
        return -1;
    args->ol._ol.Offset = (DWORD)args->offset;
    args->ol._ol.OffsetHigh = (DWORD)(args->offset >> 32);
    if (args->socket->TransmitFile(args->socket->fd, 
        args->file, args->count, 4096,
        &(args->ol._ol), NULL, TF_USE_SYSTEM_THREAD)
        && WSAGetLastError() != WSA_IO_PENDING)
        return -1;

    return 0;
}

#endif