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
#ifndef __CEL_ATOMIC_X86_H__
#define __CEL_ATOMIC_X86_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef S32 CelAtomic32;
typedef S64 CelAtomic64;

struct CelAtomicX86CpuFeature 
{
  BOOL has_sse2;            // Processor has SSE2.
  BOOL has_cmpxchg16b;      // Processor supports cmpxchg16b instruction.
};

extern struct CelAtomicX86CpuFeature atomic_internal_x86_cpu_features;


#define _CEL_ATOMIC__COMPILER_BARRIER() __asm__ __volatile__("" : : : "memory")

static __inline 
CelAtomic32 _cel_atiomic32_nobarrier_cmp_and_swap(volatile CelAtomic32 *ptr,
                                                  CelAtomic32 old_value,
                                                  CelAtomic32 new_value)
{
    CelAtomic32 prev;
    __asm__ __volatile__("lock; cmpxchgl %1,%2"
        : "=a" (prev)
        : "q" (new_value), "m" (*ptr), "0" (old_value)
        : "memory");
    return prev;
}

static __inline 
CelAtomic32 _cel_atiomic32_nobarrier_exchange(volatile CelAtomic32 *ptr,
                                              CelAtomic32 new_value) 
{
    __asm__ __volatile__("xchgl %1,%0"  // The lock prefix is implicit for xchg.
        : "=r" (new_value)
        : "m" (*ptr), "0" (new_value)
        : "memory");
    return new_value;  // Now it's the previous value.
}

static __inline 
CelAtomic32 _cel_atiomic32_exchange(volatile CelAtomic32 *ptr,
                                    CelAtomic32 new_value) 
{
    CelAtomic32 old_val = _cel_atiomic32_nobarrier_exchange(ptr, new_value);
    return old_val;
}

static __inline CelAtomic32 _cel_atiomic32_exchange(volatile CelAtomic32 *ptr,
                                                    CelAtomic32 new_value) 
{
    // xchgl already has release memory barrier semantics.
    return _cel_atiomic32_nobarrier_exchange(ptr, new_value);
}

static __inline 
CelAtomic32 _cel_atiomic32_cmp_and_swap(volatile CelAtomic32 *ptr,
                                        CelAtomic32 old_value,
                                        CelAtomic32 new_value) 
{
    CelAtomic32 x = _cel_atiomic32_nobarrier_cmp_and_swap(
        ptr, old_value, new_value);
    return x;
}

static __inline 
CelAtomic32 _cel_atiomic32_release_cmp_and_swap(volatile CelAtomic32* ptr,
                                                CelAtomic32 old_value,
                                                CelAtomic32 new_value) 
{
    return _cel_atiomic32_nobarrier_cmp_and_swap(
        ptr, old_value, new_value);
}

static __inline 
void _cel_atiomic32_nobarrier_store(volatile CelAtomic32* ptr,
                                    CelAtomic32 value)
{
    *ptr = value;
}

#if defined(__x86_64__)

// 64-bit implementations of memory barrier can be simpler, because it
// "mfence" is guaranteed to exist.
static __inline void _cel_memory_barrier() 
{
    __asm__ __volatile__("mfence" : : : "memory");
}

static __inline void _cel_acquire_store(volatile CelAtomic32* ptr, CelAtomic32 value)
{
    *ptr = value;
    memory_barrier();
}

#else

static __inline void _cel_memory_barrier()
{
    if (atomic_internal_x86_cpu_features.has_sse2) 
    {
        __asm__ __volatile__("mfence" : : : "memory");
    } 
    else
    { // mfence is faster but not present on PIII
        CelAtomic32 x = 0;
        _cel_atiomic32_acquire_exchange(&x, 0);
    }
}

static __inline 
void _cel_atiomic32_acquire_store(volatile CelAtomic32* ptr, CelAtomic32 value)
{
    if (atomic_internal_x86_cpu_features.has_sse2)
    {
        *ptr = value;
        __asm__ __volatile__("mfence" : : : "memory");
    }
    else 
    {
        Acquire_AtomicExchange(ptr, value);
    }
}
#endif

static __inline 
void _cel_atiomic32_release_store(volatile CelAtomic32* ptr, CelAtomic32 value)
{
  _CEL_ATOMIC__COMPILER_BARRIER();
  *ptr = value; // An x86 store acts as a release barrier.
  // See comments in Atomic64 version of Release_Store(), below.
}

static __inline 
CelAtomic32 _cel_atiomic32_nobarrier_load(volatile const CelAtomic32* ptr)
{
  return *ptr;
}

static __inline 
CelAtomic32 _cel_atiomic32_acquire_load(volatile const CelAtomic32* ptr)
{
  CelAtomic32 value = *ptr; // An x86 load acts as a acquire barrier.
  // See comments in Atomic64 version of Release_Store(), below.
  _CEL_ATOMIC__COMPILER_BARRIER();
  return value;
}

static __inline 
CelAtomic32 _cel_atiomic32_release_load(volatile const CelAtomic32* ptr) 
{
  memory_barrier();
  return *ptr;
}

#ifdef __cplusplus
}
#endif

#endif
