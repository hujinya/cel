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
#include "cel/allocator.h"
#include "cel/log.h"
#ifdef _CEL_UNIX

OsCoroutineEntityItem *_coroutineentityitem_new(OsCoroutineScheduler *schd,
                                                OsCoroutineFunc func,
                                                void *user_data)
{
    OsCoroutineEntityItem *coe_item;
    OsCoroutineEntity *coe;

    if ((coe_item = (OsCoroutineEntityItem *)
        cel_malloc(sizeof(OsCoroutineEntityItem))) != NULL)
    {
        coe = &(coe_item->coe);
        coe->id = 0;
        coe->func = func;
        coe->user_data = user_data;
        coe->schd = schd;
        coe->status = CEL_COROUTINE_READY;
        coe->private_stack_capacity = 0;
        coe->private_stack_size = 0;
        coe->private_stack = NULL;
        return coe_item;
    }
    return NULL;
}

void _coroutineentityitem_free(OsCoroutineEntityItem *coe_item) 
{
    //_tprintf(_T("co %d free.\r\n"), coe_item->coe.id);
    if (coe_item->coe.private_stack != NULL)
        cel_free(coe_item->coe.private_stack);
    cel_free(coe_item);
}

OsCoroutineScheduler *os_coroutinescheduler_new(void) 
{
    OsCoroutineScheduler *schd;

    if ((schd = (OsCoroutineScheduler *)
        cel_malloc(sizeof(OsCoroutineScheduler))) != NULL)
    {
        schd->coe_running = NULL;
        schd->coe_id = 0;
        cel_list_init(&(schd->coe_items), 
            (CelFreeFunc)_coroutineentityitem_free);
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
        (*coe)->id = cel_atomic_increment(&(schd->coe_id), 1);
        return (*coe)->id;
    }
    return -1;
}

static void main_ctxfunc(uint32_t low32, uint32_t hi32) 
{
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
    OsCoroutineScheduler *schd = (OsCoroutineScheduler *)ptr;
    OsCoroutineEntity *coe = schd->coe_running;
    OsCoroutineEntityItem *coe_item = 
        (OsCoroutineEntityItem *)((char *)coe - sizeof(CelListItem));

    //_tprintf(_T("co %d enter.\r\n"), coe->id);
    coe->func(coe->user_data);
    //_tprintf(_T("co %d exit.\r\n"), coe->id);
    cel_list_remove(&(schd->coe_items), (CelListItem *)coe_item);
    _coroutineentityitem_free(coe_item);
    schd->coe_running = NULL;
}

void os_coroutineentity_resume(OsCoroutineEntity *coe)
{
    OsCoroutineScheduler *schd = coe->schd;
    uintptr_t ptr;
    int status = coe->status;

    switch(status) 
    {
    case CEL_COROUTINE_READY:
        getcontext(&coe->ctx);
        coe->ctx.uc_stack.ss_sp = schd->stack[coe->id % 128];
        coe->ctx.uc_stack.ss_size = CEL_COROUTINE_STACK_SIZE;
        coe->ctx.uc_link = &schd->main_ctx;
        schd->coe_running = coe;
        coe->status = CEL_COROUTINE_RUNNING;
        ptr = (uintptr_t)schd;
        makecontext(&coe->ctx, (void (*)(void))main_ctxfunc, 
            2, (uint32_t)ptr, (uint32_t)(ptr>>32));
        swapcontext(&schd->main_ctx, &coe->ctx);
        break;
    case CEL_COROUTINE_SUSPEND:
        memcpy(coe->ctx.uc_stack.ss_sp 
        + CEL_COROUTINE_STACK_SIZE - coe->private_stack_size, 
            coe->private_stack, coe->private_stack_size);
        //_tprintf(_T("co pop stack size %ld\r\n"), coe->private_stack_size);
        schd->coe_running = coe;
        coe->status = CEL_COROUTINE_RUNNING;
        swapcontext(&schd->main_ctx, &coe->ctx);
        break;
    default:
        break;
        //assert(0);
    }
}

static void _coroutineentity_save_stack(OsCoroutineEntity *coe, char *top)
{
    char dummy = 0;

    CEL_ASSERT(top - &dummy <= CEL_COROUTINE_STACK_SIZE);
    if (coe->private_stack_capacity < top - &dummy) 
    {
        free(coe->private_stack);
        coe->private_stack_capacity = top - &dummy;
        coe->private_stack = (char *)cel_malloc(coe->private_stack_capacity);
    }
    coe->private_stack_size = top - &dummy;
    memcpy(coe->private_stack, &dummy, coe->private_stack_size);
    //_tprintf(_T("co push stack size %ld\r\n"), coe->private_stack_size);
}

void os_coroutineentity_yield(OsCoroutineEntity *old_coe,
                              OsCoroutineEntity *new_coe)
{
    OsCoroutineScheduler *schd = old_coe->schd;

    _coroutineentity_save_stack(old_coe, 
        old_coe->ctx.uc_stack.ss_sp + CEL_COROUTINE_STACK_SIZE);
    old_coe->status = CEL_COROUTINE_SUSPEND;
    schd->coe_running = NULL;
    if (new_coe == NULL)
        swapcontext(&old_coe->ctx , &schd->main_ctx);
    else
        os_coroutineentity_resume(new_coe);
}

#endif
