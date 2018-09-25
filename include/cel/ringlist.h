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
#ifndef __CEL_RINGLIST_H__
#define __CEL_RINGLIST_H__

#include "cel/types.h"
#include "cel/atomic.h"

typedef struct _CelRingList
{
    U32 size;
    CelFreeFunc value_free;
    U32 mask;
    U32 watermark;
    CelAtomic p_head, p_tail;
    CelAtomic c_head, c_tail;
    void **ring;
}CelRingList;

typedef void (*CelRingListProdFunc)(CelRingList *ring_list, 
                                    unsigned long cons_head, 
                                    void **values, size_t n, 
                                    void *user_data);
typedef void (*CelRingListConsFunc)(CelRingList *ring_list, 
                                    unsigned long prod_head,
                                    void **values, size_t n,
                                    void *user_data);

#ifdef __cplusplus
extern "C" {
#endif

int cel_ringlist_init(CelRingList *ring_list, size_t n_size, 
                      CelFreeFunc value_free);
void cel_ringlist_destroy(CelRingList *ring_list);

CelRingList *cel_ringlist_new(size_t n_size, CelFreeFunc value_free);
void cel_ringlist_free(CelRingList *ring_list);

void cel_ringlist_clear(CelRingList *ring_list);

static __inline int cel_ringlist_get_size(CelRingList *ring_list)
{
    return ring_list->size;
}
static __inline int cel_ringlist_get_count(CelRingList *ring_list)
{
    U32 prod_tail = ring_list->p_tail;
    U32 cons_tail = ring_list->c_tail;
    U32 count = ((prod_tail - cons_tail) & ring_list->mask);
    return (count > ring_list->size) ? ring_list->size : count;
}
static __inline int cel_ringlist_get_free(CelRingList *ring_list)
{
    return ring_list->size - cel_ringlist_get_count(ring_list);
}

int _cel_ringlist_push_do(CelRingList *ring_list, BOOL is_sp, 
                          void **values, size_t n,
                          CelRingListProdFunc handle, void *user_data);
/* int cel_ringlist_push_do_mp(CelRingList *ring_list, void *values, size_t n); */
#define cel_ringlist_push_do_mp(ring_list, values, n) \
    _cel_ringlist_push_do(ring_list, FALSE, (void **)&values, n, NULL, NULL)
/* int cel_ringlist_push_do_sp(CelRingList *ring_list, void *values, size_t n); */
#define cel_ringlist_push_do_sp(ring_list, values, n) \
    _cel_ringlist_push_do(ring_list, TRUE, (void **)&values, n, NULL, NULL)

int _cel_ringlist_pop_do(CelRingList *ring_list, BOOL is_sp, 
                         void **values, size_t n, 
                         CelRingListConsFunc handle, void *user_data);
/* int cel_ringlist_pop_do_mp(CelRingList *ring_list, void **values, size_t n) */
#define cel_ringlist_pop_do_mp(ring_list, values, n) \
    _cel_ringlist_pop_do(ring_list, FALSE, (void **)values, n, NULL, NULL)
/* int cel_ringlist_pop_do_sp(CelRingList *ring_list, void **values, size_t n); */
#define cel_ringlist_pop_do_sp(ring_list, values, n) \
    _cel_ringlist_pop_do(ring_list, TRUE, (void **)values, n, NULL, NULL)

#ifdef __cplusplus
}
#endif

#endif
