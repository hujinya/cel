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
#ifndef __CEL_NET_SOCKET_UNIX_H__
#define __CEL_NET_SOCKET_UNIX_H__

#include "cel/types.h"
#include "cel/refcounted.h"
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <asm/ioctls.h>
#include <sys/ioctl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int SOCKET;
#define INVALID_SOCKET -1

#define os_wsastartup()
#define os_wsacleanup()
/* void closesocket(int sockfd)*/
#define closesocket(sockfd) close(sockfd)
/* 
 * int ioctlsocket(int sock, long cmd, unsigned long *argp)
 * cmd :FIONREAD - get # bytes to read;
 *      FIONBIO - set/clear non-blocking i/o;
 *      FIOASYNC -  set/clear async i/o;
 */
#define ioctlsocket(sockfd, cmd, argp) ioctl(sockfd, cmd, argp)

typedef struct _OsSocket
{
    union {
        SOCKET fd;
        CelChannel channel;
    };
    int family, socktype, protocol;
    BOOL is_connected;
    CelSocketAsyncArgs *in, *out;
    CelRefCounted ref_counted;
}OsSocket;

int os_socket_set_keepalive(OsSocket *sock, int on, 
                            int idle_seconds, int interval_seconds, int count);

int os_socket_do_async_accept(CelSocketAsyncAcceptArgs *args);
int os_socket_do_async_connect(CelSocketAsyncConnectArgs *args);
int os_socket_do_async_send(CelSocketAsyncSendArgs *args);
int os_socket_do_async_recv(CelSocketAsyncRecvArgs *args);
int os_socket_do_async_sendto(CelSocketAsyncSendToArgs *args);
int os_socket_do_async_recvfrom(CelSocketAsyncRecvFromArgs *args);
int os_socket_do_async_sendfile(CelSocketAsyncSendFileArgs *args);

#ifdef __cplusplus
}
#endif

#endif
