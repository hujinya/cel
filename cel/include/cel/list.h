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
#ifndef __CEL_LIST_H__
#define __CEL_LIST_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelListItem
{
    struct _CelListItem *next, *prev;
}CelListItem;

typedef struct _CelList
{
    struct _CelListItem head, tail;
    size_t size;
    CelFreeFunc free_func;
}CelList;

int cel_list_init(CelList *list, CelFreeFunc free_func);
void cel_list_destroy(CelList *list);
CelList *cel_list_new(CelFreeFunc free_func);
void cel_list_free(CelList *list);

static __inline void cel_list_push_front(CelList *list, CelListItem *item)
{
    item->next = list->head.next;
    item->prev = &(list->head);
    list->head.next->prev = item;
    list->head.next = item;
    (list->size)++;
}

static __inline void cel_list_push_back(CelList *list, CelListItem *item)
{
    item->next = &(list->tail);
    item->prev = list->tail.prev;
    list->tail.prev->next = item;
    list->tail.prev = item;
    (list->size)++;
}

static __inline size_t cel_list_get_size(CelList *list) 
{ 
    return list->size; 
}
static __inline BOOL cel_list_is_empty(CelList *list)
{
    return (list->size == 0 ? TRUE : FALSE);
}

#define cel_list_get_head(list) &((list)->head)
#define cel_list_get_tail(list) &((list)->tail)

static __inline CelListItem *cel_list_get_front(CelList *list) 
{ 
    return list->head.next;
}
static __inline CelListItem *cel_list_get_back(CelList *list) 
{ 
    return list->tail.prev; 
}
//void cel_list_quick_sort(CelList *list, CelCompareFunc compare_func);
int cel_list_foreach(CelList *list, CelEachFunc each_func, void *user_data);

CelListItem *cel_list_pop_front(CelList *list);
CelListItem *cel_list_pop_back(CelList *list);

static __inline void cel_list_remove(CelList *list, CelListItem *item)
{
    item->prev->next = item->next;
    item->next->prev = item->prev;
    item->prev = NULL;
    item->next = NULL;
    (list->size)--;
}

static __inline void cel_list_remove_front(CelList *list)
{
    CelListItem *pop_item;
    if ((pop_item = cel_list_pop_front(list)) != NULL 
        && list->free_func != NULL) 
        list->free_func(pop_item);
}
static __inline void cel_list_remove_back(CelList *list)
{
    CelListItem *pop_item;
    if ((pop_item = cel_list_pop_back(list)) != NULL 
        && list->free_func != NULL) 
        list->free_func(pop_item);
}
void cel_list_clear(CelList *list);

#ifdef __cplusplus
}
#endif

#endif
