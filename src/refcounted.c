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
#include "cel/refcounted.h"
#include "cel/allocator.h"

int cel_refcounted_init(CelRefCounted *ref_counted, CelFreeFunc free_func)
{
    ref_counted->cnt = 1;
    ref_counted->free_func = free_func;

    return 0;
}

void cel_refcounted_destroy(CelRefCounted *ref_counted, void *ptr)
{
    if (cel_atomic_increment(&(ref_counted->cnt), -1) <= 0)
    {
        //printf("ref_counted->cnt = %d\r\n", ref_counted->cnt);
        if (ref_counted->free_func != NULL)
            ref_counted->free_func(ptr);
    }
}

CelRefCounted *cel_refcounted_ref(CelRefCounted *ref_counted)
{
    cel_atomic_increment(&(ref_counted->cnt), 1);
    return ref_counted;
}

void *cel_refcounted_ref_ptr(CelRefCounted *ref_counted, void *ptr)
{
    cel_atomic_increment(&(ref_counted->cnt), 1);
    return ptr;
}

void cel_refcounted_deref(CelRefCounted *ref_counted, void *ptr)
{
    if (cel_atomic_increment(&(ref_counted->cnt), -1) <= 0)
    {
        //printf("ref_counted->cnt = %d\r\n", ref_counted->cnt);
        if (ref_counted->free_func != NULL)
            ref_counted->free_func(ptr);
    }
}
