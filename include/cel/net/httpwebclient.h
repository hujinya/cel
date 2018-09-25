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
#ifndef __CEL_NET_HTTPWEBCLIENT_H__
#define __CEL_NET_HTTPWEBCLIENT_H__

#include "cel/net/httpclient.h"
#include "cel/net/httproute.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CEL_HTTPWEB_OK                              CEL_HTTPSC_REQUEST_OK
#define CEL_HTTPWEB_CREATED                         CEL_HTTPSC_CREATED
#define CEL_HTTPWEB_ACCEPTED                        CEL_HTTPSC_ACCEPTED
#define CEL_HTTPWEB_NO_CONTENT                      CEL_HTTPSC_NO_CONTENT

#define CEL_HTTPWEB_UNSUPPORTED_OPERATION_EXCEPTION CEL_HTTPSC_BAD_REQUEST
#define CEL_HTTPWEB_SECURITY_EXCEPTION              CEL_HTTPSC_UNAUTHORIZED
#define CEL_HTTPWEB_IO_EXCEPTION                    CEL_HTTPSC_FORBIDDEN
#define CEL_HTTPWEB_NOT_FOUND_EXCEPTION             CEL_HTTPSC_NOT_FOUND 
#define CEL_HTTPWEB_RUMTIME_EXCEPTION               CEL_HTTPSC_ERROR

#define CEL_HTTPWEB_CONTENT_TYPE         "application/json; charset=utf-8"
#define CEL_HTTPWEB_CONTENT_TYPE_LEN     (sizeof(CEL_HTTPWEB_CONTENT_TYPE) - 1)
#define CEL_HTTPWEB_SUCCESSED_MSG        "{\"error\":0, \"message\":\"Successed\"}"
#define CEL_HTTPWEB_SUCCESSED_MSG_LEN    (sizeof(CEL_HTTPWEB_SUCCESSED_MSG) - 1)

typedef struct _CelHttpWebClient CelHttpWebClient;
typedef void (* CelHttpWebCallbackFunc)(CelHttpWebClient *client, 
                                        CelAsyncResult *result);

typedef CelHttpWebClient *(*CelHttpWebClientNewFunc)(size_t size, int fd);
typedef void (*CelHttpWebClientFreeFunc)(CelHttpWebClient *client);

typedef struct _CelHttpWebContext
{
    char server[CEL_SNLEN];
    CelKeywordA prefix;
    CelHttpRoute route;
    CelHttpWebClientNewFunc new_func;
    CelHttpWebClientFreeFunc free_func;
}CelHttpWebContext;

struct _CelHttpWebClient
{
    CelHttpClient http_client;
    CelHttpWebContext *web_ctx;
    CelHttpRequest req;
    CelHttpResponse rsp;
    CelHttpRouteData rt_data;
    size_t file_len;
    CelHttpWebCallbackFunc execute_callback;
    void *user_data;
    CelRefCounted ref_counted;
};

void _cel_httpwebclient_destroy_derefed(CelHttpWebClient *client);
void cel_httpwebclient_destroy(CelHttpWebClient *client);

CelHttpWebClient *cel_httpwebclient_new_httpclient(CelHttpClient *http_client,
                                                   CelHttpWebContext *web_ctx);
CelHttpWebClient *cel_httpwebclient_new(CelSslContext *ssl_ctx, 
                                        CelHttpWebContext *web_ctx);
void _cel_httpwebclient_free_derefed(CelHttpWebClient *client);
void cel_httpwebclient_free(CelHttpWebClient *client);

#define cel_httpwebclient_set_nonblock(client, is_nonblock) \
    cel_socket_set_nonblock((CelSocket *)client, is_nonblock)
#define cel_httpwebclient_get_channel(client) \
    &((client)->http_client.tcp_client.sock.channel)
#define cel_httpwebclient_get_localaddr(client) \
    cel_httpclient_get_localaddr(client)
#define cel_httpwebclient_get_remoteaddr(client) \
    cel_httpclient_get_remoteaddr(client)
#define cel_httpwebclient_get_remoteaddr_str(client) \
    cel_sockaddr_ntop(cel_httpclient_get_remoteaddr(client))
#define cel_httpwebclient_get_localaddr_str(client) \
    cel_sockaddr_ntop(cel_httpclient_get_localaddr(client))

#define cel_httpwebclient_get_request(client)  &((client)->req)
#define cel_httpwebclient_get_response(client) &((client)->rsp)
#define cel_httpwebclient_get_routedata(client) &((client)->rt_data)

const char *cel_httpwebclient_get_request_file_path(CelHttpWebClient *client,
                                                    char *path, size_t *path_len);
static __inline 
const char *cel_httpwebclient_get_route_path(CelHttpWebClient *client)
{
    return (cel_httprequest_get_url_path(&(client->req))
        + client->web_ctx->prefix.key_len);
}
void cel_httpwebclient_do_recv_request(CelHttpWebClient *client, 
                                       CelHttpRequest *req,
                                       CelAsyncResult *result);
void _cel_httpwebclient_execute_callback(CelHttpWebClient *client,
                                         CelHttpRequest *req,
                                         CelHttpResponse *rsp,
                                         CelAsyncResult *result);
static __inline 
int cel_httpwebclient_async_execute_request(CelHttpWebClient *client,
                                            CelHttpWebCallbackFunc callback)
{
    client->execute_callback = callback;
    return cel_httpclient_async_execute(&(client->http_client), 
        &(client->req), &(client->rsp), 
        (CelHttpExecuteCallbackFunc)_cel_httpwebclient_execute_callback);
}

void cel_httpwebclient_do_send_response(CelHttpWebClient *client,
                                        CelHttpResponse *rsp, 
                                        CelAsyncResult *result);
static __inline 
int cel_httpwebclient_async_send_response(CelHttpWebClient *client,
                                          CelHttpWebCallbackFunc callback)
{
    client->execute_callback = callback;
    return cel_httpclient_async_send_response(&(client->http_client), 
        &(client->rsp), 
        (CelHttpSendResponseCallbackFunc)
        cel_httpwebclient_do_send_response);
}
int cel_httpwebclient_async_send_response_result(CelHttpWebClient *client,
                                                 CelHttpStatusCode status,
                                                 int code, const char *msg);
int cel_httpwebclient_async_send_response_file(CelHttpWebClient *client, 
                                               const char *file_path, 
                                               long long first, long long last,
                                               CelDateTime *if_modified_since,
                                               char *if_none_match,
                                               CelHttpWebCallbackFunc callback);
int cel_httpwebclient_async_send_response_redirect(CelHttpWebClient *client, 
                                                   const char *url,
                                                   CelHttpWebCallbackFunc callback);

#ifdef __cplusplus
}
#endif

#endif
