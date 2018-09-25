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
#ifndef __CEL_COROUTINE_H__
#define __CEL_COROUTINE_H__

#include "cel/config.h"

#define CEL_COROUTINE_GROUP_SIZE  128
#define CEL_COROUTINE_STACK_SIZE (1024 * 1024)
#define CEL_COROUTINE_CAP         16

typedef enum _CelCoroutineStatus
{
    CEL_COROUTINE_DEAD = 0,
    CEL_COROUTINE_READY = 1,
    CEL_COROUTINE_RUNNING = 2,
    CEL_COROUTINE_SUSPEND = 3
}OsCoroutineStatus, CelCoroutineStatus;

typedef void (* OsCoroutineFunc)(void *user_data);

#if defined(_CEL_UNIX)
#include "cel/_unix/_coroutine_ucontext.h"
#elif defined(_CEL_WIN)
#include "cel/_win/_coroutine_fiber.h"
#endif

typedef OsCoroutineFunc CelCoroutineFunc;

typedef OsCoroutineAttr CelCoroutineAttr;
typedef OsCoroutineEntity CelCoroutineEntity;
typedef OsCoroutineScheduler CelCoroutineScheduler;

typedef OsCoroutineEntity* CelCoroutine;

#define cel_coroutinescheduler_new os_coroutinescheduler_new
#define cel_coroutinescheduler_free os_coroutinescheduler_free

CelCoroutineScheduler *_cel_coroutinescheduler_get();
static __inline 
int cel_coroutinescheduler_running_id(CelCoroutineScheduler *schd)
{
    return schd->coe_running->id;
}
static __inline 
CelCoroutineEntity *cel_coroutinescheduler_running(CelCoroutineScheduler *schd)
{
    return (schd == NULL ? NULL : schd->coe_running);
}
static __inline 
void cel_coroutinescheduler_readies_push(CelCoroutineScheduler *schd, 
                                         CelCoroutineEntity *co_entity)
{
    cel_asyncqueue_push(&(schd->coe_readies), co_entity);
}
static __inline 
CelCoroutineEntity *cel_coroutinescheduler_readies_try_pop(OsCoroutineScheduler *schd)
{
    return (OsCoroutineEntity *)cel_asyncqueue_try_pop(&(schd->coe_readies));
}
static __inline 
CelCoroutineEntity *cel_coroutinescheduler_readies_pop(OsCoroutineScheduler *schd)
{
    return (OsCoroutineEntity *)cel_asyncqueue_pop(&(schd->coe_readies));
}

#define cel_coroutineentity_resume os_coroutineentity_resume
#define cel_coroutineentity_yeild os_coroutineentity_yeild

static __inline 
int cel_coroutine_create(CelCoroutine *co, 
                         CelCoroutineAttr *attr, 
                         CelCoroutineFunc func, void *user_data)
{
    return (os_coroutineentity_create(co,
        _cel_coroutinescheduler_get(), attr, func, user_data) == -1 ? -1 : 0);
}

#define cel_coroutine_get_entity(co) (*co)

void cel_coroutine_resume(CelCoroutine *co);
void cel_coroutine_yield(CelCoroutine *co);
static __inline CelCoroutineStatus cel_coroutine_status(CelCoroutine *co)
{
    return (*co)->status;
}
/* CelCoroutine cel_coroutine_self() */
#define cel_coroutine_self() \
    cel_coroutinescheduler_running(_cel_coroutinescheduler_get())
/* int cel_coroutine_getid() */
#define cel_coroutine_getid() \
    cel_coroutinescheduler_running_id(_cel_coroutinescheduler_get())

#endif
