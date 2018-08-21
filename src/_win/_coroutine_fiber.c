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

static void _coroutineentity_free(OsCoroutineEntity *co_entity)
{
    // If the currently running fiber calls DeleteFiber, 
    // its thread calls ExitThread and terminates.  
    // However, if a currently running fiber is deleted by another fiber,
    // the thread running the   
    // deleted fiber is likely to terminate abnormally 
    // because the fiber stack has been freed.  
    DeleteFiber(co_entity->ctx);
    free(co_entity);
}

void __stdcall _os_coroutineentity_main(OsCoroutineScheduler *schd)
{
    OsCoroutineEntity *co_entity = schd->co_entitys[schd->co_running];

    //_tprintf(_T("co %d enter.\r\n"), co_entity->id);
    (co_entity->func)(co_entity->user_data);
    //_tprintf(_T("co %d exit.\r\n"), co_entity->id);
    schd->co_running = -1;
    --schd->co_num;
    schd->co_entitys[schd->co_running] = NULL;
    //_coroutineentity_free(co_entity);
    co_entity->status = CEL_COROUTINE_DEAD;
    SwitchToFiber(schd->main_ctx);
}

OsCoroutineEntity *_coroutine_new(OsCoroutineScheduler *schd, 
                            OsCoroutineFunc func, void *user_data) 
{
    OsCoroutineEntity *co_entity;

    if ((co_entity = cel_malloc(sizeof(OsCoroutineEntity))) != NULL)
    {
        co_entity->schd = schd;
        co_entity->status = CEL_COROUTINE_READY;
        co_entity->func = func;
        co_entity->user_data = user_data;
        co_entity->ctx = CreateFiberEx(CEL_COROUTINE_STACK_SIZE, 0, 
            FIBER_FLAG_FLOAT_SWITCH, _os_coroutineentity_main, schd);
        co_entity->id = -1;
        co_entity->status = CEL_COROUTINE_READY;
        return co_entity;
    }
    return NULL;
}

OsCoroutineScheduler *os_coroutinescheduler_new()
{
    OsCoroutineScheduler *schd;

    if ((schd = cel_malloc(sizeof(OsCoroutineScheduler))) != NULL)
    {
        schd->co_capacity = CEL_COROUTINE_CAP;
        schd->co_num = 0;
        schd->co_running = -1;
        cel_list_init(&(schd->ready_list), NULL);
        if ((schd->co_entitys = cel_malloc(
            sizeof(OsCoroutineEntity *) * schd->co_capacity)) != NULL)
        {
            memset(schd->co_entitys, 0, 
                sizeof(OsCoroutineEntity *) * schd->co_capacity);
            schd->main_ctx = 
                ConvertThreadToFiberEx(NULL, FIBER_FLAG_FLOAT_SWITCH);
            return schd;
        }
        cel_free(schd);
    }
    return NULL;
}

void os_coroutinescheduler_free(OsCoroutineScheduler *schd)
{
    int i;

    cel_list_destroy(&(schd->ready_list));
    for (i = 0; i < schd->co_capacity; i++)
    {  
        if (schd->co_entitys[i] != NULL) 
            _coroutineentity_free(schd->co_entitys[i]);
    }  
    cel_free(schd->co_entitys);
    schd->co_entitys = NULL;
    cel_free(schd);
}

int os_coroutinescheduler_add(OsCoroutineScheduler *schd, 
                              OsCoroutineEntity *co_entity)
{
    int i, co_id, new_cap;

    if (schd->co_num >= schd->co_capacity)
    {
        new_cap = schd->co_capacity * 2;
        if ((schd->co_entitys = (OsCoroutineEntity **)
            realloc(schd->co_entitys, 
            new_cap * sizeof(OsCoroutineEntity *))) == NULL)
        {
            return -1;
        }
        memset(schd->co_entitys + schd->co_capacity , 
            0 , sizeof(OsCoroutineEntity *) * schd->co_capacity);
        schd->co_capacity *= 2;
        co_id = schd->co_num;
        schd->co_entitys[co_id] = co_entity;
        ++schd->co_num;
         return co_id;
    } 
    else 
    {
        for (i = 0;i < schd->co_capacity;i++)
        {
            co_id = ((i + schd->co_num) % schd->co_capacity);
            if (schd->co_entitys[co_id] == NULL) 
            {
                schd->co_entitys[co_id] = co_entity;
                ++schd->co_num;
                return co_id;
            }
        }
    }
    return -1;
}

int os_coroutineentity_create(OsCoroutineEntity **co_entity, 
                              OsCoroutineScheduler *schd,
                              OsCoroutineAttr *attr,
                              OsCoroutineFunc func, void *user_data)
{
    if ((*co_entity = _coroutine_new(schd, func, user_data)) != NULL)
    {
        if (((*co_entity)->id = 
            os_coroutinescheduler_add(schd, *co_entity)) != -1)
            return (*co_entity)->id;
        _coroutineentity_free(*co_entity);
    }
    return -1;
}

void os_coroutineentity_resume(OsCoroutineEntity *co_entity)
{
    OsCoroutineScheduler *schd = co_entity->schd;

    switch (co_entity->status)
    {  
    case CEL_COROUTINE_READY:
    case CEL_COROUTINE_SUSPEND:
        co_entity->status = CEL_COROUTINE_RUNNING;
        schd->co_running = co_entity->id;
        SwitchToFiber(co_entity->ctx);
        if (schd->co_entitys[co_entity->id] == NULL)
            _coroutineentity_free(co_entity);
        break;
    default:
        //assert(0);
        break;
    }
}

void os_coroutineentity_yield(OsCoroutineEntity *co_entity)
{
    OsCoroutineScheduler *schd = co_entity->schd;

    co_entity->status = CEL_COROUTINE_SUSPEND;
    schd->co_running = -1;
    SwitchToFiber(schd->main_ctx);
}

#endif
