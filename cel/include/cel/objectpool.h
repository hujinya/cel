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
#ifndef __CEL_OBJECTPOOL_H__
#define __CEL_OBJECTPOOL_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelObjectPool
{
    int size;
    CelConstructFunc constructor;
    CelDestroyFunc destructor;
    int n, n_used;
    int max, max_unused, max_usage;
    void *free_list;
    struct _CelObjectPool *self;
}CelObjectPool;

int cel_objectpool_init(CelObjectPool *obj_pool, int object_size, int max_num,
                        CelConstructFunc constructor, 
                        CelDestroyFunc destructor);
void cel_objectpool_destroy(CelObjectPool *obj_pool);
CelObjectPool *cel_objectpool_new(int object_size, int max_num,
                                  CelConstructFunc constructor, 
                                  CelDestroyFunc destructor);
void cel_objectpool_free(CelObjectPool *obj_pool);

void *cel_objectpool_get(CelObjectPool *obj_pool);
void  cel_objectpool_return(void *obj);

/* void cel_objectpool_set_max_objects(CelObjectPool *obj_pool, int num)*/
#define cel_objectpool_set_max_objects(obj_pool, num) \
    (obj_pool)->max = (num)
/* int cel_objectpool_get_max_objects(CelObjectPool *obj_pool)*/
#define cel_objectpool_get_max_objects(obj_pool) ((obj_pool)->max)
/* 
 * void cel_objectpool_set_max_unused_objects(CelObjectPool *obj_pool, int num)
*/
#define cel_objectpool_set_max_unused_objects(obj_pool, num) \
    (obj_pool)->max_unused = (max_unused)
/* int cel_objectpool_get_max_unused_objects(CelObjectPool *obj_pool)*/
#define cel_objectpool_get_max_unused_objects(obj_pool) \
    ((obj_pool)->max_unused)
/* int cel_objectpool_get_max_usage(CelObjectPool *obj_pool)*/
#define cel_objectpool_get_max_usage(obj_pool) ((obj_pool)->max_usage)

#ifdef __cplusplus
}
#endif

#endif
