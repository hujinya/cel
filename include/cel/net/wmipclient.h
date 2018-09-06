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
#ifndef __CEL_NET_WMIPCLIENT_H__
#define __CEL_NET_WMIPCLIENT_H__

#include "cel/net/httpclient.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CEL_WMIPSC_OK                              200
#define CEL_WMIPSC_ILLEGAL_ARGUMENT_EXCEPTION      400
#define CEL_WMIPSC_UNSUPPORTED_OPERATION_EXCEPTION 400
#define CEL_WMIPSC_SECURITY_EXCEPTION              401
#define CEL_WMIPSC_IO_EXCEPTION                    402
#define CEL_WMIPSC_NOT_FOUND_EXCEPTION             403 
#define CEL_WMIPSC_RUMTIME_EXCEPTION               500

#define WMIP_CONTENT_TYPE "application/json; charset=utf-8"
#define WMIP_CONTENT_TYPE_LEN (sizeof(WMIP_CONTENT_TYPE) - 1)
#define WMIP_SUCCESSED "{\"error\":0, \"message\":\"Successed\"}"
#define WMIP_SUCCESSED_LEN (sizeof(WMIP_SUCCESSED) - 1)

typedef struct _CelWmipClient CelWmipClient;
typedef void (* CelWmipExecuteCallbackFunc)(
    CelWmipClient *client, CelAsyncResult *result);

typedef struct _CelWmipMessage
{
    CelHttpRequest req;
    CelHttpResponse rsp;
    CelWmipExecuteCallbackFunc execute_callback;
}CelWmipMessage;

typedef int (* CelWmipCallBackFunc) (CelWmipClient *client, 
                                     CelWmipMessage *message);
typedef CelWmipClient *(*CelWmipClientNewFunc)(size_t size, int fd);
typedef void (*CelWmipClientFreeFunc)(CelWmipClient *client);

typedef struct _CelWmipContext
{
    char server[CEL_SNLEN];
    CelKeywordA version;
    size_t ops_size;
    CelKeywordA *ops;
    CelWmipClientNewFunc new_func;
    CelWmipClientFreeFunc free_func;
}CelWmipContext;

struct _CelWmipClient
{
    CelHttpClient http_client;
    CelWmipContext *wmip_ctx;
    CelWmipMessage *wmip_msg;
    int op_index;
    size_t file_len;
    void *user_data;
    CelRefCounted ref_counted;
};

static __inline char *cel_httprequest_get_query_(CelHttpRequest *req, 
                                                 const char *key,
                                                 char *value, size_t size)
{
    size_t _size = size;
    return cel_httprequest_get_query(req, key, value, &_size);
}

static __inline char *cel_httprequest_get_form_(CelHttpRequest *req, 
                                                const char *key,
                                                char *value, size_t size)
{
    size_t _size = size;
    return cel_httprequest_get_form(req, key, value, &_size);
}
static __inline 
char *cel_httprequest_get_params_(CelHttpRequest *req, const char *key,
                                  char *value, size_t size)
{
    size_t _size = size;
    return cel_httprequest_get_params(req, key, value, &_size);
}

static __inline CelWmipMessage *cel_wmipmessage_new(void)
{
    CelWmipMessage *message;

    if ((message = (CelWmipMessage *)cel_malloc(
        sizeof(CelWmipMessage))) != NULL)
    {
        cel_httprequest_init(&(message->req));
        cel_httpresponse_init(&(message->rsp));
        message->execute_callback = NULL;
    }
    return message;
}

static __inline void cel_wmipmessage_free(CelWmipMessage *message)
{
    cel_httprequest_destroy(&(message->req));
    cel_httpresponse_destroy(&(message->rsp));
    message->execute_callback = NULL;
    cel_free(message);
}

void _cel_wmipclient_destroy_derefed(CelWmipClient *client);
void cel_wmipclient_destroy(CelWmipClient *client);

CelWmipClient *cel_wmipclient_new(CelSslContext *ssl_ctx, 
                                  CelWmipContext *wmip_ctx);
void _cel_wmipclient_free_derefed(CelWmipClient *client);
void cel_wmipclient_free(CelWmipClient *client);

#define cel_wmipclient_set_nonblock(client, is_nonblock) \
    cel_socket_set_nonblock((CelSocket *)client, is_nonblock)
#define cel_wmipclient_get_channel(client) \
    &((client)->http_client.tcp_client.sock.channel)
#define cel_wmipclient_get_localaddr(client) \
    cel_httpclient_get_localaddr(client)
#define cel_wmipclient_get_remoteaddr(client) \
    cel_httpclient_get_remoteaddr(client)
#define cel_wmipclient_get_remoteaddr_str(client) \
    cel_sockaddr_ntop(cel_httpclient_get_remoteaddr(client))
#define cel_wmipclient_get_localaddr_str(client) \
    cel_sockaddr_ntop(cel_httpclient_get_localaddr(client))

#define cel_wmipclient_get_httprequest(client) \
    &((client)->wmip_msg->req)
#define cel_wmipclient_get_httpresponse(client) \
    &((client)->wmip_msg->rsp)

void cel_wmipclient_do_recv_request(CelWmipClient *client, 
                                    CelHttpRequest *req, 
                                    CelAsyncResult *result);

#define cel_wmipclient_get_response_header(client, hdr, vaule, value_len) \
    cel_httpresponse_get_header(\
    &((client)->wmip_msg->rsp), hdr, value, value_len)
#define cel_wmipclient_set_response_header(client, hdr, value, value_len) \
    cel_httpresponse_set_header(\
    &((client)->wmip_msg->rsp), hdr, value, value_len)

static __inline 
void *cel_wmipclient_get_response_send_buffer(CelWmipClient *client)
{
    if (client->wmip_msg->rsp.writing_body_offset == 0)
    {
        cel_httpresponse_set_header(&(client->wmip_msg->rsp),
            CEL_HTTPHDR_SERVER, 
            client->wmip_ctx->server, strlen(client->wmip_ctx->server));
    }
    return cel_httpresponse_get_send_buffer(&(client->wmip_msg->rsp));
}
static __inline 
size_t cel_wmipclient_get_response_send_buffer_size(CelWmipClient *client)
{
    if (client->wmip_msg->rsp.writing_body_offset == 0)
    {
        cel_httpresponse_set_header(&(client->wmip_msg->rsp), 
            CEL_HTTPHDR_SERVER, 
            client->wmip_ctx->server, strlen(client->wmip_ctx->server));
    }
    return cel_httpresponse_get_send_buffer_size(&(client->wmip_msg->rsp));
}
static __inline 
void cel_wmipclient_seek_response_send_buffer(CelWmipClient *client, 
                                              int offset)
{
    cel_httpresponse_seek_send_buffer(&(client->wmip_msg->rsp), offset);
}
static __inline 
int cel_wmipclient_resize_response_send_buffer(CelWmipClient *client, 
                                               size_t resize)
{
    if (client->wmip_msg->rsp.writing_body_offset == 0)
    {
        cel_httpresponse_set_header(&(client->wmip_msg->rsp), 
            CEL_HTTPHDR_SERVER, 
            client->wmip_ctx->server, strlen(client->wmip_ctx->server));
    }
    return cel_httpresponse_resize_send_buffer(
        &(client->wmip_msg->rsp), resize);
}

#define cel_wmipclient_response_clear(client) \
    cel_httpresponse_clear(&((client)->wmip_msg->rsp))
static __inline 
int cel_wmipclient_response_write(CelWmipClient *client, void *buf, size_t size)
{
    if (client->wmip_msg->rsp.writing_body_offset == 0)
    {
        cel_httpresponse_set_header(&(client->wmip_msg->rsp), 
            CEL_HTTPHDR_SERVER, 
            client->wmip_ctx->server, strlen(client->wmip_ctx->server));
    }
    return cel_httpresponse_write(&((client)->wmip_msg->rsp), buf, size);
}
int cel_wmipclient_response_printf(CelWmipClient *client, 
                                   const char *fmt, ...);
#define cel_wmipclient_response_end(client) \
    cel_httpresponse_end(&((client)->wmip_msg->rsp))

int cel_wmipclient_async_response_send_result(CelWmipClient *client, 
                                              CelHttpStatusCode status, 
                                              int err_no, const char *msg);
#define cel_wmipclient_result(client, msg, status, err_no, err_msg) \
    cel_wmipclient_async_response_send_result(client, status, err_no, err_msg)

int cel_wmipclient_async_send_response(CelWmipClient *client,
                                       CelWmipExecuteCallbackFunc cmpl);

int cel_wmipclient_async_send_response_redirect(CelWmipClient *client, 
                                                const char *url,
                                                CelWmipExecuteCallbackFunc cmpl);
#define cel_wmipclient_async_response_send_redirect \
    cel_wmipclient_async_send_response_redirect
int cel_wmipclient_async_send_response_file(CelWmipClient *client, 
                                            const char *file_path, 
                                            long long first, long long last,
                                            CelDateTime *if_modified_since,
                                            char *if_none_match,
                                            CelWmipExecuteCallbackFunc cmpl);
#define cel_wmipclient_async_response_send_file \
    cel_wmipclient_async_send_response_file

#define cel_wmipclient_set_method(client, method) \
    cel_httprequest_set_method(&((client)->wmip_msg->req), method)
#define cel_wmipclient_set_url(client, url) \
    cel_httprequest_set_url_str(&((client)->wmip_msg->req), url)

const char *cel_wmipclient_get_request_file_path(CelWmipClient *client,
                                                 char *path, size_t *path_len);

#define cel_wmipclient_get_request_header(client, hdr, vaule, value_len) \
    cel_httprequest_get_header(\
    &((client)->wmip_msg->req), hdr, value, value_len)
#define cel_wmipclient_set_request_header(client, hdr, value, value_len) \
    cel_httprequest_set_header(\
    &((client)->wmip_msg->req), hdr, value, value_len)

#define cel_wmipclient_request_clear(client) \
    cel_httprequest_clear(&((client)->wmip_msg->req))
#define cel_wmipclient_request_write(client, buf, size) \
    cel_httprequest_write(&((client)->wmip_msg->req), buf, size)
int cel_wmipclient_request_printf(CelWmipClient *client, const char *fmt, ...);
#define cel_wmipclient_request_end(client) \
    cel_httprequest_end(&((client)->wmip_msg->req))


int cel_wmipclient_async_send_request(CelWmipClient *client,
                                      CelWmipExecuteCallbackFunc callback);


#ifdef __cplusplus
}
#endif

#endif
