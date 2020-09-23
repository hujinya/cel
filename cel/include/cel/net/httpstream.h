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
#ifndef __CEL_NET_HTTPSTREAM_H__
#define __CEL_NET_HTTPSTREAM_H__

#include "cel/types.h"
#include "cel/allocator.h"
#include "cel/stream.h"
#include "cel/vstring.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelHttpChunked
{
    BOOL last;
    int start;
    unsigned int size;
}CelHttpChunked;

static __inline 
int cel_httpchunked_init(CelHttpChunked *chunked, int start)
{
    chunked->last = FALSE;
    chunked->start = start;
    chunked->size = 0;
    return 0;
}

long cel_httpchunked_reading(CelStream *s);

static __inline 
int cel_httpchunked_get_send_position(CelHttpChunked *chunked)
{
    return chunked->start + chunked->size + 9;
}
static __inline 
void *cel_httpchunked_get_send_pointer(CelHttpChunked *chunked, CelStream *s)
{
    cel_stream_set_position(s,
        cel_httpchunked_get_send_position(chunked));
    return cel_stream_get_pointer(s);
}
#define cel_httpchunked_get_send_buffer_size(chunked, s) \
    (cel_stream_get_remaining_capacity(s) - 11 - 5)
#define cel_httpchunked_remaing_resize(chunked, s, resize) \
    cel_stream_remaining_resize(s, resize + 11 + 5)
static __inline void cel_httpchunked_send_seek(CelHttpChunked *chunked, int offset)
{
    chunked->size += offset;
}
static __inline void cel_httpchunked_set_last(CelHttpChunked *chunked, BOOL is_last)
{
	chunked->last = TRUE;
}
static __inline BOOL cel_httpchunked_is_last(CelHttpChunked *chunked)
{
	return chunked->last;
}
long cel_httpchunked_writing_last(CelHttpChunked *chunked, CelStream *s);

typedef struct _CelHttpStreamBuf
{
    const void *buf;
    size_t size;
}CelHttpStreamBuf;

typedef struct _CelHttpStreamFmtArgs
{
    const char *fmt;
    va_list args;
    int size;
}CelHttpStreamFmtArgs;

typedef struct CelHttpStream
{
	CelStream s;
	BOOL is_chunked;
	CelHttpChunked chunked;
}CelHttpStream;

typedef int (* CelHttpStreamWriteCallBack)(CelHttpStream *hs, void *user_data);

int cel_httpstream_init(CelHttpStream *hs, size_t size);
void cel_httpstream_destroy(CelHttpStream *hs);

void cel_httpstream_clear(CelHttpStream *hs);

int cel_httpstream_reading(CelHttpStream *hs);
int cel_httpstream_writing(CelHttpStream *hs);

static __inline int cel_httpstream_remaining_resize(CelHttpStream *hs, size_t new_size)
{
	return (hs->is_chunked 
		? cel_httpchunked_remaing_resize(&(hs->chunked), &(hs->s), new_size) 
		: cel_stream_remaining_resize(&(hs->s), new_size));
}

static __inline int cel_httpstream_get_position(CelHttpStream *hs)
{
	return (hs->is_chunked
		? hs->chunked.start + hs->chunked.size + 9
		: hs->s.pointer - hs->s.buffer);
}
static __inline void *cel_httpstream_get_pointer(CelHttpStream *hs)
{
	if (hs->is_chunked )
		cel_stream_set_position(&(hs->s),
		cel_httpchunked_get_send_position(&(hs->chunked)));
    return cel_stream_get_pointer(&(hs->s));
}
static __inline size_t cel_httpsteam_get_write_buffer_size(CelHttpStream *hs)
{
	return (hs->is_chunked
		? (cel_stream_get_remaining_capacity(&(hs->s)) - 11 - 5)
		: cel_stream_get_remaining_capacity(&(hs->s)));
}
static __inline void cel_httpstream_write_seek(CelHttpStream *hs, int offset)
{
	if (hs->is_chunked)
		hs->chunked.size += offset;
	cel_stream_seek(&(hs->s), offset);
}

int cel_httpstream_write(CelHttpStream *hs, CelHttpStreamBuf *buf);
int cel_httpstream_printf(CelHttpStream *hs, CelHttpStreamFmtArgs *fmt_args);
int cel_httpstream_send_file(CelHttpStream *hs, FILE *fp);

static __inline void cel_httpstream_write_end(CelHttpStream *hs)
{
	if (hs->is_chunked)
	{
		cel_httpchunked_writing_last(&(hs->chunked), &(hs->s));
	}
}


#ifdef __cplusplus
}
#endif

#endif
