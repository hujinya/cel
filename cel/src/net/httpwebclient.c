/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2019 Hu Jinya(hu_jinya@163.com) 
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
//#define _CEL_DEBUG
#include "cel/log.h"
#include "cel/error.h"
#include "cel/allocator.h"
#include "cel/file.h"

void _cel_httpwebclient_destroy_derefed(CelHttpWebClient *client)
{
    //puts("_cel_httpwebclient_destroy_derefed");
    cel_httpclient_destroy(&(client->http_client));

    cel_httprequest_destroy(&(client->req));
    cel_httpresponse_destroy(&(client->rsp));
    cel_httpcontext_destroy(&(client->http_ctx));
    client->execute_callback = NULL;
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
        web_ctx->new_func(
        sizeof(CelHttpWebClient), http_client->tcp_client.sock.fd)
        : cel_malloc(sizeof(CelHttpWebClient)))) == NULL)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Http web client new return null.")));
        return NULL;
    }
    memcpy(&(new_client->http_client), http_client, sizeof(CelHttpClient));
    new_client->web_ctx = web_ctx;
    cel_httprequest_init(&(new_client->req));
    cel_httpresponse_init(&(new_client->rsp));
    cel_httpcontext_init(&(new_client->http_ctx),
		&(new_client->http_client), &(new_client->req), &(new_client->rsp));
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
    CEL_DEBUG((_T("Http web client %s closed."), 
        cel_httpwebclient_get_remoteaddr_str(client)));
    cel_refcounted_destroy(&(client->ref_counted), client);
}

#include "cel/net/if.h"

void cel_httpwebclient_update_host(CelHttpWebClient *client,
								   CelHttpRequest *req)
{
    int i;
    char *ipstr, _ipstr[CEL_IPSTRLEN];

	//update client host
    if ((ipstr = cel_httprequest_get_header(
        req, CEL_HTTPHDR_X_FORWARDED_FOR)) != NULL)
    {
        i = 0;
        while (ipstr[i] != '\0' && ipstr[i] != ',')
        {
            _ipstr[i] = ipstr[i];
            i++;
        }
        _ipstr[i] = '\0';
        cel_sockaddr_init_ipstr(
            &(client->http_client.tcp_client.remote_addr), _ipstr, 0);
    }
    else if((ipstr = cel_httprequest_get_header(
        req, CEL_HTTPHDR_X_REAL_IP)) != NULL)
    {
        cel_sockaddr_init_ipstr(
            &(client->http_client.tcp_client.remote_addr), ipstr, 0);
    }
	//update server host
}

const char *cel_httpwebclient_get_request_file_path(CelHttpWebClient *client,
													char *path, size_t *path_len)
{
    size_t path_size = cel_httprequest_get_url_path_size(&(client->req));
    char *path_str = cel_httprequest_get_url_path(&(client->req));

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

void cel_httpwebclient_do_shutdown(CelHttpWebClient *client, 
                                   CelAsyncResult *result)
{
    cel_httpwebclient_free(client);
}

void _cel_httpwebclient_execute_callback(CelHttpWebClient *client,
                                         CelHttpRequest *req,
										 CelHttpResponse *rsp,
										 CelAsyncResult *result)
{
    if (client->execute_callback != NULL)
        client->execute_callback(client, result);
    else
    {
        if (cel_httpclient_async_shutdown(&(client->http_client), 
            (CelHttpShutdownCallbackFunc)cel_httpwebclient_do_shutdown) == -1)
        {
            CEL_SETERR((CEL_ERR_LIB,
				_T("cel_httpwebclient_async_send_response_file %s return -1"),
                cel_httpwebclient_get_remoteaddr_str(client)));
            cel_httpwebclient_free(client);
        }
    }
}

void cel_httpwebclient_do_send_response(CelHttpWebClient *client,
                                        CelHttpResponse *rsp,
                                        CelAsyncResult *result)
{
    CelHttpConnection *connection;

    //printf("cel_httpwebclient_do_send_response result %d\r\n", result->ret);
    if (client->execute_callback != NULL)
    {
        //puts("execute_callback");
        client->execute_callback(client, result);
        return;
    }
    if (result->ret <= 0)
    {
        CEL_SETERR((CEL_ERR_LIB,  
			_T("cel_httpwebclient_do_send_response %s return -1"),
            cel_httpwebclient_get_remoteaddr_str(client)));
        cel_httpwebclient_free(client);
        return ;
    }
    if ((connection = (CelHttpConnection *)cel_httprequest_get_header(
        &(client->req), CEL_HTTPHDR_CONNECTION)) != NULL
        && *connection == CEL_HTTPCON_KEEPALIVE)
    {
        //puts("CEL_HTTPCON_KEEPALIVE");
        cel_httprequest_clear(&(client->req));
        cel_httpresponse_clear(&(client->rsp));
        cel_httpcontext_clear(&(client->http_ctx));
        client->execute_callback = NULL;
        client->file_len = 0;
        if (cel_httpclient_async_recv_request(&(client->http_client), 
            &(client->req), 
            (CelHttpRecvRequestCallbackFunc)
            cel_httpwebclient_do_recv_request) == -1)
            cel_httpwebclient_free(client);
    }
    else
    {
        //puts("cel_httpwebclient_free");
        if (cel_httpclient_async_shutdown(&(client->http_client), 
            (CelHttpShutdownCallbackFunc)cel_httpwebclient_do_shutdown) == -1)
        {
            CEL_SETERR((CEL_ERR_LIB,  
				_T("cel_httpwebclient_do_send_response %s shutdown return -1"),
                cel_httpwebclient_get_remoteaddr_str(client)));
            cel_httpwebclient_free(client);
        }
    }
}

int cel_httpwebclient_routing(CelHttpWebClient *client)
{
	int ret;

	if ((ret = cel_httproute_routing(&(client->web_ctx->route), 
		&(client->http_ctx))) == CEL_RET_AGAIN)
	{
		CEL_DEBUG((_T("Httpwebclient routing wait...")));
		return CEL_RET_AGAIN;
	}
	else if (ret == CEL_RET_ERROR)
	{
		if (cel_httpresponse_get_statuscode(&(client->rsp)) == CEL_HTTPSC_REQUEST_OK)
			cel_httpresponse_set_statuscode(&(client->rsp), CEL_HTTPSC_ERROR);
		cel_httpresponse_printf(&(client->rsp), 
			"{\"error\":%d,\"message\":\"%s\"}", 
			cel_geterrno(), cel_geterrstr());
		cel_httpresponse_end(&(client->rsp));
		//puts(cel_geterrstr());
	}
	//printf("rsp %s\r\n", client->rsp.s.buffer);
	if (cel_httpclient_async_send_response(&(client->http_client), 
		&(client->rsp),
		(CelHttpSendResponseCallbackFunc)
		cel_httpwebclient_do_send_response) == -1)
	{
		CEL_SETERR((CEL_ERR_LIB,  
			_T("cel_httpwebclient_async_send_response %s error"),
			cel_httpwebclient_get_remoteaddr_str(client)));
		cel_httpwebclient_free(client);
		return -1;
	}
	return 1;
}

void cel_httpwebclient_do_recv_request(CelHttpWebClient *client,
                                       CelHttpRequest *req,
                                       CelAsyncResult *result)
{
    /*printf("cel_httpwebclient_do_recv_request,"
             "result = %ld, reading_state %d\r\n", 
              result->ret, req->reading_state);
    puts((char *)req->hs.s.buffer);*/
    switch (req->reading_state)
    {
    case CEL_HTTPREQUEST_READING_INIT:
    case CEL_HTTPREQUEST_READING_METHOD:
    case CEL_HTTPREQUEST_READING_URL:
    case CEL_HTTPREQUEST_READING_VERSION:
    case CEL_HTTPREQUEST_READING_HEADER:
    case CEL_HTTPREQUEST_READING_BODY:
        if (result->ret <= 0)
        {
            //if (req->reading_state > CEL_HTTPREQUEST_READING_METHOD)
			CEL_SETERR((CEL_ERR_LIB,  
				_T("cel_httpwebclient_do_recv_request %s return -1"),
				cel_httpwebclient_get_remoteaddr_str(client)));
			cel_httpwebclient_free(client);
        }
        else
        {
            if (cel_httpclient_async_recv_request(&(client->http_client),
                req, 
                (CelHttpRecvRequestCallbackFunc)
                cel_httpwebclient_do_recv_request) == -1)
            {
                //if (req->reading_state > CEL_HTTPREQUEST_READING_METHOD)
				CEL_SETERR((CEL_ERR_LIB, 
					_T("cel_httpclient_async_recv_request %s return -1"),
					cel_httpwebclient_get_remoteaddr_str(client)));
				cel_httpwebclient_free(client);
            }
        }
        return ;
    case CEL_HTTPREQUEST_READING_OK:
		//cel_httpwebclient_update_host(client, req);
		/* Init response common header */
		cel_httpresponse_set_header(&(client->rsp), CEL_HTTPHDR_SERVER,
			client->web_ctx->server, strlen(client->web_ctx->server));
		cel_httpresponse_set_header(&(client->rsp), CEL_HTTPHDR_CONNECTION,
			&(req->connection), sizeof(CelHttpConnection));
		cel_httpwebclient_routing(client);
		break;
	default:
		CEL_SETERR((CEL_ERR_LIB, _T("Http web request state '%d' invaild"), 
			req->reading_state));
		cel_httpwebclient_free(client);
    }
}

int cel_httpwebclient_response_write(CelHttpWebClient *client,
									 CelHttpStatusCode status,
									 int err_no, const char *msg)
{
	switch (status)
	{
	case CEL_HTTPSC_REQUEST_OK:
		//cel_httpresponse_set_statuscode(&(client->rsp), status);
		if (client->rsp.writing_body_offset == 0)
		{
			cel_httpresponse_set_header(&(client->rsp), 
				CEL_HTTPHDR_CONTENT_TYPE,
				CEL_HTTPWEB_CONTENT_TYPE, CEL_HTTPWEB_CONTENT_TYPE_LEN);
			if (msg != NULL)
			{
				cel_httpresponse_printf(&(client->rsp), 
					"{\"error\":%d,\"message\":\"%s\"}", err_no, msg);
				cel_httpresponse_end(&(client->rsp));
			}
			else
			{
				cel_httpresponse_send(&(client->rsp), status,
					CEL_HTTPWEB_SUCCESSED_MSG, CEL_HTTPWEB_SUCCESSED_MSG_LEN);
			}
		}
		else
		{
			cel_httpresponse_end(&(client->rsp));
		}
		break;
    case CEL_HTTPSC_CREATED:
    case CEL_HTTPSC_ACCEPTED:
    case CEL_HTTPSC_NO_CONTENT:
        cel_httpresponse_set_statuscode(&(client->rsp), status);
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
        CEL_SETERR((CEL_ERR_LIB, _T("cel_httpwebclient_result error")));
        cel_httpwebclient_free(client);
        return -1;
    }
    return CEL_RET_OK;
}
