/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_NET_HTTPLISTENER_H__
#define __CEL_NET_HTTPLISTENER_H__

#include "cel/net/tcplistener.h"
#include "cel/net/httpclient.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelHttpListenerAsyncArgs CelHttpListenerAsyncArgs;
typedef struct _CelHttpListener CelHttpListener;
typedef int (* CelHttpAcceptCallbackFunc) (
    CelHttpListener *listener, CelHttpClient *new_client,  
    CelAsyncResult *result);

struct _CelHttpListenerAsyncArgs
{
    CelHttpAcceptCallbackFunc async_callback;
};

struct _CelHttpListener
{
    CelTcpListener tcp_listener;
    CelHttpListenerAsyncArgs *async_args;
    void *user_data;
    CelRefCounted ref_counted;
};

int cel_httplistener_init(CelHttpListener *listener, 
                          CelSockAddr *addr, CelSslContext *ssl_ctx);
int cel_httplistener_init_tcplistener(CelHttpListener *listener, 
                                      CelTcpListener *tcp_listener);
void cel_httplistener_destroy(CelHttpListener *listener);

CelHttpListener *cel_httplistener_new(CelSockAddr *addr, 
                                      CelSslContext *ssl_ctx);
CelHttpListener *cel_httplistener_new_tcplistener(CelTcpListener *tcp_listener);
void cel_httplistener_free(CelHttpListener *listener);

int cel_httplistener_start(CelHttpListener *listener, CelSockAddr *addr);

#define cel_httplistener_set_nonblock(listener, nonblock) \
    cel_socket_set_nonblock((CelSocket *)listener, nonblock)
#define cel_httplistener_set_reuseaddr(listener, reuseaddr) \
    cel_socket_set_reuseaddr((CelSocket *)listener, reuseaddr)

#define cel_httplistener_get_channel(listener) \
    &((listener)->tcp_listener.sock.channel)
#define cel_httplistener_get_localaddr(listener) \
    cel_tcplistener_get_localaddr(listener)

int cel_httplistener_accept(CelHttpListener *listener, 
                            CelHttpClient *new_client);
int cel_httplistener_async_accept(CelHttpListener *listener, 
                                  CelHttpClient *new_client,
                                  CelHttpAcceptCallbackFunc async_callback);
    
#ifdef __cplusplus
}
#endif

#endif
