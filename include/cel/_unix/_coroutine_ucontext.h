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
#ifndef __CEL_COROUTINE_UNIX_H__
#define __CEL_COROUTINE_UNIX_H__

#include "cel/config.h"
#include "cel/types.h"
#include <stddef.h> /* ptrdiff_t */
#if __APPLE__ && __MACH__
#include <sys/ucontext.h>
#else 
#include <ucontext.h>
#endif 

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _OsCoroutineEntity OsCoroutineEntity;

typedef struct _OsCoroutineScheduler
{
    char stack[CEL_COROUTINE_STACK_SIZE];
    ucontext_t main_ctx;
    int co_running;
    int co_num;
    int co_capacity;
    OsCoroutineEntity **co_entitys;
}OsCoroutineScheduler;

typedef struct _OsCoroutineAttr
{
    int stack_size;
}OsCoroutineAttr;

struct _OsCoroutineEntity
{
    int id;
    OsCoroutineFunc func;
    void *user_data;
    ucontext_t ctx;
    OsCoroutineScheduler *schd;
    CelCoroutineStatus status;
    ptrdiff_t stack_capacity;
    ptrdiff_t stack_size;
    char *stack;
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
