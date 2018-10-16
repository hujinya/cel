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
#ifndef __CEL_POSIX_THREAD_H__
#define __CEL_POSIX_THREAD_H__

#include "cel/types.h"
#include "cel/sys/envinfo.h"
#include <sys/time.h>
#include <sys/syscall.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define gettid()  syscall(__NR_gettid)

typedef pthread_attr_t OsThreadAttr;
typedef pthread_t OsThread;
typedef pid_t OsThreadId;

#define os_thread_attr_init(attr) pthread_attr_init(attr)
#define os_thread_attr_setstacksize(attr, stack) \
    pthread_attr_setstacksize(attr, stack)
#define os_thread_attr_setprio(attr, priority) \
    pthread_attr_setprio(attr, priority)
#define os_thread_attr_destroy(attr) pthread_attr_destroy(attr)

#define os_thread_create(tidp, attr, start_rtn, arg) \
    pthread_create(tidp, attr, (void * (*)(void *))start_rtn, arg)
int os_thread_setaffinity(OsThread *tidp, CelCpuMask *affinity_mask);
#define os_thread_self() pthread_self()
#define os_thread_getid() gettid()
#define os_thread_exit(status) pthread_exit((void *)status)
#define os_thread_join(thread, status) pthread_join(thread, (void **)status)

#define OS_THREAD_ONCE_INIT     PTHREAD_ONCE_INIT

typedef pthread_key_t OsThreadKey;
typedef pthread_once_t OsThreadOnce;

/* int os_thread_key_create(OsThreadKey *key, void (*destructor) (void *)); */
#define os_thread_key_create(key, destructor) pthread_key_create(key, destructor)
/* int os_thread_key_delete(OsThreadKey key);*/
#define os_thread_key_delete(key) pthread_key_delete(key)
/* void *cel_thread_getspecific(OsThreadKey key);*/
#define os_thread_getspecific(key) pthread_getspecific(key)
/* int os_thread_setspecific(OsThreadKey key, void *value);*/
#define os_thread_setspecific(key, value) pthread_setspecific(key, value)
/* int os_thread_once(OsThreadOnce *once_control, void (*init_routine) (void));*/
#define os_thread_once(once_ctl, init_routine) \
    pthread_once(once_ctl, init_routine)

typedef pthread_mutexattr_t OsMutexAttr;
typedef pthread_mutex_t OsMutex;

#define os_mutex_init(mutex, attr) pthread_mutex_init(mutex, attr)
#define os_mutex_destroy(mutex) pthread_mutex_destroy(mutex)

#define os_mutex_lock(mutex) pthread_mutex_lock(mutex)
#define os_mutex_trylock(mutex) pthread_mutex_trylock(mutex)
#define os_mutex_unlock(mutex) pthread_mutex_unlock(mutex)

typedef pthread_condattr_t OsCondAttr;
typedef pthread_cond_t OsCond;

#define os_cond_init(cond, attr) pthread_cond_init(cond, attr)
#define os_cond_destroy(cond) pthread_cond_destroy(cond)

#define os_cond_wait(cond, mutex) pthread_cond_wait(cond, mutex)
int os_cond_timedwait(OsCond *cond, OsMutex *mutex, int milliseconds);
#define os_cond_signal(cond) pthread_cond_signal(cond)
#define os_cond_broadcast(cond) pthread_cond_broadcast(cond)

typedef pthread_spinlock_t OsSpinLock;

#define os_spinlock_init(spinlock, pshared) \
    pthread_spin_init(spinlock, pshared)
#define os_spinlock_destroy(spinlock) pthread_spin_destroy(spinlock)

#define os_spinlock_lock(spinlock) pthread_spin_lock(spinlock)
#define os_spinlock_trylock(spinlock) pthread_spin_trylock(spinlock)
#define os_spinlock_unlock(spinlock) pthread_spin_unlock(spinlock)

typedef pthread_rwlockattr_t OsRwlockAttr;
typedef pthread_rwlock_t OsRwlock;

#define os_rwlock_init(rwlock, attr)  pthread_rwlock_init(rwlock, attr)
#define os_rwlock_destroy(rwlock)  pthread_rwlock_destroy(rwlock)

#define os_rwlock_rdlock(rwlock)  pthread_rwlock_rdlock(rwlock)
#define os_rwlock_tryrdlock(rwlock)  pthread_rwlock_tryrdlock(rwlock)
#define os_rwlock_trywrlock(rwlock)  pthread_rwlock_trywrlock(rwlock)
#define os_rwlock_wrlock(rwlock)  pthread_rwlock_wrlock(rwlock)
#define os_rwlock_unlock(rwlock)  pthread_rwlock_unlock(rwlock)

#ifdef __cplusplus
}
#endif

#endif
