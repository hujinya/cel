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
#include "cel/net/httpcontext.h"
//#define _CEL_DEBUG
#include "cel/log.h"
#include "cel/error.h"
#include "cel/allocator.h"
#include "cel/file.h"

CelHttpContext *cel_httpcontext_new(CelHttpClient *client,
									CelHttpServeContext *serve_ctx)
{
	CelHttpContext *http_ctx;

    if ((http_ctx = (serve_ctx->new_func != NULL ? 
        serve_ctx->new_func(sizeof(CelHttpContext), client->tcp_client.sock.fd)
        : (CelHttpContext *)cel_malloc(sizeof(CelHttpContext)))) == NULL)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("cel_httpcontext_new return null.")));
        return NULL;
    }
	memcpy(&(http_ctx->http_client), client, sizeof(CelHttpClient));
	cel_httprequest_init(&(http_ctx->req));
    cel_httpresponse_init(&(http_ctx->rsp));
	cel_time_init(&(http_ctx->req_start_dt));
	cel_time_init(&(http_ctx->rsp_finish_dt));
	http_ctx->user[0] = '\0';
	http_ctx->user_data = NULL;
	http_ctx->serve_ctx = serve_ctx;
	http_ctx->state = 0;
	http_ctx->current_filter = NULL;
	cel_vstring_init(&(http_ctx->err_msg));
	cel_rbtree_init(&(http_ctx->params), (CelCompareFunc)strcmp, NULL, cel_free);
	return http_ctx;
}

void cel_httpcontext_free(CelHttpContext *http_ctx)
{
	cel_httpclient_destroy(&(http_ctx->http_client));
	cel_httprequest_destroy(&(http_ctx->req));
    cel_httpresponse_destroy(&(http_ctx->rsp));
	cel_time_destroy(&(http_ctx->req_start_dt));
	cel_time_destroy(&(http_ctx->rsp_finish_dt));
	http_ctx->user[0] = '\0';
	http_ctx->user_data = NULL;
	http_ctx->state = 0;
	//http_ctx->serve_ctx = NULL;
	http_ctx->current_filter = NULL;
	cel_vstring_destroy(&(http_ctx->err_msg));
	cel_rbtree_destroy(&(http_ctx->params));

	 if (http_ctx->serve_ctx->free_func != NULL)
        http_ctx->serve_ctx->free_func(http_ctx);
    else
        cel_free(http_ctx);
}

void cel_httpcontext_clear(CelHttpContext *http_ctx)
{
	cel_httprequest_clear(&(http_ctx->req));
    cel_httpresponse_clear(&(http_ctx->rsp));
	cel_time_clear(&(http_ctx->req_start_dt));
	cel_time_clear(&(http_ctx->rsp_finish_dt));
	http_ctx->user[0] = '\0';
	http_ctx->user_data = NULL;
	http_ctx->state = 0;
	http_ctx->current_filter = NULL;
	cel_vstring_clear(&(http_ctx->err_msg));
	cel_rbtree_clear(&(http_ctx->params));
}

static void cel_httpcontext_do_recv_request(CelHttpContext *http_ctx,
											CelHttpRequest *req,
											CelAsyncResult *result)
{
    /*printf("cel_httpcontext_do_recv_request,"
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
			CEL_SETERR((CEL_ERR_LIB,  
				_T("cel_httpcontext_do_recv_request %s return -1"),
				cel_httpclient_get_remoteaddr_str(&(http_ctx->http_client))));
			cel_httpcontext_free(http_ctx);
        }
        else
        {
            if (cel_httpclient_async_recv_request(&(http_ctx->http_client),
                req, 
                (CelHttpRecvRequestCallbackFunc)
				cel_httpcontext_do_recv_request) == -1)
            {
                //if (req->reading_state > CEL_HTTPREQUEST_READING_METHOD)
				CEL_SETERR((CEL_ERR_LIB, 
					_T("cel_httpclient_async_recv_request %s return -1"),
					cel_httpclient_get_remoteaddr_str(&(http_ctx->http_client))));
				cel_httpcontext_free(http_ctx);
            }
        }
        return ;
    case CEL_HTTPREQUEST_READING_OK:
		/* Init response common header */
		cel_httpresponse_set_header(&(http_ctx->rsp), CEL_HTTPHDR_SERVER,
			http_ctx->serve_ctx->server, strlen(http_ctx->serve_ctx->server));
		cel_httpresponse_set_header(&(http_ctx->rsp), CEL_HTTPHDR_CONNECTION,
			&(req->connection), sizeof(CelHttpConnection));
		http_ctx->state = CEL_HTTPROUTEST_BEFORE_ROUTER;
		cel_httpcontext_routing_again(http_ctx);
		break;
	default:
		CEL_SETERR((CEL_ERR_LIB, _T("Http web request state '%d' invaild"), 
			req->reading_state));
		cel_httpcontext_free(http_ctx);
    }
}

static void cel_httpcontext_do_handshake(CelHttpContext *http_ctx, CelAsyncResult *result)
{
	if (result->ret != 1)
	{
		CEL_SETERR((CEL_ERR_LIB, _T("httpserve_do_handshake failed, client %s"),
			cel_httpclient_get_remoteaddr_str(&(http_ctx->http_client))));
		cel_httpcontext_free(http_ctx);
		return ;
	}
	if (cel_httpclient_async_recv_request(&(http_ctx->http_client), 
		&(http_ctx->req), 
		(CelHttpRecvRequestCallbackFunc)cel_httpcontext_do_recv_request) == -1)
		cel_httpcontext_free(http_ctx);
	cel_time_init_now(&(http_ctx->req_start_dt));
}

static void cel_httpcontext_do_shutdown(CelHttpContext *http_ctx, CelAsyncResult *result)
{
    cel_httpcontext_free(http_ctx);
}

static void cel_httpcontext_do_send_response(CelHttpContext *http_ctx,
											 CelHttpResponse *rsp,
											 CelAsyncResult *result)
{
	//printf("cel_httpcontext_do_send_response result %d\r\n", result->ret);
    if (result->ret <= 0)
    {
        CEL_SETERR((CEL_ERR_LIB,  
			_T("cel_httpcontext_do_send_response %s return -1"),
            cel_httpclient_get_remoteaddr_str(&(http_ctx->http_client))));
        cel_httpcontext_free(http_ctx);
        return ;
    }
	cel_time_init_now(&(http_ctx->rsp_finish_dt));
	http_ctx->state = CEL_HTTPROUTEST_END;
	cel_httpcontext_routing_again(http_ctx);
}

int cel_httpcontext_routing(CelHttpContext *http_ctx)
{
	int ret;
	TCHAR *err_str;
	CelHttpConnection *connection;

	//Receive client request
	if (http_ctx->state == CEL_HTTPROUTEST_RECEVIE_REQUEST)
	{
		if (cel_httpclient_async_handshake(&(http_ctx->http_client),
			(CelHttpHandshakeCallbackFunc)cel_httpcontext_do_handshake) == -1)
		{
			CEL_SETERR((CEL_ERR_LIB, _T("Http context %s init failed(%s)."), 
				cel_httpclient_get_remoteaddr_str(&(http_ctx->http_client)), 
				cel_geterrstr()));
			cel_httpcontext_free(http_ctx);
			return CEL_RET_ERROR;
		}
		return CEL_RET_AGAIN;
	}
	// Http route
	if (http_ctx->state <= CEL_HTTPROUTEST_FINISH_ROUTER 
		&& http_ctx->state >= CEL_HTTPROUTEST_BEFORE_ROUTER)
	{
		if ((ret = cel_httproute_routing(&(http_ctx->serve_ctx->route), http_ctx)) == CEL_RET_AGAIN)
		{
			CEL_DEBUG((_T("Httpwebclient routing wait...")));
			return CEL_RET_AGAIN;
		}
		else if (ret == CEL_RET_ERROR)
		{
			if (cel_httpresponse_get_statuscode(&(http_ctx->rsp)) == CEL_HTTPSC_REQUEST_OK)
				cel_httpresponse_set_statuscode(&(http_ctx->rsp), CEL_HTTPSC_ERROR);
			cel_httpresponse_printf(&(http_ctx->rsp), "{\"error\":%d,\"message\":\"%s\"}", 
				cel_geterrno(), cel_geterrstr());
			cel_httpresponse_end(&(http_ctx->rsp));
			//puts(cel_geterrstr());
		}
		//printf("rsp %s\r\n", http_ctx->rsp.s.buffer);
		http_ctx->state = CEL_HTTPROUTEST_FINISH_ROUTER;
		if (cel_httpresponse_get_statuscode(&(http_ctx->rsp))/400 >= 1)
		{
			if ((err_str = cel_geterrstr()) != NULL)
				cel_vstring_assign(&(http_ctx->err_msg), err_str, strlen(err_str));
		}
		if (cel_httpclient_async_send_response(&(http_ctx->http_client), 
			&(http_ctx->rsp),
			(CelHttpSendResponseCallbackFunc)
			cel_httpcontext_do_send_response) == -1)
		{
			CEL_SETERR((CEL_ERR_LIB,  
				_T("cel_httpcontext_async_send_response %s error"),
				cel_httpclient_get_remoteaddr_str(&(http_ctx->http_client))));
			cel_httpcontext_free(http_ctx);
			return CEL_RET_ERROR;
		}
		return CEL_RET_AGAIN;
	}
	// Send respons to client
	if (http_ctx->state == CEL_HTTPROUTEST_END)
	{
		if (http_ctx->serve_ctx->route.logger_on)
			http_ctx->serve_ctx->route.logger_filter(http_ctx);

		if ((connection = (CelHttpConnection *)cel_httprequest_get_header(
			&(http_ctx->req), CEL_HTTPHDR_CONNECTION)) != NULL
			&& *connection == CEL_HTTPCON_KEEPALIVE)
		{
			//puts("CEL_HTTPCON_KEEPALIVE");
			cel_httpcontext_clear(http_ctx);
			if (cel_httpclient_async_recv_request(&(http_ctx->http_client), 
				&(http_ctx->req), 
				(CelHttpRecvRequestCallbackFunc)cel_httpcontext_do_recv_request) == -1)
			{
				cel_httpcontext_free(http_ctx);
				return CEL_RET_ERROR;
			}
			cel_time_init_now(&(http_ctx->req_start_dt));
			return CEL_RET_OK;
		}
		else
		{
			//puts("cel_httpcontext_free");
			if (cel_httpclient_async_shutdown(&(http_ctx->http_client), 
				(CelHttpShutdownCallbackFunc)cel_httpcontext_do_shutdown) == -1)
			{
				CEL_SETERR((CEL_ERR_LIB,  
					_T("cel_httpcontext_do_send_response %s shutdown return -1"),
					cel_httpclient_get_remoteaddr_str(&(http_ctx->http_client))));
				cel_httpcontext_free(http_ctx);
				return CEL_RET_ERROR;
			}
			return CEL_RET_DONE;
		}
	}
	return CEL_RET_DONE;
}

int cel_httpcontext_response_write(CelHttpContext *http_ctx,
								   CelHttpStatusCode status,
								   int err_no, const char *msg)
{
	switch (status)
	{
	case CEL_HTTPSC_REQUEST_OK:
		//cel_httpresponse_set_statuscode(&(client->rsp), status);
		if (&(http_ctx->rsp).writing_body_offset == 0)
		{
			cel_httpresponse_set_header(&(http_ctx->rsp), 
				CEL_HTTPHDR_CONTENT_TYPE,
				CEL_HTTPCONTEXT_CONTENT_TYPE, CEL_HTTPCONTEXT_CONTENT_TYPE_LEN);
			if (msg != NULL)
			{
				cel_httpresponse_printf(&(http_ctx->rsp), 
					"{\"error\":%d,\"message\":\"%s\"}", err_no, msg);
				cel_httpresponse_end(&(http_ctx->rsp));
			}
			else
			{
				cel_httpresponse_send(&(http_ctx->rsp), status,
					CEL_HTTPCONTEXT_SUCCESSED_MSG, CEL_HTTPCONTEXT_SUCCESSED_MSG_LEN);
			}
		}
		else
		{
			cel_httpresponse_end(&(http_ctx->rsp));
		}
		break;
    case CEL_HTTPSC_CREATED:
    case CEL_HTTPSC_ACCEPTED:
    case CEL_HTTPSC_NO_CONTENT:
        cel_httpresponse_set_statuscode(&(http_ctx->rsp), status);
        cel_httpresponse_end(&(http_ctx->rsp));
        break;
    case CEL_HTTPSC_BAD_REQUEST:
    case CEL_HTTPSC_UNAUTHORIZED:
    case CEL_HTTPSC_FORBIDDEN:
    case CEL_HTTPSC_NOT_FOUND:
    case CEL_HTTPSC_ERROR:
        cel_httpresponse_set_statuscode(&(http_ctx->rsp), status);
        if (&(http_ctx->rsp).writing_body_offset == 0)
        {
            cel_httpresponse_set_header(&(http_ctx->rsp), 
                CEL_HTTPHDR_CONTENT_TYPE,
                CEL_HTTPCONTEXT_CONTENT_TYPE, CEL_HTTPCONTEXT_CONTENT_TYPE_LEN);
        }
        cel_httpresponse_printf(&(http_ctx->rsp), 
            "{\"error\":%d,\"message\":\"%s\"}", err_no, msg);
        cel_httpresponse_end(&(http_ctx->rsp));
        break;
    default:
        CEL_SETERR((CEL_ERR_LIB, _T("cel_httpcontext_result error")));
        cel_httpcontext_free(http_ctx);
        return -1;
    }
    return CEL_RET_OK;
}

