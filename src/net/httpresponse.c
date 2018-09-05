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
#include "cel/net/httpresponse.h"
#include "cel/error.h"
#include "cel/log.h"
#include "cel/datetime.h"
#include "cel/keyword.h"
#include "cel/convert.h"
#include "cel/file.h"
#include <stdarg.h>


#define CEL_HTTP_STATUS_CODE_MAX       528

typedef struct _CelHttpResponseStatus
{
    const char *code;
    const char *reason_phrase;
}CelHttpResponseStatus;

static CelHttpResponseStatus 
s_rspstatus[CEL_HTTP_STATUS_CODE_MAX / 100][CEL_HTTP_STATUS_CODE_MAX % 100] = 
{
    { /* 1XX */
        {"100", "Continue"}, 
        {"101", "Switching Protocols"}
    }, 
    { /* 2XX */
        {"200", "OK"}, 
        {"201", "Created"}, 
        {"202", "Accepted"}, 
        {"203", "Non-Authoritative Information"}, 
        {"204", "No Content"}, 
        {"205", "Reset Content"},
        {"206", "Partial Content"}
    }, 
    { /* 3XX */
        {"300", "Multiple Choices"}, 
        {"301", "Moved Permanently"}, 
        {"302", "Found"}, 
        {"303", "See Other"}, 
        {"304", "Not Modified"}, 
        {"305", "Use Proxy"}, 
        {"306", "Switch Proxy"}, 
        {"307", "Temporary Redirect"}
    }, 
    { /* 4XX */
        {"400", "Bad Request"}, 
        {"401", "Unauthorized"}, 
        {"402", "Payment Required"}, 
        {"403", "Forbidden"}, 
        {"404", "Not Found"}, 
        {"405", "Method Not Allowed"}, 
        {"406", "Not Acceptable"}, 
        {"407", "Proxy Authentication Required"}, 
        {"408", "Request Timeout"}, 
        {"409", "Conflict"}, 
        {"410", "Gone"}, 
        {"411", "Length Required"}, 
        {"412", "Precondition Failed"}, 
        {"413", "Request Entity Too Large"}, 
        {"414", "Request-URI Too Long"}, 
        {"415", "Unsupported Media Type"}, 
        {"416", "Requested Range Not Satisfiable"}, 
        {"417", "Expectation Failed"}, 
        {"418", NULL}, 
        {"419", NULL}, 
        {"420", NULL}, 
        {"421", "There are too many connections from your internet address"}, 
        {"422", "Unprocessable Entity"}, 
        {"423", "Locked"}, 
        {"425", "Failed Dependency"}, 
        {"426", "Unordered Collection"}, 
        {"427", "Upgrade Required"}, 
        {"428", "Retry With"}
    }, 
    { /* 5XX */
        {"500", "Internal Server Error"}, 
        {"501", "Not Implemented"}, 
        {"502", "Bad Gateway"}, 
        {"503", "Service Unavailable"}, 
        {"504", "Gateway Timeout"}, 
        {"505", "HTTP Version Not Supported"}
    }
};

static size_t s_httprsphdr_offset[] = 
{ 
    0, 0, 0, 0,
    CEL_OFFSET(CelHttpResponse, accept_ranges),

    CEL_OFFSET(CelHttpResponse, access_control_allow_credentials), 
    CEL_OFFSET(CelHttpResponse, access_control_allow_headers),
    CEL_OFFSET(CelHttpResponse, access_control_allow_methods), 
    CEL_OFFSET(CelHttpResponse, access_control_allow_origin), 
    CEL_OFFSET(CelHttpResponse, access_control_expose_headers), 

    CEL_OFFSET(CelHttpResponse, access_control_max_age), 
    0, 0,
    CEL_OFFSET(CelHttpResponse, age), 
    CEL_OFFSET(CelHttpResponse, allow),

    0,
    CEL_OFFSET(CelHttpResponse, cache_control),
    CEL_OFFSET(CelHttpResponse, connection),
    CEL_OFFSET(CelHttpResponse, content_disposition), 
    CEL_OFFSET(CelHttpResponse, content_encoding),

    CEL_OFFSET(CelHttpResponse, content_language),
    CEL_OFFSET(CelHttpResponse, content_length),
    CEL_OFFSET(CelHttpResponse, content_location),
    CEL_OFFSET(CelHttpResponse, content_md5),
    CEL_OFFSET(CelHttpResponse, content_range), 

    CEL_OFFSET(CelHttpResponse, content_type),
    0,
    CEL_OFFSET(CelHttpResponse, date),
    0,
    CEL_OFFSET(CelHttpResponse, etag),

    0,
    CEL_OFFSET(CelHttpResponse, expires),
    0, 0, 0,

    0, 0, 0, 0,
    CEL_OFFSET(CelHttpResponse, last_modified),

    CEL_OFFSET(CelHttpResponse, location),
    0, 0, 
    CEL_OFFSET(CelHttpResponse, pragma),
    CEL_OFFSET(CelHttpResponse, proxy_authenticate), 

    0, 0, 0,
    CEL_OFFSET(CelHttpResponse, retry_after),
    CEL_OFFSET(CelHttpResponse, server),

    CEL_OFFSET(CelHttpResponse, set_cookie),
    0,
    CEL_OFFSET(CelHttpResponse, trailer),
    CEL_OFFSET(CelHttpResponse, transfer_encoding), 
    CEL_OFFSET(CelHttpResponse, upgrade),

    0, 0,
    CEL_OFFSET(CelHttpResponse, vary),
    CEL_OFFSET(CelHttpResponse, via),
    CEL_OFFSET(CelHttpResponse, warning),

    CEL_OFFSET(CelHttpResponse, www_authenticate),
    0, 0, 0, 0,

    CEL_OFFSET(CelHttpResponse, x_powered_by), 0, 0, 
};

static __inline const char *cel_http_get_statuscode(CelHttpStatusCode status)
{
    return (status > CEL_HTTP_STATUS_CODE_MAX 
        ? NULL : s_rspstatus[status / 100 - 1][status % 100].code);
}

static __inline const char *cel_http_get_reasonphrase(CelHttpStatusCode status)
{
    return (status > CEL_HTTP_STATUS_CODE_MAX 
        ? NULL : s_rspstatus[status / 100 - 1][status % 100].reason_phrase);
}

int cel_httpresponse_init(CelHttpResponse *rsp)
{
    int i = 0;
    CelHttpHeaderHandler *handler;

    rsp->ver = CEL_HTTPVER_11;
    rsp->status = CEL_HTTPSC_REQUEST_OK;
    cel_vstring_init_a(&(rsp->reason));
    rsp->hdr_flags = ULL(0);
    while (i < CEL_HTTPHDR_COUNT)
    {
        if (s_httprsphdr_offset[i] != 0)
        {
            //printf("cel_httpresponse_init i= %d\r\n", i);
            handler = (CelHttpHeaderHandler *)g_case_httphdr[i].value2;
            if (handler->init_func == NULL)
                memset((char *)rsp + s_httprsphdr_offset[i], 0, handler->size);
            else
                handler->init_func((char *)rsp + s_httprsphdr_offset[i]);
        }
        i++;
    }
    cel_rbtree_init(
        &(rsp->ext_hdrs), (CelCompareFunc)strcmp, cel_free, cel_free);
    rsp->body_save_in = CEL_HTTPBODY_SAVE_IN_CACHE;
    cel_httpbodycache_init(&(rsp->body_cache), CEL_HTTPBODY_BUF_LEN_MAX);

    rsp->reading_state = CEL_HTTPRESPONSE_READING_INIT;
    rsp->reading_error = 0;
    rsp->reading_hdr_offset = 0;
    rsp->reading_body_offset = 0;
    rsp->body_reading_callback = NULL;

    rsp->writing_state = CEL_HTTPRESPONSE_WRITING_INIT;
    rsp->writing_error = 0;
    rsp->writing_body_offset = 0;
    rsp->body_writing_callback = NULL;

    cel_stream_init(&(rsp->s));
    cel_stream_resize(&(rsp->s), CEL_HTTPRESPONSE_STREAM_BUFFER_SIZE);

    return 0;
}

void cel_httpresponse_clear(CelHttpResponse *rsp)
{
    int i = 0;
    CelHttpHeaderHandler *handler;

    rsp->ver = CEL_HTTPVER_11;
    rsp->status = CEL_HTTPSC_REQUEST_OK;
    cel_vstring_clear(&(rsp->reason));
    rsp->hdr_flags = ULL(0);
    while (i < CEL_HTTPHDR_COUNT)
    {
        if (s_httprsphdr_offset[i] != 0)
        {
            handler = (CelHttpHeaderHandler *)g_case_httphdr[i].value2;
            if (handler->destroy_func == NULL)
                memset((char *)rsp + s_httprsphdr_offset[i],
                0, handler->size);
            else
                handler->destroy_func(
                (char *)rsp + s_httprsphdr_offset[i]);
        }
        i++;
    }
    cel_rbtree_clear(&(rsp->ext_hdrs));
    rsp->body_save_in = CEL_HTTPBODY_SAVE_IN_CACHE;
    cel_httpbodycache_clear(&(rsp->body_cache));

    rsp->reading_state = CEL_HTTPRESPONSE_READING_INIT;
    rsp->reading_error = 0;
    rsp->reading_hdr_offset = 0;
    rsp->reading_body_offset = 0;
    rsp->body_reading_callback = NULL;
    
    rsp->writing_state = CEL_HTTPRESPONSE_WRITING_INIT;
    rsp->writing_error = 0;
    rsp->writing_body_offset = 0;
    rsp->body_writing_callback = NULL;

    cel_stream_clear(&(rsp->s));
}

void cel_httpresponse_destroy(CelHttpResponse *rsp)
{
    int i = 0;
    CelHttpHeaderHandler *handler;

    rsp->ver = CEL_HTTPVER_11;
    rsp->status = CEL_HTTPSC_REQUEST_OK;
    cel_vstring_destroy_a(&(rsp->reason));
    rsp->hdr_flags = ULL(0);
    while (i < CEL_HTTPHDR_COUNT)
    {
        if (s_httprsphdr_offset[i] != 0)
        {
            handler = (CelHttpHeaderHandler *)g_case_httphdr[i].value2;
            if (handler->destroy_func == NULL)
                memset((char *)rsp + s_httprsphdr_offset[i],
                0, handler->size);
            else
                handler->destroy_func(
                (char *)rsp + s_httprsphdr_offset[i]);
        }
        i++;
    }
    cel_rbtree_destroy(&(rsp->ext_hdrs));
    rsp->body_save_in = CEL_HTTPBODY_SAVE_IN_CACHE;
    cel_httpbodycache_destroy(&(rsp->body_cache));

    rsp->reading_state = CEL_HTTPRESPONSE_READING_INIT;
    rsp->reading_error = 0;
    rsp->reading_hdr_offset = 0;
    rsp->reading_body_offset = 0;
    rsp->body_reading_callback = NULL;
    
    rsp->writing_state = CEL_HTTPRESPONSE_WRITING_INIT;
    rsp->writing_error = 0;
    rsp->writing_body_offset = 0;
    rsp->body_writing_callback = NULL;

    cel_stream_destroy(&(rsp->s));
}

CelHttpResponse *cel_httpresponse_new(void)
{
    CelHttpResponse *rsp;

    if ((rsp = (CelHttpResponse *)cel_malloc(sizeof(CelHttpResponse))) != NULL)
    {
        if (cel_httpresponse_init(rsp) == 0)
            return rsp;
        cel_free(rsp);
    }
    return NULL;
}

void cel_httpresponse_free(CelHttpResponse *rsp)
{
    cel_httpresponse_destroy(rsp); 
    cel_free(rsp);
}

static int cel_httpresponse_reading_header(CelHttpResponse *rsp, CelStream *s)
{
    int key_start, key_end, value_start, value_end;
    BYTE ch;
    char *key, *value;
    size_t key_len, value_len;
    int hdr_index;
    CelHttpHeaderHandler *handler;

    key_start = value_start = cel_stream_get_position(s);
    //puts(&buf[*cursor]);
    while (cel_stream_get_remaining_length(s) > 1)
    {
        cel_stream_read_u8(s, ch);
        if ((char)ch == ':')
        {
            cel_stream_read_u8(s, ch);
            if ((char)ch == ' ')
            {
                value_start = cel_stream_get_position(s);
                key_end = value_start - 2;
            }
        }
        else if ((char)ch == '\r')
        {
            cel_stream_read_u8(s, ch);
            if ((char)ch == '\n')
            {
                value_end = cel_stream_get_position(s) - 2;
                /*printf("key_start %p key_end %p value_start %p "
                    "value end = %p \r\n",
                    key_start, key_end, value_start, value_end);*/
                if (value_end - key_start <= 3)
                {
                    //puts("Header ok");
                    rsp->reading_hdr_offset += (value_end - key_start);
                    return 0;
                }
               /* printf("Key len %d value len %d\r\n", 
                    key_end - key_start, value_end - value_start);
                printf("Key_start %s\r\n", (char *)key_start);*/
                if ((key_len = key_end - key_start) <= 0
                    || (value_len = value_end - value_start) <= 0
                    || (hdr_index = cel_keyword_case_search_a(
                    g_case_httphdr, CEL_HTTPHDR_COUNT, 
                    (char *)(cel_stream_get_buffer(s) + key_start),
                    key_len)) == -1)
                {
                    key = cel_strdup_full(
                        (char *)(cel_stream_get_buffer(s) + key_start), 0, key_len);
                    value = cel_strdup_full(
                        (char *)(cel_stream_get_buffer(s) + value_start), 0, value_len);
                    cel_rbtree_insert(&(rsp->ext_hdrs), key, value);
                    /*printf("Http response header '%.*s' undefined.\r\n", 
                    (int)value_end - key_start, 
                    (char *)(cel_stream_get_buffer(s) + key_start));*/
                }
                else 
                {
                    if (s_httprsphdr_offset[hdr_index] == 0)
                    {
                        CEL_WARNING(("Http response header '%.*s' "
                            "call back is null",
                            (int)value_end - key_start,
                            (char *)(cel_stream_get_buffer(s) + key_start)));
                    }
                    else
                    {
                        handler = (CelHttpHeaderHandler *)
                            g_case_httphdr[hdr_index].value2;
                        if (handler->reading_func(
                            (char *)rsp + s_httprsphdr_offset[hdr_index], 
                            (char *)(cel_stream_get_buffer(s) + value_start), 
                            value_len) == -1)
                            CEL_WARNING(("Http response header '%.*s' "
                            "reading return -1(%s)", 
                            (int)value_end - key_start,
                            (char *)(cel_stream_get_buffer(s) + key_start), 
                            cel_geterrstr(cel_sys_geterrno())));
                        else
                            CEL_SETFLAG(rsp->hdr_flags, (ULL(1) << hdr_index));
                    }
                }
            }
            rsp->reading_hdr_offset += (value_end - key_start);
            key_start = cel_stream_get_position(s);
        }
    }
    cel_stream_set_position(s, key_start);
    return -1;
}

static long cel_httpresponse_reading_body_content(CelHttpResponse *rsp, 
                                                  CelStream *s, long len)
{
    long size;

    if (rsp->content_length > CEL_HTTPBODY_CACHE_LEN_MAX
        || len > CEL_HTTPBODY_CACHE_LEN_MAX - rsp->reading_body_offset)
    {
        CEL_ERR((_T("Httpresponse body too large.")));
        return -1;
    }
    if (rsp->body_reading_callback != NULL)
    {
        size = rsp->body_reading_callback(
            rsp, s, len, rsp->body_reading_user_data);
    }
    else
    {
        size = cel_httpbodycache_reading(&(rsp->body_cache), 
            (char *)cel_stream_get_pointer(s), (size_t)len);
        cel_stream_seek(s, size);
    }
    rsp->reading_body_offset += size;

    return size;
}

static int cel_httpresponse_reading_body(CelHttpResponse *rsp, CelStream *s)
{
    long len1, len2;
    long chunk_size;

    /*printf("body offset/length %lld/%lld,\r\n",
    rsp->reading_body_offset, rsp->content_length);*/
start:
    if ((len1 = (long)(rsp->content_length - rsp->reading_body_offset)) <= 0)
    {
        if (rsp->transfer_encoding != CEL_HTTPTE_CHUNKED)
        {
            return 0;
        }
        else
        {
            if ((chunk_size = cel_httpchunked_reading(s)) == 0)
                return 0;
            else if (chunk_size < 0)
            {
                if (chunk_size == -2)
                    rsp->reading_state = CEL_HTTP_ERROR;
                return -1;
            }
            else 
            {
                /*printf("chunk_size = %d %s\r\n", 
                    chunk_size, (char *)s->pointer);*/
                rsp->content_length += chunk_size;
                len1 = (long)(rsp->content_length - rsp->reading_body_offset);
            }
        }
    }
    //printf("len2 = %d\r\n", cel_stream_get_remaining_length(s));
    if ((len2 = cel_stream_get_remaining_length(s)) >= len1)
    {
        if (cel_httpresponse_reading_body_content(rsp, s, len1) != len1)
        {
            CEL_ERR((_T("cel_httpresponse_reading_body_content error")));
            rsp->reading_state = CEL_HTTP_ERROR;
            return -1;
        }
        if (rsp->transfer_encoding == CEL_HTTPTE_CHUNKED)
            goto start;
        return 0;
    }
    else
    {
        if (len2 > 0)
        {
            if (cel_httpresponse_reading_body_content(rsp, s, len2) != len2)
            {
                CEL_ERR((_T("cel_httpresponse_reading_body_content error")));
                rsp->reading_state = CEL_HTTP_ERROR;
                return -1;
            }
        }
        return -1;
    }
}

int cel_httpresponse_reading(CelHttpResponse *rsp, CelStream *s)
{
    size_t i;
    int start, end;
    BYTE ch, ch1;

    switch (rsp->reading_state)
    {
    case CEL_HTTPRESPONSE_READING_INIT:
        rsp->reading_state = CEL_HTTPRESPONSE_READING_VERSION;
    case CEL_HTTPRESPONSE_READING_VERSION:
        //puts("CEL_HTTPRESPONSE_READING_VERSION");
        start = cel_stream_get_position(s);
        do 
        {
            if (cel_stream_get_remaining_length(s) < 1)
            {
                if (cel_stream_get_position(s) - start > 
                    CEL_HTTPVERSION_LEN)
                    rsp->reading_state = CEL_HTTP_ERROR;
                cel_stream_set_position(s, start);
                return -1;
            }
            cel_stream_read_u8(s, ch);
        }while ((char)ch != ' ');
        end = cel_stream_get_position(s);
        if ((i = cel_keyword_binary_search_a(g_httpversion, CEL_HTTPVER_COUNT,
            (char *)(cel_stream_get_buffer(s) + start),
            end - start - 1)) == -1)
        {
            CEL_ERR((_T("Invalid http response version.")));
            rsp->reading_state = CEL_HTTP_ERROR;
            return -1;
        }
        //printf("version %d, %s len %d\r\n", 
        //    i, buf + start, end - start - 2);
        if ((rsp->ver = (CelHttpVersion)i) == CEL_HTTPVER_11)
            rsp->connection = CEL_HTTPCON_KEEPALIVE;
        rsp->reading_state = CEL_HTTPRESPONSE_READING_STATUS;
    case CEL_HTTPRESPONSE_READING_STATUS:
        //puts("CEL_HTTPRESPONSE_READING_STATUS");
        start = cel_stream_get_position(s);
        do 
        {
            if (cel_stream_get_remaining_length(s) < 1)
            {
                if (cel_stream_get_position(s) - start > 
                    CEL_HTTPSTATUS_LEN)
                    rsp->reading_state = CEL_HTTP_ERROR;
                cel_stream_set_position(s, start);
                return -1;
            }
            cel_stream_read_u8(s, ch);
        }while ((char)ch != ' ');
        end = cel_stream_get_position(s);
        //puts(buf + start);
        if ((rsp->status = (CelHttpStatusCode)atoi(
            (char *)(cel_stream_get_buffer(s) + start))) <= 0)
        {
            CEL_ERR((_T("Invalid http response status code.")));
            rsp->reading_state = CEL_HTTP_ERROR;
            return -1;
        }
        rsp->reading_state = CEL_HTTPRESPONSE_READING_REASON;
        //printf("statsu %s\r\n", rsp->status);
    case CEL_HTTPRESPONSE_READING_REASON:
        //puts("CEL_HTTPRESPONSE_READING_REASON");
        start = cel_stream_get_position(s);
        do 
        {
            if (cel_stream_get_remaining_length(s) < 1)
            {
                if (cel_stream_get_position(s) - start > 
                    CEL_HTTPSTATUS_LEN)
                    rsp->reading_state = CEL_HTTP_ERROR;
                cel_stream_set_position(s, start);
                return -1;
            }
            ch1 = ch;
            cel_stream_read_u8(s, ch);
        }while ((char)ch != '\n' || (char)ch1 != '\r');
        end = cel_stream_get_position(s);
        cel_vstring_assign_a(&(rsp->reason), 
            (char *)(cel_stream_get_buffer(s) + start), end - start - 2);
        //printf("reason %s\r\n", rsp->reason.str);
        rsp->reading_state = CEL_HTTPRESPONSE_READING_HEADER;
    case CEL_HTTPRESPONSE_READING_HEADER:
        //puts("CEL_HTTPRESPONSE_READING_HEADER");
        if (cel_httpresponse_reading_header(rsp, s) == -1)
            return -1;
        rsp->reading_state = CEL_HTTPRESPONSE_READING_BODY;
    case CEL_HTTPRESPONSE_READING_BODY:
        //puts("CEL_HTTPRESPONSE_READING_BODY");
        if (CEL_CHECKFLAG(rsp->hdr_flags, 
            (ULL(1) << CEL_HTTPHDR_CONTENT_LENGTH))
            || CEL_CHECKFLAG(rsp->hdr_flags, 
            (ULL(1) << CEL_HTTPHDR_TRANSFER_ENCODING)))
        {
            //puts((char *)s->pointer);
            if (cel_httpresponse_reading_body(rsp, s) == -1)
                return -1;
        }
        rsp->reading_state = CEL_HTTPRESPONSE_READING_OK;
    case CEL_HTTPRESPONSE_READING_OK:
        break;
    default:
        CEL_ERR((_T("Invalid http response state %d."), rsp->reading_state));
        rsp->reading_state = CEL_HTTP_ERROR;
        return -1;
    }
    return 0;
}

static int cel_httpresponse_writing_header(CelHttpResponse *rsp, CelStream *s)
{
    int i = 0;
    CelHttpHeaderHandler *handler;

    while (i < CEL_HTTPHDR_COUNT)
    {
        /*printf("hdr flags 0x%x, i = %d offset %d %p\r\n", rsp->hdr_flags, 
            i, s_httprsphdr_offset[i], 
            handler->writing_func);*/
        if (!CEL_CHECKFLAG(rsp->hdr_flags, (ULL(1) << i))
            || s_httprsphdr_offset[i] == 0)
        {
            i++;
            continue;
        }
        //printf("hdr flags 0x%x, i = %d\r\n", rsp->hdr_flags, i);
        handler = (CelHttpHeaderHandler *)g_case_httphdr[i].value2;
        handler->writing_func((char *)g_case_httphdr[i].value, 
            (char *)rsp + s_httprsphdr_offset[i], s);
        i++;
        //puts((char *)buf);
    }
    cel_rbtree_foreach(
        &(rsp->ext_hdrs), (CelKeyValuePairEachFunc)cel_httpextheader_writing, s);
    cel_stream_write(s, "\r\n", 2);
    cel_stream_seal_length(s);
    //puts((char *)s->buffer);
    return 0;
}

static int cel_httpresponse_writing_body(CelHttpResponse *rsp, CelStream *s)
{
    size_t _size;
    long len;

    if (rsp->transfer_encoding == CEL_HTTPTE_CHUNKED)
    {
        while (TRUE)
        {
            /* hex * 7 + \r\n(chunk size) + \r\n(chunk data or footer)*/
            if (cel_stream_get_remaining_capacity(s) < 7 + 2 + 2 + 1)
            {
                rsp->writing_error = CEL_HTTP_WANT_WRITE;
                cel_stream_seal_length(s);
                return -1;
            }
            /* Chunk data */
            if (rsp->writing_body_offset != -1)
            {
                cel_stream_set_position(s, 
                    cel_httpchunked_get_send_buffer_position(&(rsp->chunked)));
                if (rsp->body_writing_callback == NULL
                    || (_size = rsp->body_writing_callback(
                    rsp, s, rsp->body_writing_user_data)) <= 0)
                {
                    rsp->writing_error = CEL_HTTP_WANT_WRITE;
                    cel_stream_seal_length(s);
                    return -1;
                }
                cel_httpchunked_send_seek(&(rsp->chunked), _size);
                cel_stream_seal_length(s);
                rsp->writing_body_offset += _size;
                rsp->content_length += _size;
                rsp->writing_error = CEL_HTTP_WANT_WRITE;
                return -1;
            }
            else
            {
                /* Last chunk */
                cel_httpchunked_writing_last(&(rsp->chunked), s);
                cel_stream_seal_length(s);
                return 0;
            }
        }
    }
    else
    {
        //printf("%d %d\r\n", rsp->content_length, rsp->writing_body_offset);
        if (rsp->writing_body_offset != -1)
        {
            while ((len = 
                (long)(rsp->content_length - rsp->writing_body_offset)) > 0)
            {
                if (cel_stream_get_remaining_capacity(s) < 1
                    || rsp->body_writing_callback == NULL
                    || (_size = rsp->body_writing_callback(
                    rsp, s, rsp->body_writing_user_data)) == 0)
                {
                    rsp->writing_error = CEL_HTTP_WANT_WRITE;
                    cel_stream_seal_length(s);
                    return -1;
                }
                rsp->writing_body_offset += _size;
            }
        }
        cel_stream_seal_length(s);
        return 0;
    }
}

int cel_httpresponse_writing(CelHttpResponse *rsp, CelStream *s)
{
    switch (rsp->writing_state)
    {
    case CEL_HTTPRESPONSE_WRITING_INIT:
        rsp->writing_state = CEL_HTTPRESPONSE_WRITING_VERSION;
    case CEL_HTTPRESPONSE_WRITING_VERSION:
    case CEL_HTTPRESPONSE_WRITING_STATUS:
    case CEL_HTTPRESPONSE_WRITING_REASON:
        /* Package response status line */
        cel_stream_printf(s, "%s %s %s\r\n", 
            g_httpversion[rsp->ver].key_word, 
            cel_http_get_statuscode(rsp->status), 
            cel_http_get_reasonphrase(rsp->status));
        //_tprintf(_T("status :%s\r\n"),(char *)(s->buffer));
        rsp->writing_state = CEL_HTTPRESPONSE_WRITING_HEADER;
    case CEL_HTTPRESPONSE_WRITING_HEADER:
        if (cel_httpresponse_writing_header(rsp, s) == -1)
            return -1;
        cel_httpchunked_init(&(rsp->chunked), cel_stream_get_position(s));
        rsp->writing_state = CEL_HTTPRESPONSE_WRITING_BODY;
    case CEL_HTTPRESPONSE_WRITING_BODY:
        //puts("CEL_HTTPRESPONSE_WRITING_BODY");
        if (cel_httpresponse_writing_body(rsp, s) == -1)
            return -1;
        rsp->writing_state = CEL_HTTPRESPONSE_WRITING_OK;
    case CEL_HTTPRESPONSE_WRITING_OK:
        break;
    default :
        CEL_ERR((_T("Invalid http response writing state %d."), 
            rsp->writing_state));
        rsp->writing_state = CEL_HTTP_ERROR;
        return -1;
    }

    return 0;
}

void *cel_httpresponse_get_header(CelHttpResponse *rsp,
                                  CelHttpHeader hdr_index)
{
    CelHttpHeaderHandler *handler;

    if (s_httprsphdr_offset[hdr_index] == 0)
    {
        CEL_ERR((_T("Http response header(%s) unsupported."), 
            g_case_httphdr[hdr_index].key_word));
        return NULL;
    }
    if (!CEL_CHECKFLAG(rsp->hdr_flags, (ULL(1) << hdr_index)))
        return NULL;
    handler = (CelHttpHeaderHandler *)g_case_httphdr[hdr_index].value2;
    return handler->get_func == NULL
        ? ((char *)rsp + s_httprsphdr_offset[hdr_index])
        : handler->get_func(
        ((char *)rsp + s_httprsphdr_offset[hdr_index]));
}

int cel_httpresponse_set_header(CelHttpResponse *rsp, 
                                CelHttpHeader hdr_index,
                                const void *value, size_t value_len)
{
    CelHttpHeaderHandler *handler;

    if (s_httprsphdr_offset[hdr_index] == 0)
    {
        rsp->writing_error = CEL_HTTP_ERROR;
        CEL_ERR((_T("Http response header(%s) unsupported."), 
            g_case_httphdr[hdr_index].key_word));
        return -1;
    }
    handler = (CelHttpHeaderHandler *)g_case_httphdr[hdr_index].value2;
    handler->set_func(
        (char *)rsp + s_httprsphdr_offset[hdr_index], value, value_len);
    //printf("xxhdr flags 0x%x\r\n", rsp->hdr_flags);
    CEL_SETFLAG(rsp->hdr_flags, (ULL(1) << hdr_index));
    //printf("yyhdr flags 0x%x\r\n", rsp->hdr_flags);

    return 0;
}

void *cel_httpresponse_get_send_buffer(CelHttpResponse *rsp)
{
    CelHttpTransferEncoding transfer_encoding = CEL_HTTPTE_CHUNKED;
    CelStream *s = &(rsp->s);

    if (rsp->writing_body_offset == 0)
    {
        cel_httpresponse_set_header(rsp, 
            CEL_HTTPHDR_TRANSFER_ENCODING, 
            &transfer_encoding, sizeof(transfer_encoding));
        cel_httpresponse_writing(rsp, &(rsp->s));
    }

    return cel_httpchunked_get_send_buffer(&(rsp->chunked), s);
}

size_t cel_httpresponse_get_send_buffer_size(CelHttpResponse *rsp)
{
    CelHttpTransferEncoding transfer_encoding = CEL_HTTPTE_CHUNKED;
    CelStream *s = &(rsp->s);

    if (rsp->writing_body_offset == 0)
    {
        cel_httpresponse_set_header(rsp, CEL_HTTPHDR_TRANSFER_ENCODING, 
            &transfer_encoding, sizeof(transfer_encoding));
        cel_httpresponse_writing(rsp, &(rsp->s));
    }
    return cel_httpchunked_get_send_buffer_size(&(rsp->chunked), s);
}

void cel_httpresponse_seek_send_buffer(CelHttpResponse *rsp, int offset)
{
    if (offset > 0)
    {
        cel_httpchunked_send_seek(&(rsp->chunked), offset);
        cel_stream_seek(&(rsp->s), offset);
        cel_stream_seal_length(&(rsp->s));
        rsp->writing_body_offset += offset;
    }
}

int cel_httpresponse_resize_send_buffer(CelHttpResponse *rsp, size_t resize)
{
    CelHttpTransferEncoding transfer_encoding = CEL_HTTPTE_CHUNKED;
    CelStream *s = &(rsp->s);

    if (rsp->writing_body_offset == 0)
    {
        cel_httpresponse_set_header(rsp, CEL_HTTPHDR_TRANSFER_ENCODING, 
            &transfer_encoding, sizeof(transfer_encoding));
        cel_httpresponse_writing(rsp, &(rsp->s));
    }
    return cel_httpchunked_resize_send_buffer(&(rsp->chunked), s, resize);
}

typedef struct _CelHttpResponseBuf
{
    const void *buf;
    size_t size;
}CelHttpResponseBuf;

static int cel_httpresponse_body_write(CelHttpResponse *rsp, 
                                       CelStream *s, 
                                       CelHttpResponseBuf *rsp_buf)
{
    if (cel_stream_get_remaining_capacity(s) < rsp_buf->size)
    {
        cel_stream_remaining_resize(s, rsp_buf->size);
        /*printf("cel_httpresponse_body_write resize "
            "remaing %d capacity %d buf_size %d\r\n", 
            (int)cel_stream_get_remaining_capacity(s), 
            (int)s->capacity, (int)rsp_buf->size);*/
    }
    cel_stream_write(s, rsp_buf->buf, rsp_buf->size);
    return rsp_buf->size;
}

int cel_httpresponse_write(CelHttpResponse *rsp, const void *buf, size_t size)
{
    CelHttpTransferEncoding transfer_encoding = CEL_HTTPTE_CHUNKED;
    CelHttpResponseBuf rsp_buf;

    if (rsp->writing_body_offset == 0)
    {
        cel_httpresponse_set_header(rsp, 
            CEL_HTTPHDR_TRANSFER_ENCODING,
            &transfer_encoding, sizeof(transfer_encoding));
    }

    rsp_buf.buf = buf;
    rsp_buf.size = size;
    cel_httpresponse_set_body_writing_callback(rsp, 
        (CelHttpResponseBodyWriteCallBack)cel_httpresponse_body_write,
        &rsp_buf);
    if (cel_httpresponse_writing(rsp, &(rsp->s)) == -1
        && rsp->writing_error == CEL_HTTP_ERROR)
        return -1;
    cel_httpresponse_set_body_writing_callback(rsp, NULL, NULL);

    return rsp_buf.size;
}

typedef struct _CelHttpResponseFmtArgs
{
    const char *fmt;
    va_list args;
    int size;
}CelHttpResponseFmtArgs;

static int cel_httpresponse_body_printf(CelHttpResponse *rsp, 
                                        CelStream *s,
                                        CelHttpResponseFmtArgs *fmt_args)
{
    int remaining_size;
    //puts(fmt_args->fmt);
    
    remaining_size = cel_stream_get_remaining_capacity(s);
    fmt_args->size = vsnprintf((char *)cel_stream_get_pointer(s), 
        remaining_size, fmt_args->fmt, fmt_args->args);
    if (fmt_args->size < 0 || fmt_args->size >= remaining_size)
    {
        if (fmt_args->size > 0)
            cel_stream_remaining_resize(s, fmt_args->size);
        else
            cel_stream_remaining_resize(s, remaining_size + 1);
       /* printf("cel_httpresponse_body_printf remaining %d capacity %d\r\n", 
            (int)cel_stream_get_remaining_capacity(s), (int)s->capacity);*/
        remaining_size = cel_stream_get_remaining_capacity(s);
        fmt_args->size = vsnprintf((char *)cel_stream_get_pointer(s), 
            remaining_size, fmt_args->fmt, fmt_args->args);
        if (fmt_args->size < 0 || fmt_args->size >= remaining_size)
            return -1;
    }
    cel_stream_seek(s, fmt_args->size);
    return fmt_args->size;
}

int cel_httpresponse_vprintf(CelHttpResponse *rsp, 
                             const char *fmt, va_list _args)
{
    CelHttpTransferEncoding transfer_encoding = CEL_HTTPTE_CHUNKED;
    CelHttpResponseFmtArgs fmt_args;

    //_tprintf("rsp->writing_body_offset %lld\r\n", 
    //    rsp->writing_body_offset);
    if (rsp->writing_body_offset == 0)
    {
        cel_httpresponse_set_header(rsp, 
            CEL_HTTPHDR_TRANSFER_ENCODING,
            &transfer_encoding, sizeof(transfer_encoding));
    }
    fmt_args.fmt = fmt;
#if defined(_CEL_UNIX)
    *(fmt_args.args) = *_args;
#elif defined(_CEL_WIN)
    fmt_args.args = _args;
#endif
    fmt_args.size = 0;
    cel_httpresponse_set_body_writing_callback(rsp, 
        (CelHttpResponseBodyWriteCallBack)cel_httpresponse_body_printf,
        &fmt_args);
    if (cel_httpresponse_writing(rsp, &(rsp->s)) == -1
        && rsp->writing_error == CEL_HTTP_ERROR)
        return -1;
    va_end(fmt_args.args);
    cel_httpresponse_set_body_writing_callback(rsp, NULL, NULL);

    return fmt_args.size;
}

int cel_httpresponse_printf(CelHttpResponse *rsp, const char *fmt, ...)
{
    va_list _args;
    size_t size;

    va_start(_args, fmt);
    size = cel_httpresponse_vprintf(rsp, fmt, _args);
    va_end(_args);

    return size;
}

int cel_httpresponse_end(CelHttpResponse *rsp)
{
    long long len = 0;

    if (rsp->writing_body_offset == 0)
    {
        cel_httpresponse_set_header(rsp,
            CEL_HTTPHDR_CONTENT_LENGTH, &len, sizeof(len));
    }
    rsp->writing_body_offset = -1;
    if (cel_httpresponse_writing(rsp, &(rsp->s)) == -1
        && rsp->writing_error == CEL_HTTP_ERROR)
        return -1;

    return 0;
}

int cel_httpresponse_send(CelHttpResponse *rsp, 
                          CelHttpStatusCode status, 
                          const void *content, size_t content_len)
{
    long long _content_len;

    cel_httpresponse_set_statuscode(rsp, status);
    _content_len = content_len;
    cel_httpresponse_set_header(rsp,
        CEL_HTTPHDR_CONTENT_LENGTH, &_content_len, sizeof(_content_len));
    if (content != NULL)
        cel_httpresponse_write(rsp, content, content_len);
    cel_httpresponse_end(rsp);

    return 0;
}

int cel_httpresponse_send_redirect(CelHttpResponse *rsp, const char *url)
{
    cel_httpresponse_set_header(rsp, 
        CEL_HTTPHDR_LOCATION, url, strlen(url));
    return cel_httpresponse_send(rsp, CEL_HTTPSC_MOVED_TEMP, NULL, 0);
}

static int cel_httpresponse_body_read_file(CelHttpResponse *rsp, CelStream *s, 
                                           FILE *fp)
{
    size_t size;

    if ((size = fread(cel_stream_get_pointer(s), 1,
        cel_stream_get_remaining_capacity(s), fp)) <= 0)
        return -1;
    cel_stream_seek(s, size);
    /*_tprintf(_T("xx cel_httpresponse_body_file capacity %d, ")
        _T("remaing %d, size %d\r\n"), 
        _cel_stream_get_capacity(s),
        cel_stream_get_remaining_capacity(s), size);*/
    
    return size;
}

int cel_httpresponse_send_file(CelHttpResponse *rsp, 
                               const char *file_path,
                               long long first, long long last,
                               CelDateTime *if_modified_since, 
                               char *if_none_match)
{
    struct stat file_stat;
    CelDateTime dt;
    CelHttpContentRange content_range;
    long long content_length;
    char file_ext[CEL_EXTLEN];
    char disposition[256];
    size_t len;

    if (stat(file_path, &file_stat) != 0
        || (file_stat.st_mode & S_IFREG) != S_IFREG)
    {
        //printf("%d\r\n", file_stat.st_mode);
        return cel_httpresponse_send(rsp, CEL_HTTPSC_NOT_FOUND, NULL, 0);
    }
    if (if_modified_since != NULL)
    {
        //printf("%ld %ld\r\n", *if_modified_since, file_stat.st_mtime);
        if (cel_datetime_diffseconds(
            if_modified_since, &(file_stat.st_mtime)) <= 0)
        {
            //printf("not modified\r\n");
            return cel_httpresponse_send(rsp, CEL_HTTPSC_NOT_MODIFIED, NULL, 0);
        }
    }
    if (rsp->body_cache.fp != NULL)
        fclose(rsp->body_cache.fp);
    if ((rsp->body_cache.fp = fopen(file_path, "rb")) == NULL)
    {
        return cel_httpresponse_send(rsp, CEL_HTTPSC_FORBIDDEN, NULL, 0);
    }
    content_range.total = (long long)file_stat.st_size;
    if (first < 0 && last == 0)
    {
        content_range.first = content_range.total + first;
        content_range.last = content_range.total - 1;
    }
    else if (first >= 0 && last == 0)
    {
        content_range.first = first;
        content_range.last = content_range.total - 1;
    }
    else
    {
        content_range.first = first;
        content_range.last = last;
    }
    if (content_range.first != 0)
        _fseeki64(rsp->body_cache.fp, content_range.first, SEEK_SET);
    
    content_length = content_range.last - content_range.first + 1;
    if (content_length == content_range.total)
    {
        cel_httpresponse_set_statuscode(rsp, CEL_HTTPSC_REQUEST_OK);
    }
    else
    {
        cel_httpresponse_set_statuscode(rsp, CEL_HTTPSC_PARTIAL_CONTENT);
        cel_httpresponse_set_header(rsp, 
            CEL_HTTPHDR_CONTENT_RANGE, &content_range, sizeof(content_range));
    }
    cel_httpresponse_set_header(rsp,
        CEL_HTTPHDR_CONTENT_LENGTH, &content_length, sizeof(content_length));
    cel_datetime_init_now(&dt);
    cel_httpresponse_set_header(rsp, 
        CEL_HTTPHDR_DATE, &dt, sizeof(CelDateTime));
    cel_datetime_destroy(&dt);
    cel_httpresponse_set_header(rsp, 
        CEL_HTTPHDR_LAST_MODIFIED,
        &(file_stat.st_mtime), sizeof(CelDateTime));

    if (cel_fileext_r_a(file_path, file_ext, CEL_EXTLEN) != NULL)
    {
        if (strcmp(file_ext, "html") == 0)
            cel_httpresponse_set_header(rsp, 
            CEL_HTTPHDR_CONTENT_TYPE, "text/html", sizeof("text/html"));
        else if (strcmp(file_ext, "css") == 0)
            cel_httpresponse_set_header(rsp, 
            CEL_HTTPHDR_CONTENT_TYPE, "text/css", sizeof("text/css"));
        else if (strcmp(file_ext, "js") == 0)
            cel_httpresponse_set_header(rsp, 
            CEL_HTTPHDR_CONTENT_TYPE, 
            "application/x-javascript", sizeof("application/x-javascript"));
        else if (strcmp(file_ext, "jpg") == 0)
            cel_httpresponse_set_header(rsp, 
            CEL_HTTPHDR_CONTENT_TYPE, "image/jpeg", sizeof("image/jpeg"));
        else
        {
            len = snprintf(disposition, 256, 
                "*; filename=%s", cel_filename_r_a(file_path, NULL, 0));
            cel_httpresponse_set_header(rsp, 
                CEL_HTTPHDR_CONTENT_DISPOSITION, disposition, len);
            cel_httpresponse_set_header(rsp,
                CEL_HTTPHDR_CONTENT_TYPE,
                "application/octet-stream", 
                strlen("application/octet-stream"));
        }
    }
    rsp->body_cache.clear_file = FALSE;
    cel_httpresponse_set_body_writing_callback(rsp, 
        (CelHttpResponseBodyWriteCallBack)cel_httpresponse_body_read_file,
        rsp->body_cache.fp);
    if (cel_httpresponse_writing(rsp, &(rsp->s)) == -1
        && rsp->writing_error == CEL_HTTP_ERROR)
        return -1;

    return 0;
}

static int cel_httpresponse_body_write_file(CelHttpResponse *rsp, 
                                            CelStream *s, 
                                            size_t len, CelVStringA *file_path)
{
    size_t size;

    if (rsp->body_cache.fp == NULL)
    {
        if ((rsp->body_cache.fp = fopen(
            cel_vstring_str_a(file_path), "wb+")) == NULL 
            && (cel_mkdirs_a(cel_filedir_a(cel_vstring_str_a(file_path)), 
            S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) == -1 
            || (rsp->body_cache.fp = fopen(
            cel_vstring_str_a(file_path), "wb+")) == NULL))
        {
            CEL_ERR((_T("cel_httpresponse_body_write_file failed")));
            return -1;
        }
    }
    if ((size = fwrite(cel_stream_get_pointer(s),
        1, len, rsp->body_cache.fp)) <= 0)
        return -1;
    cel_stream_seek(s, size);
   /* _tprintf(_T("file %s s %p, len %d, return size %d\r\n"), 
        cel_vstring_str_a(file_path), s, len, size);*/
    return size;
}

int cel_httpresponse_recv_file(CelHttpResponse *rsp, const char *file_path)
{
    if (rsp->body_cache.fp != NULL)
        fclose(rsp->body_cache.fp);
    cel_vstring_assign_a(&(rsp->body_cache.file_path), 
        file_path, strlen(file_path));
    rsp->body_cache.clear_file = FALSE;
    cel_httpresponse_set_body_reading_callback(rsp, 
        (CelHttpResponseBodyReadCallBack)cel_httpresponse_body_write_file, 
        &(rsp->body_cache.file_path));
    return 0;
}
