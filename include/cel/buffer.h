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
#ifndef __CEL_BUFFER_H__
#define __CEL_BUFFER_H__

#include "cel/types.h"
#include "cel/byteorder.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelBuffer
{
    BYTE *p_buffer, *p_end;
    BYTE *p_read, *p_write;
    size_t capacity, remaining;
}CelBuffer;


int cel_buffer_init(CelBuffer *buf, size_t size);
void cel_buffer_destroy(CelBuffer *buf);

CelBuffer *cel_buffer_new(size_t size);
void cel_buffer_free(CelBuffer *buf);

/**
 * \brief     Move ring buffer read and write pointer.
 */
static __inline void cel_buffer_read_seek(CelBuffer *buf, size_t size)
{
    size_t end_size;

    if ((end_size = (buf->p_end- buf->p_read)) >= size)
        buf->p_read = buf->p_read + size;
    else
        buf->p_read = buf->p_buffer + (size - end_size);
    buf->remaining += size;
}
static __inline void cel_buffer_write_seek(CelBuffer *buf, size_t size)
{
    size_t end_size;

    if ((end_size = buf->p_end - buf->p_write) >= size)
        buf->p_write = buf->p_write+ size;
    else
        buf->p_write = buf->p_buffer+ size - end_size;
    buf->remaining -= size;
}

int cel_buffer_resize(CelBuffer *buf, size_t size);

int cel_buffer_read(CelBuffer *buf, void *data, size_t size);
#define cel_buffer_read_u8(_buf, _v)
#define cel_buffer_read_u16(_buf, _v)
#define cel_buffer_read_u16_be(_buf, _v)
#define cel_buffer_read_u32(_buf, _v)
#define cel_buffer_read_u32_be(_buf, _v)
#define cel_buffer_read_u64(_buf, _v)
#define cel_buffer_read_u64_be(_buf, _v)

int cel_buffer_write(CelBuffer *buf, void *data, size_t size);
#define cel_buffer_write_u8(_buf, _v)
#define cel_buffer_write_u16(_buf, _v)
#define cel_buffer_write_u16_be(_buf, _v)
#define cel_buffer_write_u32(_buf, _v)
#define cel_buffer_write_u32_be(_buf, _v)
#define cel_buffer_write_u64(_buf, _v)
#define cel_buffer_write_u64_be(_buf, _v)

#define cel_buffer_get_capacity(buf) ((buf)->capacity)
#define cel_buffer_get_remaining(buf) ((buf)->remaining)
#define cel_buffer_get_length(buf) ((buf)->capacity - (buf)->remaining)
/**
 * \brief   Only copy data to buffer, not move read pointer. 
 */
size_t cel_buffer_dup(CelBuffer *buf, void *data, size_t size);

/**
 * \brief   Get current read and write pointer.
 */
/* void *cel_buffer_get_start_cursor(CelBuffer *buf)*/
#define cel_buffer_get_start_cursor(buf) ((buf)->p_buffer)
/* void *cel_buffer_get_read_cursor(CelBuffer *buf)*/
#define cel_buffer_get_read_cursor(buf) ((buf)->p_read)
/* void *cel_buffer_get_write_cursor(CelBuffer *buf)*/
#define cel_buffer_get_write_cursor(buf) ((buf)->p_write)

static __inline size_t cel_buffer_get_read_end_size(CelBuffer *buf)
{
    return buf->p_end - buf->p_read;
}
static __inline size_t cel_buffer_get_write_end_size(CelBuffer *buf)
{
    return buf->p_end - buf->p_write;
}

static __inline void cel_buffer_clear(CelBuffer *buf)
{
    buf->p_read = buf->p_write = buf->p_buffer;
    buf->remaining = buf->capacity;
}

#ifdef __cplusplus
}
#endif

#endif
