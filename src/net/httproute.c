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

CelHttpHandleFunc cel_httproute_handler(CelHttpRoute *route,
                                        CelHttpMethod method, 
                                        const char *path,
                                        CelHttpRouteData *rt_data)
{
    return cel_pattrie_lookup(&(route->root_tries[method]), path, rt_data);
}
