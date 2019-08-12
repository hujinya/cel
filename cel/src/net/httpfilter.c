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
#include "cel/net/httpfilter.h"

int cel_httpfilter_init(CelHttpFilter *filter,
						CelHttpFilterHandlerFunc handler)
{
	filter->_item.next = NULL;
	filter->_item.prev = NULL;
	filter->handler = handler;
    return 0;
}

void cel_httpfilter_destroy(CelHttpFilter *filter)
{
	filter->_item.next = NULL;
	filter->_item.prev = NULL;
	filter->handler = NULL;
}

int cel_httpfilter_allowcors_handler(CelHttpContext *http_ctx)
{
	CelHttpFilterAllowCors *cors = (CelHttpFilterAllowCors *)(http_ctx->current_filter);
	CelHttpRequest *req = http_ctx->req;
	CelHttpResponse *rsp = http_ctx->rsp;
	char *orign;

	if (cel_vstring_size_a(&(cors->allow_origins)) != 0)
	{
		orign = cel_vstring_str_a(&(cors->allow_origins));
		if (*orign == '*'
			&& ((orign = cel_httprequest_get_header(req, CEL_HTTPHDR_ORIGIN)) != NULL
			|| (orign = cel_httprequest_get_header(req, CEL_HTTPHDR_REFERER)) != NULL))
			cel_httpresponse_set_header(rsp, 
			CEL_HTTPHDR_ACCESS_CONTROL_ALLOW_ORIGIN, orign, strlen(orign));
		else
			cel_httpresponse_set_header(rsp, 
			CEL_HTTPHDR_ACCESS_CONTROL_ALLOW_ORIGIN,
			cel_vstring_str_a(&(cors->allow_origins)), 
			cel_vstring_size_a(&(cors->allow_origins)));
	}
	if (cors->is_allow_credentials)
		cel_httpresponse_set_header(rsp, 
		CEL_HTTPHDR_ACCESS_CONTROL_ALLOW_CREDENTIALS, "true", 4);
	else
		cel_httpresponse_set_header(rsp, 
		CEL_HTTPHDR_ACCESS_CONTROL_ALLOW_CREDENTIALS, "false", 5);
	/* Process options method */
	if (req->method == CEL_HTTPM_OPTIONS)
	{
		if (cel_vstring_size_a(&(cors->allow_methods)) != 0)
			cel_httpresponse_set_header(rsp,
			CEL_HTTPHDR_ACCESS_CONTROL_ALLOW_METHODS, 
			cel_vstring_str_a(&(cors->allow_methods)), 
			cel_vstring_size_a(&(cors->allow_methods)));
		if (cel_vstring_size_a(&(cors->allow_headers)) != 0)
			cel_httpresponse_set_header(rsp, 
			CEL_HTTPHDR_ACCESS_CONTROL_ALLOW_HEADERS, 
			cel_vstring_str_a(&(cors->allow_headers)), 
			cel_vstring_size_a(&(cors->allow_headers)));
		cel_httpresponse_set_header(rsp, 
			CEL_HTTPHDR_ACCESS_CONTROL_MAX_AGE, 
			&(cors->max_age), sizeof(cors->max_age));
		if (cel_vstring_size_a(&(cors->expose_headers)) != 0)
			cel_httpresponse_set_header(rsp, 
			CEL_HTTPHDR_ACCESS_CONTROL_EXPOSE_HEADERS, 
			cel_vstring_str_a(&(cors->expose_headers)), 
			cel_vstring_size_a(&(cors->expose_headers)));

		cel_httpresponse_set_statuscode(rsp, CEL_HTTPSC_REQUEST_OK);
		return CEL_RET_DONE;
	}
	return CEL_RET_OK;
}

int cel_httpfilter_allowcors_init(CelHttpFilterAllowCors *cors,
                                  const char *allow_origins, 
                                  BOOL is_allow_credentials,
                                  const char *allow_methods, 
                                  const char *allow_headers, 
                                  const char *expose_headers,
                                  long max_age)
{
	cel_vstring_init(&(cors->allow_origins));
	if (allow_origins != NULL)
		cel_vstring_assign_a(&(cors->allow_origins), 
		allow_origins, strlen(allow_origins));

	cors->is_allow_credentials = is_allow_credentials;

	cel_vstring_init(&(cors->allow_methods));
	if (allow_methods != NULL)
		cel_vstring_assign_a(&(cors->allow_methods), 
		allow_methods, strlen(allow_methods));

	cel_vstring_init(&(cors->allow_headers));
	if (allow_headers != NULL)
		cel_vstring_assign_a(&(cors->allow_headers), 
		allow_headers, strlen(allow_headers));

	cel_vstring_init(&(cors->expose_headers));
	if (expose_headers != NULL)
		cel_vstring_assign_a(&(cors->expose_headers), 
		expose_headers, strlen(expose_headers));

	cors->max_age = max_age;

	cel_httpfilter_init(&(cors->_filter), 
		(CelHttpFilterHandlerFunc)cel_httpfilter_allowcors_handler);

    return 0;
}

void cel_httpfilter_allowcors_destroy(CelHttpFilterAllowCors *cors)
{
    cel_httpfilter_destroy(&(cors->_filter));

	cel_vstring_destroy(&(cors->allow_origins));
	cel_vstring_destroy(&(cors->allow_methods));
	cel_vstring_destroy(&(cors->allow_headers));
	cel_vstring_destroy(&(cors->expose_headers));
}

int cel_httpfilter_static_handler(CelHttpContext *http_ctx)
{
	return CEL_RET_DONE;
}

int cel_httpfilter_static_init(CelHttpFilterStatic *stic, 
							   const char *path)
{
	return 0;
}

void cel_httpfilter_static_destroy(CelHttpFilterStatic *stic)
{
	;
}
