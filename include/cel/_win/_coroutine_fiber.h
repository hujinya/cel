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
#ifndef __CEL_COROUTINE_FIBER_H__
#define __CEL_COROUTINE_FIBER_H__

#include "cel/types.h"
#include "cel/list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _OsCoroutineEntity OsCoroutineEntity;

typedef struct _OsCoroutineScheduler
{
    int co_capacity;
    int co_num;
    int co_running;
    LPVOID main_ctx;
    CelList ready_list;
    OsCoroutineEntity **co_entitys;
}OsCoroutineScheduler;

typedef struct _OsCoroutineAttr
{
    int stack_size;
}OsCoroutineAttr;

struct _OsCoroutineEntity
{  
    union {
        CelListItem list_item;
        struct {
            struct _OsCoroutineEntity *next, *prev;
        };
    };
    int id;
    OsCoroutineScheduler *schd;
    void *user_data;
    CelCoroutineStatus status;
    LPVOID ctx;
    OsCoroutineFunc func;
};

OsCoroutineScheduler *os_coroutinescheduler_new(void);
void os_coroutinescheduler_free(OsCoroutineScheduler *schd);

static __inline 
int os_coroutinescheduler_running_id(OsCoroutineScheduler *schd)
{
    return schd->co_running;
}

static __inline 
OsCoroutineEntity *os_coroutinescheduler_running(OsCoroutineScheduler *schd)
{
    return schd->co_entitys[schd->co_running];
}

static __inline 
void os_coroutinescheduler_push_ready(OsCoroutineScheduler *schd, 
                                      OsCoroutineEntity *co_entity)
{
    cel_list_push_front(&(schd->ready_list), &(co_entity->list_item));
}

static __inline 
OsCoroutineEntity *os_coroutinescheduler_pop_ready(OsCoroutineScheduler *schd)
{
    return (OsCoroutineEntity *)cel_list_pop_back(&(schd->ready_list));
}

int os_coroutineentity_create(OsCoroutineEntity **co_entity, 
                              OsCoroutineScheduler *schd,
                              OsCoroutineAttr *attr,
                              OsCoroutineFunc func, void *user_data);
void os_coroutineentity_resume(OsCoroutineEntity *co_entity);
void os_coroutineentity_yield(OsCoroutineEntity *co_entity);
static __inline 
CelCoroutineStatus os_coroutineentity_status(OsCoroutineEntity *co_entity)
{
    return co_entity->status;
}

#ifdef __cplusplus
}
#endif

#endif
