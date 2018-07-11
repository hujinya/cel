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

typedef int CelCoroutine;
typedef OsCoroutineAttr CelCoroutineAttr;
typedef OsCoroutineEntity CelCoroutineEntity;
typedef OsCoroutineScheduler CelCoroutineScheduler;

typedef OsCoroutineFunc CelCoroutineFunc;

#define cel_coroutinescheduler_new os_coroutinescheduler_new
#define cel_coroutinescheduler_free os_coroutinescheduler_free

CelCoroutineScheduler *_cel_coroutinescheduler_get();
#define cel_coroutinescheduler_running_id os_coroutinescheduler_running_id
#define cel_coroutinescheduler_running os_coroutinescheduler_running

static __inline 
int cel_coroutine_create(CelCoroutine *co, 
                         CelCoroutineAttr *attr, 
                         CelCoroutineFunc func, void *user_data)
{
    CelCoroutineEntity *co_entity;
    return ((*co = os_coroutineentity_create(&co_entity,
        _cel_coroutinescheduler_get(), attr, func, user_data)) == -1 ? -1 : 0);
}

static __inline void cel_coroutine_resume(CelCoroutine *co)
{
    CelCoroutineEntity *co_entity;
    if ((co_entity = _cel_coroutinescheduler_get()->co_entitys[*co]) != NULL)
        os_coroutineentity_resume(co_entity);
}
static __inline void cel_coroutine_yield(CelCoroutine *co)
{
    CelCoroutineEntity *co_entity;
    if ((co_entity = _cel_coroutinescheduler_get()->co_entitys[*co]) != NULL)
        os_coroutineentity_yield(co_entity);
}
static __inline CelCoroutineStatus cel_coroutine_status(CelCoroutine *co)
{
    CelCoroutineEntity *co_entity;
    if ((co_entity = _cel_coroutinescheduler_get()->co_entitys[*co]) != NULL)
        return os_coroutineentity_status(co_entity);
    return CEL_COROUTINE_DEAD;
}
#define cel_coroutine_self() \
    &(cel_coroutinescheduler_running(_cel_coroutinescheduler_get())->id)
#define cel_coroutine_getid() \
    cel_coroutinescheduler_running_id(_cel_coroutinescheduler_get())

#endif
