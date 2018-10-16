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
#ifndef __CEL_ARRARYLIST_H__
#define __CEL_ARRARYLIST_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelArrayList
{
    CelFreeFunc free_func;
    size_t size, capacity, reserve;
    void **items;
}CelArrayList;

int cel_arraylist_init(CelArrayList *array_list, CelFreeFunc free_func);
void cel_arraylist_destroy(CelArrayList *array_list);
CelArrayList *cel_arraylist_new(CelFreeFunc free_func);
void cel_arraylist_free(CelArrayList *array_list);

int cel_arraylist_push(CelArrayList *array_list, size_t position, void *value);
void cel_arraylist_push_back(CelArrayList *array_list, void *value);

static __inline size_t cel_arraylist_get_size(CelArrayList *array_list)
{
    return array_list->size;
}
static __inline BOOL cel_arraylist_is_empty(CelArrayList *array_list)
{
    return (array_list->size == 0 ? TRUE : FALSE);
}
int cel_arraylist_resize(CelArrayList *array_list, size_t num);
BOOL cel_arraylist_maybe_larger(CelArrayList *array_list, size_t *new_size);
BOOL cel_arraylist_maybe_smaller(CelArrayList *array_list, size_t *new_size);

static __inline size_t cel_arraylist_get_capacity(CelArrayList *array_list)
{
    return array_list->capacity;
}
static __inline 
void cel_arraylist_set_reserve(CelArrayList *array_list, size_t num)
{
    array_list->reserve = num;
}

static __inline void *cel_arraylist_get_front(CelArrayList *array_list)
{
    return (array_list->size > 0 ? array_list->items[0] : NULL);
}
static __inline void *cel_arraylist_get_back(CelArrayList *array_list)
{
    return (array_list->size > 0 
        ? array_list->items[array_list->size - 1] : NULL);
}
static __inline 
void *cel_arraylist_get_by_index(CelArrayList *array_list, size_t index)
{
    return (index < array_list->size ? array_list->items[index] : NULL);
}
int cel_arraylist_foreach(CelArrayList *array_list, 
                          CelEachFunc each_func, void *user_data);

void *cel_arraylist_pop(CelArrayList *array_list, size_t position);
void *cel_arraylist_pop_back(CelArrayList *array_list);
static __inline 
void cel_arraylist_remove(CelArrayList *array_list, size_t index)
{
    if (array_list->free_func != NULL)
        array_list->free_func(cel_arraylist_pop(array_list, index));
}
static __inline void cel_arraylist_remove_back(CelArrayList *array_list)
{
    if (array_list->free_func != NULL)
        array_list->free_func(cel_arraylist_pop_back(array_list));
}
void cel_arraylist_clear(CelArrayList *array_list);

#ifdef __cplusplus
}
#endif

#endif