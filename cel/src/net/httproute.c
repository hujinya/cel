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
#include "cel/net/httproute.h"

int cel_httproute_init(CelHttpRoute *route, const char *prefix)
{
    int i = 0;
    CelPatTrie *trie;

    cel_vstring_init_a(&(route->prefix));
    cel_vstring_assign_a(&(route->prefix), prefix, strlen(prefix));
    for (i = 0; i < CEL_HTTPROUTEST_END; i++)
    {
        cel_list_init(&(route->filters[i]), NULL);
    }
	route->logger_on = FALSE;
    for (i = 0; i < CEL_HTTPM_CONUT; i++)
    {
        trie = &(route->root_tries[i]);
        cel_pattrie_init(trie, NULL);
    }
    return 0;
}

void cel_httproute_destroy(CelHttpRoute *route)
{
    int i = 0;
    CelPatTrie *trie;

	cel_vstring_destroy(&(route->prefix));
    for (i = 0; i < CEL_HTTPROUTEST_END; i++)
    {
        cel_list_destroy(&(route->filters[i]));
    }
	route->logger_on = FALSE;
    for (i = 0; i < CEL_HTTPM_CONUT; i++)
    {
        trie = &(route->root_tries[i]);
        cel_pattrie_destroy(trie);
    }
}

CelHttpRoute *cel_httproute_new(const char *prefix)
{
    CelHttpRoute *route;

    if ((route = (CelHttpRoute *)cel_malloc(sizeof(CelHttpRoute))) != NULL)
    {
        if (cel_httproute_init(route, prefix) == 0)
            return route;
        cel_free(route);
    }
    return NULL;
}

void cel_httproute_free(CelHttpRoute *route)
{
    cel_httproute_destroy(route);
    cel_free(route);
}

int cel_httproute_add(CelHttpRoute *route, 
                      CelHttpMethod method, const char *path, 
                      CelHttpFilterHandlerFunc handle_func)
{
    cel_pattrie_insert(&(route->root_tries[method]), path, handle_func);
    return 0;
}

int cel_httproute_remove(CelHttpRoute *route, 
                         CelHttpMethod method, const char *path)
{
    return 0;
}

int cel_httproute_logger_filter_set(CelHttpRoute *route, 
									CelHttpFilterHandlerFunc handle)
{
	route->logger_on = TRUE;
	route->logger_filter = handle;
	return 0;
}

int cel_httproute_filter_insert(CelHttpRoute *route, 
                                CelHttpRouteState state_position, 
                                CelHttpFilter *filter)
{
    cel_list_push_back(&(route->filters[state_position]), &filter->_item);
    return 0;
}

int cel_httproute_filter_list_foreach(CelList *list, CelHttpContext *http_ctx)
{
	int ret;
	CelListItem *tail, *next;

	if (http_ctx->current_filter == NULL)
		http_ctx->current_filter = cel_list_get_head(list);
	next = http_ctx->current_filter->next;
	tail = cel_list_get_tail(list);
	while ((http_ctx->current_filter = next) != tail)
	{
		next = http_ctx->current_filter->next;
		if ((ret = ((CelHttpFilter *)
			(http_ctx->current_filter))->handler(http_ctx)) != CEL_RET_OK)
			return ret;
	}
	http_ctx->current_filter = NULL;
	return CEL_RET_OK;
}

int cel_httproute_routing(CelHttpRoute *route, CelHttpContext *http_ctx)
{
	int ret;
	char *url, *prefix;
	CelHttpFilterHandlerFunc route_handler;
	CelList *filter_list;

	if (http_ctx->state == CEL_HTTPROUTEST_END)
		abort();
	while (TRUE)
	{
		switch (http_ctx->state)
		{
		case CEL_HTTPROUTEST_BEFORE_ROUTER:
			filter_list = &(route->filters[http_ctx->state]);
			if (cel_list_get_size(filter_list) > 0
				&& (ret = cel_httproute_filter_list_foreach(filter_list, http_ctx)) != CEL_RET_OK)
			{
				CEL_SETERR((CEL_ERR_LIB,  
					_T("Http route '[%d]%s' filter handler[%d] failed."), 
					cel_httprequest_get_method(http_ctx->req), 
					cel_httprequest_get_url_path(http_ctx->req), http_ctx->state));
				break;
			}
			http_ctx->state = CEL_HTTPROUTEST_BEFORE_EXEC;
		case CEL_HTTPROUTEST_BEFORE_EXEC:
			filter_list = &(route->filters[http_ctx->state]);
			if (cel_list_get_size(filter_list) > 0
				&& (ret = cel_httproute_filter_list_foreach(filter_list, http_ctx)) != CEL_RET_OK)
			{
				CEL_SETERR((CEL_ERR_LIB,  
					_T("Http route '[%d]%s' filter handler[%d] failed."), 
					cel_httprequest_get_method(http_ctx->req), 
					cel_httprequest_get_url_path(http_ctx->req), http_ctx->state));
				break;
			}
			if ((url = cel_httprequest_get_url_path(http_ctx->req)) == NULL
				|| strncmp(cel_vstring_str_a(&(route->prefix)),
				url, cel_vstring_size_a(&(route->prefix))) != CEL_RET_OK)
			{
				//puts(cel_vstring_str_a(&(route->prefix)));
				ret = CEL_RET_ERROR;
				cel_httpresponse_set_statuscode(http_ctx->rsp, CEL_HTTPSC_NOT_FOUND);
				CEL_SETERR((CEL_ERR_LIB, _T("Http route prefix '[%d]%s' not supported"), 
					cel_httprequest_get_method(http_ctx->req), url));
				break;
			}
			prefix = url + cel_vstring_size_a(&(route->prefix));
			if ((route_handler = cel_pattrie_lookup(
				&(route->root_tries[cel_httprequest_get_method(http_ctx->req)]), 
				prefix, &(http_ctx->params))) == NULL)
			{
				ret = CEL_RET_ERROR;
				cel_httpresponse_set_statuscode(http_ctx->rsp, CEL_HTTPSC_NOT_FOUND);
				CEL_SETERR((CEL_ERR_LIB, _T("Http route path '[%d]%s' not supported."), 
					cel_httprequest_get_method(http_ctx->req), prefix));
				break;
			}
			http_ctx->state = CEL_HTTPROUTEST_AFTER_EXEC;
			if ((ret = route_handler(http_ctx)) != CEL_RET_OK)
			{
				//printf("Http route handler ret = %d\r\n", ret);
				/*if (ret == CEL_RET_ERROR)
				{
					CEL_SETERR((CEL_ERR_LIB, _T("Http route '[%d]%s' handler failed."), 
						cel_httprequest_get_method(http_ctx->req), prefix));
				}*/
				break;
			}
			//puts("Http route_handler ret = 0");
		case CEL_HTTPROUTEST_AFTER_EXEC:
			filter_list = &(route->filters[http_ctx->state]);
			if (cel_list_get_size(filter_list) > 0
				&& (ret = cel_httproute_filter_list_foreach(filter_list, http_ctx)) != CEL_RET_OK)
			{
				CEL_SETERR((CEL_ERR_LIB,  
					_T("Http route '[%d]%s' filter handler[%d] failed."), 
					cel_httprequest_get_method(http_ctx->req), 
					cel_httprequest_get_url_path(http_ctx->req), http_ctx->state));
				break;
			}
			http_ctx->state = CEL_HTTPROUTEST_FINISH_ROUTER;
		case CEL_HTTPROUTEST_FINISH_ROUTER:
			filter_list = &(route->filters[http_ctx->state]);
			if (cel_list_get_size(filter_list) > 0
				&& (ret = cel_httproute_filter_list_foreach(filter_list, http_ctx)) != CEL_RET_OK)
			{
				CEL_SETERR((CEL_ERR_LIB,  
					_T("Http route '[%d]%s' filter handler[%d] failed."), 
					cel_httprequest_get_method(http_ctx->req), 
					cel_httprequest_get_url_path(http_ctx->req), http_ctx->state));
				break;
			}
			http_ctx->state = CEL_HTTPROUTEST_END;
		case CEL_HTTPROUTEST_END:
			if (route->logger_on)
				route->logger_filter(http_ctx);
			return ret;
		default:
			ret = CEL_RET_ERROR;
			cel_httpresponse_set_statuscode(http_ctx->rsp, CEL_HTTPSC_ERROR);
			CEL_SETERR((CEL_ERR_LIB, _T("Http route '[%d]%s' state %d unfinded."), 
				cel_httprequest_get_method(http_ctx->req), 
				cel_httprequest_get_url_path(http_ctx->req), http_ctx->state));
			http_ctx->state = CEL_HTTPROUTEST_END;
			return ret;
		}
		if (ret == CEL_RET_ERROR || ret == CEL_RET_DONE)
			http_ctx->state = CEL_HTTPROUTEST_END;
		else if (ret == CEL_RET_AGAIN)
			return ret;
		else
		{
			CEL_SETERR((CEL_ERR_LIB, _T("Http route '[%d]%s' return value %d invaild."), 
				cel_httprequest_get_method(http_ctx->req), 
				cel_httprequest_get_url_path(http_ctx->req), ret));
			http_ctx->state = CEL_HTTPROUTEST_END;
			return -1;
		}
	}
}
