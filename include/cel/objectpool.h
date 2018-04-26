/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
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
