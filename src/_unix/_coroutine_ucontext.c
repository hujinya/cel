/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2016 Hu Jinya(hu_jinya@163.com) 
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

OsCoroutine *_coroutine_new(OsCoroutineScheduler *schd, 
                            OsCoroutineFunc func, void *user_data) 
{
    OsCoroutine *co;

    if ((co = (OsCoroutine *)cel_malloc(sizeof(OsCoroutine))) != NULL)
    {
        co->id = 0;
        co->func = func;
        co->user_data = user_data;
        co->schd = schd;
        co->stack_capacity = 0;
        co->stack_size = 0;
        co->status = CEL_COROUTINE_READY;
        co->stack = NULL;
        return co;
    }
    return NULL;
}

void _coroutine_free(OsCoroutine *co) 
{
    cel_free(co->stack);
    cel_free(co);
}

OsCoroutineScheduler * os_coroutinescheduler_new(void) 
{
    OsCoroutineScheduler *schd;

    if ((schd = (OsCoroutineScheduler *)
        cel_malloc(sizeof(OsCoroutineScheduler *))) != NULL)
    {
        schd->co_num = 0;
        schd->co_capacity = CEL_COROUTINE_CAP;
        schd->co_running = -1;
        if ((schd->co = (OsCoroutine **)
            cel_calloc(1, sizeof(OsCoroutine *) * schd->co_capacity)) != NULL)
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
    OsCoroutine *co;

    for (i = 0;i < schd->co_capacity;i++) 
    {
        co = schd->co[i];
        if (co != NULL) 
            _coroutine_free(co);
    }
    cel_free(schd->co);
    schd->co = NULL;
    cel_free(schd);
}

int os_coroutinescheduler_add(OsCoroutineScheduler *schd, OsCoroutine *co)
{
    int i, co_id, new_cap;

    if (schd->co_num >= schd->co_capacity)
    {
        new_cap = schd->co_capacity * 2;
        if ((schd->co = (OsCoroutine **)
            realloc(schd->co, new_cap * sizeof(OsCoroutine *))) == NULL)
        {
            return -1;
        }
        memset(schd->co + schd->co_capacity , 
            0 , sizeof(OsCoroutine *) * schd->co_capacity);
        schd->co_capacity *= 2;
        co_id = schd->co_num;
        schd->co[co_id] = co;
        ++schd->co_num;
         return co_id;
    } 
    else 
    {
        for (i = 0;i < schd->co_capacity;i++)
        {
            co_id = ((i + schd->co_num) % schd->co_capacity);
            if (schd->co[co_id] == NULL) 
            {
                schd->co[co_id] = co;
                ++schd->co_num;
                return co_id;
            }
        }
    }
    return -1;
}

int os_coroutine_create(OsCoroutine *co, 
                        OsCoroutineScheduler *schd,
                        OsCoroutineAttr *attr,
                        OsCoroutineFunc func, void *user_data) 
{
    if ((co = _coroutine_new(schd, func , user_data)) != NULL)
    {
        if ((co->id = os_coroutinescheduler_add(schd, co)) != -1)
            return co->id;
        _coroutine_free(co);
    }
    return -1;
}

static void main_ctxfunc(uint32_t low32, uint32_t hi32) 
{
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
    OsCoroutineScheduler *schd = (OsCoroutineScheduler *)ptr;
    int co_id = schd->co_running;
    OsCoroutine *co = schd->co[co_id];

    co->func(co->user_data);
    _coroutine_free(co);
    schd->co[co_id] = NULL;
    --schd->co_num;
    schd->co_running = -1;
}

void os_coroutine_resume(OsCoroutine *co) 
{
    OsCoroutineScheduler *schd = co->schd;
    uintptr_t ptr;
    int status = co->status;

    switch(status) 
    {
    case CEL_COROUTINE_READY:
        getcontext(&co->ctx);
        co->ctx.uc_stack.ss_sp = schd->stack;
        co->ctx.uc_stack.ss_size = CEL_COROUTINE_STACK_SIZE;
        co->ctx.uc_link = &schd->main_ctx;
        schd->co_running = co->id;
        co->status = CEL_COROUTINE_RUNNING;
        ptr = (uintptr_t)schd;
        makecontext(&co->ctx, (void (*)(void)) main_ctxfunc, 
            2, (uint32_t)ptr, (uint32_t)(ptr>>32));
        swapcontext(&schd->main_ctx, &co->ctx);
        break;
    case CEL_COROUTINE_SUSPEND:
        memcpy(schd->stack + CEL_COROUTINE_STACK_SIZE - co->stack_size, 
            co->stack, co->stack_size);
        schd->co_running = co->id;
        co->status = CEL_COROUTINE_RUNNING;
        swapcontext(&schd->main_ctx, &co->ctx);
        break;
    default:
        break;
        //assert(0);
    }
}

static void _coroutine_save_stack(OsCoroutine *co, char *top) 
{
    char dummy = 0;

    //assert(top - &dummy <= CEL_COROUTINE_STACK_SIZE);
    if (co->stack_capacity < top - &dummy) 
    {
        free(co->stack);
        co->stack_capacity = top - &dummy;
        co->stack = (char *)cel_malloc(co->stack_capacity);
    }
    co->stack_size = top - &dummy;
    memcpy(co->stack, &dummy, co->stack_size);
}

void os_coroutine_yield(OsCoroutine *co)
{
    OsCoroutineScheduler *schd = co->schd;

    _coroutine_save_stack(co, schd->stack + CEL_COROUTINE_STACK_SIZE);
    co->status = CEL_COROUTINE_SUSPEND;
    schd->co_running = -1;
    swapcontext(&co->ctx , &schd->main_ctx);
}

#endif
