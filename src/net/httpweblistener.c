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
#include "cel/net/httpweblistener.h"
//#define _CEL_DEBUG
#include "cel/log.h"
#include "cel/error.h"
#include "cel/allocator.h"

int cel_httpweblistener_accept(CelHttpWebListener *listener,
                               CelHttpWebClient *client);
int cel_httpweblistener_post_accept(CelHttpWebListener *listener);

int cel_httpweblistener_init(CelHttpWebListener *listener, 
                             CelSockAddr *addr, CelSslContext *ssl_ctx, 
                             CelHttpWebContext *web_ctx)
{
    if (cel_httplistener_init(&(listener->http_listener), addr, ssl_ctx) == 0)
    {
        listener->web_ctx = web_ctx;
        return 0;
    }
    return -1;
}

void cel_httpweblistener_destroy(CelHttpWebListener *listener)
{
    cel_httplistener_destroy(&(listener->http_listener));
}

int cel_httpweblistener_start(CelHttpWebListener *listener, CelSockAddr *addr)
{
    return cel_httplistener_start(&(listener->http_listener), addr);
}

void cel_httpweblistener_do_handshake(CelHttpWebClient *client, 
                                      CelAsyncResult *result)
{
    if (result->ret != 1)
    {
        CEL_ERR((_T("httpweblistener_do_handshake failed##########################")));
        cel_httpwebclient_free(client);
        return ;
    }
    //puts("httpweblistener_do_handshake ok");
    if (cel_httpclient_async_recv_request(&(client->http_client), 
        &(client->req), 
        (CelHttpRecvRequestCallbackFunc)
        cel_httpwebclient_do_recv_request) == -1)
        cel_httpwebclient_free(client);
}

static void cel_httpweblistener_do_accept(CelHttpWebListener *listener, 
                                          CelHttpClient *http_client,
                                          CelAsyncResult *result)
{
    CelHttpWebClient *new_client;

    //puts("cel_httpweblistener_do_accept");
    if (result->ret == -1)
    {
        CEL_ERR((_T("Http web request listener %s do accept %d.(%s)"), 
            cel_httpweblistener_get_localaddrs(listener), 
            result->ret, cel_geterrstr(result->error)));
        cel_httpweblistener_post_accept(listener);
        return ;
    }
    if ((new_client = cel_httpwebclient_new_httpclient(
        http_client, listener->web_ctx)) == NULL)
    {
        CEL_ERR((_T("cel_httpwebclient_new_httpclient return null")));
        cel_httpclient_destroy(http_client);
        cel_httpweblistener_post_accept(listener);
        return ;
    }
    cel_httpweblistener_post_accept(listener);
    if (cel_httpwebclient_set_nonblock(new_client, 1) == 0)
    {
        if (listener->is_run_group)
        {
            if (cel_eventloopgroup_add_channel(listener->evt_loop_group, -1,
                cel_httpwebclient_get_channel(new_client), NULL) == 0)
            {
                CEL_DEBUG((_T("Http web client %s connected."), 
                    cel_httpwebclient_get_remoteaddr_str(new_client)));
                if (cel_httpclient_async_handshake(&(new_client->http_client),
                    (CelHttpHandshakeCallbackFunc)
                    cel_httpweblistener_do_handshake) != -1)
                    return ;
            }
        }
        else 
        {
            if (cel_eventloop_add_channel(listener->evt_loop, 
                cel_httpwebclient_get_channel(new_client), NULL) == 0)
            {
                CEL_DEBUG((_T("Http web client %s connected."), 
                    cel_httpwebclient_get_remoteaddr_str(new_client)));
                if (cel_httpclient_async_handshake(&(new_client->http_client),
                    (CelHttpHandshakeCallbackFunc)
                    cel_httpweblistener_do_handshake) != -1)
                    return ;
            } 
        }
    }
    CEL_ERR((_T("Http web request client %s init failed(%s)."), 
        cel_httpwebclient_get_remoteaddr_str(new_client), 
        cel_geterrstr(cel_sys_geterrno())));
    cel_httpwebclient_free(new_client);
}

int cel_httpweblistener_post_accept(CelHttpWebListener *listener)
{
    CelHttpWebListenerAsyncArgs *args;

    if (listener->async_args == NULL
        && (listener->async_args = 
        (CelHttpWebListenerAsyncArgs *)
        cel_calloc(1, sizeof(CelHttpWebListenerAsyncArgs))) == NULL)
        return -1;
    args = listener->async_args;
    memset(&(args->client), 0, sizeof(args->client));
    return cel_httplistener_async_accept(&(listener->http_listener),
        &(args->client), 
        (CelHttpAcceptCallbackFunc)cel_httpweblistener_do_accept);
}

int cel_httpweblistener_run(CelHttpWebListener *listener,
                            CelEventLoop *evt_loop)
{
    if (cel_httplistener_set_nonblock(listener, 1) != -1
        && cel_eventloop_add_channel(evt_loop, 
        cel_httpweblistener_get_channel(listener), NULL) != -1
        && cel_httpweblistener_post_accept(listener) != -1)
    {
        listener->is_run_group = FALSE;
        listener->evt_loop = evt_loop;
        return 0;
    }
    return -1;
}

int cel_httpweblistener_run_group(CelHttpWebListener *listener, 
                                  CelEventLoopGroup *evt_loop_grp)
{
    if (cel_httplistener_set_nonblock(listener, 1) != -1
        && cel_eventloopgroup_add_channel(evt_loop_grp, -1,
        cel_httpweblistener_get_channel(listener), NULL) != -1
        && cel_httpweblistener_post_accept(listener) != -1)
    {
        listener->is_run_group = TRUE;
        listener->evt_loop_group = evt_loop_grp;
        return 0;
    }
    return -1;
}
