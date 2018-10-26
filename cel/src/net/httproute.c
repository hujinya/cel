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
#include "cel/net/httproute.h"
#include "cel/log.h"

int cel_httproute_init(CelHttpRoute *route)
{
    int i = 0;
    CelPatTrie *trie;

    for (i = 0; i < CEL_HTTPROUTEST_COUNT; i++)
    {
        cel_list_init(&(route->filters[i]), NULL);
    }
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

    for (i = 0; i < CEL_HTTPROUTEST_COUNT; i++)
    {
        cel_list_destroy(&(route->filters[i]));
    }
    for (i = 0; i < CEL_HTTPM_CONUT; i++)
    {
        trie = &(route->root_tries[i]);
        cel_pattrie_destroy(trie);
    }
}

CelHttpRoute *cel_httproute_new(void)
{
    CelHttpRoute *route;

    if ((route = (CelHttpRoute *)cel_malloc(sizeof(CelHttpRoute))) != NULL)
    {
        if (cel_httproute_init(route) == 0)
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

int cel_httproute_filter_insert(CelHttpRoute *route, 
                                CelHttpRouteState state_position, 
                                CelHttpFilter *filter)
{
    cel_list_push_back(&(route->filters[state_position]), &filter->_item);
    return 0;
}

int cel_httproute_add(CelHttpRoute *route, 
                      CelHttpMethod method, const char *path, 
                      CelHttpHandleFunc handle_func)
{
    cel_pattrie_insert(&(route->root_tries[method]), path, handle_func);
    return 0;
}

int cel_httproute_remove(CelHttpRoute *route, 
                         CelHttpMethod method, const char *path)
{
    return 0;
}

int cel_httproute_filter_handler(CelHttpFilter *filter, void *user_data)
{
    return /*filter->handler(filter, )*/0;
}

CelHttpHandleFunc cel_httproute_routing(CelHttpRoute *route, 
                                        CelHttpRouteState *state,
                                        CelHttpMethod method, const char *path,
                                        CelHttpRouteData *rt_data)
{
    int ret;
    CelHttpHandleFunc route_handler;

    switch (*state)
    {
    case CEL_HTTPROUTEST_BEFORE_START:
        if ((ret = cel_list_foreach(&(route->filters[*state]), 
            (CelEachFunc)cel_httproute_filter_handler, NULL)) != 0)
            break;
        *state = CEL_HTTPROUTEST_BEFORE_EXEC;
    case CEL_HTTPROUTEST_BEFORE_EXEC:
        if ((ret = cel_list_foreach(&(route->filters[*state]), 
            (CelEachFunc)cel_httproute_filter_handler, NULL)) != 0)
            break;
        if ((route_handler = cel_pattrie_lookup(
            &(route->root_tries[method]), path, rt_data)) == NULL)
        {
            ret = -1;
            break;
        }
        return route_handler;
        /**state = CEL_HTTPROUTEST_AFTER_EXEC;
    case CEL_HTTPROUTEST_AFTER_EXEC:
       if ((ret = cel_list_foreach(&(route->filters[*state]), 
            (CelEachFunc)cel_httproute_filter_handler, NULL)) != 0)
            break;
        *state = CEL_HTTPROUTEST_AFTER_FINISH;
    case CEL_HTTPROUTEST_AFTER_FINISH:
        if ((ret = cel_list_foreach(&(route->filters[*state]), 
            (CelEachFunc)cel_httproute_filter_handler, NULL)) != 0)
            break;*/
        break;
    default:
        break;
    }
    return NULL;
}
