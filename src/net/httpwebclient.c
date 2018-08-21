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
#include "cel/net/httpwebclient.h"
#undef _CEL_DEBUG
//#define _CEL_DEBUG
#include "cel/log.h"
#include "cel/error.h"
#include "cel/allocator.h"
#include "cel/file.h"

void _cel_httpwebclient_destroy_derefed(CelHttpWebClient *client)
{
    cel_httprequest_destroy(&(client->req));
    cel_httpresponse_destroy(&(client->rsp));
    cel_httproutedata_destroy(&(client->rt_data));
    client->execute_callback = NULL;
    //puts("_cel_httpwebclient_destroy_derefed");
    cel_httpclient_destroy(&(client->http_client));
}

void cel_httpwebclient_destroy(CelHttpWebClient *client)
{
    cel_refcounted_destroy(&(client->ref_counted), client);
}

void _cel_httpwebclient_free_derefed(CelHttpWebClient *client)
{
    //puts("_cel_httpwebclient_free_derefed");
    _cel_httpwebclient_destroy_derefed(client);
    if (client->web_ctx->free_func != NULL)
        client->web_ctx->free_func(client);
    else
        cel_free(client);
}

CelHttpWebClient *cel_httpwebclient_new_httpclient(CelHttpClient *http_client,
                                                   CelHttpWebContext *web_ctx)
{
    CelHttpWebClient *new_client;

    if ((new_client = (web_ctx->new_func != NULL ? 
        web_ctx->new_func(sizeof(CelHttpWebClient), 
        http_client->tcp_client.sock.fd)
        : cel_malloc(sizeof(CelHttpWebClient)))) == NULL)
    {
        CEL_ERR((_T("Http web client new return null.")));
        return NULL;
    }
    memcpy(&(new_client->http_client), http_client, sizeof(CelHttpClient));
    new_client->web_ctx = web_ctx;
    cel_httprequest_init(&(new_client->req));
    cel_httpresponse_init(&(new_client->rsp));
    cel_httproutedata_init(&(new_client->rt_data));
    new_client->execute_callback = NULL;
    cel_refcounted_init(&(new_client->ref_counted),
        (CelFreeFunc)_cel_httpwebclient_free_derefed);
    return new_client;
}

CelHttpWebClient *cel_httpwebclient_new(CelSslContext *ssl_ctx,
                                        CelHttpWebContext *web_ctx)
{
    CelHttpClient http_client;
    CelHttpWebClient *new_client;

    if (cel_httpclient_init_family(&http_client, AF_INET, ssl_ctx) == 0)
    {
       if ((new_client = cel_httpwebclient_new_httpclient(
           &http_client, web_ctx)) != NULL)
           return new_client;
       cel_httpclient_destroy(&http_client);

    }
    return NULL;
}

void cel_httpwebclient_free(CelHttpWebClient *client)
{
    /*_tprintf(_T("Http web client %s closed\r\n"), 
        cel_httpwebclient_get_remoteaddr_str(client));*/
    cel_refcounted_destroy(&(client->ref_counted), client);
}

void cel_httpwebclient_do_recv_request(CelHttpWebClient *client, 
                                       CelHttpRequest *req,
                                       CelAsyncResult *result)
{
    size_t ver_end;
    char *url;
    CelHttpHandleFunc handler;

    /*printf("cel_httpwebclient_do_recv_request,"
             "result = %ld, reading_state %d\r\n", 
              result->ret, req->reading_state);
    puts((char *)req->s.buffer);*/
    switch (req->reading_state)
    {
    case CEL_HTTPREQUEST_READING_INIT:
    case CEL_HTTPREQUEST_READING_METHOD:
    case CEL_HTTPREQUEST_READING_URL:
    case CEL_HTTPREQUEST_READING_VERSION:
    case CEL_HTTPREQUEST_READING_HEADER:
    case CEL_HTTPREQUEST_READING_BODY:
        if (result->ret == -1)
        {
            CEL_ERR((_T("cel_httpwebclient_do_recv_request return -1")));
            cel_httpwebclient_free(client);
        }
        else
        {
            if (cel_httpclient_async_recv_request(&(client->http_client),
                req, 
                (CelHttpRecvRequestCallbackFunc)
                cel_httpwebclient_do_recv_request) == -1)
            {
                CEL_ERR((_T("cel_httpclient_async_recv_request return -1")));
                cel_httpwebclient_free(client);
            }
        }
        return ;
    case CEL_HTTPREQUEST_READING_OK:
        /* Init response header */
        cel_httpresponse_set_header(&(client->rsp), CEL_HTTPHDR_SERVER, 
            client->web_ctx->server, strlen(client->web_ctx->server));
        cel_httpresponse_set_header(&(client->rsp), CEL_HTTPHDR_CONNECTION,
            &(client->req.connection), sizeof(CelHttpConnection));
       /* Process request */
        ver_end = client->web_ctx->prefix.key_len;
        if ((url = cel_httprequest_get_url_path(req)) == NULL
            || cel_keyword_ncmp_a(
            &client->web_ctx->prefix, url, ver_end) != 0)
        {
            printf("method %d, %s\r\n", cel_httprequest_get_method(req), url);
            cel_httpwebclient_async_send_response_result(client, 
                CEL_HTTPWEB_UNSUPPORTED_OPERATION_EXCEPTION, 0,
                "Http web request prefix not supported.");
            return ;
        }
        
        if ((handler = cel_httproute_handler(
            &(client->web_ctx->route), 
            cel_httprequest_get_method(req), 
            url + ver_end, 
            &(client->rt_data))) == NULL)
        {
            printf("method %d, %s\r\n", 
                cel_httprequest_get_method(req), url + ver_end);
            cel_httpwebclient_async_send_response_result(client, 
                CEL_HTTPWEB_UNSUPPORTED_OPERATION_EXCEPTION, 0,
                "Http web request not supported.");
            return ;
        }
        if (handler(&(client->http_client), 
            req, &(client->rsp), &(client->rt_data)) == -1)
        {
            cel_httpwebclient_async_send_response_result(client, 
                CEL_HTTPWEB_IO_EXCEPTION, 0,
                "Http web request handler failed.");
        }
        break;
    default:
        cel_httpwebclient_async_send_response_result(client, 
            CEL_HTTPWEB_UNSUPPORTED_OPERATION_EXCEPTION, 0,
            "Http web request(http) reuqest state invaild .");
    }
}

void _cel_httpwebclient_execute_callback(CelHttpWebClient *client,
                                         CelHttpRequest *req,
                                         CelHttpResponse *rsp,
                                         CelAsyncResult *result)
{
    if (client->execute_callback != NULL)
        client->execute_callback(client, result);
    else
        cel_httpwebclient_free(client);
}

void cel_httpwebclient_do_send_response(CelHttpWebClient *client,
                                        CelHttpResponse *rsp, 
                                        CelAsyncResult *result)
{
    CelHttpConnection *connection;

    if (client->execute_callback != NULL)
    {
        //puts("execute_callback");
        client->execute_callback(client, result);
        return;
    }
    if (result->ret != -1
        && (connection = (CelHttpConnection *)cel_httprequest_get_header(
        &(client->req), CEL_HTTPHDR_CONNECTION)) != NULL
        && *connection == CEL_HTTPCON_KEEPALIVE)
    {
        //puts("CEL_HTTPCON_KEEPALIVE");
        cel_httprequest_clear(&(client->req));
        cel_httpresponse_clear(&(client->rsp));
        cel_httproutedata_clear(&(client->rt_data));
        client->execute_callback = NULL;
        client->file_len = 0;
        if (cel_httpclient_async_recv_request(&(client->http_client), 
            &(client->req), 
            (CelHttpRecvRequestCallbackFunc)cel_httpwebclient_do_recv_request) == -1)
            cel_httpwebclient_free(client);
    }
    else
    {
        //puts("cel_httpwebclient_free");
        cel_httpwebclient_free(client);
    }
}

int cel_httpwebclient_async_send_response_result(CelHttpWebClient *client,
                                                 CelHttpStatusCode status,
                                                 int err_no, const char *msg)
{
    //printf("rsp %s\r\n", client->rsp.s.buffer);
    switch (status)
    {
    case CEL_HTTPSC_REQUEST_OK:
    case CEL_HTTPSC_CREATED:
    case CEL_HTTPSC_ACCEPTED:
    case CEL_HTTPSC_NO_CONTENT:
        cel_httpresponse_set_statuscode(&(client->rsp), status);
        if (client->rsp.writing_body_offset == 0)
        {
            cel_httpresponse_set_header(&(client->rsp), 
                CEL_HTTPHDR_CONTENT_TYPE,
                CEL_HTTPWEB_CONTENT_TYPE, CEL_HTTPWEB_CONTENT_TYPE_LEN);
            if (msg != NULL)
                cel_httpresponse_printf(&(client->rsp), 
                "{\"error\":%d,\"message\":\"%s\"}", err_no, msg);
            else
                cel_httpresponse_write(&(client->rsp), 
                CEL_HTTPWEB_SUCCESSED_MSG, CEL_HTTPWEB_SUCCESSED_MSG_LEN);
        }
        cel_httpresponse_end(&(client->rsp));
        break;
    case CEL_HTTPSC_BAD_REQUEST:
    case CEL_HTTPSC_UNAUTHORIZED:
    case CEL_HTTPSC_FORBIDDEN:
    case CEL_HTTPSC_NOT_FOUND:
    case CEL_HTTPSC_ERROR:
        cel_httpresponse_set_statuscode(&(client->rsp), status);
        if (client->rsp.writing_body_offset == 0)
        {
            cel_httpresponse_set_header(&(client->rsp), 
                CEL_HTTPHDR_CONTENT_TYPE,
                CEL_HTTPWEB_CONTENT_TYPE, CEL_HTTPWEB_CONTENT_TYPE_LEN);
        }
        cel_httpresponse_printf(&(client->rsp), 
            "{\"error\":%d,\"message\":\"%s\"}", err_no, msg);
        cel_httpresponse_end(&(client->rsp));
        break;
    default:
        CEL_ERR((_T("cel_httpwebclient_result error")));
        cel_httpwebclient_free(client);
        return -1;
    }
   /* if (client->web_ctx->xx_func != NULL)
        client->web_ctx->xx_func(client, message);*/
    if (cel_httpclient_async_send_response(&(client->http_client), 
        &(client->rsp),
        (CelHttpSendResponseCallbackFunc)
        cel_httpwebclient_do_send_response) == -1)
    {
        cel_httpwebclient_free(client);
        return -1;
    }
    return 0;
}

//const char *cel_httpwebclient_get_request_file_path(CelHttpWebClient *client,
//                                                    char *path, size_t *path_len)
//{
//    size_t path_size = cel_httprequest_get_url_path_size(&(client->req));
//    char *path_str = cel_httprequest_get_url_path(&(client->req));
//
//    if (client->file_len > 0)
//    {
//        //printf("*path_len = %zu\r\n", *path_len);
//        if (*path_len > client->file_len)
//            *path_len = client->file_len;
//        else
//            *path_len  = *path_len - 1;
//        //printf("file_len %zu\r\n", client->file_len);
//        //printf("*path_len = %zu\r\n", *path_len);
//        memcpy(path, path_str + (path_size - client->file_len), *path_len);
//        path[*path_len] = '\0';
//        return path;
//    }
//    path[0] = '\0';
//    *path_len = 0;
//    return NULL;
//}
