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
#ifndef __CEL_ATOMIC_H__
#define __CEL_ATOMIC_H__

#include "cel/types.h"

/*if defined(__GNUC__) && (defined(__i386) || defined(__x86_64__))
#include "base/_atomic_x86.h"
#el*/#if defined(__GNUC__) && GCC_VERSION >= 40700
#include "cel/_unix/_atomic_sync.h"
#elif defined(_CEL_WIN)
#include "cel/_win/_atomic.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef OsAtomic CelAtomic;

#define cel_atomic_exchange os_atomic_exchange
#define cel_atomic_cmp_and_swap os_atomic_cmp_and_swap
#define cel_atomic_increment os_atomic_increment

#define cel_atomic_store os_atomic_store
#define cel_atomic_load os_atomic_load
#define cel_atomic_clear os_atomic_clear

#define cel_compiler_barrier os_compiler_barrier

static __inline void yield(unsigned k)
{
    if (k < 4);
    else if (k < 16) _mm_pause();
    else if (k < 32) usleep(0); 
    else usleep(1);
}

#ifdef __cplusplus
}
#endif

#endif