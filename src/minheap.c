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
#include "cel/minheap.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

/* Debug defines */
#define Debug(args)   /* cel_log_debug args */ 
#define Warning(args) CEL_SETERRSTR(args)/* cel_log_warning args */
#define Err(args)   CEL_SETERRSTR(args)/* cel_log_err args */

int cel_minheap_init(CelMinHeap *min_heap, CelCompareFunc comparator, 
                     CelFreeFunc free_func)
{
    min_heap->comparator = comparator;
    return cel_arraylist_init(&(min_heap->array_list), free_func);
}

void cel_minheap_destroy(CelMinHeap *min_heap)
{
    cel_arraylist_destroy(&(min_heap->array_list));
    min_heap->comparator = NULL;
}

CelMinHeap *cel_minheap_new(CelCompareFunc comparator, CelFreeFunc free_func)
{
    CelMinHeap *min_heap;

    if ((min_heap = (CelMinHeap *)cel_malloc(sizeof(CelMinHeap))) != NULL)
    {
        if (cel_minheap_init(min_heap, comparator, free_func) == 0)
            return min_heap;
        cel_free(min_heap);
    }

    return NULL;
}

void cel_minheap_free(CelMinHeap *min_heap)
{
    cel_minheap_destroy(min_heap);
    cel_free(min_heap);
}

static void cel_minheap_filter_up(CelMinHeap *min_heap, size_t start)
{
    size_t j = start, i = (j - 1) / 2;
    void *temp;

    temp = min_heap->array_list.items[j];
    while (j > 0)
    {
        if (min_heap->comparator(min_heap->array_list.items[i], temp) <= 0)
            break;
        else
        {
            min_heap->array_list.items[j] = min_heap->array_list.items[i];
            j = i;
            i = (i - 1) / 2;
        }
    }
    min_heap->array_list.items[j] = temp;
}

static void cel_minheap_filter_down(CelMinHeap *min_heap, 
                                    size_t start, size_t end)
{
    size_t i = start, j = 2 * i + 1;
    void *temp;
    
    temp = min_heap->array_list.items[i];
    while (j <= end)
    {
        if (j < end 
            && min_heap->comparator(
            min_heap->array_list.items[j], 
            min_heap->array_list.items[j + 1]) > 0)
            j++;
        if (min_heap->comparator(temp, min_heap->array_list.items[j]) <= 0)
            break;
        else
        {
            min_heap->array_list.items[i] = min_heap->array_list.items[j];
            i = j;
            j = 2 * j + 1;
        }
    }
    min_heap->array_list.items[i] = temp;
}

void cel_minheap_insert(CelMinHeap *min_heap, void *item)
{
    size_t new_size;

    if (cel_arraylist_maybe_larger(&(min_heap->array_list), &new_size)
        && cel_arraylist_resize(&(min_heap->array_list), new_size) == -1)
        return ;
    min_heap->array_list.items[min_heap->array_list.size] = item;
    cel_minheap_filter_up(min_heap, min_heap->array_list.size);
    min_heap->array_list.size++;
}

void *cel_minheap_pop_min(CelMinHeap *min_heap)
{
    size_t new_size;
    void *item;

    if (min_heap->array_list.size <= 0)
        return NULL;
    item = min_heap->array_list.items[0];
    min_heap->array_list.items[0] = 
        min_heap->array_list.items[min_heap->array_list.size - 1];
    //min_heap->array_list.items[min_heap->array_list.size - 1] = NULL;
    if ((--min_heap->array_list.size) > 0)
        cel_minheap_filter_down(min_heap, 0, min_heap->array_list.size - 1);
    if (cel_arraylist_maybe_smaller(&(min_heap->array_list), &new_size)
        && cel_arraylist_resize(&(min_heap->array_list), new_size) == -1)
        return NULL;
    return item;
}
