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
#include "cel/net/balancer.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

CelKeyword balancer_types[] = 
{
    { sizeof(_T("hash")) - 1, _T("hash")},
    { sizeof(_T("lc")) - 1, _T("lc")},
    { sizeof(_T("lv")) - 1, _T("lv")},
    { sizeof(_T("rr")) - 1, _T("rr")},  
    { sizeof(_T("wlc")) - 1, _T("wlc")},
    { sizeof(_T("wlv")) - 1, _T("wlv")},
    { sizeof(_T("wrr")) - 1, _T("wrr")}
};

static int cel_balancer_wrr_gcd(CelBalancer *balancer)
{
    int i, n, weight, gcd  = CEL_MAX_WEIGHT;
    BOOL is_common = TRUE;

    n = (int)cel_arraylist_get_size(balancer->nodes);
    for (i = n - 1; i >= 0; i--) 
    {
        if ((weight = cel_node_get_weight(
            (CelNode *)cel_arraylist_get_by_index(balancer->nodes, i))) < gcd)
            gcd = weight;
    }
    while (gcd >= 1) 
    {
        for (i = n - 1; i >= 0; i--) 
        {
            if ((weight = cel_node_get_weight(
                (CelNode *)cel_arraylist_get_by_index(balancer->nodes, i))) % gcd != 0) 
            {
                is_common = FALSE;
                break;
            }
        }
        if (is_common) 
            break;
        gcd--;
    }
    //_tprintf(_T("gcd %d\r\n"), gcd);
    
    return gcd;
}

static int cel_balancer_wrr_max_weight(CelBalancer *balancer)
{
    int i, n, weight, max_weight = 0;

    n = (int)cel_arraylist_get_size(balancer->nodes);
    for (i = n - 1; i >= 0; i--) 
    {
        if ((weight = cel_node_get_weight(
            (CelNode *)cel_arraylist_get_by_index(balancer->nodes, i))) > max_weight) 
            max_weight = weight;
    }
    //_tprintf(_T("max weight %d\r\n"), max_weight);

    return max_weight;
}

static int cel_balancer_wrr_schedule(CelBalancer *balancer)
{
    int n;
    CelNode *node;

    if ((n = (int)cel_arraylist_get_size(balancer->nodes)) <= 0)
        return -1;
    while (TRUE)
    {
        if ((balancer->cur_index = ((balancer->cur_index + 1) % n)) == 0)
        {
            if (balancer->gcd <= 0)
                balancer->gcd = cel_balancer_wrr_gcd(balancer);
            balancer->cur_weight -= balancer->gcd;
            if (balancer->cur_weight <= 0)
            {
                if (balancer->max_weight <= 0)
                    balancer->max_weight = cel_balancer_wrr_max_weight(balancer);
                balancer->cur_weight = balancer->max_weight;
                /* Still zero, which means no availabe servers. */
                if (balancer->cur_weight == 0)
                    return -1;
            }
        }
        node = (CelNode *)cel_arraylist_get_by_index(balancer->nodes, balancer->cur_index);
        if (cel_node_is_active(node)
            && cel_node_get_weight(node) >= balancer->cur_weight)
        {
            balancer->cur_node = node;
            break;
        }
    }
    return 0;
}

static int cel_balancer_rr_schedule(CelBalancer *balancer)
{
    int n, last_index;

    if ((n = (int)cel_arraylist_get_size(balancer->nodes)) <= 0)
        return -1;
    last_index = balancer->cur_index % n;
    balancer->cur_index = ((balancer->cur_index + 1) % n);
    while (TRUE)
    {     
        if (cel_node_is_active((balancer->cur_node 
            = (CelNode *)cel_arraylist_get_by_index(
            balancer->nodes, balancer->cur_index))))
        {
            break;
        }
        if ((balancer->cur_index = ((balancer->cur_index + 1) % n)) == last_index)
            return -1;
    }
    return 0;
}

static int cel_balancer_wlc_schedule(CelBalancer *balancer)
{
    int i, n, cur_weight, cur_connections;
    CelNode *node;

    if ((n = (int)cel_arraylist_get_size(balancer->nodes)) <= 0)
        return -1;
    for (i = n - 1; i >= 0; i--)
    {
        node = (CelNode *)cel_arraylist_get_by_index(balancer->nodes, i);
        if (cel_node_is_active(node))
        {
            cur_weight = cel_node_get_weight(node) ;
            cur_connections = cel_node_get_connections(node);
            balancer->cur_index = i;
            balancer->cur_node = node;
            for (i = i - 1; i > 0; i--)
            {
                node = (CelNode *)cel_arraylist_get_by_index(balancer->nodes, i);
                if (cel_node_is_active(node)
                    && cel_node_get_connections(node) * cur_weight 
                    <= cur_connections * cel_node_get_weight(node))
                {
                    balancer->cur_index = i;
                    balancer->cur_node = node;
                }

            }
            return 0;
        }
    }

    return -1;
}

static int cel_balancer_lc_schedule(CelBalancer *balancer)
{
    int i, n, cur_connections;
    CelNode *node;

    if ((n = (int)cel_arraylist_get_size(balancer->nodes)) <= 0)
        return -1;
    for (i = n - 1; i >= 0; i--)
    {
        node = (CelNode *)cel_arraylist_get_by_index(balancer->nodes, i);
        if (cel_node_is_active(node))
        {
            cur_connections = cel_node_get_connections(node);
            balancer->cur_index = i;
            balancer->cur_node = node;
            for (i = i - 1; i > 0; i--)
            {
                node = (CelNode *)cel_arraylist_get_by_index(balancer->nodes, i);
                if (cel_node_is_active(node)
                    && cel_node_get_connections(node) <= cur_connections)
                {
                    balancer->cur_index = i;
                    balancer->cur_node = node;
                }
            }
            return 0;
        }
    }

    return -1;
}

static int cel_balancer_wlv_schedule(CelBalancer *balancer)
{
    int i, n, cur_loadvalue, cur_weight;
    CelNode *node;

    if ((n = (int)cel_arraylist_get_size(balancer->nodes)) <= 0)
        return -1;
    for (i = n - 1; i >= 0; i--)
    {
        node = (CelNode *)cel_arraylist_get_by_index(balancer->nodes, i);
        if (cel_node_is_active(node))
        {
            cur_loadvalue = cel_node_get_loadvalue(node);
            cur_weight = cel_node_get_weight(node);
            balancer->cur_index = i;
            balancer->cur_node = node;
            for (i = i - 1; i > 0; i--)
            {
                node = (CelNode *)cel_arraylist_get_by_index(balancer->nodes, i);
                if (cel_node_is_active(node)
                    && cel_node_get_loadvalue(node) * cur_weight
                    <= cur_loadvalue * cel_node_get_weight(node))
                {
                    balancer->cur_index = i;
                    balancer->cur_node = node;
                }
            }
            return 0;
        }
    }

    return -1;
}

static int cel_balancer_lv_schedule(CelBalancer *balancer)
{
    int i, n, cur_loadvalue;
    CelNode *node;

    if ((n = (int)cel_arraylist_get_size(balancer->nodes)) <= 0)
        return -1;
    for (i = n - 1; i >= 0; i--)
    {
        if ((node = (CelNode *)cel_arraylist_get_by_index(balancer->nodes, i)) != NULL
            && cel_node_is_active(node))
        {
            cur_loadvalue = cel_node_get_loadvalue(node);
            balancer->cur_index = i;
            balancer->cur_node = node;
            for (i = i - 1; i > 0; i--)
            {
                node = (CelNode *)cel_arraylist_get_by_index(balancer->nodes, i);
                if (cel_node_is_active(node)
                    && cel_node_get_loadvalue(node) <= cur_loadvalue)
                {
                    balancer->cur_index = i;
                    balancer->cur_node = node;
                }
            }
            return 0;
        }
    }

    return -1;
}

static int cel_balancer_hash_schedule(CelBalancer *balancer, int hash_value)
{
    int n, last_index;

    if ((n = (int)cel_arraylist_get_size(balancer->nodes)) <= 0)
        return -1;
    last_index = balancer->cur_index % n;
    balancer->cur_index = hash_value % n;
    while (TRUE)
    {
        if (cel_node_is_active((balancer->cur_node 
            = (CelNode *)cel_arraylist_get_by_index(balancer->nodes, balancer->cur_index))))
        {
            break;
        }
        if ((balancer->cur_index 
            = ((balancer->cur_index + 1) % n)) == last_index)
            return -1;
    }
    return 0;
}

int cel_balancer_init(CelBalancer *balancer, 
                      CelBalancerType type, CelArrayList *nodes)
{
    if (nodes == NULL)
        return -1;
    balancer->nodes = nodes;
    switch (type)
    {
    case CEL_BALANCER_RR:
         balancer->schedule = cel_balancer_rr_schedule;
        break;
    case CEL_BALANCER_WRR:
        balancer->gcd = -1;
        balancer->max_weight = -1;
        balancer->schedule = cel_balancer_wrr_schedule;
        break;
    case CEL_BALANCER_LC:
        balancer->schedule = cel_balancer_lc_schedule;
        break;
    case CEL_BALANCER_WLC:
        balancer->schedule = cel_balancer_wlc_schedule;
        break;
    case CEL_BALANCER_LV:
        balancer->schedule = cel_balancer_lv_schedule;
        break;
    case CEL_BALANCER_WLV:
        balancer->schedule = cel_balancer_wlv_schedule;
        break;
    case CEL_BALANCER_HASH:
        balancer->schedule_hash = cel_balancer_hash_schedule;
        break;
    default:
        CEL_SETERR((CEL_ERR_LIB, _T("Balancer type undefined.")));
        return -1;
    }
    balancer->cur_index = 0;
    balancer->cur_node = NULL;
    balancer->cur_weight = 0;    
    return 0;
}

void cel_balancer_destroy(CelBalancer *balancer)
{
    memset(balancer, 0, sizeof(CelBalancer));
}

CelBalancer *cel_balancer_new(CelBalancerType type, CelArrayList *nodes)
{
    CelBalancer *balancer;

    if ((balancer = (CelBalancer *)cel_malloc(sizeof(CelBalancer))) != NULL)
    {
        if (cel_balancer_init(balancer, type, nodes) == 0)
            return balancer;
        cel_free(balancer);
    }

    return NULL;
}

void cel_balancer_free(CelBalancer *balancer)
{
    cel_balancer_destroy(balancer);
    cel_free(balancer);
}

void cel_balancer_reset(CelBalancer *balancer)
{
    balancer->gcd = -1;
    balancer->max_weight = -1;
    balancer->cur_index = 0;
    balancer->cur_node = NULL;
    balancer->cur_weight = 0;
}
