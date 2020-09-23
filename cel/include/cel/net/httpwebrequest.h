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
#ifndef __CEL_NET_HTTPWEBREQUEST_H__
#define __CEL_NET_HTTPWEBREQUEST_H__

#include "cel/net/httpclient.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelHttpWebRequest CelHttpWebRequest;
typedef void (* CelHttpWebCallbackFunc)(CelHttpWebRequest *web_req,
										CelAsyncResult *result);

struct _CelHttpWebRequest
{
	CelHttpClient http_client;
	CelHttpRequest req;
	CelHttpResponse rsp;
	CelHttpWebCallbackFunc execute_callback;
	void *user_data;
	CelRefCounted ref_counted;
};

void _cel_httpwebrequest_destroy_derefed(CelHttpWebRequest *web_req);
void cel_httpwebrequest_destroy(CelHttpWebRequest *web_req);

CelHttpWebRequest *cel_httpwebrequest_new_httpclient(CelHttpClient *http_client);
CelHttpWebRequest *cel_httpwebrequest_new(CelSslContext *ssl_ctx);
void _cel_httpwebrequest_free_derefed(CelHttpWebRequest *web_req);
void cel_httpwebrequest_free(CelHttpWebRequest *web_req);

#define cel_httpwebrequest_set_nonblock(web_req, is_nonblock) \
    cel_socket_set_nonblock((CelSocket *)web_req, is_nonblock)
#define cel_httpwebrequest_get_channel(web_req) \
    &((web_req)->http_client.tcp_client.sock.channel)
#define cel_httpwebrequest_get_localaddr(web_req) \
    cel_httpclient_get_localaddr(web_req)
#define cel_httpwebrequest_get_remoteaddr(web_req) \
    cel_httpclient_get_remoteaddr(web_req)
#define cel_httpwebrequest_get_remoteaddr_str(web_req) \
    cel_sockaddr_ntop(cel_httpclient_get_remoteaddr(web_req))
#define cel_httpwebrequest_get_localaddr_str(web_req) \
    cel_sockaddr_ntop(cel_httpclient_get_localaddr(web_req))
#define cel_httpwebrequest_get_url_path(web_req) \
	cel_httprequest_get_url_path(&(web_req->req))

#define cel_httpwebrequest_get_request(web_req)  &((web_req)->req)
#define cel_httpwebrequest_get_response(web_req) &((web_req)->rsp)
#define cel_httpwebrequest_get_routedata(web_req) &((web_req)->http_ctx)

void _cel_httpwebrequest_execute_callback(CelHttpWebRequest *web_req,
										  CelHttpRequest *req, CelHttpResponse *rsp,
										  CelAsyncResult *result);
static __inline 
int cel_httpwebrequest_async_execute_request(CelHttpWebRequest *web_req,
                                            CelHttpWebCallbackFunc callback)
{
    web_req->execute_callback = callback;
    return cel_httpclient_async_execute(&(web_req->http_client), 
        &(web_req->req), &(web_req->rsp), 
        (CelHttpExecuteCallbackFunc)_cel_httpwebrequest_execute_callback);
}

#ifdef __cplusplus
}
#endif

#endif
