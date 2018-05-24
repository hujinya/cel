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
#ifndef __OS_CEL_COROUTINE_FIBER_H__
#define __OS_CEL_COROUTINE_FIBER_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _OsCoroutine OsCoroutine;

typedef struct _OsCoroutineScheduler
{
    int co_capacity;
    int co_num;
    int co_running;
    LPVOID main_ctx;
    OsCoroutine **co;
}OsCoroutineScheduler;

typedef void (* OsCoroutineFunc)(void *user_data);

typedef struct _OsCoroutineAttr
{
    int stack_size;
}OsCoroutineAttr;

struct _OsCoroutine
{  
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
OsCoroutine *os_coroutinescheduler_running(OsCoroutineScheduler *schd)
{
    return schd->co[schd->co_running];
}

int os_coroutine_create(OsCoroutine *co, OsCoroutineScheduler *schd,
                        OsCoroutineAttr *attr,
                        OsCoroutineFunc func, void *user_data);
void os_coroutine_resume(OsCoroutine *co);
void os_coroutine_yield(OsCoroutine *co);
static __inline CelCoroutineStatus os_coroutine_status(OsCoroutine *co) 
{
    return co->status;
}

#ifdef __cplusplus
}
#endif

#endif
