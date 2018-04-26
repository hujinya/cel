/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __OS_NET_SOCKET_UNIX_H__
#define __OS_NET_SOCKET_UNIX_H__

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

/* void closesocket(int sockfd)*/
#define closesocket(sockfd) close(sockfd)
/* 
 * int ioctlsocket(int sock, long cmd, unsigned long *argp)
 * cmd :FIONREAD - get # bytes to read;
 *      FIONBIO - set/clear non-blocking i/o;
 *      FIOASYNC -  set/clear async i/o;
 */
#define ioctlsocket(sockfd, cmd, argp) ioctl(sockfd, cmd, argp)

typedef struct OsSocket
{
    union {
        SOCKET fd;
        CelChannel channel;
    };
    int family, socktype, protocol;
    BOOL is_connected;
    CelRefCounted ref_counted;
}OsSocket;

/* Async socket */
typedef struct _OsSocketAsyncAcceptArgs
{
    CelOverLapped ol;
    void (* async_callback) (void *ol);
    OsSocket *socket;
    OsSocket accept_socket;
    void *buffer;
    size_t buffer_size;
}OsSocketAsyncAcceptArgs;

typedef struct _OsSocketAsyncConnectArgs
{
    CelOverLapped ol;
    void (* async_callback) (void *ol);
    OsSocket *socket;
    union {
        CelSockAddr remote_addr;
        struct {
            TCHAR host[CEL_HNLEN];
            TCHAR service[CEL_NPLEN];
        };
    };
    void *buffer;
    size_t buffer_size;
}OsSocketAsyncConnectArgs;

typedef struct _OsSocketAsyncSendArgs
{
    CelOverLapped ol;
    void (* async_callback) (void *ol);
    OsSocket *socket;
    CelAsyncBuf *buffers;
    int buffer_count;
}OsSocketAsyncSendArgs, OsSocketAsyncRecvArgs;

typedef struct _OsSocketAsyncSendToArgs
{
    CelOverLapped ol;
    void (* async_callback) (void *ol);
    OsSocket *socket;
    CelAsyncBuf *buffers;
    int buffer_count;
    CelSockAddr *addr;
}OsSocketAsyncSendToArgs, OsSocketAsyncRecvFromArgs;

typedef struct _OsSocketAsyncSendFileArgs
{
    CelOverLapped ol;
    void (* async_callback) (void *ol);
    OsSocket *socket;
    HANDLE file;
    off_t offset;
    size_t count;
}OsSocketAsyncSendFileArgs;

int os_socket_set_keepalive(OsSocket *sock, int on, 
                            int idle_seconds, int interval_seconds, int count);

int os_socket_do_async_accept(OsSocketAsyncAcceptArgs *args);
int os_socket_do_async_connect(OsSocketAsyncConnectArgs *args);
int os_socket_do_async_send(OsSocketAsyncSendArgs *args);
int os_socket_do_async_recv(OsSocketAsyncRecvArgs *args);
int os_socket_do_async_sendto(OsSocketAsyncSendToArgs *args);
int os_socket_do_async_recvfrom(OsSocketAsyncRecvFromArgs *args);
int os_socket_do_async_sendfile(OsSocketAsyncSendFileArgs *args);

#ifdef __cplusplus
}
#endif

#endif
