/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_FREELIST_H__
#define __CEL_FREELIST_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelBlock
{
    struct _CelBlock *next;
}CelBlock;

typedef struct _CelFreeList
{
    CelBlock *free;
    size_t size, max_size;
    size_t lowater, overages;
}CelFreeList;

int cel_freelist_init(CelFreeList *free_list);
void cel_freelist_destroy(CelFreeList *free_list);

CelFreeList *cel_freelist_new(void);
void cel_freelist_free(CelFreeList *free_list);

#define cel_freelist_is_empty(free_list) (free_list)->free == NULL
#define cel_freelist_get_size(free_list) (free_list)->size
#define cel_freelist_get_max_size(free_list) (free_list)->max_size
#define cel_freelist_set_max_size(free_list, _max_size) \
    (free_list)->max_size = _max_size
#define cel_freelist_get_lowater(free_list) (free_list)->lowater
#define cel_freelist_clear_lowater(free_list) \
    (free_list)->lowater = (free_list)->size
#define cel_freelist_get_overages(free_list) (free_list)->overages
#define cel_freelist_set_overages(free_list, _overages) \
    (free_list)->overages = _overages

#define cel_freelist_push(free_list, block) \
    (block)->next = (free_list)->free, \
    (free_list)->free = block, \
    ((free_list)->size)++

static __inline CelBlock *cel_freelist_pop(CelFreeList *free_list)
{
    CelBlock *result;

    (free_list->size)--;
    if (free_list->size < free_list->lowater)
        free_list->lowater = free_list->size;
    result = free_list->free;
    free_list->free = result->next;
    return result;
}

#define cel_freelist_push_range(free_list, start, end, num) \
    (end)->next = (free_list)->free,\
    (free_list)->free = (start),\
    ((free_list)->size) += (num)

void cel_freelist_pop_range(CelFreeList *free_list, 
                            CelBlock **start, CelBlock **end, int num);

#ifdef __cplusplus
}
#endif

#endif
