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
#include "cel/allocator.h"
#include "cel/error.h"

CelMutex *cel_mutex_new(CelMutexAttr *attr)
{
    CelMutex *mutex;

    if ((mutex = (CelMutex *)cel_malloc(sizeof(CelMutex))) != NULL)
    {
        if (cel_mutex_init(mutex, attr) == 0)
            return mutex;
        cel_free(mutex);
    }
    return NULL;
}

void cel_mutex_free(CelMutex *mutex)
{
    cel_mutex_destroy(mutex); 
    cel_free(mutex);
}

CelCond *cel_cond_new(CelCondAttr *attr)
{
    CelCond *cond;

    if ((cond = (CelCond *)cel_malloc(sizeof(CelCond))) != NULL)
    {
        if (cel_cond_init(cond, attr) == 0)
            return cond;
        cel_free(cond);
    }
    return NULL;
}

void cel_cond_free(CelCond *cond)
{
    cel_cond_destroy(cond); 
    cel_free(cond);
}

CelSpinLock *cel_spinlock_new(int pshared)
{
    CelSpinLock *spinlock;

    if ((spinlock = (CelSpinLock *)cel_malloc(sizeof(CelSpinLock))) != NULL)
    {
        if (cel_spinlock_init(spinlock, pshared) == 0)
            return spinlock;
        cel_free((void *)spinlock);
    }
    return NULL;
}

void cel_spinlock_free(CelSpinLock *spinlock)
{
    cel_spinlock_destroy(spinlock); 
    cel_free((void *)spinlock);
}

CelRwlock *cel_rwlock_new(CelRwlockAttr *attr)
{
    CelRwlock *rwlock;

    if ((rwlock = (CelRwlock *)cel_malloc(sizeof(CelRwlock))) != NULL)
    {
        if (cel_rwlock_init(rwlock, attr) == 0)
            return rwlock;
        cel_free(rwlock);
    }
    return NULL;
}

void cel_rwlock_free(CelRwlock *rwlock)
{
    cel_rwlock_destroy(rwlock); 
    cel_free(rwlock);
}
