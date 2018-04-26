/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_ATOMIC_UNIX_H__
#define __CEL_ATOMIC_UNIX_H__

#include "cel/types.h"

typedef volatile long CelAtomic;

#define cel_atomic_init(v) __sync_lock_release(v)

#define cel_atomic_get(v) (*(v))
#define cel_atomic_set(v, l) *(v) = l
#define cel_atomic_inc_fetch(v) __sync_add_and_fetch(v, 1)
#define cel_atomic_dec_fetch(v) __sync_sub_and_fetch(v, 1)
#define cel_atomic_add_fetch(v, l) __sync_add_and_fetch(v, l)
#define cel_atomic_sub_fetch(v, l) __sync_sub_and_fetch(v, l)
#define cel_atomic_or_fetch(v, l) __sync_or_and_fetch(v, l)
#define cel_atomic_and_fetch(v, l) __sync_and_and_fetch(v, l)
#define cel_atomic_xor_fetch(v, l) __sync_xor_and_fetch(v, l)

#define cel_atomic_fetch_inc(v) __sync_fetch_and_add(v, 1)
#define cel_atomic_fetch_dec(v) __sync_fetch_sub_and(v, 1)
#define cel_atomic_fetch_add(v, l) __sync_fetch_and_add(v, l)
#define cel_atomic_fetch_sub(v, l) __syn_fetch_and_sub_and(v, l)
#define cel_atomic_fetch_or(v, l) __sync_fetch_and_or_and(v, l)
#define cel_atomic_fetch_and(v, l) __sync_fetch_and_and(v, l)
#define cel_atomic_fetch_xor(v, l) __sync_fetch_and_xor(v, l)

#ifdef __cplusplus
}
#endif

#endif
