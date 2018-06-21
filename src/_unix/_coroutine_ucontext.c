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
#include "cel/multithread.h"
#ifdef _CEL_UNIX

OsCoroutineEntity *_coroutineentity_new(OsCoroutineScheduler *schd, 
                                        OsCoroutineFunc func, void *user_data) 
{
    OsCoroutineEntity *co_entity;

    if ((co_entity = (OsCoroutineEntity *)
        cel_malloc(sizeof(OsCoroutineEntity))) != NULL)
    {
        co_entity->id = 0;
        co_entity->func = func;
        co_entity->user_data = user_data;
        co_entity->schd = schd;
        co_entity->stack_capacity = 0;
        co_entity->stack_size = 0;
        co_entity->status = CEL_COROUTINE_READY;
        co_entity->stack = NULL;
        return co_entity;
    }
    return NULL;
}

void _coroutineentity_free(OsCoroutineEntity *co_entity) 
{
    cel_free(co_entity->stack);
    cel_free(co_entity);
}

OsCoroutineScheduler *os_coroutinescheduler_new(void) 
{
    OsCoroutineScheduler *schd;

    if ((schd = (OsCoroutineScheduler *)
        cel_malloc(sizeof(OsCoroutineScheduler))) != NULL)
    {
        schd->co_num = 0;
        schd->co_capacity = CEL_COROUTINE_CAP;
        schd->co_running = -1;
        if ((schd->co_entitys = (OsCoroutineEntity **)cel_calloc(1, 
            sizeof(OsCoroutineEntity *) * schd->co_capacity)) != NULL)
        {
            return schd;
        }
        cel_free(schd);
    }
    return NULL;
}

void os_coroutinescheduler_free(OsCoroutineScheduler *schd) 
{
    int i;
    OsCoroutineEntity *co_entity;

    for (i = 0;i < schd->co_capacity;i++) 
    {
        co_entity = schd->co_entitys[i];
        if (co_entity != NULL) 
            _coroutineentity_free(co_entity);
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
    if (((*co_entity) = _coroutineentity_new(schd, func , user_data)) != NULL)
    {
        if (((*co_entity)->id = 
            os_coroutinescheduler_add(schd, (*co_entity))) != -1)
            return (*co_entity)->id;
        _coroutineentity_free(*co_entity);
    }
    return -1;
}

static void main_ctxfunc(uint32_t low32, uint32_t hi32) 
{
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
    OsCoroutineScheduler *schd = (OsCoroutineScheduler *)ptr;
    int co_id = schd->co_running;
    OsCoroutineEntity *co_entity = schd->co_entitys[co_id];

    //_tprintf(_T("co %d enter.\r\n"), co_entity->id);
    co_entity->func(co_entity->user_data);
    //_tprintf(_T("co %d exit.\r\n"), co_entity->id);
    _coroutineentity_free(co_entity);
    co_entity->status = CEL_COROUTINE_DEAD;
    schd->co_entitys[co_id] = NULL;
    --schd->co_num;
    schd->co_running = -1;
}

void os_coroutineentity_resume(OsCoroutineEntity *co_entity) 
{
    OsCoroutineScheduler *schd = co_entity->schd;
    uintptr_t ptr;
    int status = co_entity->status;

    switch(status) 
    {
    case CEL_COROUTINE_READY:
        getcontext(&co_entity->ctx);
        co_entity->ctx.uc_stack.ss_sp = schd->stack;
        co_entity->ctx.uc_stack.ss_size = CEL_COROUTINE_STACK_SIZE;
        co_entity->ctx.uc_link = &schd->main_ctx;
        schd->co_running = co_entity->id;
        co_entity->status = CEL_COROUTINE_RUNNING;
        ptr = (uintptr_t)schd;
        makecontext(&co_entity->ctx, (void (*)(void))main_ctxfunc, 
            2, (uint32_t)ptr, (uint32_t)(ptr>>32));
        swapcontext(&schd->main_ctx, &co_entity->ctx);
        break;
    case CEL_COROUTINE_SUSPEND:
        memcpy(schd->stack + CEL_COROUTINE_STACK_SIZE - co_entity->stack_size, 
            co_entity->stack, co_entity->stack_size);
        schd->co_running = co_entity->id;
        co_entity->status = CEL_COROUTINE_RUNNING;
        swapcontext(&schd->main_ctx, &co_entity->ctx);
        break;
    default:
        break;
        //assert(0);
    }
}

static void _coroutineentity_save_stack(OsCoroutineEntity *co_entity, char *top)
{
    char dummy = 0;

    //assert(top - &dummy <= CEL_COROUTINE_STACK_SIZE);
    if (co_entity->stack_capacity < top - &dummy) 
    {
        free(co_entity->stack);
        co_entity->stack_capacity = top - &dummy;
        co_entity->stack = (char *)cel_malloc(co_entity->stack_capacity);
    }
    co_entity->stack_size = top - &dummy;
    memcpy(co_entity->stack, &dummy, co_entity->stack_size);
    //_tprintf(_T("co stack size %ld\r\n"), co_entity->stack_size);
}

void os_coroutineentity_yield(OsCoroutineEntity *co_entity)
{
    OsCoroutineScheduler *schd = co_entity->schd;

    _coroutineentity_save_stack(co_entity, schd->stack + CEL_COROUTINE_STACK_SIZE);
    co_entity->status = CEL_COROUTINE_SUSPEND;
    schd->co_running = -1;
    swapcontext(&co_entity->ctx , &schd->main_ctx);
}

#endif
