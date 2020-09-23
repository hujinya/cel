/**
 * CEL(C Extension Library)
 * Copyright (C)2008 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/thread.h"
#include "cel/atomic.h"
#ifdef _CEL_WIN

static DWORD s_tlsminindex = TLS_MINIMUM_AVAILABLE;
static DWORD s_tlsmaxindex = 0;
static CelDestroyFunc s_tlsdestructor[TLS_MINIMUM_AVAILABLE] = { NULL , NULL};

int os_thread_attr_init(CelThreadAttr *attr)
{
    attr->dwStackSize = 0;
    attr->dwCreatingFlag = 0;
    attr->priority = 0;
    return 0;
}

int os_thread_attr_setstacksize(CelThreadAttr *attr, DWORD stack)
{
    attr->dwStackSize = stack;
    return 0;
}

int os_thread_attr_setprio(CelThreadAttr *attr, int priority)
{
    attr->priority = priority;
    return 0;
}

int os_thread_attr_destroy(CelThreadAttr *attr)
{
    memset(attr, 0, sizeof(attr));
    return 0;
}

int os_thread_create(CelThread *tidp, const CelThreadAttr *attr, 
                      int (*start_rtn) (void *), void *arg)
{
    HANDLE hthread;

    if (attr == NULL)
    {
        hthread = (HANDLE)_beginthreadex(
            NULL, 0, (unsigned int (__stdcall *)(void *))start_rtn, arg, 0, NULL);
    }
    else
    {
        hthread = (HANDLE)_beginthreadex(NULL, attr->dwStackSize, 
            (unsigned int (__stdcall *)(void *))start_rtn, arg, 0, NULL);
        SetThreadPriority(hthread, attr->priority);
    }
    *tidp = hthread;

    return (hthread != 0 ? 0 : -1);
}

void os_thread_exit(int status)
{
    DWORD dw = s_tlsminindex;
    CelDestroyFunc destructor;

    for (; dw <= s_tlsmaxindex; dw++)
    {
        if ((destructor = s_tlsdestructor[dw]) != NULL
            && TlsGetValue(dw) != NULL)
        {
            //_tprintf(_T("tls index %d, %p\r\n"), dw, destructor);
            destructor(TlsGetValue(dw));
        }
    }
    _endthreadex((unsigned int)status);
}

int os_thread_join(CelThread thread, void **status)
{
    DWORD exit_code;

    while (TRUE)
    {
        if (GetExitCodeThread(thread, &exit_code))
        {
            if (exit_code != STILL_ACTIVE)
            {
                *status = (void *)exit_code;
                CloseHandle(thread);
                return 0;
            }
            else
            {
                WaitForSingleObject(thread, INFINITE);
                continue;
            } 
        }
        //_tprintf(_T("errno = %ld\r\n"), GetLastError());
        *status = 0;
        return -1;
    }
}

int os_thread_key_create(CelThreadKey *key, void (*destructor) (void *))
{
    if ((*key = TlsAlloc()) == TLS_OUT_OF_INDEXES || (*key) >= TLS_MINIMUM_AVAILABLE)
        return -1;
    if (*key > s_tlsmaxindex) 
        s_tlsmaxindex = *key;
    if (*key < s_tlsminindex) 
        s_tlsminindex = *key;
    s_tlsdestructor[*key] = destructor;

    return 0;
}

int os_thread_once(CelThreadOnce *once_ctl, void (*init_routine) (void))
{
    if (*once_ctl == (CelThreadOnce)CEL_THREAD_ONCE_INIT)
    {
        while (TRUE)
        {
            switch (InterlockedCompareExchange(once_ctl, 2, 0))
            {
            case 0:
                init_routine();
                InterlockedExchange(once_ctl, 1);
                return 0;
            case 1:             // The initializer has already been executed
                return 0;
            default:            // The initializer is being processed by another thread
                SwitchToThread();
            }
        }
    }
    return 0;
}

int os_cond_init(CelCond *cond, const CelCondAttr *attr)
{
    cond->waiting = 0;
    os_mutex_init(&cond->lock_waiting, NULL);

    if ((cond->events[SIGNAL] = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
    {
        return -1;
    }
    /* Create a manual-reset aioevents. */  
    if ((cond->events[BROADCAST] = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
    {
        CloseHandle(cond->events[SIGNAL]);
        return -1;
    }
    if ((cond->block_event = CreateEvent(NULL, TRUE, TRUE, NULL)) == NULL)
    {
        CloseHandle(cond->events[SIGNAL]);
        CloseHandle(cond->events[BROADCAST]);
        return -1;
    }

    return 0;
}

void os_cond_destroy(CelCond *cond)
{
    os_mutex_destroy(&cond->lock_waiting);

    if (cond->events[SIGNAL] != NULL)
        CloseHandle(cond->events[SIGNAL]);
    if (cond->events[BROADCAST] != NULL)
        CloseHandle(cond->events[BROADCAST]);
    if (cond->block_event != NULL)
        CloseHandle(cond->block_event);
}

int os_cond_signal(CelCond *cond)
{
    BOOL result = FALSE;

    os_mutex_lock(&cond->lock_waiting);
    if (cond->waiting > 0)
        result = SetEvent(cond->events[SIGNAL]);
    os_mutex_unlock(&cond->lock_waiting);

    return (result ? 0 : -1);
}

int os_cond_broadcast(CelCond *cond)
{
    BOOL result = FALSE;

    os_mutex_lock(&cond->lock_waiting);
    /*  
     * The mutex protect us from broadcasting if there isn't any thread waiting to open
     * the block gate after this call has closed it.  
     */ 
    if (cond->waiting > 0)
    {
        /* Close block gate */  
        result = ResetEvent(cond->block_event);
        /* Open broadcast gate */  
        result = SetEvent(cond->events[BROADCAST]);
    }
    os_mutex_unlock(&cond->lock_waiting);

    return (result ? 0 : -1);
}

int os_cond_timedwait(CelCond *cond, CelMutex *mutex, int milliseconds)
{
    int result;
    DWORD dwMilliseconds = (milliseconds == -1 ? INFINITE : milliseconds);
    /*   
     * Block access if previous broadcast hasn't finished.This is just for safety and
     * should normally not affect the total time spent in this function.  
     */  
    WaitForSingleObject(cond->block_event, INFINITE);
    os_mutex_lock(&cond->lock_waiting);
    cond->waiting++;
    os_mutex_unlock(&cond->lock_waiting);
    os_mutex_unlock(mutex);
    result = WaitForMultipleObjects(2, cond->events, FALSE, dwMilliseconds);

    os_mutex_lock(&cond->lock_waiting);
    cond->waiting--;
    /* 
     * We're the last waiter to be notified or to stop waiting,
     * so reset the manual event.
     */
    if (cond->waiting == 0)
    {   
        /* Close broadcast gate */
        ResetEvent(cond->events[BROADCAST]);
        /* Open block gate */
        SetEvent(cond->block_event);
    }
    os_mutex_unlock(&cond->lock_waiting);
    os_mutex_lock(mutex);

    return (result == WAIT_TIMEOUT ? -1 : 0);
}

int os_spinlock_init(CelSpinLock *spinlock, int pshared)
{
    *spinlock = 0;
    return 0;
}

void os_spinlock_destroy(CelSpinLock *spinlock)
{
    *spinlock = 0;
}

void os_spinlock_lock(CelSpinLock *spinlock)
{
    unsigned k;

    for (k = 0; cel_spinlock_trylock(spinlock) != 0; ++k)
    {           
        yield(k);
    }
}

int os_spinlock_trylock(CelSpinLock *spinlock)
{
    //return InterlockedCompareExchange(spinlock, 1, 0) == 0 ? 0 : -1;
    long r = InterlockedExchange(spinlock, 1);
    _ReadWriteBarrier();
    return (r == 0 ? 0 : -1);
}

void os_spinlock_unlock(CelSpinLock *spinlock)
{
    //InterlockedCompareExchange(spinlock, 0, 1);
    _ReadWriteBarrier();                             
    *((long volatile *)(spinlock)) = 0;
}

int os_rwlock_init(CelRwlock *rwlock, const CelRwlockAttr *attr)
{
    if ((cel_mutex_init(&(rwlock->lock), NULL)) != -1)
    {
        if ((cel_cond_init(&(rwlock->read_signal), NULL)) != -1)
        {
            if ((cel_cond_init(&(rwlock->write_signal), NULL)) != -1)
            {
                rwlock->state = 0;
                rwlock->blocked_writers = 0;
            }
            cel_cond_destroy(&(rwlock->read_signal));
        }
        cel_mutex_destroy(&(rwlock->lock));
    }
    return -1;
}

void os_rwlock_destroy(CelRwlock *rwlock)
{
    cel_mutex_destroy(&(rwlock->lock));
    cel_cond_destroy(&(rwlock->read_signal));
    cel_cond_destroy(&(rwlock->write_signal));
}

int os_rwlock_rdlock(CelRwlock *rwlock)
{
    cel_mutex_lock(&(rwlock->lock));
    /* Give writers priority over readers */
    while (rwlock->blocked_writers != 0 || rwlock->state < 0)
    {
        if ((cel_cond_wait(&(rwlock->read_signal), &(rwlock->lock))) == -1)
            return -1;
    }
    /* Indicate we are locked for reading */
    rwlock->state++;
    cel_mutex_unlock(&(rwlock->lock));

    return 0;
}

int os_rwlock_tryrdlock(CelRwlock *rwlock)
{
    cel_mutex_lock(&(rwlock->lock));
    /* Give writers priority over readers */
    if (rwlock->blocked_writers != 0 || rwlock->state < 0)
        return -1;
    else
        rwlock->state++; /**< Indicate we are locked for reading */
    cel_mutex_unlock(&(rwlock->lock));

    return 0;
}

int os_rwlock_trywrlock(CelRwlock *rwlock)
{
    cel_mutex_lock(&(rwlock->lock));
    if (rwlock->state != 0)
        return -1;
    else    
        rwlock->state = -1; /**< indicate we are locked for writing */
    cel_mutex_unlock(&(rwlock->lock));

    return 0;
}

int os_rwlock_wrlock(CelRwlock *rwlock)
{
    cel_mutex_lock(&(rwlock->lock));
    while (rwlock->state != 0)
    {
        ++rwlock->blocked_writers;
        if (cel_cond_wait(&(rwlock->write_signal), &(rwlock->lock)) == -1)
        {
            --rwlock->blocked_writers;
            cel_mutex_unlock(&rwlock->lock);
            return -1;
        }
        --rwlock->blocked_writers;
    }
    /* Indicate we are locked for writing */
    rwlock->state = -1;
    cel_mutex_unlock(&rwlock->lock);

    return 0;
}

void os_rwlock_unlock(CelRwlock *rwlock)
{
    cel_mutex_lock(&(rwlock->lock));
    if (rwlock->state > 0)
    {
        if ((--rwlock->state) == 0 && rwlock->blocked_writers != 0)
            cel_cond_signal(&(rwlock->write_signal));
    } 
    else if (rwlock->state < 0)
    {
        rwlock->state = 0;
        if (rwlock->blocked_writers != 0)
            cel_cond_signal(&(rwlock->write_signal));
        else
            cel_cond_broadcast(&(rwlock->read_signal));
    }
    cel_mutex_unlock(&(rwlock->lock));
}
#endif
