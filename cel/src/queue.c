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
#include "cel/queue.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

int cel_queue_init(CelQueue *queue, CelFreeFunc free_func)
{
    queue->head_index = 0;
    queue->tail_index = 1;
    return cel_arraylist_init(&(queue->array_list), free_func);
}

void cel_queue_destroy(CelQueue *queue)
{
    cel_queue_clear(queue);
    cel_free(queue->array_list.items);
    queue->array_list.capacity = 0;
    queue->array_list.size = 0;
    queue->array_list.free_func = NULL;
}

CelQueue *cel_queue_new(CelFreeFunc free_func)
{
    CelQueue *queue;

    if ((queue = (CelQueue *)cel_malloc(sizeof(CelQueue))) != NULL)
    {
        if (cel_queue_init(queue, free_func) == 0)
            return queue;
        cel_free(queue);
    }

    return NULL;
}

void cel_queue_free(CelQueue *queue)
{
    cel_queue_destroy(queue);
    cel_free(queue);
}

static int cel_queue_resize(CelQueue *queue, size_t num)
{
    size_t old_head, old_capacity = queue->array_list.capacity;
    void **new_items;

    /*
    _tprintf(_T("Queue resize(%p:%d), new capacity %d, old capacity %d,"
        " head %d, tail %d.\r\n"), 
        queue->array_list.items, queue->array_list.size, 
        num, queue->array_list.capacity, 
        queue->head_index , queue->tail_index);
        */
    if (num > queue->array_list.capacity)
    {
        if ((new_items = (void **)cel_realloc(
            queue->array_list.items, (num + 1) * sizeof(void *))) == NULL)
        {
            return -1;
        }
        queue->array_list.capacity = num;
        if (queue->head_index >= queue->tail_index)
        {
            old_head = queue->head_index;
            queue->head_index = 
                queue->array_list.capacity - old_capacity + old_head;
            memmove(new_items + queue->head_index + 1, 
                new_items + old_head + 1,
                (old_capacity - old_head) * sizeof(void *));
        }
    }
    else
    {
        if ((new_items = 
            (void **)cel_malloc((num + 1) * sizeof(void *))) == NULL)
            return -1;
        queue->array_list.capacity = num;
        if (queue->array_list.items != NULL)
        {
            if (queue->head_index < queue->tail_index)
            {
                memcpy(new_items + 1, 
                    queue->array_list.items + queue->head_index + 1,
                    queue->array_list.size * sizeof(void *));
                queue->head_index = 0;
                queue->tail_index = queue->array_list.size + 1;
            }
            else
            {
                memcpy(new_items, queue->array_list.items, 
                    (queue->tail_index + 1) * sizeof(void *)); 
                old_head = queue->head_index;
                queue->head_index = 
                    queue->array_list.capacity - old_capacity + old_head;
                memcpy(new_items + queue->head_index + 1, 
                    queue->array_list.items + old_head + 1,
                    (old_capacity - old_head) * sizeof(void *));
            }
            cel_free(queue->array_list.items);
        } 
    }
    queue->array_list.items = new_items;

    return 0;
}

void cel_queue_push_front(CelQueue *queue, void *item)
{
    size_t new_size;

    if (cel_arraylist_maybe_larger(&(queue->array_list), &new_size)
        && cel_queue_resize(queue, new_size) == -1)
        return ;
    queue->array_list.items[queue->head_index] = item;
    CEL_DEBUG((_T("Queue push front %p[%d]."), 
        queue->array_list.items[queue->head_index], queue->head_index));
    if (queue->head_index > 0)
        queue->head_index--;
    else
        queue->head_index = queue->array_list.capacity;
    queue->array_list.size++;
}

void cel_queue_push_back(CelQueue *queue, void *item)
{
    size_t new_size;

    if (cel_arraylist_maybe_larger(&(queue->array_list), &new_size)
        && cel_queue_resize(queue, new_size) == -1)
        return ;
    queue->array_list.items[queue->tail_index] = item;
    if (queue->tail_index < queue->array_list.capacity)
        queue->tail_index++;
    else
        queue->tail_index = 0;
    queue->array_list.size++;
}

void *cel_queue_pop_front(CelQueue *queue)
{
    size_t new_size;
    void *item;

    if (queue->array_list.size <= 0)
        return NULL;
    if (queue->head_index < queue->array_list.capacity)
        queue->head_index++;
    else
        queue->head_index = 0;
    CEL_DEBUG((_T("Queue pop front %p[%d]."), 
        queue->array_list.items[queue->head_index], queue->head_index));
    item = queue->array_list.items[queue->head_index];
    queue->array_list.size--;
    if (cel_arraylist_maybe_smaller(&(queue->array_list), &new_size)
        && cel_queue_resize(queue, new_size) == -1)
        return NULL;

    return item;
}

void *cel_queue_pop_back(CelQueue *queue)
{
    size_t new_size;
    void *item;

    if (queue->array_list.size <= 0)
        return NULL;
    if (queue->tail_index > 0)
        queue->tail_index--;
    else
        queue->tail_index = queue->array_list.capacity;
    item = queue->array_list.items[queue->tail_index];
    CEL_DEBUG((_T("Queue pop back %p[%d]."), 
        queue->array_list.items[queue->tail_index], queue->tail_index));
    queue->array_list.size--;
    if (cel_arraylist_maybe_smaller(&(queue->array_list), &new_size)
        && cel_queue_resize(queue, new_size) == -1)
        return NULL;

    return item;
}

int cel_queue_foreach(CelQueue *queue, CelEachFunc each_func, void *user_data)
{
    int ret;
    size_t i, index = queue->head_index;

    for (i = 0; i < queue->array_list.size; i++)
    {
        if (index < queue->array_list.capacity)
            index++;
        else
            index = 0;
        if ((ret = each_func(queue->array_list.items[index], user_data)) != 0)
            return ret;
    }
    return 0;
}

void cel_queue_clear(CelQueue *queue)
{
    size_t new_size;

    for (; queue->array_list.size > 0; queue->array_list.size--)
    {
        if (queue->head_index < queue->array_list.capacity)
            queue->head_index++;
        else
            queue->head_index = 0;
        if (queue->array_list.free_func != NULL)
            queue->array_list.free_func(
            queue->array_list.items[queue->head_index]); 
    }
    if (cel_arraylist_maybe_smaller(&(queue->array_list), &new_size))
        cel_queue_resize(queue, new_size);
}
