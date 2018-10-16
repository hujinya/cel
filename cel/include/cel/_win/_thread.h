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
#ifndef __CEL_THREAD_WIN_H__
#define __CEL_THREAD_WIN_H__

#include "cel/types.h"
#include <process.h> /* _beginthread, _endthread */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _OsThreadAttr 
{   
    DWORD dwStackSize;
    DWORD dwCreatingFlag;
    int priority;
}OsThreadAttr;

int os_thread_attr_init(OsThreadAttr *attr);
int os_thread_attr_setstacksize(OsThreadAttr *attr, DWORD stack);
int os_thread_attr_setprio(OsThreadAttr *attr, int priority);
int os_thread_attr_destroy(OsThreadAttr *attr);

typedef HANDLE OsThread;
typedef DWORD OsThreadId;

int os_thread_create(OsThread *tidp, const OsThreadAttr *attr, 
                      int (*start_rtn) (void *), void *arg);
/* int os_thread_setaffinity(OsThread *tidp, OsCpuMask *affinity_mask); */
#define os_thread_setaffinity(tidp, affinity_mask) \
    (SetThreadAffinityMask(*tidp, *(affinity_mask)) == 0 ? -1 : 0)
/* 
 * OsThread cel_thread_self(void)
 */
#define os_thread_self()  GetCurrentThread()
#define os_thread_getid()  GetCurrentThreadId()
void os_thread_exit(int status);
int os_thread_join(OsThread thread, void **status);

#define OS_THREAD_ONCE_INIT   0

typedef DWORD OsThreadKey;
typedef LONG OsThreadOnce;

int os_thread_key_create(OsThreadKey *key, void (*destructor) (void *));
/* void os_thread_key_delete(OsThreadKey *key)*/
#define os_thread_key_delete(key) (TlsFree(key)? 0 : -1)
/* void *cel_thread_getspecific(OsThreadKey *key)*/
#define os_thread_getspecific(key) TlsGetValue(key)
/* int os_thread_setspecific(OsThreadKey *key, void *value)*/
#define os_thread_setspecific(key, value) (TlsSetValue(key, value) ? 0 : -1)

int os_thread_once(OsThreadOnce *once_ctl, void (*init_routine) (void));

typedef struct _OsMutexAttr{ int dummy; } OsMutexAttr;
#if 1
typedef CRITICAL_SECTION  OsMutex;

static __inline int os_mutex_init(OsMutex *mutex, const OsMutexAttr *attr)
{
    InitializeCriticalSection(mutex);
    return 0;
}
/*
 * void os_mutex_destroy(OsMutex *mutex);
 */
#define os_mutex_destroy(mutex) DeleteCriticalSection(mutex)

/* void os_mutex_lock(OsMutex *mutex);*/
#define os_mutex_lock(mutex) (EnterCriticalSection(mutex), 0)
/* int os_mutex_trylock(OsMutex *mutex);*/
#define os_mutex_trylock(mutex) (TryEnterCriticalSection(mutex)? 0 : -1)
/* void os_mutex_unlock(OsMutex *mutex);*/
#define os_mutex_unlock(mutex) LeaveCriticalSection(mutex)

#else
typedef HANDLE OsMutex;

static __inline int os_mutex_init(OsMutex *mutex, const OsMutexAttr *attr)
{
    return (((*mutex = CreateMutex(NULL, FALSE, NULL)) != NULL) ? 0 : -1);
}
/*
 * void os_mutex_destroy(OsMutex *mutex);
 */
#define os_mutex_destroy(mutex) CloseHandle(*mutex)

/* void os_mutex_lock(OsMutex *mutex);*/
#define os_mutex_lock(mutex) \
    (WaitForSingleObject(*mutex, INFINITE) != WAIT_OBJECT_0 ? -1 : 0)
/* int os_mutex_trylock(OsMutex *mutex);*/
#define os_mutex_trylock(mutex) \
    (WaitForSingleObject(*mutex, 0) != WAIT_OBJECT_0 ? -1 : 0)
/* void os_mutex_unlock(OsMutex *mutex);*/
#define os_mutex_unlock(mutex) ReleaseMutex(*mutex)
#endif

typedef struct _OsCondAttr{ int dummy; } OsCondAttr;
typedef struct _OsCond
{
    unsigned int waiting;
    OsMutex lock_waiting;
    enum {
        SIGNAL = 0, 
        BROADCAST = 1, 
        MAX_EVENTS = 2
    }EVENTS;
    HANDLE events[MAX_EVENTS];
    HANDLE block_event;
}OsCond;

int os_cond_init(OsCond *cond, const OsCondAttr *attr);
void os_cond_destroy(OsCond *cond);

/* int os_cond_wait(OsCond *cond, OsMutex *mutex);*/
#define os_cond_wait(cond, mutex) cel_cond_timedwait(cond, mutex, -1)
int os_cond_timedwait(OsCond *cond, OsMutex *mutex, int milliseconds);
int os_cond_signal(OsCond *cond);
int os_cond_broadcast(OsCond *cond);

typedef long OsSpinLock;

int os_spinlock_init(OsSpinLock *spinlock, int pshared);
void os_spinlock_destroy(OsSpinLock *spinlock);

void os_spinlock_lock(OsSpinLock *spinlock);
int os_spinlock_trylock(OsSpinLock *spinlock);
void os_spinlock_unlock(OsSpinLock *spinlock);


typedef struct _OsRwlockAttr{ int pshared; } OsRwlockAttr;
typedef struct _OsRwlock
{
    OsMutex lock;
    int state;        /* 0 = idle  >0 = # of readers  -1 = writer */
    OsCond  read_signal;
    OsCond  write_signal;
    int blocked_writers;
}OsRwlock;

int os_rwlock_init(OsRwlock *rwlock, const OsRwlockAttr *attr);
void os_rwlock_destroy(OsRwlock *rwlock);

int os_rwlock_rdlock(OsRwlock *rwlock);
int os_rwlock_tryrdlock(OsRwlock *rwlock);
int os_rwlock_trywrlock(OsRwlock *rwlock);
int os_rwlock_wrlock(OsRwlock *rwlock);
void os_rwlock_unlock(OsRwlock *rwlock);

#ifdef __cplusplus
}
#endif

#endif
