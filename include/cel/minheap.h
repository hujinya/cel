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
#ifndef __CEL_MINHEAP_H__
#define __CEL_MINHEAP_H__

#include "cel/types.h"
#include "cel/arraylist.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelMinHeap
{
    CelArrayList array_list;
    CelCompareFunc comparator;
}CelMinHeap;

int cel_minheap_init(CelMinHeap *min_heap,
                     CelCompareFunc comparator, CelFreeFunc free_func);
void cel_minheap_destroy(CelMinHeap *min_heap);

CelMinHeap *cel_minheap_new(CelCompareFunc comparator, CelFreeFunc free_func);
void cel_minheap_free(CelMinHeap *min_heap);

void cel_minheap_insert(CelMinHeap *min_heap, void *item);
static __inline size_t cel_minheap_get_size(CelMinHeap *min_heap)
{
    return cel_arraylist_get_size(&((min_heap)->array_list));
}
static __inline BOOL cel_minheap_is_empty(CelMinHeap *min_heap)
{
    return cel_arraylist_is_empty(&((min_heap)->array_list));
}
static __inline size_t cel_minheap_get_capacity(CelMinHeap *min_heap)
{
    return cel_arraylist_get_capacity(&((min_heap)->array_list));
}  
static __inline void cel_minheap_set_reserve(CelMinHeap *min_heap, size_t num)
{
    cel_arraylist_set_reserve(&((min_heap)->array_list), num);
}

/* void *cel_minheap_get_min(CelMinHeap *min_heap) */
#define cel_minheap_get_min(min_heap) \
    cel_arraylist_get_front(&((min_heap)->array_list))
/* void *cel_minheap_get_by_index(CelMinHeap *min_heap, int index)*/
#define cel_minheap_get_by_index(min_heap, index) \
    cel_arraylist_get_by_index(&((min_heap)->array_list), index)
/* 
 * int cel_minheap_foreach(CelMinHeap *min_heap, 
 *                         CelEachFunc each_func, void *user_data)
 * \brief  Breadth-first.
 */
#define cel_minheap_foreach(min_heap, each_func, user_data) \
    cel_arraylist_foreach(&((min_heap)->array_list), each_func, user_data)

void *cel_minheap_pop_min(CelMinHeap *min_heap);
static __inline void cel_minheap_remove_min(CelMinHeap *min_heap)
{
    if (min_heap->array_list.free_func != NULL)
        min_heap->array_list.free_func(cel_minheap_pop_min(min_heap));
}
/* void cel_minheap_clear(CelMinHeap *min_heap); */
#define cel_minheap_clear(min_heap) \
    cel_arraylist_clear(&((min_heap)->array_list))

#ifdef __cplusplus
}
#endif

#endif
