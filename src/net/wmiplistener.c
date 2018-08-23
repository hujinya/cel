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
#include "cel/net/wmiplistener.h"
#include "cel/log.h"
#include "cel/error.h"
#include "cel/allocator.h"

int cel_wmiplistener_init(CelWmipListener *listener, 
                          CelSockAddr *addr, CelSslContext *ssl_ctx, 
                          CelWmipContext *wmip_ctx)
{
    if (cel_httplistener_init(&(listener->http_listener), addr, ssl_ctx) == 0)
    {
        listener->wmip_ctx = wmip_ctx;
        return 0;
    }
    return -1;
}

void cel_wmiplistener_destroy(CelWmipListener *listener)
{
    cel_httplistener_destroy(&(listener->http_listener));
}

int cel_wmiplistener_start(CelWmipListener *listener, CelSockAddr *addr)
{
    return cel_httplistener_start(&(listener->http_listener), addr);
}

void wmiplistener_do_handshake(CelWmipClient *client, CelAsyncResult *result)
{
    if (result->ret != 1)
    {
        puts("wmiplistener_do_handshake failed##################################");
        cel_wmipclient_free(client);
        return ;
    }
    //puts("wmiplistener_do_handshake ok");
    if (cel_httpclient_async_recv_request(&(client->http_client), 
        &(client->wmip_msg->req), 
        (CelHttpRecvRequestCallbackFunc)cel_wmipclient_do_recv_request) == -1)
        cel_wmipclient_free(client);
}

void wmiplistener_do_accept(CelWmipListener *listener, 
                            CelHttpClient *http_client, CelAsyncResult *result)
{
    CelWmipClient *new_client;

    if (result->ret == -1)
    {
        CEL_ERR((_T("Wmip listener %s do accept %d.(%s)"), 
            cel_wmiplistener_get_localaddrs(listener), 
            result->ret, cel_geterrstr(result->error)));
        return ;
    }
    if ((new_client = listener->wmip_ctx->new_func != NULL ? 
        listener->wmip_ctx->new_func(sizeof(CelWmipClient),
        http_client->tcp_client.sock.fd)
        : cel_malloc(sizeof(CelWmipClient))) == NULL)
    {
        CEL_ERR((_T("Wmip client %s new return null."), 
            cel_httpclient_get_remoteaddr_str(http_client)));
        cel_httpclient_destroy(http_client);
        return ;
    }
    memcpy(&(new_client->http_client), http_client, sizeof(CelHttpClient));
    cel_wmiplistener_post_accept(listener);

    new_client->wmip_ctx = listener->wmip_ctx;
    new_client->wmip_msg = cel_wmipmessage_new();
    new_client->user_data = NULL;
    cel_refcounted_init(&(new_client->ref_counted), 
        (CelFreeFunc)_cel_wmipclient_free_derefed);
    if (cel_wmipclient_set_nonblock(new_client, 1) == 0)
    {
        if (listener->is_run_group)
        {
            if (cel_eventloopgroup_add_channel(
                listener->evt_loop_group, -1, 
                cel_wmipclient_get_channel(new_client), NULL) == 0)
            {
                CEL_DEBUG((_T("Wmip2 client %s connected."), 
                    cel_wmipclient_get_remoteaddr_str(new_client)));
                if (cel_httpclient_async_handshake(&(new_client->http_client), 
                    (CelHttpHandshakeCallbackFunc)wmiplistener_do_handshake) != -1)
                    return ;
            }
        }
        else 
        {
            if (cel_eventloop_add_channel(
                listener->evt_loop, 
                cel_wmipclient_get_channel(new_client), NULL) == 0)
            {
                CEL_DEBUG((_T("Wmip2 client %s connected."), 
                    cel_wmipclient_get_remoteaddr_str(new_client)));
                if (cel_httpclient_async_handshake(&(new_client->http_client), 
                    (CelHttpHandshakeCallbackFunc)wmiplistener_do_handshake) != -1)
                    return ;
            } 
        }
    }
    cel_wmipclient_free(new_client);
    CEL_ERR((_T("Wmip client %s init failed(%s)."), 
        cel_wmipclient_get_remoteaddr_str(new_client), cel_geterrstr(cel_sys_geterrno())));
}

int cel_wmiplistener_post_accept(CelWmipListener *listener)
{
    CelWmipListenerAsyncArgs *args;

    if (listener->async_args == NULL
        && (listener->async_args = (CelWmipListenerAsyncArgs *)
        cel_calloc(1, sizeof(CelWmipListenerAsyncArgs))) == NULL)
        return -1;
    args = listener->async_args;
    return cel_httplistener_async_accept(&(listener->http_listener), 
        &(args->client), (CelHttpAcceptCallbackFunc)wmiplistener_do_accept);
}

int cel_wmiplistener_run(CelWmipListener *listener, CelEventLoop *evt_loop)
{
    if (cel_httplistener_set_nonblock(listener, 1) != -1
        && cel_eventloop_add_channel(evt_loop, 
        cel_wmiplistener_get_channel(listener), NULL) != -1
        && cel_wmiplistener_post_accept(listener) != -1)
    {
        listener->is_run_group = FALSE;
        listener->evt_loop = evt_loop;
        return 0;
    }
    return -1;
}

int cel_wmiplistener_run_group(CelWmipListener *listener, 
                               CelEventLoopGroup *evt_loop_grp)
{
    if (cel_httplistener_set_nonblock(listener, 1) != -1
        && cel_eventloopgroup_add_channel(evt_loop_grp, -1,
        cel_wmiplistener_get_channel(listener), NULL) != -1
        && cel_wmiplistener_post_accept(listener) != -1)
    {
        listener->is_run_group = TRUE;
        listener->evt_loop_group = evt_loop_grp;
        return 0;
    }
    return -1;
}
