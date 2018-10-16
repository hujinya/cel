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
#ifndef __CEL_ATOMIC_UNIX_H__
#define __CEL_ATOMIC_UNIX_H__

#include "cel/types.h"

#if defined(__x86_64__)
typedef volatile S64 OsAtomic;
#else
typedef volatile S32 OsAtomic;
#endif

#include <xmmintrin.h>
//static __inline void _mm_pause(void)
//{
//    __asm__ __volatile__ ("pause" : : :"memory");
//}
#define os_compiler_barrier() __sync_synchronize()

#define os_atomic_store(ptr, val) *(ptr) = val
#define os_atomic_load(ptr) (*(ptr))

static __inline OsAtomic os_atomic_exchange(OsAtomic *ptr, OsAtomic newval)
{
    OsAtomic oldval;
    do {
        oldval = *ptr;
    } while (__sync_val_compare_and_swap(ptr, oldval, newval) != oldval);
    return oldval;
}
#define os_atomic_cmp_and_swap(ptr, oldval, newval, mem_order) \
    __sync_val_compare_and_swap(ptr, oldval, newval)
#define os_atomic_increment(ptr, increment) __sync_add_and_fetch(ptr, increment)

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif
