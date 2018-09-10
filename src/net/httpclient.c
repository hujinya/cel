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
#include "cel/net/httpclient.h"
#include "cel/log.h"
#include "cel/error.h"

void _cel_httpclient_destroy_derefed(CelHttpClient *client)
{
    //puts("_cel_httpclient_destroy_derefed");
    cel_tcpclient_destroy(&(client->tcp_client));
}

void _cel_httpclient_free_derefed(CelHttpClient *client)
{
    _cel_httpclient_destroy_derefed(client);
    cel_free(client);
}

int cel_httpclient_init(CelHttpClient *client)
{
    return cel_refcounted_init(
        &(client->ref_counted), (CelFreeFunc)_cel_httpclient_destroy_derefed);
}

int cel_httpclient_init_tcpclient(CelHttpClient *client, 
                                  CelTcpClient *tcp_client)
{
    if (&(client->tcp_client) != tcp_client)
        memcpy(&(client->tcp_client), tcp_client, sizeof(CelTcpClient));
    return cel_httpclient_init(client);
}

int cel_httpclient_init_family(CelHttpClient *client, 
                               int family, CelSslContext *ssl_ctx)
{
    if (cel_tcpclient_init_family(&(client->tcp_client), family, ssl_ctx) == 0)
    {
        return cel_httpclient_init(client);
    }
    return -1;
}

void cel_httpclient_destroy(CelHttpClient *client)
{
    cel_refcounted_destroy(&(client->ref_counted), client);
}

CelHttpClient *cel_httpclient_new_tcpclient(CelTcpClient *tcp_client)
{
    CelHttpClient *client;

    if ((client = (CelHttpClient *)cel_malloc(sizeof(CelHttpClient))) != NULL)
    {
        if (cel_httpclient_init_tcpclient(client, tcp_client) != -1)
        {
            cel_refcounted_init(
                &(client->ref_counted), 
                (CelFreeFunc)_cel_httpclient_free_derefed);
            return client;
        }
        cel_free(client);
    }
    return NULL;
}

CelHttpClient *cel_httpclient_new_family(int family, CelSslContext *ssl_ctx)
{
    CelHttpClient *client;

    if ((client = (CelHttpClient *)cel_malloc(sizeof(CelHttpClient))) != NULL)
    {
        if (cel_httpclient_init_family(client, family, ssl_ctx) != -1)
        {
            cel_refcounted_init(
                &(client->ref_counted), 
                (CelFreeFunc)_cel_httpclient_free_derefed);
            return client;
        }
        cel_free(client);
    }
    return NULL;
}

void cel_httpclient_free(CelHttpClient *client)
{
    cel_refcounted_destroy(&(client->ref_counted), client);
}

int cel_httpclient_reading_recv_request(CelHttpClient *client, 
                                        CelStream *s, CelHttpRequest *req)
{
    while (req->reading_state != CEL_HTTPREQUEST_READING_OK)
    {
        if (cel_stream_get_remaining_length(&(req->s)) != 0)
            return 0;
        cel_stream_set_position(s, 0);
        if (cel_httprequest_reading(req, s) == -1)
        {
            if (req->reading_error == CEL_HTTP_ERROR)
            {
                CEL_ERR((_T("cel_httprequest_reading error")));
                return -1;
            }
        }
        if (cel_stream_get_remaining_length(s) > 0)
        {
            memmove(cel_stream_get_buffer(s), 
                cel_stream_get_pointer(s), cel_stream_get_remaining_length(s));
            cel_stream_set_position(s, cel_stream_get_remaining_length(s));
        }
        else
        {
            cel_stream_set_position(s, 0);
        }
    }
    return 1;
}

int cel_httpclient_recv_request(CelHttpClient *client, CelHttpRequest *req)
{
    int ret;

    do
    {
        if ((ret = cel_tcpclient_recv(&(client->tcp_client), &(req->s)) <= 0)
            || (ret = cel_httpclient_reading_recv_request(
            client, &(req->s), req)) == -1)
            return ret;
    }while (ret != 1);
    return 0;
}

void cel_httpclient_do_recv_request(CelHttpClient *client, 
                                    CelStream *s, CelAsyncResult *result)
{
    int ret;
    CelHttpRequest *req = (CelHttpRequest *)s;
    CelHttpRecvRequestCallbackFunc recv_req_callback = 
        (CelHttpRecvRequestCallbackFunc)(req->_async_callback);

    if (result->ret <= 0
        || (ret = cel_httpclient_reading_recv_request(client, s, req)) == -1)
    {
        //puts("cel_httpclient_do_recv_request return -1");
        result->ret = -1;
        recv_req_callback(client, req, result);
        return ;
    }
    if (ret == 0)
    {
        if (cel_tcpclient_async_recv(&(client->tcp_client), s, 
            (CelTcpSendCallbackFunc)cel_httpclient_do_recv_request) == -1)
        {
            result->ret = -1;
            recv_req_callback(client, req, result);
        }
        return;
    }
    recv_req_callback(client, req, result);
}

int cel_httpclient_async_recv_request(CelHttpClient *client, 
                                      CelHttpRequest *req, 
                                      CelHttpRecvRequestCallbackFunc callback)
{
    CelStream *s = cel_httprequest_get_stream(req);

    req->_async_callback = callback;
    return cel_tcpclient_async_recv(&(client->tcp_client), s, 
        (CelTcpSendCallbackFunc)cel_httpclient_do_recv_request);
}

int cel_httpclient_writing_send_requset(CelHttpClient *client, 
                                        CelStream *s, CelHttpRequest *req)
{
    while (cel_stream_get_remaining_length(s) == 0)
    {
        cel_stream_set_position(s, 0);
        if (req->writing_state == CEL_HTTPREQUEST_WRITING_OK)
            return 1;
        if (cel_httprequest_writing(req, s) == -1)
        {
            if (req->writing_error == CEL_HTTP_ERROR)
            {
                CEL_ERR((_T("cel_httprequest_writing error")));
                return -1;
            }
        }
        cel_stream_set_position(s, 0);
    }
    return 0;
}

int cel_httpclient_send_request(CelHttpClient *client, CelHttpRequest *req)
{
    int ret;
    CelStream *s = cel_httprequest_get_stream(req);

    cel_stream_set_position(s, 0);
    do 
    {
        if ((ret = cel_httpclient_writing_send_requset(client, s, req)) == -1)
            return -1;
        if (ret == 1)
            break;
    }while(cel_tcpclient_send(&(client->tcp_client), &(req->s)) > 0);
    return 0;
}

void cel_httpclient_do_send_requset(CelHttpClient *client, 
                                    CelStream *s, CelAsyncResult *result)
{
    int ret;
    CelHttpRequest *req = (CelHttpRequest *)s;
    CelHttpSendRequestCallbackFunc send_req_callback 
        = (CelHttpSendRequestCallbackFunc)(req->_async_callback);

    //_tprintf("cel_httpclient_do_send_requset ret = %ld\r\n", result->ret);
    if (result->ret <= 0
        || (ret = cel_httpclient_writing_send_requset(client, s, req)) == -1)
    {
        //CEL_ERR((_T("cel_httpclient_do_send_requset return -1")));
        result->ret = -1;
        send_req_callback(client, req, result);
        return;
    }
    if (ret == 0)
    {
        if (cel_tcpclient_async_send(&(client->tcp_client), s, 
            (CelTcpSendCallbackFunc)cel_httpclient_do_send_requset) == -1)
        {
            result->ret = -1;
            send_req_callback(client, req, result);
        }
        return;
    }
    send_req_callback(client, req, result);
}

int cel_httpclient_async_send_request(CelHttpClient *client,
                                      CelHttpRequest *req, 
                                      CelHttpSendRequestCallbackFunc callback)
{
    int ret;
    CelStream *s = cel_httprequest_get_stream(req);
    CelAsyncResult _result;

    CEL_ASSERT(callback == NULL);
    req->_async_callback = callback;
    //puts(req->s.buffer);
    cel_stream_set_position(s, 0);
    if ((ret = cel_httpclient_writing_send_requset(client, s, req)) == 0)
    {
        return cel_tcpclient_async_send(&(client->tcp_client), s, 
            (CelTcpSendCallbackFunc)cel_httpclient_do_send_requset);
    }
    if (ret == 1)
    {
        _result.ret = 0;
        _result.error = CEL_HTTP_NO_ERROR;
        callback(client, req, &_result);
        return 0;
    }
    return -1;
}

int cel_httpclient_reading_recv_response(CelHttpClient *client, 
                                         CelStream *s, CelHttpResponse *rsp)
{
    while (rsp->reading_state != CEL_HTTPRESPONSE_READING_OK)
    {
        if (cel_stream_get_remaining_length(s) != 0)
            return 0;
        cel_stream_set_position(s, 0);
        if (cel_httpresponse_reading(rsp, s) == -1)
        {
            if (rsp->reading_error == CEL_HTTP_ERROR)
            {
                CEL_ERR((_T("cel_httpresponse_reading error")));
                return -1;
            }
        }
        if (cel_stream_get_remaining_length(s) > 0)
        {
            memmove(cel_stream_get_buffer(s), cel_stream_get_pointer(s),
                cel_stream_get_remaining_length(s));
            cel_stream_set_position(s, cel_stream_get_remaining_length(s));
        }
        else
        {
            cel_stream_set_position(s, 0);
        }
    }
    return 1;
}

int cel_httpclient_recv_response(CelHttpClient *client, CelHttpResponse *rsp)
{
    int ret;

    do
    {
        if ((ret = cel_tcpclient_recv(&(client->tcp_client), &(rsp->s))) <= 0
            || (ret = cel_httpclient_reading_recv_response(
            client, &(rsp->s), rsp)) == -1)
            return ret;
    }while (ret != 1);
    return 0;
}

void cel_httpclient_do_recv_response(CelHttpClient *client, 
                                     CelStream *s, CelAsyncResult *result)
{
    int ret;
    CelHttpResponse *rsp = (CelHttpResponse *)s;
    CelHttpRecvResponseCallbackFunc recv_rsp_callback 
        = (CelHttpRecvResponseCallbackFunc)(rsp->_async_callback);

    /*_tprintf("cel_httpclient_do_recv_response return %ld, error %d\r\n", 
    result->ret, result->error);*/
    //puts((char *)s->buffer);
    if (result->ret <= 0
        || (ret = cel_httpclient_reading_recv_response(client, s, rsp)) == -1)
    {
        result->ret = -1;
        recv_rsp_callback(client, rsp, result);
        return;
    }
    if (ret == 0)
    {
        if (cel_tcpclient_async_recv(&(client->tcp_client), s, 
            (CelTcpRecvCallbackFunc)cel_httpclient_do_recv_response) == -1)
        {
            result->ret = -1;
            recv_rsp_callback(client, rsp, result);
            return ;
        }
        return;
    }
    recv_rsp_callback(client, rsp, result);
}

int cel_httpclient_async_recv_response(CelHttpClient *client,
                                       CelHttpResponse *rsp, 
                                       CelHttpRecvResponseCallbackFunc callback)
{
    CelStream *s = cel_httpresponse_get_stream(rsp);

    rsp->_async_callback = callback;
    return cel_tcpclient_async_recv(&(client->tcp_client), s, 
        (CelTcpRecvCallbackFunc)cel_httpclient_do_recv_response);
}

int cel_httpclient_writing_send_response(CelHttpClient *client, 
                                         CelStream *s, CelHttpResponse *rsp )
{
    while (cel_stream_get_remaining_length(s) <= 0)
    {
        cel_stream_set_position(s, 0);
        if (rsp->writing_state == CEL_HTTPRESPONSE_WRITING_OK)
            return 1;
        if (cel_httpresponse_writing(rsp, s) == -1)
        {
            if (rsp->writing_error == CEL_HTTP_ERROR)
            {
                CEL_ERR((_T("cel_httpresponse_writing error")));
                return -1;
            }
        }
        cel_stream_set_position(s, 0);
    }
    //_tprintf(_T("length = %d\r\n"), cel_stream_get_remaining_length(s));
    return 0;
}

int cel_httpclient_send_response(CelHttpClient *client, CelHttpResponse *rsp)
{
    int ret;
    CelStream *s = cel_httpresponse_get_stream(rsp);

    cel_stream_set_position(s, 0);
    do 
    {
        if ((ret = cel_httpclient_writing_send_response(client, s, rsp)) == -1)
            return -1;
        if (ret == 1)
            break;
    }while(cel_tcpclient_send(&(client->tcp_client), &(rsp->s)) > 0);
    return 0;
}

void cel_httpclient_do_send_response(CelHttpClient *client, 
                                     CelStream *s, CelAsyncResult *result)
{
    int ret;
    CelHttpResponse *rsp = (CelHttpResponse *)s;
    CelHttpSendResponseCallbackFunc send_rsp_callback 
        = (CelHttpSendResponseCallbackFunc)(rsp->_async_callback);

    //_tprintf("cel_httpclient_do_send_response ret = %ld\r\n", result->ret);
    if (result->ret <= 0
        || (ret = cel_httpclient_writing_send_response(client, s, rsp)) == -1)
    {
        //puts("cel_httpclient_do_send_response return -1");
        result->ret = -1;
        send_rsp_callback(client, rsp, result);
        return;
    }
    if (ret == 0)
    {
        if (cel_tcpclient_async_send(&(client->tcp_client), &(rsp->s), 
            (CelTcpSendCallbackFunc)cel_httpclient_do_send_response) == -1)
        {
            result->ret = -1;
            send_rsp_callback(client, rsp, result);
            return;
        }
        return ;
    }
    send_rsp_callback(client, rsp, result);
}

int cel_httpclient_async_send_response(CelHttpClient *client, 
                                       CelHttpResponse *rsp,
                                       CelHttpSendResponseCallbackFunc callback)
{
    int ret;
    CelStream *s = cel_httpresponse_get_stream(rsp);
    CelAsyncResult _result;

    CEL_ASSERT(callback == NULL);
    rsp->_async_callback = callback;
    cel_stream_set_position(s, 0);
    if ((ret = cel_httpclient_writing_send_response(client, s, rsp)) == 0)
    {
        return cel_tcpclient_async_send(&(client->tcp_client), s, 
            (CelTcpSendCallbackFunc)cel_httpclient_do_send_response);
    }
    if (ret == 1)
    {
        _result.ret = 0;
        _result.error = CEL_HTTP_NO_ERROR;
        callback(client, rsp, &_result);
        return 0;
    }
    return -1;
}

int cel_httpclient_execute(CelHttpClient *client,
                           CelHttpRequest *req, CelHttpResponse *rsp)
{
    char *host;
    unsigned short port;
    int ret;

    if (!cel_httpclient_is_connected(client))
    {
        if (req->url.scheme == CEL_HTTPSCHEME_HTTPS)
            cel_tcpclient_set_ssl(&(client->tcp_client), TRUE);
        else
            cel_tcpclient_set_ssl(&(client->tcp_client), FALSE);
        if ((host = cel_httprequest_get_url_host(req)) == NULL
            || (port = cel_httprequest_get_url_port(req)) == 0)
        {
            CEL_ERR((_T("cel_httpclient_execute error")));
            return -1;
        }
        if ((ret = cel_httpclient_connect_host(client, host, port)) != 0
            || (ret = cel_httpclient_handshake(client)) != 0)
            return ret;
    }
    if ((ret = cel_httpclient_send_request(client, req)) != 0
        || (ret = cel_httpclient_recv_response(client, rsp)) != 0)
        return ret;
    return 0;
}

static void cel_httpclient_response_async_callback(CelHttpClient *client, 
                                                   CelHttpResponse *rsp, 
                                                   CelAsyncResult *result)
{
    CelHttpRequest *req = client->execute_req;
    CelHttpExecuteCallbackFunc execute_callback = client->execute_callback;

    if (result->ret <= 0)
    {
        CEL_ERR((_T("cel_httpclient_do_recv_response failed############1")));
        result->ret = -1;
        execute_callback(client, req, rsp, result);
        return;
    }
    //printf("%s\r\n",(char *)rsp->s.buffer);
    result->ret = 0;
    execute_callback(client, req, rsp, result);
}

static void cel_httpclient_send_callback(CelHttpClient *client, 
                                         CelHttpRequest *req,
                                         CelAsyncResult *result)
{
    CelHttpResponse *rsp = client->execute_rsp;
    CelHttpExecuteCallbackFunc execute_callback = client->execute_callback;

    if (result->ret <= 0)
    {
        CEL_ERR((_T("cel_httpclient_do_send_request failed############1")));
        result->ret = -1;
        execute_callback(client, req, rsp, result);
        return;
    }
    //puts("send request ok");
    if (cel_httpclient_async_recv_response(
        client, rsp, cel_httpclient_response_async_callback) == -1)
    {
        CEL_ERR((_T("cel_httpclient_do_send_request failed############3")));
        result->ret = -1;
        execute_callback(client, req, rsp, result);
    }
}

static void cel_httpclient_handshake_callback(CelHttpClient *client,
                                              CelAsyncResult *result)
{
    CelHttpRequest *req = client->execute_req;
    CelHttpResponse *rsp = client->execute_rsp;
    CelHttpExecuteCallbackFunc execute_callback = client->execute_callback;

    if (result->ret != 1)
    {
        CEL_ERR((_T("cel_httpclient_do_handshake failed############1")));
        result->ret = -1;
        execute_callback(client, req, rsp, result);
        return;
    }
    //puts("handshake ok");
    cel_httprequest_end(req);
    //puts((char *)args->req->s.buffer);
    if (cel_httpclient_async_send_request(
        client, req, cel_httpclient_send_callback) == -1)
    {
        CEL_ERR((_T("cel_httpclient_do_handshake failed############2")));
        result->ret = -1;
        execute_callback(client, req, rsp, result);
    }
}

static void cel_httpclient_connect_callback(CelHttpClient *client,
                                            CelAsyncResult *result)
{
    CelHttpRequest *req = client->execute_req;
    CelHttpResponse *rsp = client->execute_rsp;
    CelHttpExecuteCallbackFunc execute_callback = client->execute_callback;

    if (result->ret != 0)
    {
        CEL_ERR((_T("cel_httpclient_do_conenct failed############1")));
        result->ret = -1;
        execute_callback(client, req, rsp, result);
        return;
    }
    //puts("connect ok");
    if (cel_httpclient_async_handshake(client, 
        cel_httpclient_handshake_callback) == -1)
    {
        CEL_ERR((_T("cel_httpclient_do_conenct failed############2")));
        result->ret = -1;
        execute_callback(client, req, rsp, result);
    }
}

int cel_httpclient_async_execute(CelHttpClient *client, 
                                 CelHttpRequest *req, CelHttpResponse *rsp,
                                 CelHttpExecuteCallbackFunc async_callback)
{
    char *host;
    unsigned short port;

    //printf("http client fd %d\r\n", client->tcp_client.sock.fd);
    CEL_ASSERT(async_callback == NULL);
    client->execute_req = req;
    client->execute_rsp = rsp;
    client->execute_callback = async_callback;
    if (!cel_httpclient_is_connected(client))
    {
        if (req->url.scheme == CEL_HTTPSCHEME_HTTPS)
            cel_tcpclient_set_ssl(&(client->tcp_client), TRUE);
        else
            cel_tcpclient_set_ssl(&(client->tcp_client), FALSE);
        if ((host = cel_httprequest_get_url_host(req)) == NULL
            || (port = cel_httprequest_get_url_port(req)) == 0)
        {
            CEL_ERR((_T("cel_httpclient_async_execute error")));
            return -1;
        }
        return cel_httpclient_async_connect_host(client,
            host, port, cel_httpclient_connect_callback);
    }
    return cel_httpclient_async_send_request(client,
        req, cel_httpclient_send_callback);
}
