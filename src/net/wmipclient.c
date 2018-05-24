/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2016 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/net/wmipclient.h"
#include "cel/log.h"
#include "cel/error.h"
#include "cel/allocator.h"
#include "cel/file.h"

/* Debug defines */
#define Debug(args)   /*cel_log_debug args*/
#define Warning(args) CEL_SETERRSTR(args) /*cel_log_warning args*/
#define Err(args)   CEL_SETERRSTR(args) /*cel_log_err args*/

void _cel_wmipclient_destroy_derefed(CelWmipClient *client)
{
    if (client->wmip_msg != NULL)
        cel_wmipmessage_free(client->wmip_msg);
    //puts("_cel_wmipclient_destroy_derefed");
    cel_httpclient_destroy(&(client->http_client));
}

void cel_wmipclient_destroy(CelWmipClient *client)
{
    cel_refcounted_destroy(&(client->ref_counted), client);
}

void _cel_wmipclient_free_derefed(CelWmipClient *client)
{
    //puts("_cel_wmipclient_free_derefed");
    _cel_wmipclient_destroy_derefed(client);
    if (client->wmip_ctx->free_func != NULL)
        client->wmip_ctx->free_func(client);
    else
        cel_free(client);
}

CelWmipClient *cel_wmipclient_new(CelSslContext *ssl_ctx, 
                                  CelWmipContext *wmip_ctx)
{
    CelTcpClient tcp_client;
    CelWmipClient *new_client;
    CelWmipMessage *message;

    if (cel_tcpclient_init_family(&tcp_client, AF_INET, ssl_ctx) == 0)
    {
        if ((new_client = wmip_ctx->new_func != NULL ? 
            wmip_ctx->new_func(sizeof(CelWmipClient), tcp_client.sock.fd)
            : cel_malloc(sizeof(CelWmipClient))) == NULL)
        {
            Err((_T("Wmip client new return null.")));
            cel_tcpclient_destroy(&tcp_client);
            return NULL;
        }
        cel_httpclient_init_tcpclient(&(new_client->http_client), &tcp_client);
        if ((message = cel_wmipmessage_new()) != NULL)
        {
            new_client->wmip_ctx = wmip_ctx;
            new_client->wmip_msg = message;
            new_client->user_data = NULL;
            cel_refcounted_init(&(new_client->ref_counted),
                (CelFreeFunc)_cel_wmipclient_free_derefed);
            return new_client;
        }
        cel_httpclient_destroy(&(new_client->http_client));
        cel_free(new_client);
    }
    return NULL;
}

void cel_wmipclient_free(CelWmipClient *client)
{
    /*_tprintf(_T("Wmip2 client %s closed\r\n"), 
        cel_wmipclient_get_remoteaddrs(client));*/
    cel_refcounted_destroy(&(client->ref_counted), client);
}

int cel_wmipclient_response_printf(CelWmipClient *client, const char *fmt, ...)
{
    CelWmipMessage *message = client->wmip_msg;
    va_list _args;
    size_t size;

    if (message->rsp.writing_body_offset == 0)
    {
        cel_httpresponse_set_header(&(message->rsp), CEL_HTTPHDR_SERVER, 
            client->wmip_ctx->server, strlen(client->wmip_ctx->server));
    }
    va_start(_args, fmt);
    size = cel_httpresponse_vprintf(&(message->rsp), fmt, _args);
    va_end(_args);

    return size;
}

int cel_wmipclient_request_printf(CelWmipClient *client, const char *fmt, ...)
{
    CelWmipMessage *message = client->wmip_msg;
    va_list _args;
    size_t size;

    va_start(_args, fmt);
    size = cel_httprequest_vprintf(&(message->req), fmt, _args);
    va_end(_args);

    return size;
}

void cel_wmipclient_do_send_response(CelWmipClient *client, 
                                     CelHttpResponse *rsp, 
                                     CelAsyncResult *result)
{
    CelHttpConnection *connection;

    if (result->ret != -1
        && rsp->writing_state != CEL_HTTPRESPONSE_WRITING_OK)
    {
        //puts("cel_httpclient_async_send_response");
        if (cel_httpclient_async_send_response(&(client->http_client), rsp,
            (CelHttpSendResponseCallbackFunc)
            cel_wmipclient_do_send_response) != -1)
            return ;
    }
    if (client->wmip_msg->execute_callback != NULL)
    {
        //puts("execute_callback");
        client->wmip_msg->execute_callback(client, result);
        return;
    }
    if ((connection = (CelHttpConnection *)cel_httprequest_get_header(
        &(client->wmip_msg->req), CEL_HTTPHDR_CONNECTION)) != NULL
        && *connection == CEL_HTTPCON_KEEPALIVE)
    {
        //puts("CEL_HTTPCON_KEEPALIVE");
        cel_httprequest_clear(&(client->wmip_msg->req));
        cel_httpresponse_clear(&(client->wmip_msg->rsp));
        client->wmip_msg->execute_callback = NULL;
        client->op_index = -1;
        client->file_len = 0;
        cel_httpclient_async_recv_request(&(client->http_client), 
            &(client->wmip_msg->req), 
            (CelHttpRecvRequestCallbackFunc)cel_wmipclient_do_recv_request);
    }
    else
    {
        //puts("cel_wmipclient_free");
        cel_wmipclient_free(client);
    }
}

int cel_wmipclient_async_response_send_result(CelWmipClient *client, 
                                              CelHttpStatusCode status, 
                                              int err_no, const char *msg)
{
    CelWmipMessage *message = client->wmip_msg;

    //printf("rsp %s\r\n", message->rsp.s.buffer);
    cel_httpresponse_set_header(&(message->rsp), 
        CEL_HTTPHDR_CONNECTION,
        &(message->req.connection), sizeof(CelHttpConnection));
    switch (status)
    {
    case CEL_HTTPSC_REQUEST_OK:
        cel_httpresponse_set_statuscode(&(message->rsp), status);
        if (message->rsp.writing_body_offset == 0)
        {
            cel_httpresponse_set_header(&(message->rsp), 
                CEL_HTTPHDR_CONTENT_TYPE,
                WMIP_CONTENT_TYPE, WMIP_CONTENT_TYPE_LEN);
            cel_httpresponse_set_header(&(message->rsp), 
                CEL_HTTPHDR_SERVER, 
                client->wmip_ctx->server, strlen(client->wmip_ctx->server));
            if (msg != NULL)
                cel_httpresponse_printf(&(message->rsp), 
                "{\"error\":0,\"message\":\"%s\"}", err_no, msg);
            else
                cel_httpresponse_write(&(message->rsp), 
                WMIP_SUCCESSED, WMIP_SUCCESSED_LEN);
        }
        cel_httpresponse_end(&(message->rsp));
        break;
    case CEL_HTTPSC_BAD_REQUEST:
    case CEL_HTTPSC_UNAUTHORIZED:
    case CEL_HTTPSC_FORBIDDEN:
    case CEL_HTTPSC_NOT_FOUND:
    case CEL_HTTPSC_ERROR:
        cel_httpresponse_set_statuscode(&(message->rsp), status);
        if (message->rsp.writing_body_offset == 0)
        {
            cel_httpresponse_set_header(&(message->rsp), 
                CEL_HTTPHDR_CONTENT_TYPE,
                WMIP_CONTENT_TYPE, WMIP_CONTENT_TYPE_LEN);
            cel_httpresponse_set_header(&(message->rsp), 
                CEL_HTTPHDR_SERVER, 
                client->wmip_ctx->server, strlen(client->wmip_ctx->server));
        }
        cel_httpresponse_printf(&(message->rsp), 
            "{\"error\":%d,\"message\":\"%s\"}", err_no, msg);
        cel_httpresponse_end(&(message->rsp));
        break;
    default:
        Err((_T("cel_wmipclient_result error")));
        cel_wmipclient_free(client);
        return -1;
    }
   /* if (client->wmip_ctx->xx_func != NULL)
        client->wmip_ctx->xx_func(client, message);*/
    if (cel_httpclient_async_send_response(&(client->http_client), 
        &(message->rsp),
        (CelHttpSendResponseCallbackFunc)cel_wmipclient_do_send_response)
        == -1)
    {
        cel_wmipclient_free(client);
        return -1;
    }
    return 0;
}

void cel_wmipclient_do_recv_request(CelWmipClient *client, 
                                    CelHttpRequest *req,
                                    CelAsyncResult *result)
{
    size_t ver_end, action_end;
    char *url;

    /*printf("cel_wmipclient_do_recv_request result = %ld, reading_state %d\r\n", 
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
            Err((_T("cel_wmipclient_do_recv_request return -1")));
            cel_wmipclient_free(client);
        }
        else
        {
            if (cel_httpclient_async_recv_request(&(client->http_client),
                req, 
                (CelHttpRecvRequestCallbackFunc)
                cel_wmipclient_do_recv_request) == -1)
            {
                Err((_T("cel_httpclient_async_recv_request return -1")));
                cel_wmipclient_free(client);
            }
        }
        return ;
    case CEL_HTTPREQUEST_READING_OK:
        action_end = ver_end = client->wmip_ctx->version.key_len;
        if ((url = cel_httprequest_get_url_path(req)) == NULL
            || cel_keyword_ncmp_a(
            &client->wmip_ctx->version, url, ver_end) != 0)
        {
            //puts(url);
            cel_wmipclient_result(client, client->wmip_msg, 
                CEL_WMIPSC_ILLEGAL_ARGUMENT_EXCEPTION, 0,
                "Wmip version not supported.");
            return ;
        }
        action_end = cel_httprequest_get_url_path_size(req);
        //printf("%s, len %d\r\n", url + ver_end, action_end - ver_end);
        if ((client->op_index = cel_keyword_regex_search_a(
            client->wmip_ctx->ops, client->wmip_ctx->ops_size,
            url + ver_end, action_end - ver_end)) == -1)
        {
            cel_wmipclient_result(client, client->wmip_msg, 
                CEL_WMIPSC_ILLEGAL_ARGUMENT_EXCEPTION, 0,
                "Wmip operation not supported.");
            return ;
        }
        if (client->wmip_ctx->ops[client->op_index].value == NULL)
        {
            cel_wmipclient_result(client, client->wmip_msg, 
                CEL_WMIPSC_ILLEGAL_ARGUMENT_EXCEPTION, 0,
                "Wmip operation callback is null.");
            return ;
        }
        client->file_len = action_end - ver_end 
            - client->wmip_ctx->ops[client->op_index].key_len;
        ((CelWmipCallBackFunc)client->wmip_ctx->ops[client->op_index].value)
            (client, client->wmip_msg);
        break;
    default:
        cel_wmipclient_result(client, client->wmip_msg, 
            CEL_WMIPSC_ILLEGAL_ARGUMENT_EXCEPTION, 0,
            "Wmip(http) reuqest state invaild .");
    }
}

const char *cel_wmipclient_get_request_file_path(CelWmipClient *client,
                                                 char *path, size_t *path_len)
{
    size_t path_size = cel_httprequest_get_url_path_size(
        &(client->wmip_msg->req));
    char *path_str = cel_httprequest_get_url_path(
        &(client->wmip_msg->req));

    if (client->file_len > 0)
    {
        //printf("*path_len = %zu\r\n", *path_len);
        if (*path_len > client->file_len)
            *path_len = client->file_len;
        else
            *path_len  = *path_len - 1;
        //printf("file_len %zu\r\n", client->file_len);
        //printf("*path_len = %zu\r\n", *path_len);
        memcpy(path, path_str + (path_size - client->file_len), *path_len);
        path[*path_len] = '\0';
        return path;
    }
    path[0] = '\0';
    *path_len = 0;
    return NULL;
}

int cel_wmipclient_async_send_response(CelWmipClient *client,
                                       CelWmipExecuteCallbackFunc callback)
{
    CelWmipMessage *message = client->wmip_msg;

    message->execute_callback = callback;
   /* if (client->wmip_ctx->xx_func != NULL)
        client->wmip_ctx->xx_func(client, message);*/
    if (cel_httpclient_async_send_response(&(client->http_client), 
        &(message->rsp),
        (CelHttpSendResponseCallbackFunc)
        cel_wmipclient_do_send_response) == -1)
    {
        cel_wmipclient_free(client);
        return -1;
    }
    return 0;
}

int cel_wmipclient_async_response_send_redirect(CelWmipClient *client, 
                                                const char *url,
                                                CelWmipExecuteCallbackFunc callback)
{
    CelWmipMessage *message = client->wmip_msg;

    message->execute_callback = callback;

    cel_httpresponse_set_header(&(message->rsp), CEL_HTTPHDR_SERVER, 
        client->wmip_ctx->server, strlen(client->wmip_ctx->server));
    cel_httpresponse_send_redirect(&(message->rsp), url);
    /*if (client->wmip_ctx->xx_func != NULL)
        client->wmip_ctx->xx_func(client, message);*/
    if (cel_httpclient_async_send_response(&(client->http_client), 
        &(message->rsp),
        (CelHttpSendResponseCallbackFunc)
        cel_wmipclient_do_send_response) == -1)
    {
        cel_wmipclient_free(client);
        return -1;
    }
    return 0;
}

int cel_wmipclient_async_response_send_file(CelWmipClient *client, 
                                            const char *file_path, 
                                            long long first, long long last,
                                            CelDateTime *if_modified_since,
                                            char *if_none_match,
                                            CelWmipExecuteCallbackFunc callback)
{
    CelWmipMessage *message = client->wmip_msg;

    message->execute_callback = callback;
    cel_httpresponse_set_header(&(message->rsp), CEL_HTTPHDR_SERVER, 
        client->wmip_ctx->server, strlen(client->wmip_ctx->server));
    cel_httpresponse_send_file(&(message->rsp), 
        file_path, first, last, if_modified_since, if_none_match);
    /*if (client->wmip_ctx->xx_func != NULL)
        client->wmip_ctx->xx_func(client, message);*/
    if (cel_httpclient_async_send_response(&(client->http_client), 
        &(message->rsp),
        (CelHttpSendResponseCallbackFunc)
        cel_wmipclient_do_send_response) == -1)
    {
        cel_wmipclient_free(client);
        return -1;
    }
    return 0;
}

void cel_wmipclient_execute_callback(CelWmipClient *client, 
                                     CelHttpRequest *req, CelHttpResponse *rsp,
                                     CelAsyncResult *result)
{
    //puts("cel_wmipclient_execute_callback");
    if (client->wmip_msg->execute_callback != NULL)
        client->wmip_msg->execute_callback(client, result);
    else
        cel_wmipclient_free(client);
}

int cel_wmipclient_async_send_request(CelWmipClient *client, 
                                      CelWmipExecuteCallbackFunc async_callback)
{
    client->wmip_msg->execute_callback = async_callback;
    if (cel_httpclient_async_execute(&(client->http_client),
        &(client->wmip_msg->req), &(client->wmip_msg->rsp),
        (CelHttpExecuteCallbackFunc)cel_wmipclient_execute_callback) == -1)
    {
        puts("cel_httpclient_async_execute return -1");
        cel_wmipclient_free(client);
        return -1;
    }
    return 0;
}
