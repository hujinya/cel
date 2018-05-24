/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2016 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/multithread.h"
#include "cel/allocator.h"

#ifdef _CEL_MULTITHREAD
static CelMutex s_mutex[CEL_MT_MUTEX_MAX];

static BOOL cel_thread_mutex_callback(CelMultithreadMutexType type, 
                                      CelMultithreadMutexOpType lock, 
                                      void *user_data)
{
    switch (lock)
    {
    case CEL_MT_MUTEX_LOCK:
        cel_mutex_lock(&s_mutex[type]);
        break;
    case CEL_MT_MUTEX_UNLOCK:
        cel_mutex_unlock(&s_mutex[type]);
        break;
    case CEL_MT_MUTEX_TRYLOCK:
        return (cel_mutex_trylock(&s_mutex[type]) != 0 ? FALSE : TRUE);
    default:
        break;
    }
    return TRUE;
}
#endif

void *mutex_callback_userdata = NULL;
CelMultithreadMutexCallbackFunc mutex_callback_func = NULL;

void cel_multithread_mutex_callback_register(
    CelMultithreadMutexCallbackFunc func, void *user_data)
{
#ifdef _CEL_MULTITHREAD
    int i;
#endif

    mutex_callback_userdata = user_data;
    if (func == NULL)
    {
#ifdef _CEL_MULTITHREAD
        for (i = 0; i < CEL_MT_MUTEX_MAX; i++)
            cel_mutex_init(&s_mutex[i], NULL);
        mutex_callback_func = cel_thread_mutex_callback;
#endif
        return ;
    }
    mutex_callback_func = func;
}

void cel_multithread_mutex_callback_unregister(void)
{
    mutex_callback_func = NULL;
    mutex_callback_userdata = NULL;
}

#ifdef _CEL_MULTITHREAD
static CelThreadOnce s_threadonce = CEL_THREAD_ONCE_INIT;
static CelThreadKey s_threadkey;

static void cel_clean_thread_key(void *key)
{
    int i = 0;
    CelMultithreadSpecific *thread_specific = (CelMultithreadSpecific *)key;
    CelDestroyFunc destructor;

    for (; i < CEL_MT_KEY_MAX; i++)
    {
        if (thread_specific[i].value != NULL
            && (destructor = thread_specific[i].destructor) != NULL)
        {
            destructor(thread_specific[i].value);
            //printf("Multithread thread destructor %d-%p\r\n", 
            //    i, thread_specific[i].value);
        }
        thread_specific[i].value = NULL;
        thread_specific[i].destructor = NULL;
    }
    //printf("Multithread thread key %p\r\n", key);
    _cel_sys_free(thread_specific);
}

static void cel_make_thread_key(void)
{
    (void)cel_thread_key_create(&s_threadkey, &cel_clean_thread_key);
}

CelMultithreadSpecific *cel_multithread_getspecific_callback(
    CelMultithreadKey key, void *user_data)
{
    int i; 
    CelMultithreadSpecific *ptr;

    (void)cel_thread_once(&s_threadonce, cel_make_thread_key);
    if ((ptr = (CelMultithreadSpecific *)
        cel_thread_getspecific(s_threadkey)) == NULL)
    {
        if ((ptr = (CelMultithreadSpecific *)_cel_sys_malloc(
            sizeof(CelMultithreadSpecific) * CEL_MT_KEY_MAX)) != NULL)
        {
            if (cel_thread_setspecific(s_threadkey, ptr) != -1)
            {
                for (i = 0; i < CEL_MT_KEY_MAX; i++)
                {
                    ptr[i].value = NULL;
                    ptr[i].destructor = NULL;
                }
                /*
                printf("Multithread thread key %d-%d, pointer %p.\r\n", 
                    s_threadkey, key, ptr);
                    */
                return &(ptr[key]);
            }
            _cel_sys_free(ptr);
        }
        return NULL;
    }

    return &(ptr[key]);
}
#endif

static CelMultithreadSpecific s_multithreadspecific[CEL_MT_KEY_MAX];
CelMultithreadSpecific *cel_multithread_getspecific_default(
    CelMultithreadKey key, void *user_data)
{
    return &(s_multithreadspecific[key]);
}

void *getspecific_userdata = NULL;
CelMultithreadSpecificCallbackFunc \
getspecific_callback_func = cel_multithread_getspecific_default;

void cel_multithread_getspecific_callback_register(
    CelMultithreadSpecificCallbackFunc func,
    void *user_data)
{
    getspecific_userdata = user_data;
    if (func == NULL)
    {
        getspecific_callback_func = cel_multithread_getspecific_callback;
        return ;
    }
    getspecific_callback_func = func;
}

void cel_multithread_getspecific_callback_unregister(void)
{
    getspecific_callback_func = NULL;
    getspecific_userdata = NULL;
}
