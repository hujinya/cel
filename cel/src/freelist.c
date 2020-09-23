/**
 * CEL(C Extension Library)
 * Copyright (C)2008 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/freelist.h"
#include "cel/allocator.h"
#include "cel/log.h"

int cel_freelist_init(CelFreeList *free_list)
{
    free_list->free = NULL;
    free_list->size = 0;
    free_list->max_size = 1;
    free_list->lowater = 0;
    free_list->overages = 0;

    return 0;
}

void cel_freelist_destroy(CelFreeList *free_list)
{
    free_list->free = NULL;
    free_list->size = 0;
    free_list->max_size = 0;
    free_list->lowater = 0;
    free_list->overages = 0;
}

CelFreeList *cel_freelist_new(void)
{
    return NULL;
}

void cel_freelist_free(CelFreeList *free_list)
{
    cel_freelist_destroy(free_list);
    cel_free(free_list);
}

void cel_freelist_pop_range(CelFreeList *free_list, 
                            CelBlock **start, CelBlock **end, int num)
{
    int i;
    CelBlock *tmp = free_list->free;

    for (i = 1; i < num; i++)
    {
        tmp = tmp->next;
        //printf("%p\r\n", tmp);
    }
    *start = free_list->free;
    *end = tmp;
    free_list->free = tmp->next;
    //printf("start %p, end %p, free %p\r\n", *start, *end, free_list->free);
    (*end)->next = NULL;
    (free_list->size) -= num;
    if (free_list->size < free_list->lowater)
        free_list->lowater = free_list->size;
}
