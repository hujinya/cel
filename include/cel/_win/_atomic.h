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

#define os_compiler_barrier() 
#define os_atomic_cmp_and_swap(ptr, oldval, newval, mem_order) \
    _InterlockedCompareExchange(ptr, newval, oldval)

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif


#endif
