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
#include "cel/net/httprequest.h"
#include "cel/error.h"
#include "cel/log.h"
#include "cel/datetime.h"
#include "cel/keyword.h"
#include "cel/file.h"
#include <stdarg.h>


static CelKeywordA s_reqmethod[] = 
{
    { sizeof("CONNECT") - 1, "CONNECT" },
    { sizeof("DELETE") - 1, "DELETE" },
    { sizeof("GET") - 1, "GET" }, 
    { sizeof("HEAD") - 1, "HEAD" },
    { sizeof("OPTIONS") - 1, "OPTIONS" },
    { sizeof("POST") - 1, "POST" },
    { sizeof("PUT") - 1, "PUT" },
    { sizeof("TRACE") - 1, "TRACE" }
};

static size_t s_httpreqhdr_offset[] = 
{
    CEL_OFFSET(CelHttpRequest, accept),
    CEL_OFFSET(CelHttpRequest, accept_charset),
    CEL_OFFSET(CelHttpRequest, accept_encoding),
    CEL_OFFSET(CelHttpRequest, accept_language),
    0,

    0, 0, 0, 0, 0,

    0,
    CEL_OFFSET(CelHttpRequest, access_control_request_headers),
    CEL_OFFSET(CelHttpRequest, access_control_request_method),
    0,
    CEL_OFFSET(CelHttpRequest, allow),

    CEL_OFFSET(CelHttpRequest, authorization),
    CEL_OFFSET(CelHttpRequest, cache_control), 
    CEL_OFFSET(CelHttpRequest, connection),
    0,
    CEL_OFFSET(CelHttpRequest, content_encoding),

    CEL_OFFSET(CelHttpRequest, content_language),
    CEL_OFFSET(CelHttpRequest, content_length),
    CEL_OFFSET(CelHttpRequest, content_location),
    CEL_OFFSET(CelHttpRequest, content_md5),
    CEL_OFFSET(CelHttpRequest, content_range),

    CEL_OFFSET(CelHttpRequest, content_type),
    CEL_OFFSET(CelHttpRequest, cookie),
    CEL_OFFSET(CelHttpRequest, date),
    CEL_OFFSET(CelHttpRequest, dnt),
    0,

    CEL_OFFSET(CelHttpRequest, expect),
    CEL_OFFSET(CelHttpRequest, expires), 
    CEL_OFFSET(CelHttpRequest, from), 
    CEL_OFFSET(CelHttpRequest, host),
    CEL_OFFSET(CelHttpRequest, if_match),

    CEL_OFFSET(CelHttpRequest, if_modified_since),
    CEL_OFFSET(CelHttpRequest, if_none_match),
    CEL_OFFSET(CelHttpRequest, if_range), 
    CEL_OFFSET(CelHttpRequest, if_unmodified_since),
    CEL_OFFSET(CelHttpRequest, last_modified),

    0,
    CEL_OFFSET(CelHttpRequest, max_forwards),
    CEL_OFFSET(CelHttpRequest, origin),
    CEL_OFFSET(CelHttpRequest, pragma),
    CEL_OFFSET(CelHttpRequest, proxy_authorization),

    0,
    CEL_OFFSET(CelHttpRequest, range),
    CEL_OFFSET(CelHttpRequest, referer), 
    0,
    0,

    0,
    CEL_OFFSET(CelHttpRequest, te), 
    CEL_OFFSET(CelHttpRequest, trailer),
    CEL_OFFSET(CelHttpRequest, transfer_encoding),
    CEL_OFFSET(CelHttpRequest, upgrade),

    CEL_OFFSET(CelHttpRequest, upgrade_insecure_requests),
    CEL_OFFSET(CelHttpRequest, user_agent),
    0,
    CEL_OFFSET(CelHttpRequest, via),
    CEL_OFFSET(CelHttpRequest, warning),

    0, 0, 0, 0, 0,

    0, 0,
    CEL_OFFSET(CelHttpRequest, x_requested_with),

};

int cel_httpurl_init(CelHttpUrl *url)
{
    url->scheme = CEL_HTTPSCHEME_HTTP;
    cel_vstring_init_a(&(url->host));
    url->port = 0;
    cel_vstring_init_a(&(url->path));
    cel_vstring_init_a(&(url->query));
    cel_vstring_init_a(&(url->fragment));

    return 0;
}

void cel_httpurl_destroy(CelHttpUrl *url)
{
    url->scheme = CEL_HTTPSCHEME_HTTP;
    cel_vstring_destroy_a(&(url->host));
    url->port = 0;
    cel_vstring_destroy_a(&(url->path));
    cel_vstring_destroy_a(&(url->query));
    cel_vstring_destroy_a(&(url->fragment));
}

int cel_httpurl_reading(CelHttpUrl *url, const char *value, size_t size)
{
    int i = 0;
    int path_start = 0, path_end = (int)size;
    int query_start = 0, query_end = (int)size;
    int fragment_start = 0, fragmet_end = (int)size;
    size_t len;

    //printf("%d %s\r\n", size, value);
    while (i < (int)size)
    {
        if (query_start == 0 && value[i] == '?')
        {
            path_end = i;
            query_start = i + 1;
        }
        else if (value[i] == '#')
        {
            if (query_start != 0)
                query_end = i;
            else
                path_end = i;
            fragment_start = i + 1;
            break;
        }
        i++;
    }
    len = path_end - path_start + 1;
    cel_vstring_assign_a(&(url->path), value + path_start, len -1);
    //puts(url->path.str);
    if (query_start != 0)
    {
        len = query_end - query_start + 1;
        cel_vstring_assign_a(&(url->query), value + query_start, len -1);
        //puts(url->query.str);
    }
    if (fragment_start != 0)
    {
        len = fragmet_end - query_start + 1;
        cel_vstring_assign_a(&(url->fragment), value + fragment_start, len -1);
        //puts(url->fragment.str);
    }

    return 0;
}

int cel_httprequest_init(CelHttpRequest *req)
{
    int i = 0;
    CelHttpHeaderHandler *handler;

    req->ver = CEL_HTTPVER_11;
    req->method =CEL_HTTPM_GET;
    cel_httpurl_init(&(req->url));
    req->hdr_flags = 0;
    while (i < CEL_HTTPHDR_COUNT)
    {
        if (s_httpreqhdr_offset[i] != 0)
        {
            handler = (CelHttpHeaderHandler *)g_case_httphdr[i].value2;
            if (handler->init_func == NULL)
                memset((char *)req + s_httpreqhdr_offset[i], 0, handler->size);
            else
                handler->init_func((char *)req + s_httpreqhdr_offset[i]);
        }
        i++;
    }
    cel_rbtree_init(
        &(req->ext_hdrs), (CelCompareFunc)strcmp, cel_free, cel_free);

    req->body_save_in = CEL_HTTPBODY_SAVE_IN_CACHE;
    cel_httpbodycache_init(&(req->body_cache), CEL_HTTPBODY_BUF_LEN_MAX);
    cel_httpmultipart_init(&(req->multipart));

    req->reading_state = CEL_HTTPREQUEST_READING_INIT;
    req->reading_error = 0;
    req->reading_hdr_offset = 0;
    req->reading_body_offset = 0;
    req->body_reading_callback = NULL;
    
    req->writing_state = CEL_HTTPREQUEST_WRITING_INIT;
    req->writing_error = 0;
    req->writing_body_offset = 0;
    req->body_writing_callback = NULL;

    cel_stream_init(&(req->s));
    cel_stream_resize(&(req->s), CEL_HTTPREQUEST_STREAM_BUFFER_SIZE);

    return 0;
}

void cel_httprequest_clear(CelHttpRequest *req)
{
    int i = 0;
    CelHttpHeaderHandler *handler;

    req->ver = CEL_HTTPVER_11;
    req->method =CEL_HTTPM_GET;
    cel_httpurl_destroy(&(req->url));
    while (i < CEL_HTTPHDR_COUNT)
    {
        if (s_httpreqhdr_offset[i] != 0)
        {
            handler = (CelHttpHeaderHandler *)g_case_httphdr[i].value2;
            if (handler->destroy_func == NULL)
                memset((char *)req + s_httpreqhdr_offset[i], 0, handler->size);
            else
                handler->destroy_func((char *)req + s_httpreqhdr_offset[i]);
        }
        i++;
    }
    cel_rbtree_clear(&(req->ext_hdrs));
    //cel_stream_clear(&(req->body_data));
    req->body_save_in = CEL_HTTPBODY_SAVE_IN_CACHE;
    if (req->body_save_in == CEL_HTTPBODY_SAVE_IN_CACHE)
        cel_httpbodycache_clear(&(req->body_cache));
    else if (req->body_save_in == CEL_HTTPBODY_SAVE_IN_MULTIPART)
        cel_httpmultipart_destroy(&(req->multipart));

    req->hdr_flags = 0;
    req->reading_state = CEL_HTTPREQUEST_READING_INIT;
    req->reading_error = 0;
    req->reading_hdr_offset = 0;
    req->reading_body_offset = 0;
    req->body_reading_callback = NULL;

    req->writing_state = CEL_HTTPREQUEST_WRITING_INIT;
    req->writing_error = 0;
    req->writing_body_offset = 0;
    req->body_writing_callback = NULL;

    cel_stream_clear(&(req->s));
}

void cel_httprequest_destroy(CelHttpRequest *req)
{
    int i = 0;
    CelHttpHeaderHandler *handler;

    req->ver = CEL_HTTPVER_11;
    req->method =CEL_HTTPM_GET;
    cel_httpurl_destroy(&(req->url));
    while (i < CEL_HTTPHDR_COUNT)
    {
        if (s_httpreqhdr_offset[i] != 0)
        {
            handler = (CelHttpHeaderHandler *)g_case_httphdr[i].value2;
            if (handler->destroy_func == NULL)
                memset((char *)req + s_httpreqhdr_offset[i], 0, handler->size);
            else
                handler->destroy_func((char *)req + s_httpreqhdr_offset[i]);
        }
        i++;
    }
    cel_rbtree_destroy(&(req->ext_hdrs));

    req->body_save_in = CEL_HTTPBODY_SAVE_IN_CACHE;
    if (req->body_save_in == CEL_HTTPBODY_SAVE_IN_CACHE)
        cel_httpbodycache_destroy(&(req->body_cache));
    else if (req->body_save_in == CEL_HTTPBODY_SAVE_IN_MULTIPART)
        cel_httpmultipart_destroy(&(req->multipart));

    req->hdr_flags = 0;
    req->reading_state = CEL_HTTPREQUEST_READING_INIT;
    req->reading_error = 0;
    req->reading_hdr_offset = 0;
    req->reading_body_offset = 0;
    req->body_reading_callback = NULL;

    req->writing_state = CEL_HTTPREQUEST_WRITING_INIT;
    req->writing_error = 0;
    req->writing_body_offset = 0;
    req->body_writing_callback = NULL;

    cel_stream_destroy(&(req->s));
}

CelHttpRequest *cel_httprequest_new(void)
{
    CelHttpRequest *req;

    if ((req = (CelHttpRequest *)cel_malloc(sizeof(CelHttpRequest))) != NULL)
    {
        if (cel_httprequest_init(req) == 0)
            return req;
        cel_free(req);
    }
    return NULL;
}

void cel_httprequest_free(CelHttpRequest *req)
{
    cel_httprequest_destroy(req); 
    cel_free(req);
}

char *cel_httprequest_get_params(CelHttpRequest *req, const char *key, 
                                 char *value, size_t *size)
{
    size_t _size = *size;
    char *_value;

    if (req->method ==CEL_HTTPM_GET)
    {
        if ((_value = cel_httprequest_get_query(
            req, key, value, &_size)) == NULL)
            return cel_httprequest_get_form(req, key, value, size);
    }
    else
    {
        if ((_value = cel_httprequest_get_form(
            req, key, value, &_size)) == NULL)
            return cel_httprequest_get_query(req, key, value, size);
    }
    *size = _size;
    return _value;
}

static int cel_httprequest_reading_header(CelHttpRequest *req, CelStream *s)
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
                /*printf("key_start %p key_end %p value_start %p 
                    value end = %p \r\n",
                    key_start, key_end, value_start, value_end);*/
                if (value_end - key_start <= 3)
                {
                    //puts("Header ok");
                    req->reading_hdr_offset += (value_end - key_start);
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
                    cel_rbtree_insert(&(req->ext_hdrs), key, value);
                    /*printf("Http request header '%.*s' undefined.\r\n", 
                        (int)value_end - key_start,
                        (char *)(cel_stream_get_buffer(s) + key_start));*/
                }
                else 
                {
                    if (s_httpreqhdr_offset[hdr_index] == 0)
                    {
                        CEL_WARNING(("Http request header '%.*s' "
                            "call back is null",
                            (int)value_end - key_start, 
                            (char *)(cel_stream_get_buffer(s) + key_start)));
                    }
                    else
                    {
                        /*printf("Http request header '%.*s' reading start.req %p offset %d %p\r\n",
                        (int)value_end - key_start,
                        (char *)(cel_stream_get_buffer(s) + key_start), req,
                        s_httpreqhdr_offset[hdr_index], (char *)req + s_httpreqhdr_offset[hdr_index]);*/
                        handler = (CelHttpHeaderHandler *)g_case_httphdr[hdr_index].value2;
                        if (handler->reading_func(
                            (char *)req + s_httpreqhdr_offset[hdr_index], 
                            (char *)(cel_stream_get_buffer(s) + value_start), 
                            value_len) == -1)
                            CEL_WARNING(("Http request header '%.*s' "
                            "reading return -1(%s)",
                            (int)value_end - key_start,
                            (char *)(cel_stream_get_buffer(s) + key_start),
                            cel_geterrstr(cel_sys_geterrno())));
                        else
                            CEL_SETFLAG(req->hdr_flags, (ULL(1) << hdr_index));
                        /*printf("Http request header '%.*s' reading end i= %d.\r\n",
                            (int)value_end - key_start,
                            (char *)(cel_stream_get_buffer(s) + key_start), hdr_index);*/
                    }
                }
            }
            req->reading_hdr_offset += (value_end - key_start);
            key_start = cel_stream_get_position(s);
        }
    }
    cel_stream_set_position(s, key_start);
    return -1;
}

static long cel_httprequest_reading_body_content(CelHttpRequest *req, 
                                                 CelStream *s, long len)
{
    long size;

    //_tprintf("cel_httprequest_reading_body_content len %ld\r\n", len);
    if (req->content_length > CEL_HTTPBODY_CACHE_LEN_MAX
        || len > CEL_HTTPBODY_CACHE_LEN_MAX - req->reading_body_offset)
    {
        CEL_ERR((_T("Httprequest body too large")));
        return -1;
    }
    if (req->body_reading_callback != NULL)
    {
        size = req->body_reading_callback(
            req, s, len, req->body_reading_user_data);
    }
    else if (req->body_content_type == CEL_HTTPCONTENTTYPE_MULTIPART)
    {
        //puts("CEL_HTTPCONTENTTYPE_MULTIPART");
        size = cel_httpmultipart_reading(&(req->multipart), s, len);
    }
    else
    {
        size = cel_httpbodycache_reading(&(req->body_cache), 
            (char *)cel_stream_get_pointer(s), (size_t)len);
        cel_stream_seek(s, size);
    }
    req->reading_body_offset += size;

    return size;
}

static int cel_httprequest_reading_body(CelHttpRequest *req, CelStream *s)
{
    long chunk_size;
    long len1, len2;

    //printf("body offset/length %ld/%ld,\r\n",
    //    req->reading_body_offset, req->content_length);
start:
    if ((len1 = (long)(req->content_length - req->reading_body_offset)) <= 0)
    {
        if (req->transfer_encoding != CEL_HTTPTE_CHUNKED)
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
                {
                    CEL_ERR((_T("cel_httprequest_reading_body(chunk size error)")));
                    req->reading_state = CEL_HTTP_ERROR;
                    return -1;
                }
                return -1;
            }
            else 
            {
                //printf("chunk_size = %d %s\r\n", 
                //    chunk_size, (char *)s->pointer);
                req->content_length += chunk_size;
                len1 = (long)(req->content_length - req->reading_body_offset);
            }
        }
    }
    if ((len2 = cel_stream_get_remaining_length(s)) >= len1)
    {
        if (cel_httprequest_reading_body_content(req, s, len1) != len1)
        {
            CEL_ERR((_T("cel_httprequest_reading_body_content error")));
            req->reading_state = CEL_HTTP_ERROR;
            return -1;
        }
        if (req->transfer_encoding == CEL_HTTPTE_CHUNKED)
            goto start;
        return 0;
    }
    else
    {
        if (len2 > 0)
        {
            if (cel_httprequest_reading_body_content(req, s, len2) != len2)
            {
                CEL_ERR((_T("cel_httprequest_reading_body_content error")));
                req->reading_state = CEL_HTTP_ERROR;
                return -1;
            }
        }
        return -1;
    }
}

int cel_httprequest_reading(CelHttpRequest *req, CelStream *s)
{
    size_t i, size1, size2;
    int start, end, _end;
    BYTE ch = '\0', ch1;
    char *str;

    //puts((char *)s->pointer);
    switch (req->reading_state)
    {
    case CEL_HTTPREQUEST_READING_INIT:
        req->reading_state = CEL_HTTPREQUEST_READING_METHOD;
    case CEL_HTTPREQUEST_READING_METHOD:
        start = cel_stream_get_position(s);
        do {
            if (cel_stream_get_remaining_length(s) < 1)
            {
                if (cel_stream_get_position(s) - start > 
                    CEL_HTTPMETHOD_LEN_MAX)
                    req->reading_state = CEL_HTTP_ERROR;
                cel_stream_set_position(s, start);
                return -1;
            }
            cel_stream_read_u8(s, ch);
        }while ((char)ch != ' ');
        end = cel_stream_get_position(s);
        if ((req->method = (CelHttpMethod)cel_keyword_binary_search_a(
            s_reqmethod,CEL_HTTPM_CONUT, 
            (char *)(cel_stream_get_buffer(s) + start), 
            end - start - 1)) == -1)
        {
            CEL_ERR((_T("Invalid http request method.")));
            req->reading_state = CEL_HTTP_ERROR;
            return -1;
        }
        //printf("method: %d\r\n", req->method);
        req->reading_hdr_offset += (end - start);
        req->reading_state = CEL_HTTPREQUEST_READING_URL;
    case CEL_HTTPREQUEST_READING_URL:
    case CEL_HTTPREQUEST_READING_VERSION:
        start = cel_stream_get_position(s);
        do {
            if (cel_stream_get_remaining_length(s) < 1)
            {
                if (cel_stream_get_position(s) - start > 
                    CEL_HTTPVERSION_LEN + CEL_HTTPURL_LEN_MAX)
                    req->reading_state = CEL_HTTP_ERROR;
                cel_stream_set_position(s, start);
                return -1;
            } 
            ch1 = ch;
            cel_stream_read_u8(s, ch);
            if (ch == ' ')
                _end = cel_stream_get_position(s);
        }while ((char)ch != '\n' || (char)ch1 != '\r');
        cel_httpurl_reading(&(req->url), 
            (char *)(cel_stream_get_buffer(s) + start), _end - start - 1);
        start = _end;
        end = cel_stream_get_position(s);
        //printf("len %d %s \r\n", end - start - 2, 
        //    (char *)(cel_stream_get_buffer(s) + start));
        if ((i = cel_keyword_binary_search_a(g_httpversion, CEL_HTTPVER_COUNT,
            (char *)(cel_stream_get_buffer(s) + start), 
            end - start - 2)) == -1)
        {
            CEL_ERR((_T("Invalid http version.")));
            req->reading_state = CEL_HTTP_ERROR;
            return -1;
        }
        if ((req->ver = (CelHttpVersion)i) == CEL_HTTPVER_11)
            req->connection = CEL_HTTPCON_KEEPALIVE;
        req->reading_hdr_offset += (end - start);
        req->reading_state = CEL_HTTPREQUEST_READING_HEADER;
    case CEL_HTTPREQUEST_READING_HEADER:
        //puts("CEL_HTTPREQUEST_READING_HEADER");
        if (cel_httprequest_reading_header(req, s) == -1)
            return -1;
        if ((str = cel_vstring_str_a(&(req->content_type))) != NULL)
        {
            //puts(str);
            if (strncmp(str, "application/x-www-form-urlencoded", 33) == 0)
                req->body_content_type = CEL_HTTPCONTENTTYPE_URLENCODED;
            else if (strncmp(str, "multipart/form-data", 19) == 0)
            {
                /* "; boundary=" */
                if ((size1 = cel_vstring_len(&(req->content_type))) > 30)
                    cel_httpmultipart_set_boundary(
                    &(req->multipart), str + 30);
                req->body_content_type = CEL_HTTPCONTENTTYPE_MULTIPART;
                req->body_save_in = CEL_HTTPBODY_SAVE_IN_MULTIPART;
            }
        }
        else
        {
            req->body_content_type = CEL_HTTPCONTENTTYPE_TEXT;
        }
        req->reading_state = CEL_HTTPREQUEST_READING_BODY;
    case CEL_HTTPREQUEST_READING_BODY:
        //puts("CEL_HTTPREQUEST_READING_BODY");
        if (CEL_CHECKFLAG(req->hdr_flags, (ULL(1) << CEL_HTTPHDR_CONTENT_LENGTH))
            || CEL_CHECKFLAG(req->hdr_flags, 
            (ULL(1) << CEL_HTTPHDR_TRANSFER_ENCODING)))
        {
            //puts("cel_httprequest_reading_body");
            if (cel_httprequest_reading_body(req, s) == -1)
                return -1;
        }
        if (req->body_content_type == CEL_HTTPCONTENTTYPE_URLENCODED)
        {
            if ((size1 = cel_vstring_len(&(req->url.query))) > 0)
            {
                size2 = size1;
                cel_http_url_decode(
                    cel_vstring_str_a(&(req->url.query)), &size2,
                    cel_vstring_str_a(&(req->url.query)), size1);
                req->url.query.size = size2;
            }
            if ((size1 = cel_stream_get_length(&(req->body_cache.buf))) > 0)
            {
                size2 = _cel_stream_get_capacity(&(req->body_cache.buf));
                cel_http_url_decode(
                    (char *)cel_stream_get_buffer(&(req->body_cache.buf)), &size2,
                    (char *)cel_stream_get_buffer(&(req->body_cache.buf)), size1);
                cel_stream_set_length(&(req->body_cache.buf), size2);
            }
        }
        req->reading_state = CEL_HTTPREQUEST_READING_OK;
    case CEL_HTTPREQUEST_READING_OK:
        //puts("CEL_HTTPREQUEST_READING_OK");
        break;
    default:
        CEL_ERR((_T("Invalid http request state %d."), req->reading_state));
        req->reading_state = CEL_HTTP_ERROR;
        return -1;
    }
    return 0;
}

static int cel_httprequest_writing_header(CelHttpRequest *req, CelStream *s)
{
    int i = 0;
    CelHttpHeaderHandler *handler;

    while (i < CEL_HTTPHDR_COUNT)
    {
        /*
        printf("hdr flags 0x%x, i = %d offset %d %p\r\n", req->hdr_flags, 
        i, s_httpreqhdr_offset[i].offset, 
        s_httpreqhdr_offset[i].handler->writing_func);
        */
        if (!CEL_CHECKFLAG(req->hdr_flags, (ULL(1) << i))
            || s_httpreqhdr_offset[i] == 0)
        {
            i++;
            continue;
        }
        //printf("hdr flags 0x%x, i = %d\r\n", req->hdr_flags, i);
        handler = (CelHttpHeaderHandler *)g_case_httphdr[i].value2;
        handler->writing_func(
            (char *)g_case_httphdr[i].value, 
            (char *)req + s_httpreqhdr_offset[i], s);
        i++;
        //puts((char *)s->buffer);
    }
    cel_rbtree_foreach(
        &(req->ext_hdrs), (CelKeyValuePairEachFunc)cel_httpextheader_writing, s);
    cel_stream_write(s, "\r\n", 2);
    cel_stream_seal_length(s);
    //puts((char *)s->buffer);

    return 0;
}

static int cel_httprequest_writing_body(CelHttpRequest *req, CelStream *s)
{
    size_t _size;
    long len;

    if (req->transfer_encoding == CEL_HTTPTE_CHUNKED)
    {
        while (TRUE)
        {
            /* hex * 7 + \r\n(chunk size) + \r\n(chunk data or footer)*/
            if (cel_stream_get_remaining_capacity(s) < 7 + 2 + 2 + 1)
            {
                req->writing_error = CEL_HTTP_WANT_WRITE;
                cel_stream_seal_length(s);
                return -1;
            }
            /* Chunk data */
            if (req->writing_body_offset != -1)
            {
                cel_stream_set_position(s, 
                    cel_httpchunked_get_send_buffer_position(&(req->chunked)));
                if (req->body_writing_callback == NULL
                    || (_size = req->body_writing_callback(
                    req, s, req->body_writing_user_data)) <= 0)
                {
                    req->writing_error = CEL_HTTP_WANT_WRITE;
                    cel_stream_seal_length(s);
                    return -1;
                }
                cel_httpchunked_send_seek(&(req->chunked), _size);
                req->writing_body_offset += _size;
                req->content_length += _size;
                req->writing_error = CEL_HTTP_WANT_WRITE;
                cel_stream_seal_length(s);
                return -1;
            }
            else
            {
                /* Last chunk */
                cel_httpchunked_writing_last(&(req->chunked), s);
                cel_stream_seal_length(s);
                return 0;
            }
        }
    }
    else
    {
        if (req->writing_body_offset != -1)
        {
            while ((len = (long)(req->content_length 
                - req->writing_body_offset)) > 0)
            {
                if (cel_stream_get_remaining_capacity(s) < 1
                    || req->body_writing_callback == NULL
                    || (_size = req->body_writing_callback(
                    req, s, req->body_writing_user_data)) == 0)
                {
                    req->writing_error = CEL_HTTP_WANT_WRITE;
                    cel_stream_seal_length(s);
                    return -1;
                }
                req->writing_body_offset += _size;
            }
        }
        return 0;
    }
}

int cel_httprequest_writing(CelHttpRequest *req, CelStream *s)
{
    switch (req->writing_state)
    {
    case CEL_HTTPREQUEST_WRITING_INIT:
        req->writing_state = CEL_HTTPREQUEST_WRITING_METHOD;
    case CEL_HTTPREQUEST_WRITING_METHOD:
    case CEL_HTTPREQUEST_WRITING_URL:
    case CEL_HTTPREQUEST_WRITING_VERSION:
        /* Package request line */
        if (cel_vstring_len(&(req->url.query)) != 0)
        {
            if (cel_vstring_len(&(req->url.fragment)) != 0)
            {
                cel_stream_printf(s, "%s %s?%s#%s %s\r\n", 
                    s_reqmethod[req->method].key_word, 
                    cel_vstring_str_a(&(req->url.path)),
                    cel_vstring_str_a(&(req->url.query)), 
                    cel_vstring_str_a(&(req->url.fragment)),
                    g_httpversion[req->ver].key_word);
            }
            else
            {
                cel_stream_printf(s, "%s %s?%s %s\r\n", 
                    s_reqmethod[req->method].key_word, 
                    cel_vstring_str_a(&(req->url.path)),
                    cel_vstring_str_a(&(req->url.query)),
                    g_httpversion[req->ver].key_word);
            }
        }
        else
        {
            if (cel_vstring_len(&(req->url.fragment)) != 0)
            {
                cel_stream_printf(s, "%s %s#%s %s\r\n", 
                    s_reqmethod[req->method].key_word, 
                    cel_vstring_str_a(&(req->url.path)),
                    cel_vstring_str_a(&(req->url.fragment)),
                    g_httpversion[req->ver].key_word);
            }
            else
            {
                cel_stream_printf(s, "%s %s %s\r\n", 
                    s_reqmethod[req->method].key_word, 
                    cel_vstring_str_a(&(req->url.path)),
                    g_httpversion[req->ver].key_word);
            }
        }
        req->writing_state = CEL_HTTPREQUEST_WRITING_HEADER;
    case CEL_HTTPREQUEST_WRITING_HEADER:
        if (cel_httprequest_writing_header(req, s) == -1)
            return -1;
        cel_httpchunked_init(&(req->chunked), cel_stream_get_position(s));
        req->writing_state = CEL_HTTPREQUEST_WRITING_BODY;
    case CEL_HTTPREQUEST_WRITING_BODY:
        if (cel_httprequest_writing_body(req, s) == -1)
            return -1;
        req->writing_state = CEL_HTTPREQUEST_WRITING_OK;
    case CEL_HTTPREQUEST_WRITING_OK:
        break;
    default :
        CEL_ERR((_T("Invalid http request writing state %d."), 
            req->writing_state));
        req->writing_state = CEL_HTTP_ERROR;
        return -1;
    }

    return 0;
}

int cel_httprequest_set_url_str(CelHttpRequest *req, const char *url)
{
    size_t scheme_start = 0, scheme_end = 0;
    size_t host_start = 0, host_end = 0;
    size_t port_start = 0/*, port_end = 0*/;
    size_t url_start = 0, url_end = 0;

    while (url[url_end] != '\0')
    {
        if (url[url_end] == ':')
        {
            scheme_end = url_end;
            if (scheme_end - scheme_start == 4
                && memcmp(url + scheme_start, "http", 4) == 0)
            {
                req->url.scheme = CEL_HTTPSCHEME_HTTP;
                req->url.port = 80;
            }
            else if (scheme_end - scheme_start == 5 
                && memcmp(url + scheme_start, "https", 5) == 0)
            {
                req->url.scheme = CEL_HTTPSCHEME_HTTPS;
                req->url.port = 443;
            }
            else if (scheme_end - scheme_start == 3 
                && memcmp(url + scheme_start, "ftp", 3) == 0)
            {
                req->url.scheme = CEL_HTTPSCHEME_FTP;
                req->url.port = 21;
            }
            //puts(url + scheme_start);
            url_end += 3;
            host_start = url_end;
            while (TRUE)
            {
                if (url[url_end] == ':' 
                    || url[url_end] == '/' || url[url_end] == '\0')
                {
                    url_start = url_end;
                    host_end = url_end;
                    cel_vstring_assign_a(&(req->url.host),
                        url + host_start, host_end - host_start);
                    //_tprintf(_T("%s, %d\r\n"), 
                    //    host.name.str, host_end - host_start);
                    while (url[url_end] != '\0')
                    {
                        if (url[url_end] == '/')
                        {
                            //port_end = url_start;
                            break;
                        }
                        if (url[url_end] == ':')
                        {
                            port_start = (url_end += 1);
                            while (url[url_end] != '\0')
                            {
                                if (url[url_end] == '/')
                                {
                                    url_start = url_end;
                                    //port_end = url_end;
                                    break;
                                }
                                url_end++;
                            }
                            req->url.port = 
                                (unsigned short)atoi(url + port_start);
                            //puts(url + url_start);
                        }
                        url_end++;
                    }
                    goto end;
                }
                url_end++;
            }
        }
        url_end++;
    }
end:
    //puts(url + host_start);
    //puts(url + url_start);
    if (url_start - host_start != 0)
    {
        cel_httprequest_set_header(req, CEL_HTTPHDR_HOST,
            url + host_start, url_start - host_start);
        if (url[url_start] != '\0')
            cel_vstring_assign_a(&((req)->url.path), 
            url + url_start, strlen(url + url_start));
        else
            cel_vstring_assign_a(&((req)->url.path), "/", 1);
        return 0; 
    }

    return -1;
}

void *cel_httprequest_get_header(CelHttpRequest *req, CelHttpHeader hdr_index)
{
    CelHttpHeaderHandler *handler;

    if (s_httpreqhdr_offset[hdr_index] == 0)
    {
        CEL_ERR((_T("Http request header(%s) unsupported."), 
            g_case_httphdr[hdr_index].key_word));
        return NULL;
    }
    if (!CEL_CHECKFLAG(req->hdr_flags, (ULL(1) << hdr_index)))
        return NULL;
    handler = (CelHttpHeaderHandler *)g_case_httphdr[hdr_index].value2;
    return handler->get_func == NULL
        ? ((char *)req + s_httpreqhdr_offset[hdr_index])
        : handler->get_func(((char *)req + s_httpreqhdr_offset[hdr_index]));
}

int cel_httprequest_set_header(CelHttpRequest *req, 
                               CelHttpHeader hdr_index,
                               const void *value, size_t value_len)
{
    CelHttpHeaderHandler *handler;

    if (s_httpreqhdr_offset[hdr_index] == 0)
    {
         CEL_ERR((_T("Http request header(%s) unsupported."), 
            g_case_httphdr[hdr_index].key_word));
        return -1;
    }
    handler = (CelHttpHeaderHandler *)g_case_httphdr[hdr_index].value2;
    handler->set_func(
        (char *)req + s_httpreqhdr_offset[hdr_index], value, value_len);
    CEL_SETFLAG(req->hdr_flags, (ULL(1) << hdr_index));

    return 0;
}

void *cel_httprequest_get_send_buffer(CelHttpRequest *req)
{
    CelHttpTransferEncoding transfer_encoding = CEL_HTTPTE_CHUNKED;
    CelStream *s = &(req->s);

    if (req->writing_body_offset == 0)
    {
        cel_httprequest_set_header(req, 
            CEL_HTTPHDR_TRANSFER_ENCODING, 
            &transfer_encoding, sizeof(transfer_encoding));
        cel_httprequest_writing(req, &(req->s));
    }
    return cel_httpchunked_get_send_buffer(&(req->chunked), s);
}

size_t cel_httprequest_get_send_buffer_size(CelHttpRequest *req)
{
    CelHttpTransferEncoding transfer_encoding = CEL_HTTPTE_CHUNKED;
    CelStream *s = &(req->s);

    if (req->writing_body_offset == 0)
    {
        cel_httprequest_set_header(req, 
            CEL_HTTPHDR_TRANSFER_ENCODING, 
            &transfer_encoding, sizeof(transfer_encoding));
        cel_httprequest_writing(req, &(req->s));
    }
    return cel_httpchunked_get_send_buffer_size(&(req->chunked), s);
}

void cel_httprequest_seek_send_buffer(CelHttpRequest *req, int offset)
{
    if (offset > 0)
    {
        cel_httpchunked_send_seek(&(req->chunked), offset);
        cel_stream_seek(&(req->s), offset);
        cel_stream_seal_length(&(req->s));
        req->writing_body_offset += offset;
    }
}

int cel_httprequest_resize_send_buffer(CelHttpRequest *req, size_t resize)
{
    CelHttpTransferEncoding transfer_encoding = CEL_HTTPTE_CHUNKED;
    CelStream *s = &(req->s);

    if (req->writing_body_offset == 0)
    {
        cel_httprequest_set_header(req, 
            CEL_HTTPHDR_TRANSFER_ENCODING, 
            &transfer_encoding, sizeof(transfer_encoding));
        cel_httprequest_writing(req, &(req->s));
    }
    return cel_httpchunked_resize_send_buffer(&(req->chunked), s, resize);
}

typedef struct _CelHttpRequestBuf
{
    const void *buf;
    size_t size;
}CelHttpRequestBuf;

static int cel_httprequest_body_write(CelHttpRequest *req, 
                                      CelStream *s, 
                                      CelHttpRequestBuf *req_buf)
{
    if (cel_stream_get_remaining_capacity(s) < req_buf->size)
        cel_stream_remaining_resize(s, req_buf->size);
    cel_stream_write(s, req_buf->buf, req_buf->size);
    return req_buf->size;
}

int cel_httprequest_write(CelHttpRequest *req,
                          const void *buf, size_t size)
{
    CelHttpTransferEncoding transfer_encoding = CEL_HTTPTE_CHUNKED;
    CelHttpRequestBuf req_buf;

    if (req->writing_body_offset == 0)
    {
        cel_httprequest_set_header(req, 
            CEL_HTTPHDR_TRANSFER_ENCODING, 
            &transfer_encoding, sizeof(transfer_encoding));
    }
    req_buf.buf = buf;
    req_buf.size = size;
    cel_httprequest_set_body_writing_callback(req, 
        (CelHttpRequestBodyWriteCallBack)cel_httprequest_body_write,
        &req_buf);
    if (cel_httprequest_writing(req, &(req->s)) == -1
        && req->writing_error == CEL_HTTP_ERROR)
        return -1;
    cel_httprequest_set_body_writing_callback(req, NULL, NULL);

    return req_buf.size;
}

typedef struct _CelHttpRequestFmtArgs
{
    const char *fmt;
    va_list args;
    int size;
}CelHttpRequestFmtArgs;

static int cel_httprequest_body_printf(CelHttpRequest *req, 
                                       CelStream *s, 
                                       CelHttpRequestFmtArgs *fmt_args)
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
        /* printf("cel_httprequest_body_printf remaining %d capacity %d\r\n", 
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

int cel_httprequest_vprintf(CelHttpRequest *req,
                            const char *fmt, va_list _args)
{
    CelHttpTransferEncoding transfer_encoding = CEL_HTTPTE_CHUNKED;
    CelHttpRequestFmtArgs fmt_args;

    if (req->writing_body_offset == 0)
    {
        cel_httprequest_set_header(req,
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
    cel_httprequest_set_body_writing_callback(req, 
        (CelHttpRequestBodyWriteCallBack)cel_httprequest_body_printf,
        &fmt_args);
    if (cel_httprequest_writing(req, &(req->s)) == -1
        && req->writing_error == CEL_HTTP_ERROR)
        return -1;
    cel_httprequest_set_body_writing_callback(req, NULL, NULL);

    return fmt_args.size;
}

int cel_httprequest_printf(CelHttpRequest *req, const char *fmt, ...)
{
    va_list _args;
    size_t size;

    va_start(_args, fmt);
    size = cel_httprequest_vprintf(req, fmt, _args);
    va_end(_args);

    return size;
}

int cel_httprequest_end(CelHttpRequest *req)
{
    long long len = 0;

    if (req->writing_body_offset == 0)
    {
        cel_httprequest_set_header(req, 
            CEL_HTTPHDR_CONTENT_LENGTH, &len, sizeof(len));
    }
    req->writing_body_offset = -1;
    if (cel_httprequest_writing(req, &(req->s)) == -1
        && req->writing_error == CEL_HTTP_ERROR)
        return -1;

    return 0;
}

int cel_httprequest_send(CelHttpRequest *req, 
                         CelHttpMethod method, const char *url,
                         const void *content, size_t content_len)
{
    long long _content_len;

    cel_httprequest_set_method(req, method);
    cel_httprequest_set_url_str(req, url);
    _content_len = content_len;
    cel_httprequest_set_header(req,
        CEL_HTTPHDR_CONTENT_LENGTH, &_content_len, sizeof(_content_len));
    if (content != NULL)
        cel_httprequest_write(req, content, content_len);
    cel_httprequest_end(req);

    return 0;
}

static int cel_httprequest_body_post_file(CelHttpRequest *req,
                                          CelStream *s, FILE *fp)
{
    size_t size;

    if ((size = fread(cel_stream_get_pointer(s), 1,
        cel_stream_get_remaining_capacity(s), fp)) <= 0)
        return -1;
    cel_stream_seek(s, size);
    /*_tprintf(_T("cel_httprequest_body_file capacity %d, size %d\r\n"), 
        cel_stream_get_remaining_capacity(s), size);*/
    return size;
}

int cel_httprequest_post_file(CelHttpRequest *req,
                              CelHttpMethod method, const char *url,
                              const char *file_path, 
                              long long first, long long last)
{
    struct stat file_stat;
    CelDateTime dt;
    CelHttpContentRange content_range;
    long long content_length;
    char file_ext[CEL_EXTLEN];

    if (stat(file_path, &file_stat) != 0
        || (file_stat.st_mode & S_IFREG) != S_IFREG)
    {
        //printf("%d\r\n", file_stat.st_mode);
        return -1;
    }
    if ((req->body_cache.fp = fopen(file_path, "rb")) == NULL)
    {
        return -1;
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
        _fseeki64(req->body_cache.fp, content_range.first, SEEK_SET);
    
    cel_httprequest_set_method(req, method);
    cel_httprequest_set_url_str(req, url);
    content_length = content_range.last - content_range.first + 1;
    if (content_length != content_range.total)
        cel_httprequest_set_header(req, 
        CEL_HTTPHDR_CONTENT_RANGE, &content_range, sizeof(content_range));

    cel_httprequest_set_header(req,
        CEL_HTTPHDR_CONTENT_LENGTH, &content_length, sizeof(content_length));
    cel_datetime_init_now(&dt);
    cel_httprequest_set_header(req,
        CEL_HTTPHDR_DATE, &dt, sizeof(CelDateTime));
    cel_datetime_destroy(&dt);
    cel_httprequest_set_header(req, 
        CEL_HTTPHDR_LAST_MODIFIED, &(file_stat.st_mtime), sizeof(CelDateTime));

    if (cel_fileext_r_a(file_path, file_ext, CEL_EXTLEN) != NULL)
    {
        if (strcmp(file_ext, "html") == 0)
            cel_httprequest_set_header(req, 
            CEL_HTTPHDR_CONTENT_TYPE, "text/html", sizeof("text/html"));
        else if (strcmp(file_ext, "css") == 0)
            cel_httprequest_set_header(req, 
            CEL_HTTPHDR_CONTENT_TYPE, "text/css", sizeof("text/css"));
        else if (strcmp(file_ext, "js") == 0)
            cel_httprequest_set_header(req, 
            CEL_HTTPHDR_CONTENT_TYPE, 
            "application/x-javascript", sizeof("application/x-javascript"));
        else if (strcmp(file_ext, "jpg") == 0)
            cel_httprequest_set_header(req, 
            CEL_HTTPHDR_CONTENT_TYPE, "image/jpeg", sizeof("image/jpeg"));
    }
    req->body_cache.clear_file = FALSE;
    cel_httprequest_set_body_writing_callback(req, 
        (CelHttpRequestBodyWriteCallBack)cel_httprequest_body_post_file,
        req->body_cache.fp);
    if (cel_httprequest_writing(req, &(req->s)) == -1
        && req->writing_error == CEL_HTTP_ERROR)
        return -1;

    return 0;
}
