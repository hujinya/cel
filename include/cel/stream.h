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
#ifndef __CEL_STREAM_H__
#define __CEL_STREAM_H__

#include "cel/types.h"
#include "cel/byteorder.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelStream
{
    BYTE *buffer, *pointer;
    size_t length, capacity;
}CelStream;

int cel_stream_init(CelStream *s);
void cel_stream_destroy(CelStream *s);

CelStream *cel_stream_new(void);
void cel_stream_free(CelStream *s);

#define cel_stream_get_buffer(_s) (_s)->buffer
#define cel_stream_set_buffer(_s, _b) (_s)->buffer = (_b)
#define cel_stream_get_pointer(_s) (_s)->pointer
#define _cel_stream_set_pointer(_s, _p) (_s)->pointer = (_p)
#define cel_stream_get_length(_s) (_s)->length
#define cel_stream_set_length(_s, _l) (_s)->length = (_l)
#define _cel_stream_get_capacity(_s) (_s)->capacity
#define _cel_stream_set_capacity(_s, _c) (_s)->capacity = _c
#define cel_stream_get_position(_s) ((_s)->pointer - _s->buffer)
#define cel_stream_set_position(_s, _p) (_s)->pointer = (_s)->buffer + (_p)
#define cel_stream_seal_length(_s) \
    ((_s)->length = ((_s)->pointer - (_s)->buffer))
#define cel_stream_get_remaining_length(_s) \
    ((_s)->length - ((_s)->pointer - (_s)->buffer))
#define cel_stream_get_remaining_capacity(_s) \
    ((_s)->capacity - ((_s)->pointer - (_s)->buffer))

int _cel_stream_resize(CelStream *s, size_t _size);
static __inline int cel_stream_resize(CelStream *_s, size_t _size)
{
    return ((_size >= (_s)->capacity) ? _cel_stream_resize(_s, _size) : 0);
}
static __inline int cel_stream_remaining_resize(CelStream *_s, size_t _size)
{
    return ((cel_stream_get_position(_s) + _size) > 
        _cel_stream_get_capacity(_s)
    ? _cel_stream_resize(_s, cel_stream_get_position(_s) + _size) : 0);
};

#define cel_stream_seek(_s, _offset) (_s)->pointer += (_offset)
static __inline int cel_stream_safe_seek(CelStream *_s, size_t _size) 
{
    if (cel_stream_get_remaining_capacity(_s) < _size)
        return FALSE;
    cel_stream_seek(_s, _size);
    return TRUE;
}

#define cel_stream_read_u8(_s, _v) \
    _v = *((U8 *)(_s->pointer)), cel_stream_seek(_s, 1)
#define cel_stream_read_u16(_s, _v) \
    CEL_GET_LE16(_v, _s->pointer, 0), cel_stream_seek(_s, 2) 
#define cel_stream_read_u16_be(_s, _v) \
    CEL_GET_BE16(_v, _s->pointer, 0), cel_stream_seek(_s, 2)
#define cel_stream_read_u32(_s, _v) \
    CEL_GET_LE32(_v, _s->pointer, 0), cel_stream_seek(_s, 4) 
#define cel_stream_read_u32_be(_s, _v) \
    CEL_GET_BE32(_v, _s->pointer, 0), cel_stream_seek(_s, 4)
#define cel_stream_read_u64(_s, _v) \
    CEL_GET_LE64(_v, _s->pointer, 0), cel_stream_seek(_s, 8)
#define cel_stream_read_u64_be(_s, _v) \
    CEL_GET_BE64(_v, _s->pointer, 0), cel_stream_seek(_s, 8)

static __inline void cel_stream_read(CelStream *_s, void* _b, size_t _n)
{
    memcpy(_b, (_s->pointer), (_n));
    cel_stream_seek(_s, _n);
}

#define cel_stream_write_u8(_s, _v) \
        *((U8 *)(_s->pointer)) = _v, cel_stream_seek(_s, 1)
#define cel_stream_write_u16(_s, _v) \
        CEL_PUT_LE16(_v, _s->pointer, 0), cel_stream_seek(_s, 2)
#define cel_stream_write_u16_be(_s, _v) \
        CEL_PUT_BE16(_v, _s->pointer, 0), cel_stream_seek(_s, 2)
#define cel_stream_write_u32(_s, _v) \
        CEL_PUT_LE32(_v, _s->pointer, 0), cel_stream_seek(_s, 4)
#define cel_stream_write_u32_be(_s, _v)  \
        CEL_PUT_BE32(_v, _s->pointer, 0), cel_stream_seek(_s, 4)
#define cel_stream_write_u64(_s, _v) \
        CEL_PUT_LE64(_v, _s->pointer, 0), cel_stream_seek(_s, 8)
#define cel_stream_write_u64_be(_s, _v) \
        CEL_PUT_BE64(_v, _s->pointer, 0), cel_stream_seek(_s, 8)

static __inline int cel_stream_write(CelStream *_s, const void* _b, size_t _n)
{
    memcpy(_s->pointer, (_b), (_n));
    cel_stream_seek(_s, _n);
    return _n;
}
int cel_stream_printf(CelStream *_s, const char *fmt, ...);
static __inline void cel_stream_zero(CelStream *_s, size_t _n)
{
    memset(_s->pointer, '\0', (_n));
    cel_stream_seek(_s, _n);
}
static __inline void cel_stream_fill(CelStream *_s, int _v, size_t _n)
{
    memset(_s->pointer, _v, (_n));
    cel_stream_seek(_s, _n);
}

static __inline void cel_stream_copy(CelStream *_src, CelStream *_dst, size_t _n)
{
    memcpy(_dst->pointer, _src->pointer, _n);
    cel_stream_seek(_dst, _n);
    cel_stream_seek(_src, _n);
}

static __inline void cel_stream_clear(CelStream *_s)
{
    _s->length = 0;
    _s->pointer = _s->buffer;
    if (_s->buffer != NULL)
        (*(char *)(_s->buffer)) = '\0';
}

#ifdef __cplusplus
}
#endif

#endif
