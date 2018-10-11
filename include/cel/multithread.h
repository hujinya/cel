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
#ifndef __CEL_MULTITHREAD_H__
#define __CEL_MULTITHREAD_H__

#include "cel/types.h"
#include "cel/thread.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Multithread local mutex */
typedef enum _CelMultithreadMutexType
{
    CEL_MT_MUTEX_ALLOCATOR = 0,
    CEL_MT_MUTEX_LOG,
    CEL_MT_MUTEX_SSL_HANDSHAKE,
    CEL_MT_MUTEX_PERFOINFO,
    CEL_MT_MUTEX_USER1,
    CEL_MT_MUTEX_USER2,
    CEL_MT_MUTEX_USER3,
    CEL_MT_MUTEX_MAX
}CelMultithreadMutexType;

typedef enum _CelMultithreadMutexOpType
{
    CEL_MT_MUTEX_UNKNOW, 
    CEL_MT_MUTEX_LOCK, 
    CEL_MT_MUTEX_TRYLOCK, 
    CEL_MT_MUTEX_UNLOCK
}CelMultithreadMutexOpType;

typedef BOOL (* CelMultithreadMutexCallbackFunc) (
    CelMultithreadMutexType type, 
    CelMultithreadMutexOpType lock,
    void *user_data);

extern void *mutex_callback_userdata;
extern CelMultithreadMutexCallbackFunc mutex_callback_func;

void cel_multithread_mutex_callback_register(
    CelMultithreadMutexCallbackFunc func, void *user_data);
void cel_multithread_mutex_callback_unregister(void);


/* void cel_multithread_mutex_lock(CelMultithreadMutexType type)*/
#define cel_multithread_mutex_lock(type) if (mutex_callback_func != NULL) \
        mutex_callback_func(type, CEL_MT_MUTEX_LOCK, mutex_callback_userdata)
/* void cel_multithread_mutex_trylock(CelMultithreadMutexType type)*/
#define cel_multithread_mutex_trylock(type) \
    ((mutex_callback_func != NULL) \
    ? mutex_callback_func(type, CEL_MT_MUTEX_TRYLOCK, mutex_callback_userdata)\
    : FALSE)
/* void cel_multithread_mutex_unlock(CelMultithreadMutexType type)*/
#define cel_multithread_mutex_unlock(type) if (mutex_callback_func != NULL) \
        mutex_callback_func(type, CEL_MT_MUTEX_UNLOCK, mutex_callback_userdata)


/* Multithread local storage */
typedef enum _CelMultithreadKey
{
    CEL_MT_KEY_ERROR = 0,
    CEL_MT_KEY_ALLOCATOR,
    CEL_MT_KEY_LOG,
    CEL_MT_KEY_COROUTINE,
    CEL_MT_KEY_EVENTLOOPTHREADID,
    CEL_MT_KEY_USER1,
    CEL_MT_KEY_USER2,
    CEL_MT_KEY_USER3,
    CEL_MT_KEY_MAX
}CelMultithreadKey;

typedef struct _CelMultithreadSpecific
{
    void *value;
    CelDestroyFunc destructor;
}CelMultithreadSpecific;

typedef CelMultithreadSpecific * (* CelMultithreadSpecificCallbackFunc) (
    CelMultithreadKey key, void *user_data);

extern void *getspecific_userdata;
extern CelMultithreadSpecificCallbackFunc getspecific_callback_func;

void cel_multithread_getspecific_callback_register(
    CelMultithreadSpecificCallbackFunc func, void *user_data);
void cel_multithread_getspecific_callback_unregister(void);
static __inline 
int cel_multithread_set_keydestructor(CelMultithreadKey key,
                                      CelDestroyFunc destructor)
{
    CelMultithreadSpecific *specific;

    if ((specific = 
        getspecific_callback_func(key, getspecific_userdata)) != NULL)
    {
        specific->destructor = destructor;
        return 0;
    }
    return -1;
}
static __inline 
int cel_multithread_set_keyvalue(CelMultithreadKey key, void *value)
{
    CelMultithreadSpecific *specific;

    if ((specific = 
        getspecific_callback_func(key, getspecific_userdata)) != NULL)
    {
        specific->value = value;
        return 0;
    }
    return -1;
}
static __inline void *cel_multithread_get_keyvalue(CelMultithreadKey key)
{
    CelMultithreadSpecific *specific;

    return ((specific = getspecific_callback_func(
        key, getspecific_userdata)) != NULL)
        ? specific->value : NULL;
}

#define cel_multithread_support() \
    cel_multithread_mutex_callback_register(NULL, NULL);\
    cel_multithread_getspecific_callback_register(NULL, NULL)

#ifdef __cplusplus
}
#endif

#endif
