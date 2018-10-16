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
#ifndef __CEL_NET_NODE_H__
#define __CEL_NET_NODE_H__

#include "cel/types.h"
#include "cel/refcounted.h"
#include "cel/arraylist.h"
#include "cel/net/sockaddr.h"
#include "cel/net/monitor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelNodeContext
{
    TCHAR name[CEL_KNLEN];
    TCHAR address[CEL_ADDRLEN]; 
    CelRefCounted ref_counted;
}CelNodeContext;

//typedef int (* CelNodeGetWeightFunc) (void *node);
//typedef BOOL (* CelNodeIsActiveFunc) (void *node);
//typedef int (* CelNodeGetConnectionsFunc) (void *node);
//typedef int (* CelNodeGetLoadValueFunc) (void *node);

typedef struct _CelNode
{
    CelNodeContext *ctx;
    int weight;
    int connections;
    int loadvalue;
    CelRefCounted ref_counted;
    CelArrayList monitors;
}CelNode;

int cel_nodeconext_init(CelNodeContext *node_ctx,
                        const TCHAR *name, const TCHAR *address);
void cel_nodecontext_destroy(CelNodeContext *node_ctx);

CelNodeContext *cel_nodecontext_new(const TCHAR *name, const TCHAR *address);
void cel_nodecontext_free(CelNodeContext *node_ctx);

/* CelNodeContext *cel_nodecontext_ref(CelNodeContext *node_ctx); */
#define cel_nodecontext_ref(node_ctx) \
    (CelNodeContext *)cel_refcounted_ref_ptr(&(node_ctx->ref_counted), node_ctx)
#define cel_nodecontext_deref(node_ctx) \
    cel_refcounted_deref(&(node_ctx->ref_counted), node_ctx)

int cel_node_init(CelNode *node, CelNodeContext *node_ctx);
void cel_node_destroy(CelNode *node);

CelNode *cel_node_new(CelNodeContext *node_ctx);
void cel_node_free(CelNode *node);

#define cel_node_ref(node) cel_refcounted_ref_ptr(&(node->ref_counted), node)
#define cel_node_deref(node) cel_refcounted_deref(&(node->ref_counted), node)

BOOL cel_node_is_active(CelNode *node);
/* void cel_node_set_weight(CelNode *node); */
#define cel_node_set_weight(node, weight) (node)->weight = weight
static __inline int cel_node_get_weight(CelNode *node)
{
    return node->weight;
}
static __inline int cel_node_get_connections(CelNode *node)
{
    return node->connections;
}
static __inline int cel_node_get_loadvalue(CelNode *node)
{
    return node->loadvalue;
}

int cel_node_monitors_add(CelNode *node, CelMonitorContext *mntr_ctx);
void cel_node_monitors_clear(CelNode *node);


#ifdef __cplusplus
}
#endif

#endif