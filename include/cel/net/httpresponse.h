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
#ifndef __CEL_NET_HTTPRESPONSE_H__
#define __CEL_NET_HTTPRESPONSE_H__

#include "cel/file.h"
#include "cel/rbtree.h"
#include "cel/net/http.h"

#define CEL_HTTPSTATUS_LEN               3
#define CEL_HTTPREASON_LEN_MAX         256
#define CEL_HTTPRESPONSE_STREAM_BUFFER_SIZE  8192

typedef enum _CelHttpResponseReadingState
{
    CEL_HTTPRESPONSE_READING_INIT = 0,
    CEL_HTTPRESPONSE_READING_VERSION,
    CEL_HTTPRESPONSE_READING_STATUS,
    CEL_HTTPRESPONSE_READING_REASON,
    CEL_HTTPRESPONSE_READING_HEADER,
    CEL_HTTPRESPONSE_READING_BODY,
    CEL_HTTPRESPONSE_READING_OK,
}CelHttpResponseReadingState;

typedef enum _CelHttpResponseWritingState
{
    CEL_HTTPRESPONSE_WRITING_INIT = 0,
    CEL_HTTPRESPONSE_WRITING_VERSION,
    CEL_HTTPRESPONSE_WRITING_STATUS,
    CEL_HTTPRESPONSE_WRITING_REASON,
    CEL_HTTPRESPONSE_WRITING_HEADER,
    CEL_HTTPRESPONSE_WRITING_BODY,
    CEL_HTTPRESPONSE_WRITING_OK
}CelHttpResponseWritingState;

/* Response status code */
typedef enum _CelHttpStatusCode
{
    CEL_HTTPSC_CONTINUE = 100,
    CEL_HTTPSC_SWICHING = 101,

    CEL_HTTPSC_REQUEST_OK = 200,
    CEL_HTTPSC_CREATED = 201,
    CEL_HTTPSC_ACCEPTED = 202,
    CEL_HTTPSC_NON_AUTH = 203,              /* Provisional information */
    CEL_HTTPSC_NO_CONTENT = 204,
    CEL_HTTPSC_RESET_CONTENT = 205,
    CEL_HTTPSC_PARTIAL_CONTENT = 206,

    CEL_HTTPSC_MULTIPLE = 300,              /* Multiple choices */
    CEL_HTTPSC_MOVED_PERM  = 301,
    CEL_HTTPSC_MOVED_TEMP = 302,
    CEL_HTTPSC_SEE_OTHER = 303,
    CEL_HTTPSC_NOT_MODIFIED = 304,
    CEL_HTTPSC_USE_PROXY = 305,
    CEL_HTTPSC_TEMP_REDIRECT = 307,

    CEL_HTTPSC_BAD_REQUEST =400,
    CEL_HTTPSC_UNAUTHORIZED = 401,
    CEL_HTTPSC_PAYMENT = 402,               /* Payment required */
    CEL_HTTPSC_FORBIDDEN = 403, 
    CEL_HTTPSC_NOT_FOUND = 404, 
    CEL_HTTPSC_METHOD_NA = 405,             /* Method not allowed */
    CEL_HTTPSC_NONE_ACC = 406,              /* None acceptable */
    CEL_HTTPSC_PROXY = 407,                 /* Proxy authentication required */
    CEL_HTTPSC_REQUEST_TO = 408,            /* Request timeout */
    CEL_HTTPSC_CONFLICT = 409,
    CEL_HTTPSC_GONE = 410,
    CEL_HTTPSC_LENGTH_REQUIRED = 411,
    CEL_HTTPSC_PRECONDIT_FAILED = 412,
    CEL_HTTPSC_ENTITY_LARGE = 413,
    CEL_HTTPSC_URL_LARGE = 414,
    CEL_HTTPSC_UNSUPPORTED_MEDIA = 415,
    CEL_HTTPSC_RANGE_ERROR = 416,
    CEL_HTTPSC_EXPECTATION_FAILED = 417,
    CEL_HTTPSC_CLIENT_MANY = 421,
    /* There are too many connections from your internet address */
    CEL_HTTPSC_ENITY_ERROR = 422,           /* Unprocessable Entity */
    CEL_HTTPSC_LOCKED = 423, 
    CEL_HTTPSC_FAILED_DEPENDENCY = 424,
    CEL_HTTPSC_UNORDERED_COLLECT = 425,     /* Unordered Collection */
    CEL_HTTPSC_UPGRADE_REQUIRED = 426,
    CEL_HTTPSC_RETRY_WITH = 427,

    CEL_HTTPSC_ERROR = 500,                 /* Internal server error */
    CEL_HTTPSC_NOT_IMP = 501,               /* Not implemented */
    CEL_HTTPSC_BAD_GATEWAY = 502,
    CEL_HTTPSC_SERVICE_UNAV = 503,          /* Service unavailable */
    CEL_HTTPSC_GATEWAY_TO = 504,            /* Gateway timeout */
    CEL_HTTPSC_BAD_VERSION = 505
}CelHttpStatusCode;

typedef struct _CelHttpResponse CelHttpResponse;

typedef int (* CelHttpResponseBodyReadCallBack)(
    CelHttpResponse *rsp, CelStream *s, size_t len, void *user_data);
typedef int (* CelHttpResponseBodyWriteCallBack)(
    CelHttpResponse *rsp, CelStream *s, void *user_data);
/*
 * Response = Status-Line ; RFC2616.6.1
 *            *((general-header ;   RFC2616.4.5
 *             | response-header ;   RFC2616.6.2
 *             | entity-header)CRLF);  RFC2616.7.1
 *           CRLF
 *           [ message-body ] ;  RFC2616.7.2
 *
 * Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
 */
struct _CelHttpResponse
{
    CelStream s;
    void *_req;
    void *_async_callback;

    CelHttpResponseReadingState reading_state;
    CelHttpError reading_error;
    size_t reading_hdr_offset;
    long long reading_body_offset;
    CelHttpResponseBodyReadCallBack body_reading_callback;
    void *body_reading_user_data;

    CelHttpResponseWritingState writing_state;
    CelHttpError writing_error;
    long long writing_body_offset;
    CelHttpResponseBodyWriteCallBack body_writing_callback;
    void *body_writing_user_data;

    CelHttpVersion ver;
    CelHttpStatusCode status;
    CelVStringA reason;
    U64 hdr_flags;
    CelVStringA accept_ranges;
    CelVStringA access_control_allow_credentials;
    CelVStringA access_control_allow_headers;
    CelVStringA access_control_allow_methods;
    CelVStringA access_control_allow_origin;
    CelVStringA access_control_expose_headers;
    long access_control_max_age;
    long age;
    CelVStringA allow;
    CelVString cache_control;
    CelHttpConnection connection; /* is keepalive */
    CelVStringA content_disposition;
    CelVStringA content_encoding;
    CelVStringA content_language;
    long long content_length;
    CelVStringA content_location;
    CelVStringA content_md5;
    CelHttpContentRange content_range;
    CelVStringA content_type;
    CelDateTime date;
    CelVStringA etag;
    CelDateTime expires;
    CelDateTime last_modified;
    CelVStringA location;
    CelVStringA pragma;
    CelVStringA proxy_authenticate;
    CelVStringA retry_after;
    CelVStringA server;
    CelHttpCookie set_cookie;
    CelVStringA trailer;
    CelHttpTransferEncoding transfer_encoding;
    CelVStringA upgrade;
    CelVStringA vary;
    CelVStringA via;
    CelVStringA warning;
    CelVStringA www_authenticate;
    CelVStringA x_powered_by;
    CelRbTree ext_hdrs;

    CelStream hdr_cache;
    CelHttpChunked chunked;
    CelHttpBodySaveType body_save_in;
    CelHttpBodyCache body_cache;
};

#ifdef __cplusplus
extern "C" {
#endif

int cel_httpresponse_init(CelHttpResponse *rsp);
void cel_httpresponse_destroy(CelHttpResponse *rsp);
/*
 * Cel http response functions
 */
CelHttpResponse *cel_httpresponse_new(void);
void cel_httpresponse_free(CelHttpResponse *rsp);

void cel_httpresponse_clear(CelHttpResponse *rsp);

#define cel_httpresponse_set_body_reading_callback(rsp, _callback, _user_data)\
    (rsp)->body_reading_callback = _callback, \
    (rsp)->body_reading_user_data = _user_data
#define cel_httpresponse_set_body_writing_callback(rsp, _callback, _user_data)\
    (rsp)->body_writing_callback = _callback, \
    (rsp)->body_writing_user_data = _user_data

#define cel_httpresponse_get_reading_state(rsp) (rsp)->reading_state
#define cel_httpresponse_get_reading_error(rsp) (rsp)->reading_error

#define cel_httpresponse_get_writing_state(rsp) (rsp)->writing_state
#define cel_httpresponse_get_writing_error(rsp) (rsp)->writing_error

int cel_httpresponse_reading(CelHttpResponse *rsp, CelStream *s);
int cel_httpresponse_writing(CelHttpResponse *rsp, CelStream *s);

#define cel_httpresponse_get_version(rsp) (rsp)->ver
#define cel_httpresponse_set_version(rsp, ver) (rsp)->ver = ver
#define cel_httpresponse_get_statuscode(rsp) (rsp)->status
#define cel_httpresponse_set_statuscode(rsp, _status) (rsp)->status = _status

void *cel_httpresponse_get_header(CelHttpResponse *rsp, 
                                  CelHttpHeader hdr_index);
int cel_httpresponse_set_header(CelHttpResponse *rsp,
                                CelHttpHeader hdr_index,
                                const void *value, size_t value_len);
static __inline 
char *cel_httpresponse_get_ext_header(CelHttpResponse *rsp, char *hdr_name)
{
    return (char *)cel_rbtree_lookup(&(rsp->ext_hdrs), hdr_name);
}
static __inline 
void cel_httpresponse_set_ext_header(CelHttpResponse *rsp, 
                                     char *hdr_name, char *hdr_value)
{
    cel_rbtree_insert(&(rsp->ext_hdrs), 
        cel_strdup(hdr_name), cel_strdup(hdr_value));
}
/* 
 * bytes=1000-2000  first = 1000, last = 2000
 * bytes=1000-      first = 1000, last = 0
 * bytes=-1000      first = -1000, last = 0
 */
#define cel_httpresponse_get_bodey_size(rsp) (rsp)->reading_body_offset
static __inline 
long long cel_httpresponse_get_body_data(CelHttpResponse *rsp, 
                                        long long first, long long last,
                                        void *buf, size_t buf_size)
{
    return cel_httpbodycache_read(
        &(rsp->body_cache), first, last, buf, buf_size);
}
static __inline 
long long cel_httpresponse_save_body_data(CelHttpResponse *rsp,
                                         long long first, long long last,
                                         const char *file_path) 
{
    return cel_httpbodycache_save_file(
        &(rsp->body_cache), first, last, file_path);
}
static __inline int cel_httpresponse_move_body_data(CelHttpResponse *rsp,
                                                    const char *file_path)
{
    return cel_httpbodycache_move_file(&(rsp->body_cache), file_path);
}

#define cel_httpresponse_get_stream(rsp) (&(rsp->s))
void *cel_httpresponse_get_send_buffer(CelHttpResponse *rsp);
size_t cel_httpresponse_get_send_buffer_size(CelHttpResponse *rsp);
void cel_httpresponse_seek_send_buffer(CelHttpResponse *rsp, int offset);
int cel_httpresponse_resize_send_buffer(CelHttpResponse *rsp, size_t resize);

int cel_httpresponse_write(CelHttpResponse *rsp, const void *buf, size_t size);
int cel_httpresponse_vprintf(CelHttpResponse *rsp, 
                             const char *fmt, va_list _args);
int cel_httpresponse_printf(CelHttpResponse *rsp, const char *fmt, ...);
int cel_httpresponse_end(CelHttpResponse *rsp);

int cel_httpresponse_send(CelHttpResponse *rsp, 
                          CelHttpStatusCode status, 
                          const void *content, size_t content_len);
int cel_httpresponse_send_redirect(CelHttpResponse *rsp, const char *url);
/* 
 * bytes=1000-2000  first = 1000, last = 2000
 * bytes=1000-      first = 1000, last = 0
 * bytes=-1000      first = -1000, last = 0
 */
int cel_httpresponse_send_file(CelHttpResponse *rsp, 
                               const char *file_path, 
                               long long first, long long last,
                               CelDateTime *if_modified_since, 
                               char *if_none_match);

int cel_httpresponse_recv_file(CelHttpResponse *rsp, const char *file_path);

#ifdef __cplusplus
}
#endif

#endif
