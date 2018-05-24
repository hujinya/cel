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
#ifndef __CEL_NET_POOL_H__
#define __CEL_NET_POOL_H__

#include "cel/types.h"
#include "cel/refcounted.h"
#include "cel/arraylist.h"
#include "cel/net/monitor.h"
#include "cel/net/node.h"
#include "cel/net/balancer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelPool
{
    TCHAR name[CEL_KNLEN];
    CelRefCounted ref_counted;
    CelArrayList mntr_ctxs;
    CelArrayList nodes;
    CelBalancer balancer;
}CelPool;


int cel_pool_init(CelPool *pool, const TCHAR *name, CelBalancerType balancer);
void cel_pool_destroy(CelPool *pool);

CelPool *cel_pool_new(const TCHAR *name, CelBalancerType balancer);
void cel_pool_free(CelPool *pool);

#define cel_pool_ref(pool) cel_refcounted_ref_ptr(&(pool->ref_counted), pool)
#define cel_pool_deref(pool) cel_refcounted_deref(&(pool->ref_counted), pool)

void cel_pool_monitors_add(CelPool *pool, CelMonitorContext *mntr_ctx);
void cel_pool_monitors_clear(CelPool *pool);

void cel_pool_nodes_add(CelPool *pool, CelNode *node);
void cel_pool_nodes_clear(CelPool *pool);

static __inline CelNode *cel_pool_balancer_schedule(CelPool *pool)
{
    return cel_balancer_schedule(&(pool->balancer));
}
static __inline 
void *cel_pool_balancer_schedule_hash(CelPool *pool, int hash_value)
{
    return cel_balancer_schedule_hash(&(pool->balancer), hash_value);
}

#ifdef __cplusplus
}
#endif

#endif
