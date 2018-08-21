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
#ifndef __CEL_NET_HTTPCLIENT_H__
#define __CEL_NET_HTTPCLIENT_H__

#include "cel/net/tcpclient.h"
#include "cel/net/httprequest.h"
#include "cel/net/httpresponse.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelHttpClient CelHttpClient;

typedef void (* CelHttpConnectCallbackFunc)(
    CelHttpClient *client, CelAsyncResult *result);
typedef void (* CelHttpHandshakeCallbackFunc)(
    CelHttpClient *client, CelAsyncResult *result);

typedef void (* CelHttpSendRequestCallbackFunc)(
    CelHttpClient *client, CelHttpRequest *req, CelAsyncResult *result);
typedef void (* CelHttpSendResponseCallbackFunc) (
    CelHttpClient *client, CelHttpResponse *rsp, CelAsyncResult *result);
typedef void (* CelHttpRecvRequestCallbackFunc)(
    CelHttpClient *client, CelHttpRequest *req, CelAsyncResult *result);
typedef void (* CelHttpRecvResponseCallbackFunc) (
    CelHttpClient *client, CelHttpResponse *rsp, CelAsyncResult *result);

typedef void (* CelHttpExecuteCallbackFunc) (CelHttpClient *client, 
    CelHttpRequest *req, CelHttpResponse *rsp, CelAsyncResult *result);

struct _CelHttpClient
{
    CelTcpClient tcp_client;
    CelHttpRequest *execute_req;
    CelHttpResponse *execute_rsp;
    CelHttpExecuteCallbackFunc execute_callback;
    void *user_data;
    CelRefCounted ref_counted;
};

int cel_httpclient_init_tcpclient(CelHttpClient *client, 
                                  CelTcpClient *tcp_client);
int cel_httpclient_init_family(CelHttpClient *client, 
                               int family, CelSslContext *ssl_ctx);
void _cel_httpclient_destroy_derefed(CelHttpClient *client);
void cel_httpclient_destroy(CelHttpClient *client);

CelHttpClient *cel_httpclient_new_tcpclient(CelTcpClient *tcp_client);
CelHttpClient *cel_httpclient_new_family(int family, CelSslContext *ssl_ctx);
void _cel_httpclient_free_derefed(CelHttpClient *client);
void cel_httpclient_free(CelHttpClient *client);

#define cel_httpclient_ref(client) \
    (CelHttpClient *)cel_refcounted_ref(&((client)->ref_counted), client)
#define cel_httpclient_deref(client) \
    cel_refcounted_deref(&((client)->ref_counted), client)

#define cel_httpclient_is_connected(client) \
    cel_socket_is_connected((CelSocket *)client)

#define cel_httpclient_set_nonblock(client, _nonblock) \
    cel_socket_set_nonblock((CelSocket *)client, _nonblock)
#define cel_httpclient_get_channel(client) &((client)->tcp_client.sock.channel)

#define cel_httpclient_get_localaddr(client) \
    cel_tcpclient_get_localaddr(client)
#define cel_httpclient_get_localaddrs(client) \
    cel_sockaddr_ntop(cel_tcpclient_get_localaddr(client))
#define cel_httpclient_get_remoteaddr(client) \
    cel_tcpclient_get_remoteaddr_str(client)
#define cel_httpclient_get_remoteaddrs(client) \
    cel_sockaddr_ntop(cel_tcpclient_get_remoteaddr_str(client))

int cel_httpclient_reading_recv_request(CelHttpClient *client, 
                                        CelStream *s, CelHttpRequest *req);

static __inline 
int cel_httpclient_connect(CelHttpClient *client, 
                           CelSockAddr *remote_addr, CelCoroutine *co)
{
    return cel_tcpclient_connect(&(client->tcp_client), remote_addr, co);
}
static __inline 
int cel_httpclient_handshake(CelHttpClient *client, CelCoroutine *co)
{
    return cel_tcpclient_handshake(&(client->tcp_client), co);
}
int cel_httpclient_recv_request(CelHttpClient *client, 
                                CelHttpRequest *req, CelCoroutine *co);
int cel_httpclient_send_request(CelHttpClient *client, 
                                CelHttpRequest *req, CelCoroutine *co);
int cel_httpclient_recv_response(CelHttpClient *client, 
                                 CelHttpResponse *rsp, CelCoroutine *co);
int cel_httpclient_send_response(CelHttpClient *client, 
                                 CelHttpResponse *rsp, CelCoroutine *co);
int cel_httpclient_execute(CelHttpClient *client,
                           CelHttpRequest *req, CelHttpResponse *rsp, 
                           CelCoroutine *co);

static __inline 
int cel_httpclient_async_connect(CelHttpClient *client, 
                                 CelSockAddr *remote_addr,
                                 CelHttpConnectCallbackFunc async_callback)
{
    return cel_tcpclient_async_connect(&(client->tcp_client), remote_addr,
        (CelTcpConnectCallbackFunc)async_callback);
}
static __inline 
int cel_httpclient_async_connect_host(CelHttpClient *client, 
                                      const TCHAR *host, unsigned short port,
                                      CelHttpConnectCallbackFunc callback)
{
    return cel_tcpclient_async_connect_host(&(client->tcp_client), host, port,
        (CelTcpConnectCallbackFunc)callback);
}
static __inline 
int cel_httpclient_async_handshake(CelHttpClient *client,
                                   CelHttpHandshakeCallbackFunc async_callback)
{
    return cel_tcpclient_async_handshake(
        &(client->tcp_client), (CelTcpHandshakeCallbackFunc)async_callback);
}
int cel_httpclient_async_recv_request(CelHttpClient *client, 
                                      CelHttpRequest *req, 
                                      CelHttpRecvRequestCallbackFunc func);
int cel_httpclient_async_send_request(CelHttpClient *client,
                                      CelHttpRequest *req, 
                                      CelHttpSendRequestCallbackFunc func);
int cel_httpclient_async_recv_response(CelHttpClient *client, 
                                       CelHttpResponse *rsp, 
                                       CelHttpRecvResponseCallbackFunc func);
int cel_httpclient_async_send_response(CelHttpClient *client, 
                                       CelHttpResponse *rsp,
                                       CelHttpSendResponseCallbackFunc func);

int cel_httpclient_async_execute(CelHttpClient *client, 
                                 CelHttpRequest *req, CelHttpResponse *rsp,
                                 CelHttpExecuteCallbackFunc func);

#ifdef __cplusplus
}
#endif

#endif
