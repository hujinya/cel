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
#include "cel/list.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

int cel_list_init(CelList *list, CelFreeFunc free_func)
{
    if (list == NULL) 
        return -1;

    list->head.next = &(list->tail);
    list->head.prev = NULL;
    list->tail.prev = &(list->head);
    list->tail.next = NULL;
    list->size = 0;
    list->free_func = free_func;

    return 0;
}

void cel_list_destroy(CelList *list)
{
    cel_list_clear(list);
}

CelList *cel_list_new(CelFreeFunc free_func)
{
    CelList *list;

    if ((list = (CelList *)cel_malloc(sizeof(CelList))) != NULL)
    {
        if (cel_list_init(list, free_func) == 0)
            return list;
        cel_free(list);
    }
    return NULL;
}

void cel_list_free(CelList *list)
{
    cel_list_clear(list); 
    cel_free(list);
}

CelListItem *cel_list_pop_front(CelList *list)
{
    CelListItem *pop_item;

    if (list->head.next == &(list->tail))
        return NULL;
    pop_item = list->head.next;
    cel_list_remove(list, pop_item);

    return pop_item;
}

CelListItem *cel_list_pop_back(CelList *list)
{
    CelListItem *pop_item;

    if (list->tail.prev == &(list->head))
        return NULL;
    pop_item = list->tail.prev;
    cel_list_remove(list, pop_item);

    return pop_item;
}

static void cel_list_quick_sort_intern(CelListItem *low_item, 
                                       CelListItem *high_item,
                                       CelCompareFunc compare_func)
{
    CelListItem *pivot, *tmp;
    CelListItem *item1, *item2;

    pivot = low_item;
    item1 = high_item->prev;
    item2 = low_item->next;

    while (item1 == item2)
    {
        while (item1 != low_item && compare_func(item1, pivot) >= 0)
            item1 = item1->prev;
        tmp = low_item;
        low_item = item1;
        item1 = tmp;

        while (item2 != high_item && compare_func(item2, pivot) <= 0)
            item2 = item2->next;
        tmp = high_item;
        high_item = item2;
        item2 = tmp;
    }
    if (item2 != low_item)
        cel_list_quick_sort_intern(low_item, item2->prev,compare_func);
    if (item1 != high_item)
        cel_list_quick_sort_intern(item1->next, high_item, compare_func);
}

void cel_list_quick_sort(CelList *list, CelCompareFunc compare_func)
{
    if (list == NULL 
        || list->head.next == NULL
        || list->head.next == list->tail.prev
        || compare_func == NULL)
        return ;
    cel_list_quick_sort_intern(list->head.next, list->tail.prev, compare_func);
}

int cel_list_foreach(CelList *list, CelEachFunc each_func, void *user_data)
{
    int ret;
    CelListItem *next_item = list->head.next, *cur_item;

    while ((cur_item = next_item) != &(list->tail))
    {
        next_item = cur_item->next;
        if ((ret = each_func(cur_item, user_data)) != 0)
            return ret;
    }
    return 0;
}

void cel_list_clear(CelList *list)
{
    CelListItem *next_item, *cur_item = list->head.next;

    while (cur_item != &(list->tail))
    {
        next_item = cur_item->next;
        /* Free item's value */
        if (list->free_func != NULL)
            list->free_func(cur_item);
        cel_free(cur_item);
        cur_item = next_item;
    }
    list->size = 0;
    list->head.next = &(list->tail);
    list->head.prev = NULL;
    list->tail.prev = &(list->head);
    list->tail.next = NULL;
}
