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
#include "cel/net/_pool.h"

void _cel_pool_destroy_derefed(CelPool *pool)
{
    pool->name[0] = _T('\0');
    cel_balancer_destroy(&(pool->balancer));
}

int cel_pool_init(CelPool *pool, const TCHAR *name, CelBalancerType balancer)
{
    size_t len;

    if ((len = _tcslen(name)) >= CEL_KNLEN)
        len = CEL_KNLEN - 1;
    memcpy(pool->name, name, len * sizeof(TCHAR));
    pool->name[len] = _T('\0');
    cel_arraylist_init(&(pool->mntr_ctxs), NULL);
    cel_arraylist_init(&(pool->nodes), NULL);
    cel_balancer_init(&(pool->balancer), balancer, &(pool->nodes));
    cel_refcounted_init(&(pool->ref_counted), 
        (CelFreeFunc)_cel_pool_destroy_derefed);

    return 0;
}

void cel_pool_destroy(CelPool *pool)
{
    cel_refcounted_destroy(&(pool->ref_counted), pool);
}

void _cel_pool_free_derefed(CelPool *pool)
{
    cel_pool_destroy(pool);
    cel_free(pool);
}

CelPool *cel_pool_new(const TCHAR *name, CelBalancerType balancer)
{
    CelPool *pool;

    if ((pool = (CelPool *)cel_malloc(sizeof(CelPool))) != NULL)
    {
        if (cel_pool_init(pool, name, balancer) == 0)
        {
            cel_refcounted_init(&(pool->ref_counted),
                (CelFreeFunc)_cel_pool_free_derefed);
            return pool;
        }
        cel_free(pool);
    }
    return NULL;
}

void cel_pool_free(CelPool *pool)
{
    cel_refcounted_destroy(&(pool->ref_counted), pool);
}

void cel_pool_monitors_add(CelPool *pool, CelMonitorContext *mntr_ctx)
{
    cel_arraylist_push_back(&(pool->mntr_ctxs), mntr_ctx);
}

void cel_pool_monitors_clear(CelPool *pool)
{
    cel_arraylist_clear(&(pool->mntr_ctxs));
}

void cel_pool_nodes_add(CelPool *pool, CelNode *node)
{
    cel_arraylist_push_back(&(pool->nodes), node);
    cel_balancer_reset(&(pool->balancer));
}

void cel_pool_nodes_clear(CelPool *pool)
{
    cel_arraylist_clear(&(pool->nodes));
    cel_balancer_reset(&(pool->balancer));
}
