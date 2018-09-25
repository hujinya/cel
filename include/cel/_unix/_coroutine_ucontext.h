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

#include "cel/types.h"
#include "cel/atomic.h"
#include "cel/list.h"
#include "cel/thread.h"
#include "cel/asyncqueue.h"
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
    char stack[CEL_COROUTINE_GROUP_SIZE][CEL_COROUTINE_STACK_SIZE];
    ucontext_t main_ctx;
    OsCoroutineEntity *coe_running;
    CelAtomic coe_id;
    CelList coe_items;
    CelAsyncQueue coe_readies;
}OsCoroutineScheduler;

typedef struct _OsCoroutineAttr
{
    int stack_size;
}OsCoroutineAttr;

struct _OsCoroutineEntity
{
    int evt_type;
    int id;
    OsCoroutineFunc func;
    void *user_data;
    ucontext_t ctx;
    OsCoroutineScheduler *schd;
    CelCoroutineStatus status;
    ptrdiff_t private_stack_capacity;
    ptrdiff_t private_stack_size;
    char *private_stack;
};

typedef struct _OsCoroutineEntityItem
{
    CelListItem item;
    OsCoroutineEntity coe;
}OsCoroutineEntityItem;

OsCoroutineScheduler *os_coroutinescheduler_new(void);
void os_coroutinescheduler_free(OsCoroutineScheduler *schd);

int os_coroutineentity_create(OsCoroutineEntity **coe, 
                              OsCoroutineScheduler *schd, 
                              OsCoroutineAttr *attr,
                              OsCoroutineFunc func, void *user_data);
void os_coroutineentity_resume(OsCoroutineEntity *coe);
void os_coroutineentity_yield(OsCoroutineEntity *old_coe, 
                              OsCoroutineEntity *new_coe);

#ifdef __cplusplus
}
#endif

#endif
