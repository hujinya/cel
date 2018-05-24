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
#ifndef __CEL_QUEUE_H__
#define __CEL_QUEUE_H__

#include "cel/types.h"
#include "cel/arraylist.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelQueue
{
    CelArrayList array_list;
    size_t head_index, tail_index;
}CelQueue;

int cel_queue_init(CelQueue *queue, CelFreeFunc free_func);
void cel_queue_destroy(CelQueue *queue);
CelQueue *cel_queue_new(CelFreeFunc free_func);
void cel_queue_free(CelQueue *queue);

void cel_queue_push_front(CelQueue *queue, void *item);
void cel_queue_push_back(CelQueue *queue, void *item);

static __inline size_t cel_queue_get_size(CelQueue *queue)
{
    return cel_arraylist_get_size(&((queue)->array_list));
}
static __inline BOOL cel_queue_is_empty(CelQueue *queue)
{
    return cel_arraylist_is_empty(&((queue)->array_list));
}
static __inline size_t cel_queue_get_capacity(CelQueue *queue)
{
    return cel_arraylist_get_capacity(&((queue)->array_list));
}
static __inline void cel_queue_set_reserve(CelQueue *queue, size_t num)
{
    cel_arraylist_set_reserve(&((queue)->array_list), num);
}

static __inline void *cel_queue_get_front(CelQueue *queue)
{
    size_t index;

    if (queue->array_list.size <= 0) return NULL;
    index = ((queue->head_index < queue->array_list.capacity)
        ? (queue->head_index + 1): 0);

    return queue->array_list.items[index];
}
static __inline void *cel_queue_get_back(CelQueue *queue)
{
    size_t index;

    if (queue->array_list.size <= 0) return NULL;
    index = ((queue->tail_index > 0)
        ? (queue->tail_index - 1): (queue->array_list.capacity));

    return queue->array_list.items[index];
}

int cel_queue_foreach(CelQueue *queue, CelEachFunc each_func, void *user_data);

void *cel_queue_pop_front(CelQueue *queue);
void *cel_queue_pop_back(CelQueue *queue);
static __inline void cel_queue_remove_front(CelQueue *queue)
{
    if (queue->array_list.free_func != NULL)
        queue->array_list.free_func(cel_queue_pop_front(queue));
}
static __inline void cel_queue_remove_back(CelQueue *queue)
{
    if (queue->array_list.free_func != NULL)
        queue->array_list.free_func(cel_queue_pop_back(queue));
}
void cel_queue_clear(CelQueue *queue);

#ifdef __cplusplus
}
#endif

#endif
