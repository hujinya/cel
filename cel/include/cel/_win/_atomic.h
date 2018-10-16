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
#ifndef __CEL_ATOMIC_WIN_H__
#define __CEL_ATOMIC_WIN_H__

#include "cel/types.h"
#include <intrin.h>

#if defined(_WIN64)
typedef volatile S64 OsAtomic;
#else
typedef volatile S32 OsAtomic;
#endif

#define os_atomic_store(ptr, val) *(ptr) = val
#define os_atomic_load(ptr) (*(ptr))

#define os_atomic_exchange(ptr, newval) _InterlockedExchange(ptr, newval);
/* if ptr == oldval , ptr = oldval, return oldval */
#define os_atomic_cmp_and_swap(ptr, oldval, newval, mem_order) \
    _InterlockedCompareExchange(ptr, newval, oldval)
#define os_atomic_increment(ptr, increment) \
    (_InterlockedExchangeAdd(ptr, increment) + (increment))

// In msvc8/vs2005, winnt.h already contains a definition for
// MemoryBarrier in the global namespace.  Add it there for earlier
// versions and forward to it from within the namespace.
#if !(defined(_MSC_VER) && _MSC_VER >= 1400)
inline void MemoryBarrier() {
  OsAtomic value = 0;
  os_atomic_exchange(&value, 0);
}
#endif
#define os_compiler_barrier() MemoryBarrier()

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif


#endif
