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
#ifndef __CEL_THREAD_H__
#define __CEL_THREAD_H__

#include "cel/config.h"
#if defined(_CEL_UNIX)
#include "cel/_unix/_thread_posix.h"
#elif defined(_CEL_WIN)
#include "cel/_win/_thread.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef OsThreadAttr CelThreadAttr;
typedef OsThread CelThread;
typedef OsThreadId CelThreadId;

/* int cel_thread_attr_init(CelThreadAttr *attr); */
#define cel_thread_attr_init os_thread_attr_init
/* int cel_thread_attr_setstacksize(CelThreadAttr *attr, DWORD stack);*/
#define cel_thread_attr_setstacksize os_thread_attr_setstacksize
/* int cel_thread_attr_setprio(CelThreadAttr *attr, int priority); */
#define cel_thread_attr_setprio os_thread_attr_setprio
/* int cel_thread_attr_destroy(CelThreadAttr *attr); */
#define cel_thread_attr_destroy os_thread_attr_destroy

/* 
int cel_thread_create(CelThread *tidp, const OsThreadAttr *attr, 
                      int (*start_rtn) (void *), void *arg); 
*/
#define cel_thread_create os_thread_create
/* int cel_thread_setaffinity(CelThread *tidp, OsCpuMask *affinity_mask); */
#define cel_thread_setaffinity os_thread_setaffinity
/*  OsThread cel_thread_self(void) */
#define cel_thread_self() os_thread_self()
/* */
#define cel_thread_getid() os_thread_getid()
/* void os_thread_exit(int status); */
#define cel_thread_exit os_thread_exit
/* int cel_thread_join(CelThread thread, void **status); */
#define cel_thread_join os_thread_join

#define CEL_THREAD_ONCE_INIT OS_THREAD_ONCE_INIT

typedef OsThreadKey CelThreadKey;
typedef OsThreadOnce CelThreadOnce;

/* int cel_thread_key_create(CelThreadKey *key, void (*destructor) (void *));*/
#define cel_thread_key_create os_thread_key_create
/* int cel_thread_key_delete(CelThreadKey key);*/
#define cel_thread_key_delete os_thread_key_delete
/* void *cel_thread_getspecific(CelThreadKey key);*/
#define cel_thread_getspecific os_thread_getspecific
/* int cel_thread_setspecific(CelThreadKey key, void *value);*/
#define cel_thread_setspecific os_thread_setspecific
/* 
int cel_thread_once(CelThreadOnce *once_control, void (*init_routine) (void));
*/
#define cel_thread_once os_thread_once

typedef OsMutexAttr CelMutexAttr;
typedef OsMutex CelMutex;

/* int cel_mutex_init(CelMutex *mutex, const OsMutexAttr *attr) */
#define cel_mutex_init os_mutex_init
/* void os_mutex_destroy(CelMutex *mutex); */
#define cel_mutex_destroy os_mutex_destroy

CelMutex *cel_mutex_new(CelMutexAttr *attr);
void cel_mutex_free(CelMutex *mutex);

/* void os_mutex_lock(CelMutex *mutex);*/
#define cel_mutex_lock os_mutex_lock
/* int cel_mutex_trylock(CelMutex *mutex); */
#define cel_mutex_trylock os_mutex_trylock
/* void os_mutex_unlock(CelMutex *mutex); */
#define cel_mutex_unlock os_mutex_unlock

typedef OsCondAttr CelCondAttr;
typedef OsCond CelCond;

/* int cel_cond_init(CelCond *cond, const OsCondAttr *attr); */
#define cel_cond_init os_cond_init
/* void os_cond_destroy(CelCond *cond); */
#define cel_cond_destroy os_cond_destroy

CelCond *cel_cond_new(CelCondAttr *attr);
void cel_cond_free(CelCond *cond);

/* int cel_cond_wait(CelCond *cond, OsMutex *mutex); */
#define cel_cond_wait os_cond_wait
/* int cel_cond_timedwait(CelCond *cond, OsMutex *mutex, int milliseconds); */
#define cel_cond_timedwait os_cond_timedwait
/* int cel_cond_signal(CelCond *cond); */
#define cel_cond_signal os_cond_signal
/* int cel_cond_broadcast(CelCond *cond); */
#define cel_cond_broadcast os_cond_broadcast

typedef OsSpinLock CelSpinLock;

/* int cel_spinlock_init(CelSpinLock *spinlock, int pshared); */
#define cel_spinlock_init os_spinlock_init
/* void os_spinlock_destroy(CelSpinLock *spinlock); */
#define cel_spinlock_destroy os_spinlock_destroy

CelSpinLock *cel_spinlock_new(int pshared);
void cel_spinlock_free(CelSpinLock *spinlock);

/* int cel_spinlock_init(CelSpinLock *spinlock, int pshared); */
#define cel_spinlock_lock os_spinlock_lock
/* void os_spinlock_destroy(CelSpinLock *spinlock); */
#define cel_spinlock_trylock os_spinlock_trylock
/* void os_spinlock_unlock(CelSpinLock *spinlock); */
#define cel_spinlock_unlock os_spinlock_unlock

typedef OsRwlockAttr CelRwlockAttr;
typedef OsRwlock CelRwlock;

/* int cel_rwlock_init(CelRwlock *rwlock, const OsRwlockAttr *attr); */
#define cel_rwlock_init  os_rwlock_init
/* void os_rwlock_destroy(CelRwlock *rwlock); */
#define cel_rwlock_destroy  os_rwlock_destroy

CelRwlock *cel_rwlock_new(CelRwlockAttr *attr);
void cel_rwlock_free(CelRwlock *rwlock);

/* int cel_rwlock_rdlock(CelRwlock *rwlock); */
#define cel_rwlock_rdlock os_rwlock_rdlock
/* int cel_rwlock_tryrdlock(CelRwlock *rwlock); */
#define cel_rwlock_tryrdlock os_rwlock_tryrdlock
/* int cel_rwlock_trywrlock(CelRwlock *rwlock); */
#define cel_rwlock_trywrlock os_rwlock_trywrlock
/* int cel_rwlock_wrlock(CelRwlock *rwlock); */
#define cel_rwlock_wrlock os_rwlock_wrlock
/* void os_rwlock_unlock(CelRwlock *rwlock); */
#define cel_rwlock_unlock os_rwlock_unlock

#ifdef __cplusplus
}
#endif

#endif
