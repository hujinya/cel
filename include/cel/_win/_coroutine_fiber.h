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
#include "cel/atomic.h"
#include "cel/list.h"
#include "cel/thread.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _OsCoroutineEntity OsCoroutineEntity;

typedef struct _OsCoroutineScheduler
{
    LPVOID main_ctx;
    OsCoroutineEntity *co_running;
    CelAtomic co_id;
    CelList coes;
    CelList co_readies;
    CelSpinLock lock;
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
    OsCoroutineFunc func;
    void *user_data;
    OsCoroutineScheduler *schd;
    CelCoroutineStatus status;
    LPVOID ctx;
    
};

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
