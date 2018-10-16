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
#include "cel/stream.h"
#include "cel/allocator.h"
#include <stdarg.h>

int cel_stream_init(CelStream *s)
{
    s->buffer = NULL;
    s->pointer = NULL;
    s->capacity = 0;
    s->length = 0;
    return 0;
}

void cel_stream_destroy(CelStream *s)
{
    if (s->buffer != NULL)
    {
        cel_free(s->buffer);
        s->buffer = NULL;
    }
    s->pointer = NULL;
    s->capacity = 0;
    s->length = 0;
}

int _cel_stream_resize(CelStream *s, size_t size)
{
    size_t position, length;

    if (size >= CEL_CAPACITY_MAX)
        return -1;
    position = cel_stream_get_position(s);
    length = cel_stream_get_length(s);

    size = cel_capacity_get_min(size);
    if ((s->buffer = (BYTE *)cel_realloc(s->buffer, size)) == NULL)
        return -1;

    _cel_stream_set_capacity(s, size);
    cel_stream_set_position(s, position);
    cel_stream_set_length(s, length);
    /*_tprintf(_T("s %p buffer %p, pointer %p, length %d, capacity %d\r\n"),
        s, s->buffer, s->pointer, (int)s->length, (int)s->capacity);*/

    return 0;
}

int cel_stream_printf(CelStream *_s, const char *fmt, ...)
{
    va_list args;
    int size;

    va_start(args, fmt);
    size = vsnprintf((char *)cel_stream_get_pointer(_s), 
        cel_stream_get_remaining_capacity(_s), fmt, args);
    va_end(args);
    cel_stream_seek(_s, size);

    return size;
}
