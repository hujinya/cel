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
#ifndef __CEL_NET_BALANCER_H__
#define __CEL_NET_BALANCER_H__

#include "cel/types.h"
#include "cel/arraylist.h"
#include "cel/keyword.h"
#include "cel/net/node.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CEL_MAX_WEIGHT 128

typedef enum _CelBalancerType
{
    CEL_BALANCER_UNDEFINED = -1,
    CEL_BALANCER_HASH,           /* Hash-Value */
    CEL_BALANCER_LC,             /* Least-Connection */
    CEL_BALANCER_LV,             /* Load-Value */
    CEL_BALANCER_RR,             /* Round-Robin */
    CEL_BALANCER_WLC,            /* Weighted Least-Connection */
    CEL_BALANCER_WLV,            /* Weighted Load-Value */
    CEL_BALANCER_WRR,            /* Weighted Round-Robin */
    CEL_BALANCER_COUNT
}CelBalancerType;

typedef struct _CelBalancer
{
    CelBalancerType type;
    int max_weight, gcd; /* Greatest common divisor */
    int cur_weight, cur_index; 
    CelNode *cur_node;
    CelArrayList *nodes;
    union {
        int (* schedule) (struct _CelBalancer *balancer);
        int (* schedule_hash) (struct _CelBalancer *balancer, int hash_value);
    };
}CelBalancer;

extern CelKeyword balancer_types[];

#define cel_balancer_type(type_str) \
    (CelBalancerType)cel_keyword_binary_search(\
    balancer_types, CEL_BALANCER_COUNT, type_str, _tcslen(type_str))

int cel_balancer_init(CelBalancer *balancer, \
                      CelBalancerType type, CelArrayList *nodes);
#define cel_balancer_init_rr(balancer, nodes) \
    cel_balancer_init(balancer, CEL_BALANCER_RR, nodes)
#define cel_balancer_init_wrr(balancer, nodes) \
    cel_balancer_init(balancer, CEL_BALANCER_WRR, nodes)
#define cel_balancer_init_lc(balancer, nodes) \
    cel_balancer_init(balancer, CEL_BALANCER_LC, nodes)
#define cel_balancer_init_wlc(balancer, nodes) \
    cel_balancer_init(balancer, CEL_BALANCER_WLC, nodes)
#define cel_balancer_init_lv(balancer, nodes) \
    cel_balancer_init(balancer, CEL_BALANCER_LV, nodes)
#define cel_balancer_init_wlv(balancer, nodes) \
    cel_balancer_init(balancer, CEL_BALANCER_WLV, nodes)
#define cel_balancer_init_hash(balancer, nodes) \
    cel_balancer_init(balancer, CEL_BALANCER_HASH, nodes)
void cel_balancer_destroy(CelBalancer *balancer);

CelBalancer *cel_balancer_new(CelBalancerType type, CelArrayList *nodes);
#define cel_balancer_new_rr(nodes) cel_balancer_new(CEL_BALANCER_RR, nodes)
#define cel_balancer_new_wrr(nodes) cel_balancer_new(CEL_BALANCER_WRR, nodes)
#define cel_balancer_new_lc(nodes) cel_balancer_new(CEL_BALANCER_LC, nodes)
#define cel_balancer_new_wlc(nodes) cel_balancer_new(CEL_BALANCER_WLC, nodes)
#define cel_balancer_new_lv(nodes) cel_balancer_new(CEL_BALANCER_LV, nodes)
#define cel_balancer_new_wlv(nodes) cel_balancer_new(CEL_BALANCER_WLV, nodes)
#define cel_balancer_new_hash(nodes) cel_balancer_new(CEL_BALANCER_HASH, nodes)
void cel_balancer_free(CelBalancer *balancer);

void cel_balancer_reset(CelBalancer *balancer);
/* void *cel_balancer_schedule(CelBalancer *balancer); */
#define cel_balancer_schedule(balancer) \
    (((balancer)->schedule(balancer) == -1) ? NULL : (balancer)->cur_node)
#define cel_balancer_schedule_hash(balancer, hash_value) \
    (((balancer)->schedule_hash(balancer, hash_value) == -1) \
    ? NULL : (balancer)->cur_node)

#ifdef __cplusplus
}
#endif

#endif