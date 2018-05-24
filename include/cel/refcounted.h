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
