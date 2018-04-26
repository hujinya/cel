/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __OS_CEL_COROUTINE_UNIX_H__
#define __OS_CEL_COROUTINE_UNIX_H__

#include "cel/config.h"
#include "cel/types.h"
#include <stddef.h> /* ptrdiff_t */
#if __APPLE__ && __MACH__
#include <sys/ucontext.h>
#else 
#include <ucontext.h>
#endif 

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _OsCoroutine OsCoroutine;

typedef struct _OsCoroutineScheduler
{
    char stack[CEL_COROUTINE_STACK_SIZE];
    ucontext_t main_ctx;
    int co_running;
    int co_num;
    int co_capacity;
    OsCoroutine **co;
}OsCoroutineScheduler;

typedef struct _OsCoroutineAttr
{
    int stack_size;
}OsCoroutineAttr;

typedef void (* OsCoroutineFunc)(void *user_data);

struct _OsCoroutine 
{
    int id;
    OsCoroutineFunc func;
    void *user_data;
    ucontext_t ctx;
    OsCoroutineScheduler *schd;
    CelCoroutineStatus status;
    ptrdiff_t stack_capacity;
    ptrdiff_t stack_size;
    char *stack;
};

OsCoroutineScheduler *os_coroutinescheduler_new(void);
void os_coroutinescheduler_free(OsCoroutineScheduler *schd);

static __inline 
int os_coroutinescheduler_running_id(OsCoroutineScheduler *schd)
{
    return schd->co_running;
}

static __inline 
OsCoroutine *os_coroutinescheduler_running(OsCoroutineScheduler *schd)
{
    return schd->co[schd->co_running];
}

int os_coroutine_create(OsCoroutine *co, OsCoroutineScheduler *schd, 
                        OsCoroutineAttr *attr,
                        OsCoroutineFunc func, void *user_data);
void os_coroutine_resume(OsCoroutine *co);
void os_coroutine_yield(OsCoroutine *co);
static __inline CelCoroutineStatus os_coroutine_status(OsCoroutine *co) 
{
    return co->status;
}

#ifdef __cplusplus
}
#endif

#endif
