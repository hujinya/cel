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
#include "cel/taskscheduler.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

void cel_task_destroy_derefed(CelTask *task)
{
    memset(task, 0, sizeof(CelTask));
    //puts("cel_task_destroy_derefed");
}

int cel_task_init(CelTask *task, 
                  CelTaskTriggerType type, 
                  BYTE hour, BYTE min, BYTE sec,
                  unsigned int mdays, unsigned int wdays, unsigned int months,
                  CelTaskFunc task_func, void *user_data)
{
    task->id = task;
    task->enable = TRUE;
    memset(&(task->start_dt), 0, sizeof(task->start_dt));
    memset(&(task->end_dt), 0, sizeof(task->end_dt));
    task->trigger.type = type;
    task->trigger.hour = hour;
    task->trigger.min = min;
    task->trigger.sec = sec;
    task->trigger.mdays = mdays;
    task->trigger.wdays = wdays;
    task->trigger.months = months;
    task->task_func = task_func;
    task->user_data = user_data;
    cel_refcounted_init(&(task->ref_counted),
        (CelFreeFunc)cel_task_destroy_derefed);

    return 0;
}

void cel_task_destroy(CelTask *task)
{
    cel_refcounted_destroy(&(task->ref_counted), task);
}

void cel_task_free_derefed(CelTask *task)
{
    cel_task_destroy_derefed(task);
    //puts("cel_task_free_derefed");
    cel_free(task);
}

CelTask *cel_task_new(CelTaskTriggerType type, 
                      BYTE hour, BYTE min, BYTE sec,
                      unsigned int mdays, unsigned int wdays,
                      unsigned int months,
                      CelTaskFunc task_func, void *user_data)
{
    CelTask *task;

    if ((task = (CelTask *)cel_malloc(sizeof(CelTask))) != NULL)
    {
        if (cel_task_init(task, type, hour, min, sec, mdays, wdays, months, 
            task_func, user_data) == 0)
        {
            cel_refcounted_init(
                &(task->ref_counted), (CelFreeFunc)cel_task_free_derefed);
            return task;
        }
        cel_free(task); 
    }
    return NULL;
}

void cel_task_free(CelTask *task)
{
    cel_refcounted_destroy(&(task->ref_counted), task);
}

int cel_task_compare(CelTask *task2, CelTask *task1)
{
    if (task1->trigger.hour < task2->trigger.hour)
        return 1;
    else if (task1->trigger.hour == task2->trigger.hour)
    {
        if (task1->trigger.min < task2->trigger.min)
            return 1;
        else if (task1->trigger.min == task2->trigger.min)
        {
            if (task1->trigger.sec < task2->trigger.sec)
                return 1;
            else if (task1->trigger.sec > task2->trigger.sec)
                return -1;
            else 
                return 0;
        }
        else
            return -1;
    }
    else
        return -1;
}

BOOL cel_task_is_expired(CelTask *task, struct tm *now)
{
    if (task->trigger.hour < now->tm_hour)
        return TRUE;
    else if (task->trigger.hour == now->tm_hour)
    {
        if (task->trigger.min < now->tm_min)
            return TRUE;
        else if (task->trigger.min == now->tm_min)
        {
            if (task->trigger.sec <= now->tm_sec)
                return TRUE;
            else
                return FALSE;
        }
        else
            return FALSE;
    }
    else
        return FALSE;
}

int cel_task_start(CelTask *task, CelDateTime *dt, struct tm *now)
{
    switch (task->trigger.type)
    {
    case CEL_TASKTRIGGER_ONLYONCE:
        return task->task_func(task, dt, task->user_data);
    case CEL_TASKTRIGGER_EVERYDAY:
        if ((task->trigger.mdays & (1 << now->tm_mday)) != 0)
            return task->task_func(task, dt, task->user_data);
    case CEL_TASKTRIGGER_WEEKLY:
        if ((task->trigger.wdays & (1 << now->tm_wday)) != 0)
            return task->task_func(task, dt, task->user_data);
    case CEL_TASKTRIGGER_PERMONTH:
        if ((task->trigger.months & (1 << now->tm_mon)) != 0
            && (task->trigger.mdays & (1 << now->tm_mday)) != 0)
            return task->task_func(task, dt, task->user_data);
    default:
        CEL_ERR((_T("Task trigger type %d undefined."), task->trigger.type));
        return -1;
    }

    return 0;
}

int cel_taskscheduler_init(CelTaskScheduler *scheduler, CelFreeFunc task_free)
{
    CelDateTime dt;

    cel_datetime_init_now(&dt);
    localtime_r(&dt, &(scheduler->tms));
    cel_minheap_init(
        &(scheduler->tasks[0]),(CelCompareFunc)cel_task_compare, task_free);
    cel_minheap_init(
        &(scheduler->tasks[1]), (CelCompareFunc)cel_task_compare, task_free);
    scheduler->expired_tasks = &(scheduler->tasks[0]);
    scheduler->unexpired_tasks = &(scheduler->tasks[1]);
    return 0;
}

void cel_taskscheduler_destroy(CelTaskScheduler *scheduler)
{
    cel_minheap_destroy(&(scheduler->tasks[0]));
    cel_minheap_destroy(&(scheduler->tasks[1]));
    scheduler->expired_tasks = NULL;
    scheduler->unexpired_tasks = NULL;
}

CelTaskScheduler *cel_taskscheduler_new(CelFreeFunc task_free)
{
    CelTaskScheduler *scheduler;

    if ((scheduler = 
        (CelTaskScheduler *)cel_malloc(sizeof(CelTaskScheduler))) != NULL)
    {
        if (cel_taskscheduler_init(scheduler, task_free) == 0)
        {
            return scheduler;
        }
        cel_free(scheduler); 
    }
    return NULL;
}

void cel_taskscheduler_free(CelTaskScheduler *scheduler)
{
    cel_taskscheduler_destroy(scheduler);
    cel_free(scheduler);
}

CelTaskId cel_taskscheduler_add_task(CelTaskScheduler *scheduler,
                                     CelTask *task)
{
    if (cel_task_is_expired(task, &(scheduler->tms)))
        cel_minheap_insert(scheduler->expired_tasks, task);
    else
        cel_minheap_insert(scheduler->unexpired_tasks, task);
    return task->id;
}

void cel_taskscheduler_remove_task(CelTaskScheduler *scheduler, 
                                   CelTaskId task_id)
{
    CelTask *task = (CelTask *)task_id;

    if (task->id == task_id)
        task->enable = FALSE;
}

static void cel_taskscheduler_switch(CelTaskScheduler *scheduler)
{
    CelMinHeap *tasks = scheduler->unexpired_tasks;

    scheduler->unexpired_tasks = scheduler->expired_tasks;
    scheduler->expired_tasks = tasks;
}

long cel_taskscheduler_expired_timeout(CelTaskScheduler *scheduler, 
                                       CelDateTime *dt)
{
    long diffmilliseconds;
    CelTask *task;

    //_tprintf(_T("cel_taskscheduler_expired_timeout is null **************"));
    if ((task = 
        (CelTask *)cel_minheap_get_min(scheduler->unexpired_tasks)) == NULL)
    {
        //_tprintf(_T("cel_minheap_get_min null"));
        return -1;
    }
    diffmilliseconds = 1000;
    //_tprintf(_T("diffmilliseconds = %ld \r\n"), diffmilliseconds);
    return diffmilliseconds < 0 ? 0 : diffmilliseconds;
}

int cel_taskscheduler_expired(CelTaskScheduler *scheduler)
{
    int ret = 0;
    int mday = scheduler->tms.tm_mday;
    CelFreeFunc free_func = scheduler->expired_tasks->array_list.free_func;
    CelTask *task;
    CelDateTime dt;

    cel_datetime_init_now(&dt);
    localtime_r(&dt, &(scheduler->tms));
    if (mday != scheduler->tms.tm_mday)
    {
        scheduler->tms.tm_hour = 23;
        scheduler->tms.tm_min = 59;
        scheduler->tms.tm_sec = 59;
    }
    while ((task = 
        (CelTask *)cel_minheap_get_min(scheduler->unexpired_tasks)) != NULL)
    {
        if (cel_task_is_expired(task, &(scheduler->tms))
            && (task = (CelTask *)cel_minheap_pop_min(
            scheduler->unexpired_tasks)) != NULL)
        {
            cel_task_start(task, &dt, &(scheduler->tms));
            if (task->enable)
                cel_minheap_insert(scheduler->expired_tasks, task);
            else if (free_func != NULL)
                free_func(task);
            ret++;
            continue;
        }
        break;
    }
    if (mday != scheduler->tms.tm_mday)
    {
        cel_taskscheduler_switch(scheduler);
    }    

    return ret;
}
