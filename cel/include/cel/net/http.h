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
#ifndef __CEL_NET_HTTP_H__
#define __CEL_NET_HTTP_H__

#include "cel/types.h"
#include "cel/allocator.h"
#include "cel/stream.h"
#include "cel/vstring.h"
#include "cel/datetime.h"
#include "cel/keyword.h"
#include "cel/convert.h"

#define CEL_HTTPVERSION_LEN             8
#define CEL_HTTPHDR_LEN_MAX           128
#define CEL_HTTPURL_LEN_MAX          2083

#define CEL_HTTPBODY_BUF_LEN_MAX     8192

#define CEL_HTTPBODY_CACHE_LEN_MAX   1024 * 1024 * 1024
#define CEL_HTTPBODY_CACHE_PATH      "./cache/bdy/"

typedef enum _CelHttpError
{
    CEL_HTTP_NO_ERROR = 0,
    CEL_HTTP_ERROR,
    CEL_HTTP_WANT_READ,
    CEL_HTTP_WANT_WRITE
}CelHttpError;

typedef enum _CelHttpVersion
{
    CEL_HTTPVER_UNDEFINED = -1,
    CEL_HTTPVER_09,
    CEL_HTTPVER_10,
    CEL_HTTPVER_11,
    CEL_HTTPVER_20,
    CEL_HTTPVER_COUNT
}CelHttpVersion;

typedef enum _CelHttpHeader
{
    CEL_HTTPHDR_ACCEPT,                    /* RFC2616.14.1 */
    CEL_HTTPHDR_ACCEPT_CHARSET,            /* RFC2616.14.2 */
    CEL_HTTPHDR_ACCEPT_ENCODING,           /* RFC2616.14.3 */
    CEL_HTTPHDR_ACCEPT_LANGUAGE,           /* RFC2616.14.4 */
    CEL_HTTPHDR_ACCEPT_RANGES,             /* RFC2616.14.5 */

    CEL_HTTPHDR_ACCESS_CONTROL_ALLOW_CREDENTIALS,
    CEL_HTTPHDR_ACCESS_CONTROL_ALLOW_HEADERS,
    CEL_HTTPHDR_ACCESS_CONTROL_ALLOW_METHODS,
    CEL_HTTPHDR_ACCESS_CONTROL_ALLOW_ORIGIN,
    CEL_HTTPHDR_ACCESS_CONTROL_EXPOSE_HEADERS,

    CEL_HTTPHDR_ACCESS_CONTROL_MAX_AGE,
    CEL_HTTPHDR_ACCESS_CONTROL_REQUEST_HEADERS,
    CEL_HTTPHDR_ACCESS_CONTROL_REQUEST_METHOD,
    /* https://developer.mozilla.org/zh-CN/docs/Web/HTTP/Access_control_CORS */
    CEL_HTTPHDR_AGE,                       /* RFC2616.14.6 */
    CEL_HTTPHDR_ALLOW,                     /* RFC2616.14.7 */

    CEL_HTTPHDR_AUTHORIZATION,             /* RFC2616.14.8 */
    CEL_HTTPHDR_CACHE_CONTROL,             /* RFC2616.14.9 */
    CEL_HTTPHDR_CONNECTION,                /* RFC2616.14.10 */
    CEL_HTTPHDR_CONTENT_DISPOSITION,       /* MIME */
    CEL_HTTPHDR_CONTENT_ENCODING,          /* RFC2616.14.11 */

    CEL_HTTPHDR_CONTENT_LANGUAGE,          /* RFC2616.14.12 */
    CEL_HTTPHDR_CONTENT_LENGTH,            /* RFC2616.14.13 */
    CEL_HTTPHDR_CONTENT_LOCATION,          /* RFC2616.14.14 */
    CEL_HTTPHDR_CONTENT_MD5,               /* RFC2616.14.15 */
    CEL_HTTPHDR_CONTENT_RANGE,             /* RFC2616.14.16 */

    CEL_HTTPHDR_CONTENT_TYPE,              /* RFC2616.14.17 */ 
    CEL_HTTPHDR_COOKIE,                    /* RFC6265 */
    CEL_HTTPHDR_DATE,                      /* RFC2616.14.18 */
    CEL_HTTPHDR_DNT,                       /* Do not track, N < a */
    CEL_HTTPHDR_ETAG,                      /* RFC2616.14.19 */

    CEL_HTTPHDR_EXPECT,                    /* RFC2616.14.20 */
    CEL_HTTPHDR_EXPIRES,                   /* RFC2616.14.21 */
    CEL_HTTPHDR_FROM,                      /* RFC2616.14.22 */
    CEL_HTTPHDR_HOST,                      /* RFC2616.14.23 */
    CEL_HTTPHDR_IF_MATCH,                  /* RFC2616.14.24 */

    CEL_HTTPHDR_IF_MODIFIED_SINCE,         /* RFC2616.14.25 */
    CEL_HTTPHDR_IF_NONE_MATCH,             /* RFC2616.14.26 */
    CEL_HTTPHDR_IF_RANGE,                  /* RFC2616.14.27 */
    CEL_HTTPHDR_IF_UNMODIFIED_SINCE,       /* RFC2616.14.28 */
    CEL_HTTPHDR_LAST_MODIFIED,             /* RFC2616.14.29 */

    CEL_HTTPHDR_LOCATION,                  /* RFC2616.14.30 */
    CEL_HTTPHDR_MAX_FORWARDS,              /* RFC2616.14.31 */
    CEL_HTTPHDR_ORIGIN,
    /* https://developer.mozilla.org/zh-CN/docs/Web/HTTP/Access_control_CORS */
    CEL_HTTPHDR_PRAGMA,                    /* RFC2616.14.32 */
    CEL_HTTPHDR_PROXY_AUTHENTICATE,        /* RFC2616.14.33 */

    CEL_HTTPHDR_PROXY_AUTHORIZATION,       /* RFC2616.14.34 */
    CEL_HTTPHDR_RANGE,                     /* RFC2616.14.35 */
    CEL_HTTPHDR_REFERER,                   /* RFC2616.14.36 */
    CEL_HTTPHDR_RETRY_AFTER,               /* RFC2616.14.37 */
    CEL_HTTPHDR_SERVER,                    /* RFC2616.14.38 */

    CEL_HTTPHDR_SET_COOKIE,                /* RFC6265 */
    CEL_HTTPHDR_TE,                        /* RFC2616.14.39 */
    CEL_HTTPHDR_TRAILER,                   /* RFC2616.14.40 */
    CEL_HTTPHDR_TRANSFER_ENCODING,         /* RFC2616.14.41 */
    CEL_HTTPHDR_UPGRADE,                   /* RFC2616.14.42 */

    CEL_HTTPHDR_UPGRADE_INSECURE_REQUESTS, 
    /* https://www.w3.org/TR/upgrade-insecure-requests */
    CEL_HTTPHDR_USER_AGENT,                /* RFC2616.14.43 */
    CEL_HTTPHDR_VARY,                      /* RFC2616.14.44 */
    CEL_HTTPHDR_VIA,                       /* RFC2616.14.45 */
    CEL_HTTPHDR_WARNING,                   /* RFC2616.14.46 */

    CEL_HTTPHDR_WWW_AUTHENTICATE,          /* RFC2616.14.47 */
    CEL_HTTPHDR_X_FORWARDED_FOR,           /* RFC 7239 X-FORWARD-XXX */
    CEL_HTTPHDR_X_FORWARDED_HOST,
    CEL_HTTPHDR_X_FORWARDED_PROTO,
    CEL_HTTPHDR_X_FORWARDED_SERVER,

    CEL_HTTPHDR_X_POWERED_BY,
    CEL_HTTPHDR_X_REAL_IP,
    CEL_HTTPHDR_X_REQUESTED_WITH,
    
    CEL_HTTPHDR_COUNT
}CelHttpHeader;

typedef enum _CelHttpConnection
{
    CEL_HTTPCON_UNDEFINED = -1,
    CEL_HTTPCON_CLOSE,
    CEL_HTTPCON_KEEPALIVE
}CelHttpConnection;

/* 
 * bytes=1000-2000   1000-2000/3000
 * bytes=1000-       1000-2999/3000
 * bytes=-1000       1999-2999/3000
 */
typedef struct _CelHttpContentRange
{
    long long first, last, total;
}CelHttpContentRange;

typedef enum _CelHttpContentType
{
    CEL_HTTPCONTENTTYPE_UNDEFINED,
    CEL_HTTPCONTENTTYPE_TEXT,
    CEL_HTTPCONTENTTYPE_URLENCODED,
    CEL_HTTPCONTENTTYPE_MULTIPART
}CelHttpContentType;

typedef struct _CelHttpCookie
{
    CelVStringA values;
    CelDateTime expires;
    long max_age;
    CelVStringA domain;
    CelVStringA path;
    BOOL secure, httponly;
}CelHttpCookie, CelHttpSetCookie;

#define cel_httpsetcookie_init cel_httpcookie_init
#define cel_httpsetcookie_destroy cel_httpcookie_destroy
#define cel_httpsetcookie_new cel_httpcookie_new
#define cel_httpsetcookie_free cel_httpcookie_free
#define cel_httpsetcookie_set_value cel_httpcookie_set_value
#define cel_httpsetcookie_set_attribute cel_httpcookie_set_attribute
#define cel_httpsetcookie_set cel_httpcookie_set

/* 
 * bytes=1000-2000  first = 1000, last = 2000
 * bytes=1000-      first = 1000, last = 0
 * bytes=-1000      first = -1000, last = 0
 */
typedef struct _CelHttpRange
{
    long long first, last;
}CelHttpRange;

typedef enum _CelHttpTransferEncoding
{
    CEL_HTTPTE_UNDEFINED = -1,
    CEL_HTTPTE_IDENTITY,
    CEL_HTTPTE_CHUNKED
}CelHttpTransferEncoding;

typedef int (* CelHttpHeaderInitFunc) (void *hdr_value);
typedef void (* CelHttpHeaderDestroyFunc) (void *hdr_value);
typedef void *(* CelHttpHeaderGetFunc) (void *hdr_value);
typedef void *(* CelHttpHeaderSetFunc) (void *hdr_value, 
                                        const void *value, size_t size);
typedef int (* CelHttpHeaderReadingFunc)(void *hdr, 
                                         const char *value, size_t size);
typedef int (* CelHttpHeaderWritingFunc)(
    const char *hdr_name, void *hdr_value, CelStream *s);

typedef struct _CelHttpHeaderHandler
{
    size_t size;
    CelHttpHeaderInitFunc init_func;
    CelHttpHeaderDestroyFunc destroy_func;
    CelHttpHeaderGetFunc get_func;
    CelHttpHeaderSetFunc set_func;
    CelHttpHeaderReadingFunc reading_func;
    CelHttpHeaderWritingFunc writing_func;
}CelHttpHeaderHandler;

extern CelKeywordA g_httpversion[];
extern CelKeywordA g_case_httphdr[];
extern CelHttpHeaderHandler g_httpvstring_handler;
extern CelHttpHeaderHandler g_httpvdatetime_handler;
extern CelHttpHeaderHandler g_httpint_handler;
extern CelHttpHeaderHandler g_httpl_handler;
extern CelHttpHeaderHandler g_httpll_handler;
extern CelHttpHeaderHandler g_httpconnection_handler;
extern CelHttpHeaderHandler g_httpcontentrange_handler;
extern CelHttpHeaderHandler g_httpcookie_handler;
extern CelHttpHeaderHandler g_httprange_handler;
extern CelHttpHeaderHandler g_httptransferencoding_handler;

typedef struct _CelHttpChunked
{
    BOOL last;
    int start;
    unsigned int size;
}CelHttpChunked;

typedef enum _CelHttpBodySaveType
{
    CEL_HTTPBODY_SAVE_UNDEFINED = -1,
    CEL_HTTPBODY_SAVE_IN_CACHE,
    CEL_HTTPBODY_SAVE_IN_MULTIPART
}CelHttpBodySaveType;

typedef struct _CelHttpBodyCache
{
    long long size;
    size_t buf_max_size;
    CelStream buf;
    BOOL clear_file;
    FILE *fp;
    CelVStringA file_path;
}CelHttpBodyCache;

#ifdef __cplusplus
extern "C" {
#endif

char *cel_http_file_etag();

/* Max(dest_size) = src_size * 3 + 1 */
char *cel_http_url_encode(char *_dst, size_t *dest_size,
                          const char *_src, size_t src_size);
/* Max(dest_size) = src_size + 1 */
char *cel_http_url_decode(char *_dst, size_t *dest_size,
                          const char *_src, size_t src_size);

static __inline 
char *cel_httpvstring_get_value(CelVString *vstr, 
                                char key_delimiter, char value_delimiter,
                                const char *key, char *value, size_t *size)
{
    return cel_strgetkeyvalue_a(cel_vstring_str_a(vstr), 
        key_delimiter, value_delimiter, key, value, size);
}
int cel_httpvstring_set_value(CelVString *vstr, 
                              char key_delimiter, char value_delimiter,
                              const char *key,
                              const char *value, size_t value_size);
int cel_httpvstring_reading(CelVString *vstr, const char *value, size_t size);
int cel_httpvstring_writing(const char *hdr_name, 
                            CelVString *vstr, CelStream *s);

int cel_httpdatetime_reading(CelDateTime *dt, const char *value, size_t size);
int cel_httpdatetime_writing(const char *hdr_name, 
                             CelDateTime *dt, CelStream *sr);

int cel_httpint_reading(int *i, const char *value, size_t size);
int cel_httpint_writing(const char *hdr_name, int *i, CelStream *s);

int cel_httpl_reading(long *l, const char *value, size_t size);
int cel_httpl_writing(const char *hdr_name, long *l, CelStream *s);

int cel_httpll_reading(long long *ll, const char *value, size_t size);
int cel_httpll_writing(const char *hdr_name, long long *ll, CelStream *s);


int cel_httpconnection_reading(CelHttpConnection *connection, 
                               const char *value, size_t size);
int cel_httpconnection_writing(const char *hdr_name, 
                               CelHttpConnection *connection, 
                               CelStream *s);

int cel_httpcontentrange_reading(CelHttpContentRange *content_range, 
                                 const char *value, size_t size);
int cel_httpcontentrange_writing(const char *hdr_name, 
                                 CelHttpContentRange *content_range, 
                                 CelStream *s);

static __inline 
char *cel_httpcookie_get_value(const char *values, 
                               const char *key, char *value, size_t *size)
{
    return cel_strgetkeyvalue_a(values, ';', '=', key, value, size);
}

int cel_httpcookie_init(CelHttpCookie *cookie);
void cel_httpcookie_destroy(CelHttpCookie *cookie);

CelHttpCookie *cel_httpcookie_new();
void cel_httpcookie_free(CelHttpCookie *cookie);

static __inline 
int cel_httpcookie_set_value(CelHttpCookie *set_cookie, const char *key,
                             const char *value, size_t value_size)
{
    return cel_httpvstring_set_value(
        &(set_cookie->values), ';', '=', key, value, value_size);
}
int cel_httpcookie_set_attribute(CelHttpCookie *set_cookie, 
                                 CelDateTime *expires, long max_age,
                                 const char *domain, const char *path, 
                                 BOOL secure, BOOL httponly);
void *cel_httpcookie_set(CelHttpCookie *set_cookie1,
                         CelHttpCookie *set_cookie2, size_t len);
int cel_httpcookie_reading(CelHttpCookie *set_cookie, 
                           const char *value, size_t size);
int cel_httpcookie_writing(const char *hdr_name, 
                           CelHttpCookie *set_cookie, CelStream *s);

int cel_httprange_reading(CelHttpRange *range, const char *value, size_t size);
int cel_httprange_writing(const char *hdr_name, 
                          CelHttpRange *range, CelStream *s);

int cel_httptransferencoding_reading(CelHttpTransferEncoding *encoding, 
                                     const char *value, size_t size);
int cel_httptransferencoding_writing(const char *hdr_name, 
                                     CelHttpTransferEncoding *encoding,
                                     CelStream *s);

int cel_httpextheader_writing(char *hdr_name, char *value, CelStream *s);

static __inline int cel_httpchunked_init(CelHttpChunked *chunked, int start)
{
    chunked->last = FALSE;
    chunked->start = start;
    chunked->size = 0;
    return 0;
}

long cel_httpchunked_reading(CelStream *s);

static __inline 
int cel_httpchunked_get_send_buffer_position(CelHttpChunked *chunked)
{
    return chunked->start + chunked->size + 9;
}
static __inline 
void *cel_httpchunked_get_send_buffer(CelHttpChunked *chunked, CelStream *s)
{
    cel_stream_set_position(s,
        cel_httpchunked_get_send_buffer_position(chunked));
    return cel_stream_get_pointer(s);
}
#define cel_httpchunked_get_send_buffer_size(chunked, s) \
    (cel_stream_get_remaining_capacity(s) - 11 - 5)
#define cel_httpchunked_resize_send_buffer(chunked, s, resize) \
    cel_stream_remaining_resize(s, resize + 11 + 5)
static __inline void cel_httpchunked_send_seek(CelHttpChunked *chunked, int offset)
{
    chunked->size += offset;
}
long cel_httpchunked_writing_last(CelHttpChunked *chunked, CelStream *s);


int cel_httpbodycache_init(CelHttpBodyCache *cache, size_t buf_max_size);
void cel_httpbodycache_destroy(CelHttpBodyCache *cache);
void cel_httpbodycache_clear(CelHttpBodyCache *cache);

int cel_httpbodycache_reading(CelHttpBodyCache *cache, 
                              const char *value, size_t size);
int cel_httpbodycache_writing(CelHttpBodyCache *cache, CelStream *s);

long long cel_httpbodycache_read(CelHttpBodyCache *cache, 
                                 long long first, long long last,
                                 void *buf, size_t buf_size);
long long cel_httpbodycache_save_file(CelHttpBodyCache *cache,
                                      long long first, long long last,
                                      const char *file_path);
int cel_httpbodycache_move_file(CelHttpBodyCache *cache, 
                                const char *file_path);

#ifdef __cplusplus
}
#endif

#endif
