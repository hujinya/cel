/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_REFCOUNTED_H__
#define __CEL_REFCOUNTED_H__

#include "cel/types.h"
#include "cel/atomic.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelRefCounted
{
    CelAtomic cnt;
    CelFreeFunc free_func;
}CelRefCounted;

int cel_refcounted_init(CelRefCounted *ref_counted, CelFreeFunc free_func);
void cel_refcounted_destroy(CelRefCounted *ref_counted, void *ptr);

CelRefCounted *cel_refcounted_ref(CelRefCounted *ref_counted);
void *cel_refcounted_ref_ptr(CelRefCounted *ref_counted, void *ptr);
void cel_refcounted_deref(CelRefCounted *ref_counted, void *ptr);

#ifdef __cplusplus
}
#endif

#endif
