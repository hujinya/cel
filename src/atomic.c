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
#include "cel/atomic.h"
#include "cel/multithread.h"

#ifdef CEL_ARCH_ATOMIC_INC
long cel_arch_atomic_inc(CelAtomic *v)
{ 
    cel_multithread_mutex_lock(CEL_MT_MUTEX_ATOMIC);
    (*v)++;
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_ATOMIC);
    return *v;
}
#endif

#ifdef CEL_ARCH_ATOMIC_DEC
long cel_arch_atomic_dec(CelAtomic *v)
{
    cel_multithread_mutex_lock(CEL_MT_MUTEX_ATOMIC);
    (*v)--;
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_ATOMIC);

    return *v;
}
#endif

#ifdef CEL_ARCH_ATOMIC_ADD
long cel_arch_atomic_add(CelAtomic *v, long i)
{
    cel_multithread_mutex_lock(CEL_MT_MUTEX_ATOMIC);
    (*v)--;
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_ATOMIC);

    return *v;
}
#endif

#ifdef CEL_ARCH_ATOMIC_SUB
long cel_arch_atomic_sub(CelAtomic *v, long i)
{
    cel_multithread_mutex_lock(CEL_MT_MUTEX_ATOMIC);
    (*v)--;
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_ATOMIC);

    return *v;
}
#endif

#ifdef CEL_ARCH_ATOMIC_OR
long cel_arch_atomic_or(CelAtomic *v, long i)
{
    cel_multithread_mutex_lock(CEL_MT_MUTEX_ATOMIC);
    *v = *v|i;
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_ATOMIC);
    return *v;
}
#endif

#ifdef CEL_ARCH_ATOMIC_AND
long cel_arch_atomic_and(CelAtomic *v, long i)
{
    cel_multithread_mutex_lock(CEL_MT_MUTEX_ATOMIC);
    *v = *v & i;
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_ATOMIC);

    return *v;
}
#endif

#ifdef CEL_ARCH_ATOMIC_XOR
long cel_arch_atomic_xor(CelAtomic *v, long i)
{
    cel_multithread_mutex_lock(CEL_MT_MUTEX_ATOMIC);
    *v = *v ^ i;
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_ATOMIC);

    return *v;
}
#endif
