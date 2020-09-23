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
#include "cel/net/httpstream.h"
#include "cel/error.h"
#include "cel/log.h"
#include <stdarg.h>

/*
       Chunked-Body   = *chunk
                        last-chunk
                        trailer
                        CRLF

       chunk          = chunk-size [ chunk-extension ] CRLF
                        chunk-data CRLF
       chunk-size     = 1*HEX
       last-chunk     = 1*("0") [ chunk-extension ] CRLF

       chunk-extension= *( ";" chunk-ext-name [ "=" chunk-ext-val ] )
       chunk-ext-name = token
       chunk-ext-val  = token | quoted-string
       chunk-data     = chunk-size(OCTET)
       trailer        = *(entity-header CRLF)
*/
long cel_httpchunked_reading(CelStream *s)
{
    int position;
    size_t ch = 0, ch1;
    char *ptr;
    long chunk_size;

    position = cel_stream_get_position(s);
    if (cel_stream_get_remaining_length(s) < 3)
    {
        cel_stream_set_position(s, position);
        return -1;
    }
    /* Read chunk size */
    chunk_size = strtol((char *)cel_stream_get_pointer(s), &ptr, 16);
    if ((BYTE *)ptr - s->buffer > (int)s->length)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Http chunk decode error.")));
        return -2;
    }
    //printf("chunk %ld %ld\r\n", (int)s->length, (BYTE *)ptr - s->buffer);
    _cel_stream_set_pointer(s, (BYTE *)ptr);
    /* Read chunk extension */
    while (TRUE)
    {
        //puts((char *)s->pointer);
        if (cel_stream_get_remaining_length(s) < 1)
        {
            cel_stream_set_position(s, position);
            return -1;
        }
        ch1 = ch;
        cel_stream_read_u8(s, ch);
        if (ch == '\n' && ch1 == '\r')
            break;
    }
    /* Not last chunk */
    if (chunk_size > 0)
    {
        return chunk_size;
    }
    /* Last chunk */
    else if (chunk_size == 0)
    {
        /* Read trailer */
        while (TRUE)
        {
            if (cel_stream_get_remaining_length(s) < 1)
            {
                cel_stream_set_position(s, position);
                return -1;
            }
            ch1 = ch;
            cel_stream_read_u8(s, ch);
            if (ch == '\n' && ch1 == '\r')
            {
                return 0;
            }
        }
    }
    else
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Http chunk decode error.")));
        return -2;
    }
}

long cel_httpchunked_writing_last(CelHttpChunked *chunked, CelStream *s)
{
    cel_stream_set_position(s, chunked->start);
    if (chunked->size > 0)
    {
        sprintf((char *)cel_stream_get_pointer(s), "%07x", chunked->size);
        cel_stream_seek(s, 7);
        cel_stream_write(s, "\r\n", 2);
        cel_stream_seek(s, chunked->size);
        cel_stream_write(s, "\r\n", 2);
    }
    cel_stream_write_u8(s, '0');
    cel_stream_write(s, "\r\n\r\n", 4);
    //puts((char *)s->buffer);
    return chunked->size;
}

int cel_httpstream_init(CelHttpStream *hs, size_t size)
{
	cel_stream_init(&(hs->s));
	cel_stream_resize(&(hs->s), size);
	hs->is_chunked = FALSE;
	cel_httpchunked_init(&(hs->chunked), 0);
	return 0;
}

void cel_httpstream_destroy(CelHttpStream *hs)
{
	cel_httpchunked_init(&(hs->chunked), 0);
	hs->is_chunked = FALSE;
	cel_stream_destroy(&(hs->s));
}

void cel_httpstream_clear(CelHttpStream *hs)
{
	cel_httpchunked_init(&(hs->chunked), 0);
	hs->is_chunked = FALSE;
	cel_stream_clear(&(hs->s));
}

int cel_httpstream_write(CelHttpStream *hs, CelHttpStreamBuf *buf)
{
    if (cel_httpsteam_get_write_buffer_size(hs) < buf->size)
		cel_httpstream_remaining_resize(hs, buf->size);
    cel_stream_write(&(hs->s), buf->buf, buf->size);
	if (hs->is_chunked)
		hs->chunked.size += buf->size;
    return buf->size;
}

int cel_httpstream_printf(CelHttpStream *hs, CelHttpStreamFmtArgs *fmt_args)
{
	int remaining_size;
	//puts(fmt_args->fmt);

	remaining_size = cel_httpsteam_get_write_buffer_size(hs);
	fmt_args->size = vsnprintf((char *)cel_httpstream_get_pointer(hs), 
		remaining_size, fmt_args->fmt, fmt_args->args);
	if (fmt_args->size < 0 || fmt_args->size >= remaining_size)
	{
		if (fmt_args->size > 0)
			cel_httpstream_remaining_resize(hs, fmt_args->size);
		else
			cel_httpstream_remaining_resize(hs, remaining_size + 1);
		/* printf("cel_httprequest_body_printf remaining %d capacity %d\r\n", 
		(int)cel_stream_get_remaining_capacity(s), (int)s->capacity);*/
		remaining_size = cel_stream_get_remaining_capacity(&(hs->s));
		fmt_args->size = vsnprintf((char *)cel_stream_get_pointer(&(hs->s)), 
			remaining_size, fmt_args->fmt, fmt_args->args);
		if (fmt_args->size < 0 || fmt_args->size >= remaining_size)
			return -1;
	}
	cel_stream_seek(&(hs->s), fmt_args->size);
	if (hs->is_chunked)
		hs->chunked.size += fmt_args->size;
	return fmt_args->size;
}

int cel_httpstream_send_file(CelHttpStream *hs, FILE *fp)
{
    size_t size;

    if ((size = fread(cel_stream_get_pointer(&(hs->s)), 1,
        cel_stream_get_remaining_capacity(&(hs->s)), fp)) <= 0)
        return -1;
    cel_stream_seek(&(hs->s), size);
    /*_tprintf(_T("cel_httprequest_body_file capacity %d, size %d\r\n"), 
        cel_stream_get_remaining_capacity(s), size);*/
    return size;
}
