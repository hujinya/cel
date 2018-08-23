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
#ifndef __CEL_NET_SOCKET_WIN_H__
#define __CEL_NET_SOCKET_WIN_H__

#include "cel/types.h"
#include "cel/refcounted.h"
#include <Mswsock.h> /* AcceptEx, ConnectEx */

#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "Ws2_32.lib")

#ifdef __cplusplus
extern "C" {
#endif

#define PF_PACKET   17  /* Packet family.  */
#define AF_PACKET   PF_PACKET
#define SOCK_PACKET 10

#define SHUT_RD     SD_RECEIVE
#define SHUT_WR     SD_SEND 
#define SHUT_RDWR   SD_BOTH 

#ifndef EINPROGRESS
#define EINPROGRESS WSAEWOULDBLOCK
#endif
#ifndef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif
#ifndef EAGAIN
#define EAGAIN      WSAEWOULDBLOCK
#endif

static __inline int os_wsastartup(void)
{
    WORD wVersionRequested = MAKEWORD(2, 2);  
    WSADATA wsaData;
    return WSAStartup(wVersionRequested, &wsaData); 
}
#define os_wsacleanup WSACleanup

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
    union {
        LPFN_ACCEPTEX AcceptEx;
        LPFN_TRANSMITFILE TransmitFile;
    };
}OsSocket;

int os_socket_set_keepalive(OsSocket *sock, int on, 
                            int idle_seconds, int interval_seconds, int count);

static __inline int os_socket_update_connectcontext(OsSocket *connect)
{
    unsigned long optval = 1;
    return (setsockopt(connect->fd,  SOL_SOCKET,  SO_UPDATE_CONNECT_CONTEXT,  
        (char *)&optval,  sizeof(optval)) != 0 ? -1 : 0);
}
static __inline 
int os_socket_update_acceptcontext(OsSocket *accept, OsSocket *listen)
{
    return (setsockopt(accept->fd,  SOL_SOCKET,  SO_UPDATE_ACCEPT_CONTEXT, 
        (char *)&listen->fd, sizeof(listen->fd)) != 0 ? -1 : 0);
}

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
