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
#include "cel/coroutine.h"
#include "cel/multithread.h"

CelCoroutineScheduler *_cel_coroutinescheduler_get()
{
    CelCoroutineScheduler *co_schd;

    if ((co_schd = (CelCoroutineScheduler *)
        cel_multithread_get_keyvalue(CEL_MT_KEY_COROUTINE)) == NULL)
    {
        if ((co_schd = cel_coroutinescheduler_new()) != NULL)
        {
            if (cel_multithread_set_keyvalue(
                CEL_MT_KEY_COROUTINE, co_schd) != -1
                && cel_multithread_set_keydestructor(
                CEL_MT_KEY_COROUTINE, 
                (CelFreeFunc)cel_coroutinescheduler_free) != -1)
                return co_schd;
            cel_coroutinescheduler_free(co_schd);
        }
        return NULL;
    }
    return co_schd;
}

void cel_coroutine_resume(CelCoroutine *co)
{
    CelCoroutineScheduler *schd = (*co)->schd;
    if (schd == _cel_coroutinescheduler_get())
    {
        os_coroutineentity_resume(*co);
        return ;
    }
    cel_spinlock_lock(&(schd->lock));
    cel_coroutinescheduler_readies_push(schd, *co);
    cel_spinlock_unlock(&(schd->lock));
}

void cel_coroutine_yield(CelCoroutine *co)
{
    CelCoroutineScheduler *schd = (*co)->schd;
    OsCoroutineEntity *new_coe = NULL;

    cel_spinlock_lock(&(schd->lock));
    if (cel_coroutinescheduler_readies_is_empty(schd) > 0)
        new_coe = (OsCoroutineEntity *)
        cel_coroutinescheduler_readies_pop(schd);
    cel_spinlock_unlock(&(schd->lock));
    os_coroutineentity_yield(*co, new_coe);
}
