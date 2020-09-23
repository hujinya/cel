/**
 * CEL(C Extension Library)
 * Copyright (C)2008 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/vstring.h"
#include "cel/error.h"
#include "cel/log.h"
#include "cel/allocator.h"

int cel_vstring_init_a(CelVStringA *str)
{
    str->capacity = str->size = 0;
    str->str = NULL;
    return 0;
}

int cel_vstring_init_w(CelVStringW *str)
{
    str->capacity = str->size = 0;
    str->str = NULL;
    return 0;
}

void cel_vstring_destroy_a(CelVStringA *str)
{
    if (str->str != NULL)
        cel_free(str->str);
    str->capacity = str->size = 0;
    str->str = NULL;
}

void cel_vstring_destroy_w(CelVStringW *str)
{
    if (str->str != NULL)
        cel_free(str->str);
    str->capacity = str->size = 0;
    str->str = NULL;
}

CelVStringA *cel_vstring_new_a(void)
{
    CelVStringA *str;

    if ((str = (CelVStringA *)cel_malloc(sizeof(CelVStringA))) != NULL)
    {
        if (cel_vstring_init_a(str) == 0)
            return str;
        cel_free(str);
    }

    return NULL;
}

CelVStringW *cel_vstring_new_w(void)
{
    CelVStringW *str;

    if ((str = (CelVStringW *)cel_malloc(sizeof(CelVStringW))) != NULL)
    {
        if (cel_vstring_init_w(str) == 0)
            return str;
        cel_free(str);
    }

    return NULL;
}

void cel_vstring_free_a(CelVStringA *str)
{
    cel_vstring_destroy_a(str);
    cel_free(str);
}

void cel_vstring_free_w(CelVStringW *str)
{
    cel_vstring_destroy_w(str);
    cel_free(str);
}

int __cel_vstring_resize(CelVString *vstr, size_t size, size_t cw)
{
    if (size >= CEL_CAPACITY_MAX)
        return -1;
    size = cel_capacity_get_min(size);
    if ((vstr->str = (CHAR *)cel_realloc(vstr->str, size * cw)) == NULL)
        return -1;
    vstr->capacity = size;
    return 0;
}
