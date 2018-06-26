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
#ifdef _CEL_UNIX
#include <sys/sendfile.h>
#include "cel/error.h"

int os_socket_set_keepalive(OsSocket *sock, 
                            int on, int idle, int interval, int count)
{
    int keep_alive = on;
    int keep_idle = idle;
    int keep_interval = interval;
    int keep_count = count;

    if (setsockopt(sock->fd, 
        SOL_SOCKET, SO_KEEPALIVE, (void *)&keep_alive, sizeof(keep_alive)) == -1
        || setsockopt(sock->fd, 
        SOL_TCP, TCP_KEEPIDLE, (void *)&keep_idle, sizeof(keep_idle)) == -1
        || setsockopt(sock->fd, 
        SOL_TCP, TCP_KEEPINTVL, (void *)&keep_interval, sizeof(keep_interval)) == -1
        || setsockopt(sock->fd, 
        SOL_TCP, TCP_KEEPCNT, (void *)&keep_count, sizeof(keep_count)) == -1)
    {
        return -1;
    }
    return 0;
}

int os_socket_do_async_accept(CelSocketAsyncAcceptArgs *args)
{
    if ((args->ol.result = cel_socket_accept(
        args->socket, &(args->accept_socket), (CelSockAddr *)args->buffer)) == -1)
    {
        //_tprintf("errno = %d\r\n", errno);
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_WANTIN);
        else
        {
            args->ol.error = errno;
            CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_ERROR);
        }
        return -1;
    }
    //_tprintf("%ld %p\r\n", args->ol.result, args);
    args->socket->is_connected = TRUE;
    CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_OK);

    return 0;
}

int os_socket_do_async_connect(CelSocketAsyncConnectArgs *args)
{
   /*_tprintf(_T("event %x addr %s len %d fd %d\r\n"), args->ol._ol.events, 
        cel_sockaddr_ntop(&(args->remote_addr)),
        cel_sockaddr_get_len(&(args->remote_addr)), args->socket->fd);*/
    if ((args->ol.result = connect(args->socket->fd, 
        (struct sockaddr *)(&(args->remote_addr)), 
        cel_sockaddr_get_len(&(args->remote_addr)))) == -1)
    {
        //_tprintf(_T("errno = %d\r\n"), cel_sys_geterrno());
        if (errno == EINPROGRESS /*|| errno == EALREADY*/)
            CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_WANTOUT);
        else
        {
            if (POLLERROR_CHK(args->ol._ol.events)
                && (errno == EALREADY || errno == EINVAL))
                cel_seterrno(ETIMEDOUT);
            args->ol.error = errno;
            CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_ERROR);
        }
        return -1;
    }
    args->socket->is_connected = TRUE;
    CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_OK);

    return 0;
}

int os_socket_do_async_send(CelSocketAsyncSendArgs *args)
{
    if ((args->ol.result = writev(args->socket->fd, 
        (struct iovec *)args->buffers, args->buffer_count)) == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_WANTOUT);
        else
        {
            args->ol.error = errno;
            CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_ERROR);
        }
        return -1;
    }
    //_tprintf(_T("send %ld, errno %d\r\n"), args->ol.result, errno);
    CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_OK);

    return 0;
}

int os_socket_do_async_recv(CelSocketAsyncRecvArgs *args)
{
    if ((args->ol.result = readv(args->socket->fd, 
        (struct iovec *)args->buffers, args->buffer_count)) == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_WANTIN);
        else
        {
            args->ol.error = errno;
            CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_ERROR);
        }
        //_tprintf(_T("receive %ld, errno %d\r\n"), args->ol.result, args->ol.error);
        return -1;
    }
    CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_OK);
    //_tprintf(_T("XXXXreceive %ld, errno %d\r\n"), args->ol.result, errno);

    return 0;
}

int os_socket_do_async_sendto(CelSocketAsyncSendToArgs *args)
{
    if ((args->ol.result = sendto(args->socket->fd, 
        args->buffers->buf, args->buffers->len, 0,
        (struct sockaddr *)(args->addr), cel_sockaddr_get_len(args->addr))) == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_WANTOUT);
        else
        {
            args->ol.error = errno;
            CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_ERROR);
        }
        return -1;
    }
    CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_OK);

    return 0;
}

int os_socket_do_async_recvfrom(CelSocketAsyncRecvFromArgs *args)
{
    socklen_t len = sizeof(CelSockAddr);

    if ((args->ol.result = recvfrom(args->socket->fd, 
        args->buffers->buf, args->buffers->len, 0,
        (struct sockaddr *)(&args->addr), &len)) == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_WANTIN);
        else
        {
            args->ol.error = errno;
            CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_ERROR);
        }
        return -1;
    }
    CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_OK);

    return 0;
}

int os_socket_do_async_sendfile(CelSocketAsyncSendFileArgs *args)
{
    if ((args->ol.result 
        = sendfile(args->socket->fd, args->file, &args->offset, args->count)) == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_WANTIN);
        else
        {
            args->ol.error = errno;
            CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_ERROR);
        }
        return -1;
    }
    CEL_POLLSTATE_SET(&(args->ol), CEL_POLLSTATE_OK);

    return 0;
}

#endif