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
#ifndef __CEL_NET_SOCKET_H__
#define __CEL_NET_SOCKET_H__

#include "cel/types.h"
#include "cel/poll.h"
#include "cel/net/sockaddr.h"
#ifdef _CEL_UNIX
#include "cel/_unix/_socket.h"
#elif defined(_CEL_WIN)
#include "cel/_win/_socket.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef OsSocket CelSocket;
typedef OsSocketAsyncAcceptArgs CelSocketAsyncAcceptArgs;
typedef OsSocketAsyncConnectArgs CelSocketAsyncConnectArgs;
typedef OsSocketAsyncRecvArgs CelSocketAsyncRecvArgs;
typedef OsSocketAsyncSendArgs CelSocketAsyncSendArgs;
typedef OsSocketAsyncRecvFromArgs CelSocketAsyncRecvFromArgs;
typedef OsSocketAsyncSendToArgs CelSocketAsyncSendToArgs;
typedef OsSocketAsyncSendFileArgs CelSocketAsyncSendFileArgs;

typedef union _CelSocketAsyncArgs
{
    CelOverLapped ol;
    CelSocketAsyncAcceptArgs accept_args;
    CelSocketAsyncConnectArgs connect_args;
    CelSocketAsyncRecvArgs recv_args;
    CelSocketAsyncSendArgs send_args;
    CelSocketAsyncRecvFromArgs recvfrom_args;
    CelSocketAsyncSendToArgs sendto_args;
    CelSocketAsyncSendFileArgs sendfile_args;
}CelSocketAsyncArgs;

#define ACCEPTEX_LOCALADDRESS_LEN (sizeof(CelSockAddr) + 16)
#define ACCEPTEX_REMOTEADDRESS_LEN (sizeof(CelSockAddr) + 16)
#define ACCEPTEX_RECEIVEDATA_OFFSET \
    (sizeof(CelSocket) \
    +  ACCEPTEX_LOCALADDRESS_LEN + ACCEPTEX_REMOTEADDRESS_LEN)

#define cel_wsastartup() os_wsastartup()
#define cel_wsacleanup() os_wsacleanup()

int cel_socket_init(CelSocket *sock, int family, int socktype, int protocol);
void cel_socket_destroy(CelSocket *sock);

CelSocket *cel_socket_new(int family, int socktype, int protocol);
void cel_socket_free(CelSocket *sock);

#define cel_socket_ref(sock) \
    cel_refcounted_ref_ptr(&((sock)->ref_counted), sock);
#define cel_socket_deref(sock) \
    cel_refcounted_deref(&((sock)->ref_counted), sock);

#define cel_socket_shutdown(sock, how) shutdown((sock)->fd, how)
#define cel_socket_bind(sock, sock_addr)\
    bind((sock)->fd, &((sock_addr)->addr), cel_sockaddr_get_len(sock_addr))
int cel_socket_bind_host(CelSocket *sock, 
                         const TCHAR *host, unsigned short port);
static __inline int cel_socket_listen(CelSocket *sock, 
                                      CelSockAddr *addr, int backlog)
{
    return ((cel_socket_bind(sock, addr) == -1 
        || listen((sock)->fd, backlog) == -1) ? -1 : 0);
}
;
int cel_socket_listen_host(CelSocket *sock, 
                          const TCHAR *host, unsigned short port, 
                          int backlog);
int cel_socket_listen_str(CelSocket *sock, const TCHAR *str, int backlog);
static __inline 
int cel_socket_connect(CelSocket *sock, CelSockAddr *remote_addr)
{
    if (connect((sock)->fd, 
        &((remote_addr)->addr), cel_sockaddr_get_len(remote_addr)) == 0)
    {
        sock->is_connected = TRUE;
        return 0;
    }
    return -1;
}
int cel_socket_connect_host(CelSocket *sock, 
                            const TCHAR *host, unsigned short port);

int cel_socket_accept(CelSocket *sock, 
                      CelSocket *accept_sock, CelSockAddr *addr);
#define cel_socket_send(sock, buf, size) send((sock)->fd, buf, size, 0)
#define cel_socket_recv(sock, buf, size) recv((sock)->fd, buf, size, 0)
#define cel_socket_sendto(sock, buf, size, to) \
    sendto((sock)->fd, buf, size, 0, \
    (struct sockaddr *)to, cel_sockaddr_get_len(to))
static __inline 
int cel_socket_recvfrom(CelSocket *sock, void *buf, int size, 
                        CelSockAddr *from)
{
    socklen_t len = sizeof(CelSockAddr);
    return recvfrom((sock)->fd, buf, size, 0, (struct sockaddr *)from, &len);
}

static __inline BOOL cel_socket_is_connected(CelSocket *sock)
{
    return sock->is_connected;
}

static __inline int cel_socket_get_errno(CelSocket *sock)
{
    int sock_errno = 0;
    socklen_t len = sizeof(int);
    getsockopt(sock->fd, SOL_SOCKET, SO_ERROR, (char *)&sock_errno, &len);
    return sock_errno;
}
static __inline 
CelSockAddr *cel_socket_get_localaddr(CelSocket *sock, CelSockAddr *addr)
{
    socklen_t len = sizeof(CelSockAddr);
    return (getsockname(sock->fd, (struct sockaddr *)addr, &len) == -1 
        ? NULL : addr);
}
static __inline 
CelSockAddr *cel_socket_get_remoteaddr(CelSocket *sock, CelSockAddr *addr)
{
    socklen_t len = sizeof(CelSockAddr);
    return (getpeername(sock->fd, (struct sockaddr *)addr, &len) == -1 
        ? NULL : addr);
}

/* 
int cel_socket_set_keepalive(CelSocket *sock, int on, 
                             int idle_seconds, int interval_seconds, int count);
*/
#define cel_socket_set_keepalive os_socket_set_keepalive
static __inline int cel_socket_set_linger(CelSocket *sock, int on, int seconds)
{
    struct linger lg = { on, seconds };
    return setsockopt(sock->fd, SOL_SOCKET, SO_LINGER, (char *)&lg, sizeof(lg));
}
static __inline int cel_socket_set_nodelay(CelSocket *sock, int on)
{
    return setsockopt((sock)->fd, IPPROTO_TCP, TCP_NODELAY, 
        (char *)&on, sizeof(on));
}
static __inline int cel_socket_set_nonblock(CelSocket *sock, int nonblock)
{
    return ioctlsocket((sock)->fd, FIONBIO, &nonblock);
}
static __inline int cel_socket_set_reuseaddr(CelSocket *sock, int reuseaddr)
{
    return setsockopt((sock)->fd, 
        SOL_SOCKET, SO_REUSEADDR, (char *)&reuseaddr, sizeof(reuseaddr));
}
static __inline int cel_socket_set_rcvbuffer(CelSocket *sock, int size)
{
    return setsockopt((sock)->fd, SOL_SOCKET, SO_RCVBUF, 
        (char *)&size, sizeof(size));
}
static __inline 
int cel_socket_set_sndbuffer(CelSocket *sock, int size)
{
    return setsockopt((sock)->fd, SOL_SOCKET, SO_SNDBUF,
        (char *)&size, sizeof(size));
}
static __inline 
int cel_socket_set_rcvtimeout(CelSocket *sock, int milliseconds)
{
    struct timeval tmo;
    tmo.tv_sec = milliseconds / 1000;
    tmo.tv_usec = milliseconds % 1000 * 1000;
    return setsockopt(sock->fd, SOL_SOCKET, SO_RCVTIMEO, 
        (char *)&tmo, sizeof(tmo));
}
static __inline int cel_socket_set_sndtimeout(CelSocket *sock, int milliseconds)
{
    struct timeval tmo;
    tmo.tv_sec = milliseconds / 1000;
    tmo.tv_usec = milliseconds % 1000 * 1000;
    return setsockopt(sock->fd, SOL_SOCKET, SO_SNDTIMEO, 
        (char *)&tmo, sizeof(tmo));
}

/* int cel_socket_update_connectcontext(CelSocket *connect); */
#define cel_socket_update_connectcontext os_socket_update_connectcontext
/* 
 * int cel_socket_update_acceptcontext(CelSocket *accept, CelSocket *listen); 
 */
#define cel_socket_update_acceptcontext os_socket_update_acceptcontext

/* int cel_socket_do_async_accept(CelSocketAsyncAcceptArgs *args); */
#define cel_socket_do_async_accept os_socket_do_async_accept
/* int cel_socket_do_async_connect(CelSocketAsyncConnectArgs *args); */
#define cel_socket_do_async_connect os_socket_do_async_connect
/* int cel_socket_do_async_send(CelSocketAsyncSendArgs *args); */
#define cel_socket_do_async_send os_socket_do_async_send
/* int cel_socket_do_async_recv(CelSocketAsyncRecvArgs *args); */
#define cel_socket_do_async_recv os_socket_do_async_recv
/* int cel_socket_do_async_sendto(CelSocketAsyncSendToArgs *args); */
#define cel_socket_do_async_sendto os_socket_do_async_sendto
/* int cel_socket_do_async_recvfrom(CelSocketAsyncRecvFromArgs *args); */
#define cel_socket_do_async_recvfrom os_socket_do_async_recvfrom
/* int cel_socket_do_async_sendfile(CelSocketAsyncSendFileArgs *args); */
#define cel_socket_do_async_sendfile os_socket_do_async_sendfile

int cel_socket_async_accept(CelSocketAsyncAcceptArgs *args);
int cel_socket_async_connect(CelSocketAsyncConnectArgs *args);
int cel_socket_async_connect_host(CelSocketAsyncConnectArgs *args);
int cel_socket_async_send(CelSocketAsyncSendArgs *args);
int cel_socket_async_recv(CelSocketAsyncRecvArgs *args);
int cel_socket_async_sendto(CelSocketAsyncSendToArgs *args);
int cel_socket_async_recvfrom(CelSocketAsyncRecvFromArgs *args);
int cel_socket_async_sendfile(CelSocketAsyncSendFileArgs *args);

#ifdef __cplusplus
}
#endif

#endif
