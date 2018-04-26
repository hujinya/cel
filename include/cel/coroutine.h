/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_CEL_COROUTINE_H__
#define __CEL_CEL_COROUTINE_H__

#include "cel/config.h"

#define CEL_COROUTINE_STACK_SIZE (1024 * 1024)
#define CEL_COROUTINE_CAP         16

typedef enum _CelCoroutineStatus
{
    CEL_COROUTINE_DEAD = 0,
    CEL_COROUTINE_READY = 1,
    CEL_COROUTINE_RUNNING = 2,
    CEL_COROUTINE_SUSPEND = 3
}OsCoroutineStatus, CelCoroutineStatus;

#if defined(_CEL_UNIX)
#include "cel/_unix/_coroutine_ucontext.h"
#elif defined(_CEL_WIN)
#include "cel/_win/_coroutine_fiber.h"
#endif

typedef OsCoroutine CelCoroutine;
typedef OsCoroutineScheduler CelCoroutineScheduler;
typedef OsCoroutineFunc CelCoroutineFunc;

#define cel_coroutinescheduler_new os_coroutinescheduler_new
#define cel_coroutinescheduler_free os_coroutinescheduler_free

CelCoroutineScheduler *_cel_coroutinescheduler_get();
#define cel_coroutinescheduler_running_id os_coroutinescheduler_running_id
#define cel_coroutinescheduler_running os_coroutinescheduler_running

#define cel_coroutine_create(co, attr, func, user_data) \
    os_coroutine_create(co, \
    _cel_coroutinescheduler_get(), attr, func, user_data)
#define cel_coroutine_resume os_coroutine_resume
#define cel_coroutine_yield os_coroutine_yield
#define cel_coroutine_status os_coroutine_status
#define cel_coroutine_self() \
    cel_coroutinescheduler_running(_cel_coroutinescheduler_get())
#define cel_coroutine_getid() \
    cel_coroutinescheduler_running_id(_cel_coroutinescheduler_get())

#endif
