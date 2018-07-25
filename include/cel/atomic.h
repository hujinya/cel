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
#if defined(_CEL_UNIX)
#include "cel/_unix/_atomic_sync.h"
#elif defined(_CEL_WIN)
#include "cel/_win/_atomic.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define cel_atomic_exchange os_atomic_exchange
#define cel_atomic_cmp_and_swap os_atomic_cmp_and_swap
#define cel_atomic_store os_atomic_store
#define cel_atomic_load os_atomic_load
#define cel_atomic_clear os_atomic_clear

#ifdef _CEL_WIN
#define CEL_ARCH_ATOMIC_INC
#define CEL_ARCH_ATOMIC_DEC
#define CEL_ARCH_ATOMIC_ADD
#define CEL_ARCH_ATOMIC_SUB
#define CEL_ARCH_ATOMIC_OR
#define CEL_ARCH_ATOMIC_AND
#define CEL_ARCH_ATOMIC_XOR
#endif

typedef OsAtomic CelAtomic;


static __inline long cel_atomic_get(CelAtomic *v){ return *v; }
static __inline long cel_atomic_set(CelAtomic *v, long i) { return (*v = i); }

/* 
 * Increments (increases by one), Decrements (decreases by one)
 * the value of the specified 32-bit variable as an atomic operation.
 */
#ifdef CEL_ARCH_ATOMIC_INC
long cel_arch_atomic_inc(CelAtomic *v);
#endif
static __inline long cel_atomic_inc(CelAtomic *v)
{
#ifdef CEL_ARCH_ATOMIC_INC
    return cel_arch_atomic_inc(v);
#else
#ifdef _CEL_UNIX
    return __sync_add_and_fetch(v, 1);
#endif
#ifdef _CEL_WIN
    return _InterlockedIncrement(v);
#endif
#endif
}

#ifdef CEL_ARCH_ATOMIC_DEC
long cel_arch_atomic_dec(CelAtomic *v);
#endif
static __inline long cel_atomic_dec(CelAtomic *v)
{
#ifdef CEL_ARCH_ATOMIC_DEC
    return cel_arch_atomic_dec(v);
#else
#ifdef _CEL_UNIX
    return __sync_sub_and_fetch(v, 1);
#endif
#ifdef _CEL_WIN
    return _InterlockedDecrement(v);
#endif
#endif
}

/* 
 * Performs an atomic addition, subtract operation on the specified LONG values 
 */
#ifdef CEL_ARCH_ATOMIC_ADD
long cel_arch_atomic_add(CelAtomic *v, long i);
#endif
static __inline long cel_atomic_add(CelAtomic *v, long i)
{
#ifdef CEL_ARCH_ATOMIC_ADD
    return cel_arch_atomic_add(v, i);
#else
#ifdef _CEL_UNIX
    return __sync_add_and_fetch(v, i);
#endif
#ifdef _CEL_WIN
    return _InterlockedAdd(v, i);
#endif
#endif
}

#ifdef CEL_ARCH_ATOMIC_SUB
long cel_arch_atomic_sub(CelAtomic *v, long i);
#endif
static __inline long cel_atomic_sub(CelAtomic *v, long i)
{
#ifdef CEL_ARCH_ATOMIC_SUB
    return cel_arch_atomic_sub(v, i);
#else
#ifdef _CEL_UNIX
    return __sync_sub_and_fetch(v, i);
#endif
#ifdef _CEL_WIN
    return _InterlockedAdd(v, -i);
#endif
#endif
}

/* 
 * Performs an atomic OR, AND, XOR, NAND operation on the specified LONG values
 */
#ifdef CEL_ARCH_ATOMIC_OR
long cel_arch_atomic_or(CelAtomic *v, long i);
#endif
static __inline long cel_atomic_or(CelAtomic *v, long i)
{
#ifdef CEL_ARCH_ATOMIC_OR
    return cel_arch_atomic_or(v, i);
#else
#ifdef _CEL_UNIX
    return __sync_or_and_fetch(v, i);
#endif
#ifdef _CEL_WIN
    return _InterlockedOr(v, i);
#endif
#endif
}

#ifdef CEL_ARCH_ATOMIC_AND
long cel_arch_atomic_and(CelAtomic *v, long i);
#endif
static __inline long cel_atomic_and(CelAtomic *v, long i)
{
#ifdef CEL_ARCH_ATOMIC_AND
    return cel_arch_atomic_and(v, i);
#else
#ifdef _CEL_UNIX
    return __sync_and_and_fetch(v, i);
#endif
#ifdef _CEL_WIN
    return _InterlockedAnd(v, i);
#endif
#endif
}

#ifdef CEL_ARCH_ATOMIC_XOR
long cel_arch_atomic_xor(CelAtomic *v, long i);
#endif
static __inline long cel_atomic_xor(CelAtomic *v, long i)
{
#ifdef CEL_ARCH_ATOMIC_XOR
    return cel_arch_atomic_xor(v, i);
#else
#ifdef _CEL_UNIX
    return __sync_xor_and_fetch(v, i);
#endif
#ifdef _CEL_WIN
    return _InterlockedXor(v, i);
#endif
#endif
}

#ifdef __cplusplus
}
#endif

#endif