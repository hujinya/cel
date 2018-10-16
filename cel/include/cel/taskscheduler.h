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
#ifndef __CEL_TASKSCHEDULER_H__
#define __CEL_TASKSCHEDULER_H__

#include "cel/types.h"
#include "cel/refcounted.h"
#include "cel/minheap.h"
#include "cel/datetime.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _CelTaskTriggerType
{
    CEL_TASKTRIGGER_UNDEFINED,
    CEL_TASKTRIGGER_ONLYONCE,
    CEL_TASKTRIGGER_EVERYDAY,
    CEL_TASKTRIGGER_WEEKLY,
    CEL_TASKTRIGGER_PERMONTH
}CelTaskTriggerType;

typedef struct _CelTaskTrigger
{
    CelTaskTriggerType type;
    BYTE hour;      /**< hours since midnight - [0,23] */
    BYTE min;       /**< minutes after the hour - [0,59] */
    BYTE sec;       /**< seconds after the minute - [0,59] */
    unsigned int mdays;      /**< day of the month - [1,31] */
    unsigned int wdays;      /**< days since Sunday - [0,6] */
    unsigned int months;     /**< months since January - [0,11] */
}CelTaskTrigger;

typedef void * CelTaskId;
typedef struct _CelTask CelTask;
typedef int (* CelTaskFunc) (CelTask *task, CelDateTime *now, void *user_data);

struct _CelTask
{
    CelTaskId id;
    BOOL enable;
    CelDateTime start_dt;
    CelDateTime end_dt;
    CelTaskTrigger trigger;
    CelTaskFunc task_func;
    void *user_data;
    CelRefCounted ref_counted; 
};

typedef struct _CelTaskScheduler
{
    struct tm tms;
    CelMinHeap tasks[2];
    CelMinHeap *expired_tasks, *unexpired_tasks;
}CelTaskScheduler;

int cel_task_init(CelTask *task, 
                  CelTaskTriggerType type, 
                  BYTE hour, BYTE min, BYTE sec,
                  unsigned int mdays, unsigned int wdays, unsigned int months,
                  CelTaskFunc task_func, void *user_data);
void cel_task_destroy(CelTask *task);

CelTask *cel_task_new(CelTaskTriggerType type, 
                      BYTE hour, BYTE min, BYTE sec,
                      unsigned int mdays, unsigned int wdays, 
                      unsigned int months,
                      CelTaskFunc task_func, void *user_data);
void cel_task_free(CelTask *task);

#define cel_task_ref(task) cel_refcounted_ref_ptr(&(task->ref_counted))
#define cel_task_deref(task) cel_refcounted_deref(&(task->ref_counted))

int cel_task_compare(CelTask *task2, CelTask *task1);
BOOL cel_task_is_expired(CelTask *task, struct tm *now);
int cel_task_start(CelTask *task, CelDateTime *dt, struct tm *now);

int cel_taskscheduler_init(CelTaskScheduler *scheduler, CelFreeFunc task_free);
void cel_taskscheduler_destroy(CelTaskScheduler *scheduler);

CelTaskScheduler *cel_taskscheduler_new(CelFreeFunc task_free);
void cel_taskscheduler_free(CelTaskScheduler *scheduler);

CelTaskId cel_taskscheduler_add_task(CelTaskScheduler *scheduler,
                                     CelTask *task);
void cel_taskscheduler_remove_task(CelTaskScheduler *scheduler, 
                                   CelTaskId task_id);

long cel_taskscheduler_expired_timeout(CelTaskScheduler *scheduler, 
                                       CelDateTime *dt);
int cel_taskscheduler_expired(CelTaskScheduler *scheduler);

#ifdef __cplusplus
}
#endif

#endif
