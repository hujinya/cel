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
#include "cel/objectpool.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

/* Debug defines */
#define Debug(args)   /* cel_log_debug args */
#define Warning(args) CEL_SETERRSTR(args)/* cel_log_warning args */
#define Err(args)   CEL_SETERRSTR(args)/* cel_log_err args */

typedef union _CelObjectChunk
{
    union _CelObjectChunk *next_free;
    CelObjectPool *obj_pool;
}CelObjectChunk;

int cel_objectpool_init(CelObjectPool *obj_pool, 
                        int object_size, int max_num,
                        CelConstructFunc constructor, 
                        CelDestroyFunc destructor)
{
    if (object_size <= 0)
    {
        Err((_T("Invalid object size \"%d\"."), object_size));
        return -1;
    }
    obj_pool->max = (max_num <= -1 ? -1 : max_num);
    obj_pool->max_unused = -1;
    obj_pool->n = 0;
    obj_pool->n_used = obj_pool->max_usage = 0;

    obj_pool->size = object_size + sizeof(CelObjectChunk);
    obj_pool->constructor = constructor;
    obj_pool->destructor = destructor;

    obj_pool->free_list = NULL;
    obj_pool->self = obj_pool;

    return 0;
}

void cel_objectpool_destroy(CelObjectPool *obj_pool)
{
    CelObjectChunk *free_obj;

    if (obj_pool == NULL || obj_pool->self != obj_pool)
    {
        Err((_T("Invalid object pool pointer.")));
        return ;
    }
    while (obj_pool->free_list != NULL)
    {
        free_obj = (CelObjectChunk *)obj_pool->free_list;
        obj_pool->free_list = free_obj->next_free;
        if (obj_pool->destructor != NULL)
            obj_pool->destructor((char *)free_obj + sizeof(CelObjectChunk));
        cel_free(free_obj);
        obj_pool->n--;
        obj_pool->n_used--;
    }
}

CelObjectPool *cel_objectpool_new(int object_size, int max_num,
                                  CelConstructFunc constructor,
                                  CelDestroyFunc destructor)
{
    CelObjectPool *obj;

    if ((obj = (CelObjectPool *)cel_malloc(sizeof(CelObjectPool))) != NULL)
    {
        if (cel_objectpool_init(
            obj, object_size, max_num, constructor, destructor) == 0)
            return obj;
        cel_free(obj);
    }
    return NULL;
}

void cel_objectpool_free(CelObjectPool *obj_pool)
{
    cel_objectpool_destroy(obj_pool); 
    cel_free(obj_pool);
}

void *cel_objectpool_get(CelObjectPool *obj_pool)
{
    CelObjectChunk *new_obj;

    if (obj_pool == NULL || obj_pool->self != obj_pool)
    {
        Err((_T("Invalid object pool pointer.")));
        return NULL;
    }
    if (obj_pool->free_list == NULL)
    {
        if (obj_pool->n > obj_pool->max)
        {
            Warning((_T("")));
            return NULL;
        }
        if ((new_obj = (CelObjectChunk *)cel_malloc(obj_pool->size)) != NULL)
        {
            if (obj_pool->constructor != NULL
                && obj_pool->constructor(
                (char *)new_obj + sizeof(CelObjectChunk)) == -1)
            {
                cel_free(new_obj);
                return NULL;
            }
        }
        obj_pool->n++;
    }
    else
    {
        new_obj = (CelObjectChunk *)obj_pool->free_list;
        obj_pool->free_list = new_obj->next_free;
    }
    if ((++obj_pool->n_used) > obj_pool->max_usage)
        obj_pool->max_usage = obj_pool->n_used;

    return ((char *)new_obj + sizeof(CelObjectChunk));
}

void cel_objectpool_return(void *obj)
{
    CelObjectPool *obj_pool;
    CelObjectChunk *free_obj;
 
    free_obj = (CelObjectChunk *)((char *)obj - sizeof(CelObjectChunk));
    obj_pool = free_obj->obj_pool;
    if (obj_pool->self != obj_pool || obj_pool->n_used == 0)
    {
        Err((_T("Invalid object pointer.")));
        return;
    }
    free_obj->next_free = (CelObjectChunk *)obj_pool->free_list;
    obj_pool->free_list = free_obj;
    while (obj_pool->max_unused != -1
        || (obj_pool->n - obj_pool->n_used) >= obj_pool->max_unused)
    {
        if (obj_pool->destructor != NULL)
            obj_pool->destructor(obj);
        cel_free(free_obj);
        obj_pool->n--;
    }
    obj_pool->n_used--;
}
