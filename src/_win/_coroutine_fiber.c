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
#include "cel/allocator.h"
#ifdef _CEL_WIN

static void _coroutineentityitem_free(OsCoroutineEntityItem *coe_item)
{
    // If the currently running fiber calls DeleteFiber, 
    // its thread calls ExitThread and terminates.  
    // However, if a currently running fiber is deleted by another fiber,
    // the thread running the   
    // deleted fiber is likely to terminate abnormally 
    // because the fiber stack has been freed.
    //puts("_coroutineentityitem_free");
    DeleteFiber(coe_item->coe.ctx);
    cel_free(coe_item);
}

void __stdcall _os_coroutineentity_main(OsCoroutineScheduler *schd)
{
    OsCoroutineEntity *coe = schd->coe_running;
    OsCoroutineEntityItem *coe_item = 
        (OsCoroutineEntityItem *)((char *)coe - sizeof(CelListItem));

    //_tprintf(_T("co %d enter.\r\n"), coe->id);
    (coe->func)(coe->user_data);
    //_tprintf(_T("co %d exit.\r\n"), coe->id);
    schd->coe_running = NULL;
    cel_list_remove(&(schd->coe_items), (CelListItem *)coe_item);
    coe->status = CEL_COROUTINE_DEAD;
    //_coroutineentityitem_free(coe_item);
    SwitchToFiber(schd->main_ctx);
}

OsCoroutineEntityItem *_coroutineentityitem_new(OsCoroutineScheduler *schd, 
                                                OsCoroutineFunc func, void *user_data) 
{
    OsCoroutineEntityItem *coe_item;
    OsCoroutineEntity *coe;

    if ((coe_item = cel_malloc(sizeof(OsCoroutineEntityItem))) != NULL)
    {
        coe = &(coe_item->coe);
        coe->schd = schd;
        coe->status = CEL_COROUTINE_READY;
        coe->func = func;
        coe->user_data = user_data;
        coe->ctx = CreateFiberEx(CEL_COROUTINE_STACK_SIZE, 0, 
            FIBER_FLAG_FLOAT_SWITCH, _os_coroutineentity_main, schd);
        coe->id = -1;
        coe->status = CEL_COROUTINE_READY;
        return coe_item;
    }
    return NULL;
}

OsCoroutineScheduler *os_coroutinescheduler_new()
{
    OsCoroutineScheduler *schd;

    if ((schd = cel_malloc(sizeof(OsCoroutineScheduler))) != NULL)
    {
        schd->coe_running = NULL;
        schd->coe_id = 0;
        schd->main_ctx = 
            ConvertThreadToFiberEx(NULL, FIBER_FLAG_FLOAT_SWITCH);
        cel_list_init(&(schd->coe_items), _coroutineentityitem_free);
        cel_asyncqueue_init(&(schd->coe_readies), NULL);
        
        return schd;
    }
    return NULL;
}

void os_coroutinescheduler_free(OsCoroutineScheduler *schd)
{
    cel_asyncqueue_destroy(&(schd->coe_readies));
    cel_list_destroy(&(schd->coe_items));
    cel_free(schd);
}

int os_coroutineentity_create(OsCoroutineEntity **coe, 
                              OsCoroutineScheduler *schd,
                              OsCoroutineAttr *attr,
                              OsCoroutineFunc func, void *user_data)
{
    OsCoroutineEntityItem *coe_item;

    if ((coe_item = _coroutineentityitem_new(schd, func, user_data)) != NULL)
    {
        cel_list_push_back(&(schd->coe_items), (CelListItem *)coe_item);
        (*coe) = &(coe_item->coe);
        (*coe)->id = (int)cel_atomic_increment(&(schd->coe_id), 1);
        return (*coe)->id;
    }
    return -1;
}

void os_coroutineentity_resume(OsCoroutineEntity *coe)
{
    OsCoroutineScheduler *schd = coe->schd;
    OsCoroutineEntityItem *coe_item;

    switch (coe->status)
    {  
    case CEL_COROUTINE_READY:
    case CEL_COROUTINE_SUSPEND:
        coe->status = CEL_COROUTINE_RUNNING;
        schd->coe_running = coe;
        SwitchToFiber(coe->ctx);
        if (coe->status == CEL_COROUTINE_DEAD)
        {
            coe_item =
                (OsCoroutineEntityItem *)((char *)coe - sizeof(CelListItem));
            _coroutineentityitem_free(coe_item);
        }
        break;
    default:
        //assert(0);
        break;
    }
}

void os_coroutineentity_yield(OsCoroutineEntity *old_coe, 
                              OsCoroutineEntity *new_coe)
{
    OsCoroutineScheduler *schd = old_coe->schd;

    old_coe->status = CEL_COROUTINE_SUSPEND;
    schd->coe_running = NULL;
    if (new_coe == NULL)
        SwitchToFiber(schd->main_ctx);
    else
        os_coroutineentity_resume(new_coe);
}

#endif
