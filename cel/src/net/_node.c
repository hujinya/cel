/**
 * CEL(C Extension Library)
 * Copyright (C)2008 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/net/_node.h"

void _cel_nodecontext_destroy_derefed(CelNodeContext *node_ctx)
{
    node_ctx->name[0] = _T('\0');
    node_ctx->address[0] = _T('\0');
}

int cel_nodeconext_init(CelNodeContext *node_ctx, 
                        const TCHAR *name, const TCHAR *address)
{
    size_t len;

    if ((len = _tcslen(name)) >= CEL_KNLEN)
        len = CEL_KNLEN - 1;
    memcpy(node_ctx->name, name, len * sizeof(TCHAR));
    node_ctx->name[len] = _T('\0');
    if ((len = _tcslen(address)) >= CEL_ADDRLEN)
        len = CEL_ADDRLEN - 1;
    memcpy(node_ctx->address, address, len * sizeof(TCHAR));
    node_ctx->address[len] = _T('\0');
    cel_refcounted_init(
        &(node_ctx->ref_counted), (CelFreeFunc)_cel_nodecontext_destroy_derefed);
    return 0;
}

void cel_nodecontext_destroy(CelNodeContext *node_ctx)
{
    cel_refcounted_destroy(&(node_ctx->ref_counted), node_ctx);
}

void _cel_nodecontext_free_derefed(CelNodeContext *node_ctx)
{
    _cel_nodecontext_destroy_derefed(node_ctx);
    cel_free(node_ctx);
}

CelNodeContext *cel_nodecontext_new(const TCHAR *name, const TCHAR *address)
{
    CelNodeContext *node_ctx;

    if ((node_ctx = (CelNodeContext *)cel_malloc(sizeof(CelNodeContext))) != NULL)
    {
        if (cel_nodeconext_init(node_ctx, name, address) == 0)
        {
            cel_refcounted_init(
                &(node_ctx->ref_counted), (CelFreeFunc)_cel_nodecontext_free_derefed);
            return node_ctx;
        }
        cel_free(node_ctx);
    }

    return NULL;
}

void cel_nodecontext_free(CelNodeContext *node_ctx)
{
    cel_refcounted_destroy(&(node_ctx->ref_counted), node_ctx);
}

void _cel_node_destroy_derefed(CelNode *node)
{
    cel_nodecontext_deref(node->ctx);
    cel_arraylist_destroy(&(node->monitors));
}

int cel_node_init(CelNode *node, CelNodeContext *node_ctx)
{
    node->ctx = cel_nodecontext_ref(node_ctx);
    node->connections = 0;
    node->loadvalue = 0;
    node->weight = 0;
    cel_arraylist_init(&(node->monitors), (CelFreeFunc)cel_monitor_free);
    cel_refcounted_init(&(node->ref_counted), (CelFreeFunc)_cel_node_destroy_derefed);
    return 0;
}

void cel_node_destroy(CelNode *node)
{
    cel_refcounted_destroy(&(node->ref_counted), node);
}

void _cel_node_free_derefed(CelNode *node)
{
    _cel_node_destroy_derefed(node);
    cel_free(node);
}

CelNode *cel_node_new(CelNodeContext *node_ctx)
{
    CelNode *node;

    if ((node = (CelNode *)cel_malloc(sizeof(CelNode))) != NULL)
    {
        if (cel_node_init(node, node_ctx) == 0)
        {
            cel_refcounted_init(&(node->ref_counted),
                (CelFreeFunc)_cel_node_free_derefed);
            return node;
        }
        cel_free(node);
    }

    return NULL;
}

void cel_node_free(CelNode *node)
{
    cel_refcounted_destroy(&(node->ref_counted), node);
}

BOOL cel_node_is_active(CelNode *node)
{
    int i;
    CelMonitor *mntr;

    i = (int)cel_arraylist_get_size(&(node->monitors));
    while ((--i) >= 0)
    {
        mntr = (CelMonitor *)cel_arraylist_get_by_index(&(node->monitors), i);
        if (!(mntr->active))
            return FALSE;
    }
    return TRUE;
}

int cel_node_monitors_add(CelNode *node, CelMonitorContext *mntr_ctx)
{
    CelMonitor *monitor;

    if ((monitor = cel_monitor_new(mntr_ctx, node->ctx->address)) != NULL)
        cel_arraylist_push_back(&(node->monitors), monitor);

    return 0;
}

void cel_node_monitors_clear(CelNode *node)
{
    cel_arraylist_clear(&(node->monitors));
}
