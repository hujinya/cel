/**
 * CEL(C Extension Library)
 * Copyright (C)2008 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/net/httplistener.h"

int cel_httplistener_init(CelHttpListener *listener,
                          CelSockAddr *addr, CelSslContext *ssl_ctx)
{
    listener->async_args = NULL;
    return cel_tcplistener_init_addr(&(listener->tcp_listener), addr, ssl_ctx);
}

void cel_httplistener_destroy(CelHttpListener *listener)
{
    if (listener->async_args != NULL)
    {
        cel_free(listener->async_args);
        listener->async_args = NULL;
    }
    cel_tcplistener_destroy(&(listener->tcp_listener));
}

int cel_httplistener_start(CelHttpListener *listener, CelSockAddr *addr)
{
    return cel_tcplistener_start(&(listener->tcp_listener), addr);
}

int cel_httplistener_accept(CelHttpListener *listener, CelHttpClient *client)
{
    return cel_tcplistener_accept(&(listener->tcp_listener), &(client->tcp_client));
}

void cel_httplistener_do_accept(CelHttpListener *listener, 
                                CelHttpClient *client, CelAsyncResult *result)
{
    if (cel_httpclient_init_tcpclient(client, &(client->tcp_client)) == -1)
        result->ret = -1;
    listener->async_args->async_callback(listener, client, result);
}

int cel_httplistener_async_accept(CelHttpListener *listener, 
                                  CelHttpClient *client, 
                                  CelHttpAcceptCallbackFunc async_callback)
{
    CelHttpListenerAsyncArgs *args;

    if (listener->async_args == NULL
        && (listener->async_args = (CelHttpListenerAsyncArgs *)
        cel_calloc(1, sizeof(CelHttpListenerAsyncArgs))) == NULL)
        return -1;
    args = listener->async_args;
    args->async_callback = async_callback;
    return cel_tcplistener_async_accept(&(listener->tcp_listener), 
        &(client->tcp_client), 
        (CelTcpAcceptCallbackFunc)cel_httplistener_do_accept);
}
