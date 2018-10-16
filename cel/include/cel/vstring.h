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
#ifndef __CEL_VSTRING_H__
#define __CEL_VSTRING_H__

#include "cel/types.h"
#include "cel/convert.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelVStringA
{
    size_t capacity, size;
    CHAR *str;
}CelVStringA;

typedef struct _CelVStringW
{
    size_t capacity, size;
    WCHAR *str;
}CelVStringW;

#ifdef _UNICODE
#define CelVString CelVStringW
#else
#define CelVString CelVStringA
#endif

int cel_vstring_init_a(CelVStringA *vstr);
int cel_vstring_init_w(CelVStringW *vstr);
void cel_vstring_destroy_a(CelVStringA *vstr);
void cel_vstring_destroy_w(CelVStringW *vstr);

CelVStringA *cel_vstring_new_a(void);
CelVStringW *cel_vstring_new_w(void);
void cel_vstring_free_a(CelVStringA *vstr);
void cel_vstring_free_w(CelVStringW *vstr);

int __cel_vstring_resize(CelVString *vstr, size_t size, size_t cw);
#define _cel_vstring_resize(vstr, size, cw) (((size + 1) >= (vstr)->capacity) \
    ? __cel_vstring_resize((CelVString *)vstr, size + 1, cw) : 0)
#define cel_vstring_resize_a(vstr, size) \
    _cel_vstring_resize(vstr, size, sizeof(CHAR))
#define cel_vstring_resize_w(vstr, size) \
    _cel_vstring_resize(vstr, size, sizeof(WCHAR))

static __inline 
void cel_vstring_assign_a(CelVStringA *vstr, const CHAR *val, size_t n)
{
    if (n > 0 && _cel_vstring_resize(vstr, n, sizeof(CHAR)) == 0) {
        memcpy(vstr->str, val, n * sizeof(CHAR));
        vstr->size = n;
    }
}
static __inline 
void cel_vstring_assign_w(CelVStringW *vstr, const WCHAR *val, size_t n)
{
    if (_cel_vstring_resize(vstr, n, sizeof(WCHAR)) == 0) {
        memcpy(vstr->str, val, n * sizeof(WCHAR));
        vstr->size = n;
    }
}

static __inline void cel_vstring_append_a(CelVStringA *vstr, CHAR ch)
{
    if (_cel_vstring_resize(vstr, vstr->size + 1, sizeof(CHAR)) == 0)
        vstr->str[vstr->size++] = ch;
}
static __inline void cel_vstring_append_w(CelVStringW *vstr, WCHAR ch)
{
    if (_cel_vstring_resize(vstr, vstr->size + 1, sizeof(WCHAR)) == 0)
        vstr->str[vstr->size++] = ch;
}

static __inline 
void _cel_vstring_nappend_a(CelVStringA *vstr, const CHAR *val, size_t n)
{
    if (_cel_vstring_resize(vstr, vstr->size + n, sizeof(CHAR)) == 0) {
        memcpy(vstr->str + vstr->size, val, n * sizeof(CHAR));
        vstr->size += n;
    }
}
static __inline 
void _cel_vstring_nappend_w(CelVStringW *vstr, const WCHAR *val, size_t n)
{
    if (_cel_vstring_resize(vstr, vstr->size + n, sizeof(WCHAR)) == 0) {
        memcpy(vstr->str + vstr->size, val, n * sizeof(WCHAR));
        vstr->size += n;
    }
}

#define cel_vstring_nappend_a(vstr, val) \
    _cel_vstring_nappend_a(vstr, val, strlen(val))
#define cel_vstring_nappend_w(vstr, val) \
    _cel_vstring_nappend_w(vstr, val, wcslen(val))

static __inline void cel_vstring_prepend_a(CelVStringA *vstr, CHAR ch)
{
    if (_cel_vstring_resize(vstr, vstr->size + 1, sizeof(CHAR)) == 0) {
        if (vstr->size > 0) 
            memmove(vstr->str + 1, vstr->str, vstr->size * sizeof(CHAR));
        vstr->str[0] = ch;
        vstr->size += 1;
    }
}
static __inline void cel_vstring_prepend_w(CelVStringW *vstr, WCHAR ch)
{
    if (_cel_vstring_resize(vstr, vstr->size + 1, sizeof(WCHAR)) == 0) {
        if (vstr->size > 0) 
            memmove(vstr->str + 1, vstr->str, vstr->size * sizeof(WCHAR));
        vstr->str[0] = ch;
        vstr->size += 1;
    }
}

static __inline 
void _cel_vstring_nprepend_a(CelVStringA *vstr, const CHAR *val, size_t n)
{
    if (_cel_vstring_resize(vstr, vstr->size + n, sizeof(CHAR)) == 0) {
        if (vstr->size > 0) 
            memmove(vstr->str + n, vstr->str, vstr->size * sizeof(CHAR)); 
        memcpy(vstr->str, val, n * sizeof(CHAR));
        vstr->size += n; 
    } 
}
static __inline 
void _cel_vstring_nprepend_w(CelVStringW *vstr, const WCHAR *val, size_t n)
{
    if (_cel_vstring_resize(vstr, vstr->size + n, sizeof(WCHAR)) == 0) {
        if (vstr->size > 0) 
            memmove(vstr->str + n, vstr->str, vstr->size * sizeof(WCHAR)); 
        memcpy(vstr->str, val, n * sizeof(WCHAR));
        vstr->size += n; 
    } 
}

#define cel_vstring_nprepend_a(vstr, val) \
    _cel_vstring_nprepend_a(vstr, val, strlen(val)
#define cel_vstring_nprepend_w(vstr, val) \
    _cel_vstring_nprepend_w(vstr, val, wcslen(val)

#define cel_vstring_len(vstr) ((vstr)->size)
#define cel_vstring_size_a(vstr) ((vstr)->size * sizeof(CHAR))
#define cel_vstring_size_w(vstr) ((vstr)->size * sizeof(WCHAR))

static __inline 
CHAR *cel_vstring_str_r_a(CelVStringA *vstr, void *buf, size_t *size)
{
    size_t _size;

    if ((*size = (*size > (_size = vstr->size * sizeof(CHAR)) 
        ? _size - 1 * sizeof(CHAR) : *size - 1)) > 0)
        memcpy(buf, vstr->str, *size);
    *((char *)buf + *size) = '\0';
    return (CHAR *)buf;
}
static __inline 
WCHAR *cel_vstring_str_r_w(CelVStringW *vstr, void *buf, size_t *size)
{
    size_t _size;

    if ((*size = (*size > (_size = vstr->size * sizeof(WCHAR)) 
        ? _size - 1 * sizeof(WCHAR): *size -1)) > 0)
        memcpy(buf, vstr->str, *size);
    *((char *)buf + *size) = L'\0';
    return (WCHAR *)buf;
}

static __inline CHAR *cel_vstring_str_a(CelVStringA *vstr)
{
    return ((vstr)->str != NULL ? 
        ((vstr)->str[(vstr)->size] = '\0', (vstr)->str) : NULL);
}
static __inline WCHAR *cel_vstring_str_w(CelVStringW *vstr)
{
    return ((vstr)->str != NULL ? 
        ((vstr)->str[(vstr)->size] = L'\0', (vstr)->str) : NULL);
}

static __inline void cel_vstring_rtrim_a(CelVStringA *vstr) 
{   
    if (vstr->str != NULL) _cel_strrtrim_a(vstr->str, &(vstr->size)); 
}
static __inline void cel_vstring_rtrim_w(CelVStringW *vstr) 
{   
    if (vstr->str != NULL) _cel_strrtrim_w(vstr->str, &(vstr->size)); 
}
static __inline void cel_vstring_ltrim_a(CelVStringA *vstr) 
{
    if (vstr->str != NULL) _cel_strltrim_a(vstr->str, &(vstr->size)); 
}
static __inline void cel_vstring_ltrim_w(CelVStringW *vstr) 
{
    if (vstr->str != NULL) _cel_strltrim_w(vstr->str, &(vstr->size)); 
}

#define cel_vstring_clear_a(vstr) (vstr)->size = 0
#define cel_vstring_clear_w(vstr) (vstr)->size = 0
#define cel_vstring_clear(vstr) (vstr)->size = 0

#ifdef _UNICODE
#define cel_vstring_init cel_vstring_init_w
#define cel_vstring_destroy cel_vstring_destroy_w
#define cel_vstring_new cel_vstring_new_w
#define cel_vstring_free cel_vstring_free_w
#define cel_vstring_resize cel_vstring_resize_w
#define cel_vstring_assign cel_vstring_assign_w
#define cel_vstring_append cel_vstring_append_w
#define _cel_vstring_nappend _cel_vstring_nappend_w
#define cel_vstring_nappend cel_vstring_nappend_w
#define cel_vstring_prepend cel_vstring_prepend_w
#define _cel_vstring_nprepend _cel_vstring_nprepend_w
#define cel_vstring_nprepend cel_vstring_nprepend_w
#define cel_vstring_size cel_vstring_size_w
#define cel_vstring_str_r cel_vstring_str_r_w
#define cel_vstring_str cel_vstring_str_w
#define cel_vstring_rtrim cel_vstring_rtrim_w
#define cel_vstring_ltrim cel_vstring_ltrim_w
#else
#define cel_vstring_init cel_vstring_init_a
#define cel_vstring_destroy cel_vstring_destroy_a
#define cel_vstring_new cel_vstring_new_a
#define cel_vstring_free cel_vstring_free_a
#define cel_vstring_resize cel_vstring_resize_a
#define cel_vstring_assign cel_vstring_assign_a
#define cel_vstring_append cel_vstring_append_a
#define _cel_vstring_nappend _cel_vstring_nappend_a
#define cel_vstring_nappend cel_vstring_nappend_a
#define cel_vstring_prepend cel_vstring_prepend_a
#define _cel_vstring_nprepend _cel_vstring_nprepend_a
#define cel_vstring_nprepend cel_vstring_nprepend_a
#define cel_vstring_size cel_vstring_size_a
#define cel_vstring_str_r cel_vstring_str_r_a
#define cel_vstring_str cel_vstring_str_a
#define cel_vstring_rtrim cel_vstring_rtrim_a
#define cel_vstring_ltrim cel_vstring_ltrim_a
#endif

#ifdef __cplusplus
}
#endif

#endif
