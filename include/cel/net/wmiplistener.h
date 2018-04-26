/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_NET_WMIPLISTENER_H__
#define __CEL_NET_WMIPLISTENER_H__

#include "cel/eventloop.h"
#include "cel/net/httplistener.h"
#include "cel/net/wmipclient.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelWmipListenerAsyncArgs CelWmipListenerAsyncArgs;
typedef struct _CelWmipListener CelWmipListener;
//typedef int (*CelWmipListenerAcceptCallbackFunc) (
//    CelWmipListener *listener, CelWmipClient *new_client,  
//    CelAsyncResult *result);

struct _CelWmipListenerAsyncArgs
{
    CelHttpClient client;
    //CelWmipListenerAcceptCallbackFunc async_callback;
};

struct _CelWmipListener
{
    CelHttpListener http_listener;
    CelWmipContext *wmip_ctx;
    CelEventLoop *evt_loop;
    CelWmipListenerAsyncArgs *async_args;
    CelRefCounted ref_counted;
};

int cel_wmiplistener_init(CelWmipListener *listener, 
                          CelSockAddr *addr, CelSslContext *ssl_ctx, 
                          CelWmipContext *wmip_ctx);
void cel_wmiplistener_destroy(CelWmipListener *listener);

CelWmipListener *cel_wmiplistener_new(CelSockAddr *addr, 
                                      CelWmipContext *wmip_ctx);
void cel_wmiplistener_free(CelWmipListener *listener);

int cel_wmiplistener_start(CelWmipListener *listener, CelSockAddr *addr);

#define cel_wmiplistener_set_nonblock(listener, nonblock) \
    cel_socket_set_nonblock((CelSocket *)listener, nonblock)
#define cel_wmiplistener_set_reuseaddr(listener, reuseaddr) \
    cel_socket_set_reuseaddr((CelSocket *)listener, reuseaddr)

#define cel_wmiplistener_get_channel(listener) \
    &((listener)->http_listener.tcp_listener.sock.channel)

#define cel_wmiplistener_get_localaddr(listener) \
    cel_httplistener_get_localaddr(listener)
#define cel_wmiplistener_get_localaddrs(listener) \
    cel_sockaddr_ntop(cel_wmiplistener_get_localaddr(listener))

int cel_wmiplistener_accept(CelWmipListener *listener, CelWmipClient *client);
int cel_wmiplistener_post_accept(CelWmipListener *listener);
int cel_wmiplistener_run(CelWmipListener *listener, CelEventLoop *evt_loop);

#ifdef __cplusplus
}
#endif

#endif
