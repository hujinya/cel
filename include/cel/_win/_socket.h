/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
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

#define EINPROGRESS WSAEWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
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
    CelRefCounted ref_counted;
    union {
        LPFN_ACCEPTEX AcceptEx;
        LPFN_TRANSMITFILE TransmitFile;
    };
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
    unsigned long long offset;
    DWORD count;
}OsSocketAsyncSendFileArgs;

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
