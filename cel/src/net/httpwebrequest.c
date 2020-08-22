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
#include "cel/net/httpwebrequest.h"
//#define _CEL_DEBUG
#include "cel/log.h"
#include "cel/error.h"
#include "cel/allocator.h"
#include "cel/file.h"

void _cel_httpwebrequest_destroy_derefed(CelHttpWebRequest *web_req)
{
	//puts("_cel_httpwebrequest_destroy_derefed");
	cel_httpclient_destroy(&(web_req->http_client));

	cel_httprequest_destroy(&(web_req->req));
	cel_httpresponse_destroy(&(web_req->rsp));
	web_req->execute_callback = NULL;
}

void cel_httpwebrequest_destroy(CelHttpWebRequest *web_req)
{
	cel_refcounted_destroy(&(web_req->ref_counted), web_req);
}

void _cel_httpwebrequest_free_derefed(CelHttpWebRequest *web_req)
{
	//puts("_cel_httpwebrequest_free_derefed");
	_cel_httpwebrequest_destroy_derefed(web_req);
    cel_free(web_req);
}

CelHttpWebRequest *cel_httpwebrequest_new_httpclient(CelHttpClient *http_client)
{
	CelHttpWebRequest *new_web_req;

	if ((new_web_req = cel_malloc(sizeof(CelHttpWebRequest))) == NULL)
	{
		CEL_SETERR((CEL_ERR_LIB, _T("Http web request new return null.")));
		return NULL;
	}
	memcpy(&(new_web_req->http_client), http_client, sizeof(CelHttpClient));
	cel_httprequest_init(&(new_web_req->req));
	cel_httpresponse_init(&(new_web_req->rsp));
	new_web_req->execute_callback = NULL;
	cel_refcounted_init(&(new_web_req->ref_counted),
		(CelFreeFunc)_cel_httpwebrequest_free_derefed);
	return new_web_req;
}

CelHttpWebRequest *cel_httpwebrequest_new(CelSslContext *ssl_ctx)
{
	CelHttpClient http_client;
	CelHttpWebRequest *new_web_req;

	if (cel_httpclient_init_family(&http_client, AF_INET, ssl_ctx) == 0)
	{
		if ((new_web_req = cel_httpwebrequest_new_httpclient(&http_client)) != NULL)
			return new_web_req;
		cel_httpclient_destroy(&http_client);
	}
	return NULL;
}

void cel_httpwebrequest_free(CelHttpWebRequest *web_req)
{
	CEL_DEBUG((_T("Http web request %s closed."), 
		cel_httpwebrequest_get_remoteaddr_str(web_req)));
	cel_refcounted_destroy(&(web_req->ref_counted), web_req);
}

void cel_httpwebrequest_do_shutdown(CelHttpWebRequest *web_req, 
									CelAsyncResult *result)
{
	cel_httpwebrequest_free(web_req);
}

void _cel_httpwebrequest_execute_callback(CelHttpWebRequest *web_req,
										  CelHttpRequest *req, CelHttpResponse *rsp,
										  CelAsyncResult *result)
{
	if (web_req->execute_callback != NULL)
		web_req->execute_callback(web_req, result);
	else
	{
		if (cel_httpclient_async_shutdown(&(web_req->http_client), 
			(CelHttpShutdownCallbackFunc)cel_httpwebrequest_do_shutdown) == -1)
		{
			CEL_SETERR((CEL_ERR_LIB,
				_T("cel_httpwebrequest_async_send_response_file %s return -1"),
				cel_httpwebrequest_get_remoteaddr_str(web_req)));
			cel_httpwebrequest_free(web_req);
		}
	}
}
