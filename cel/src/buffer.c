/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2019 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/buffer.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

int cel_buffer_resize(CelBuffer *buf, size_t size)
{
    size_t rp, wp;

    if (size >= CEL_CAPACITY_MAX)
        return -1;
    rp = buf->p_read - buf->p_buffer;
    wp = buf->p_write - buf->p_buffer;
    size = cel_capacity_get_min(size);
    //_tprintf(_T("size %d new_size %d\r\n"), (int)buf->capacity, (int)size);
    if ((buf->p_buffer = (BYTE *)cel_realloc(buf->p_buffer, size)) == NULL)
        return -1;
    buf->p_end = buf->p_buffer + size + 1;
    buf->remaining += size - buf->capacity;
    buf->capacity = size;
    buf->p_read = buf->p_buffer + rp;
    buf->p_write = buf->p_buffer + wp; 

    return 0;
}

int cel_buffer_init(CelBuffer *buf, size_t size)
{
    memset(buf, 0, sizeof(CelBuffer));
    return cel_buffer_resize(buf, size);
}

void cel_buffer_destroy(CelBuffer *buf)
{
    if (buf->p_buffer != NULL)
    {
        cel_free(buf->p_buffer);
        buf->p_buffer = NULL;
        buf->p_end = NULL;
    }
    buf->p_read = buf->p_write = NULL;
    buf->capacity = buf->remaining = 0;
}

CelBuffer *cel_buffer_new(size_t size)
{
    CelBuffer *buf;
    
    if ((buf = (CelBuffer *)cel_malloc(sizeof(CelBuffer))) != NULL)
    {
        if (cel_buffer_init(buf, size) == 0)
            return buf;
        cel_free(buf);
    }
    return NULL;
}

void cel_buffer_free(CelBuffer *buf)
{
    cel_buffer_destroy(buf);
    cel_free(buf);
}

int cel_buffer_read(CelBuffer *buf, void *data, size_t size)
{
    size_t used_size, end_size;

    /* Ring buffer is empty */
    if ((used_size = buf->capacity - buf->remaining) == 0)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Ring buffer is empty.")));
        return 0;
    }
    if (size > used_size) size = used_size;

    if ((end_size = buf->p_end- buf->p_read) >= size)
    {
        memcpy(data, buf->p_read, size);
        buf->p_read = buf->p_read + size;
    }
    else
    {
        memcpy(data, buf->p_read, end_size);
        memcpy((char *)data + end_size, buf->p_buffer, size - end_size);
        buf->p_read = buf->p_buffer+ (size - end_size);
    }
    buf->remaining += size;

    return size;
}

size_t cel_buffer_dup(CelBuffer *buf, void *data, size_t size)
{
    size_t used_size, end_size;

    /* Ring buffer is empty */
    if ((used_size = buf->capacity - buf->remaining) == 0)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Ring buffer is empty.")));
        return 0;
    }
    if (size > used_size)
        size = used_size;

    if ((end_size = buf->p_end- buf->p_read) >= size)
    {
        memcpy(data, buf->p_read, size);
    }
    else
    {
        memcpy(data, buf->p_read, end_size);
        memcpy((char *)data + end_size, buf->p_buffer, size - end_size);
    }

    return size;
}

int cel_buffer_write(CelBuffer *buf, void *data, size_t size)
{
    size_t end_size;

    /* Ring buffer is full */
    if (buf->remaining <= 0)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Ring buffer is full.")));
        return 0;
    }
    if (size > buf->remaining)
        size = buf->remaining;
    if ((end_size = buf->p_end - buf->p_write) >= size)
    {
        memcpy(buf->p_write, data, size);
        buf->p_write = buf->p_write + size;
    }
    else
    {
        memcpy(buf->p_write, data, end_size);
        memcpy(buf->p_buffer, (char *)data + end_size, size - end_size);
        buf->p_write = buf->p_buffer + size - end_size;
    }
    buf->remaining -= size;

    return size;
}
