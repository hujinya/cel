/**
 * CEL(C Extension Library)
 * Copyright (C)2008-2017 Hu Jinya(hu_jinya@163.com) 
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
#ifndef __CEL_ALLOCATER_H__
#define __CEL_ALLOCATER_H__

#include "cel/config.h"
#if defined(_CEL_UNIX)
#include "cel/_unix/_allocator.h"
#elif defined(_CEL_WIN)
#include "cel/_win/_allocator.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Aligned n bytes */
#define CEL_ALIGNED(size, n)   ((size) + (n) - 1) / (n) * (n)

extern CelMallocFunc _cel_sys_malloc;
extern CelFreeFunc _cel_sys_free;
extern CelReallocFunc _cel_sys_realloc;
extern CelMallocFunc cel_malloc;
extern CelFreeFunc cel_free;
extern CelReallocFunc cel_realloc;

/* Capacity */
#define CEL_CAPACITY_MIN       11
#define CEL_CAPACITY_MAX       13845163

size_t cel_capacity_get_min(size_t capacity);

/* 
 * void *cel_system_alloc(size_t size, size_t *actual_size, size_t alignment);
 */
#define cel_system_alloc os_system_alloc
/* BOOL cel_system_release(void *start, size_t length); */
#define cel_system_release os_system_release
/* void cel_system_commit(void *start, size_t length); */
#define cel_system_commit os_system_commit

int cel_allocator_hook_register(CelMallocFunc malloc_func, 
                                CelFreeFunc free_func,
                                CelReallocFunc realloc_func);
void cel_allocator_hook_unregister(void);

void *cel_allocate(size_t size);
void cel_deallocate(void *buf);
void *cel_reallocate(void *memory, size_t new_size);
/* ThreadCache->CentralCache->PageHeap->Span */
int cel_allocator_dump(char *buf, size_t size);

static __inline void *cel_calloc(int num_elems, int elem_size)
{
    void *mem;
    if ((mem = cel_malloc(num_elems * elem_size)) != NULL)
        memset(mem, 0, num_elems * elem_size);
    return mem;
}

static __inline CHAR *cel_strdup_full(const CHAR *str, 
                                      size_t start, size_t end)
{
    size_t len = end -start;
    CHAR *newstr;

    if ((newstr = (CHAR *)cel_malloc((len + 1) * sizeof(CHAR))) != NULL){
        memcpy(newstr, &str[start], len  * sizeof(CHAR));
        newstr[len] = '\0';
    }
    return newstr;
}

static __inline CHAR *cel_strdup(const CHAR *str)
{
    size_t len = strlen(str);
    CHAR *newstr;
    if ((newstr = (CHAR *)cel_malloc((len + 1) * sizeof(CHAR))) != NULL){
        memcpy(newstr, str, len * sizeof(CHAR));
        newstr[len] = '\0';
    }
    return newstr;
}

static __inline WCHAR *cel_wcsdup_full(const WCHAR *str, 
                                       size_t start, size_t end)
{
    size_t len = end -start;
    WCHAR *newstr;

    if ((newstr = (WCHAR *)cel_malloc((len + 1) * sizeof(WCHAR))) != NULL){
        memcpy(newstr, &str[start], len  * sizeof(WCHAR));
        newstr[len] = L'\0';
    }
    return newstr;
}

static __inline WCHAR *cel_wcsdup(const WCHAR *str)
{
    size_t len = wcslen(str);
    WCHAR *newstr;
    if ((newstr = (WCHAR *)cel_malloc((len + 1) * sizeof(WCHAR))) != NULL){
        memcpy(newstr, str, len * sizeof(WCHAR));
        newstr[len] = L('\0');
    }
    return newstr;
}
#ifndef _UNICODE
#define cel_tcsdup_full cel_strdup_full
#define cel_tcsdup cel_strdup
#else
#define cel_tcsdup_full cel_wcsdup_full
#define cel_tcsdup cel_wcsdup
#endif

#define CEL_PTR_STRDUP(ptr, value)  if ((ptr) != value) { \
    if ((ptr) != NULL) { cel_free(ptr); } ptr = cel_strdup(value); }
#define CEL_PTR_TCSDUP(ptr, value) if ((ptr) != value) { \
    if ((ptr) != NULL) { cel_free(ptr); } ptr = cel_tcsdup(value); }
#define CEL_PTR_FREE(ptr) if ((ptr) != NULL) { cel_free(ptr); (ptr) = NULL; }

#ifdef __cplusplus
}
#endif

#endif
