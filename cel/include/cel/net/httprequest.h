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
#ifndef __CEL_NET_HTTPREQUEST_H__
#define __CEL_NET_HTTPREQUEST_H__

#include "cel/file.h"
#include "cel/rbtree.h"
#include "cel/net/http.h"
#include "cel/net/httpstream.h"
#include "cel/net/httpmultipart.h"

#define CEL_HTTPMETHOD_LEN_MAX                  7
#define CEL_HTTPREQUEST_STREAM_BUFFER_SIZE   8192

typedef enum _CelHttpRequsetReadingState
{
    CEL_HTTPREQUEST_READING_INIT = 0,
    CEL_HTTPREQUEST_READING_METHOD,
    CEL_HTTPREQUEST_READING_URL,
    CEL_HTTPREQUEST_READING_VERSION,
    CEL_HTTPREQUEST_READING_HEADER,
    CEL_HTTPREQUEST_READING_BODY,
    CEL_HTTPREQUEST_READING_OK,
}CelHttpRequestReadingState;

typedef enum _CelHttpRequsetWritingState
{
    CEL_HTTPREQUEST_WRITING_INIT = 0, 
    CEL_HTTPREQUEST_WRITING_METHOD,
    CEL_HTTPREQUEST_WRITING_URL,
    CEL_HTTPREQUEST_WRITING_VERSION,
    CEL_HTTPREQUEST_WRITING_HEADER, 
    CEL_HTTPREQUEST_WRITING_BODY, 
    CEL_HTTPREQUEST_WRITING_OK,
}CelHttpRequsetWritingState;

/* Request method */
typedef enum _CelHttpMethod
{
    CEL_HTTPM_UNDEFINED = -1,
    CEL_HTTPM_CONNECT = 0,
    CEL_HTTPM_DELETE = 1,
    CEL_HTTPM_GET = 2,
    CEL_HTTPM_HEAD = 3,
    CEL_HTTPM_OPTIONS = 4,
	CEL_HTTPM_PATCH = 5,
    CEL_HTTPM_POST = 6,
    CEL_HTTPM_PUT = 7,
    CEL_HTTPM_TRACE = 8,
    CEL_HTTPM_CONUT = 9
}CelHttpMethod;

typedef enum _CelHttpScheme
{
    CEL_HTTPSCHEME_UNDEFINED = -1,
    CEL_HTTPSCHEME_HTTP,
    CEL_HTTPSCHEME_HTTPS,
    CEL_HTTPSCHEME_FTP
}CelHttpScheme;

typedef struct _CelHttpUrl
{
    CelHttpScheme scheme;
    CelVStringA host;
    unsigned short port;
    CelVStringA path;
    CelVStringA query;
    CelVStringA fragment;
}CelHttpUrl;

typedef struct _CelHttpRequest CelHttpRequest;
typedef int (* CelHttpRequestBodyReadCallBack)(
	CelHttpRequest *req, CelStream *s, size_t len, void *user_data);

/*
 * Request = Request-Line ; RFC2616.5.1
 *           *((general-header ; RFC2616.4.5
 *            | request-header ; RFC2616.5.3
 *            | entity-header)CRLF); RFC2616.7.1
 *           CRLF
 *           [ message-body ] ;  RFC2616.4.3
 *
 * Request-Line = Method SP Request-URI SP HTTP-Version CRLF
 */
struct _CelHttpRequest
{
	CelHttpStream hs;
	void *_async_callback;

	CelHttpContentType body_content_type;
    CelHttpBodySaveType body_save_in;
    CelHttpBodyCache body_cache;
    CelHttpMultipart multipart;
    
    CelHttpRequestReadingState reading_state;
    size_t reading_hdr_offset;
    long long reading_body_offset;
    CelHttpRequestBodyReadCallBack body_reading_callback;
    void *body_reading_user_data;

    CelHttpRequsetWritingState writing_state;
    long long writing_body_offset;
    CelHttpStreamWriteCallBack body_writing_callback;
    void *body_writing_user_data;

    CelHttpMethod method;
    CelHttpVersion ver;
    CelHttpUrl url;

    U64 hdr_flags;
    CelVStringA accept;
    CelVStringA accept_charset;
    CelVStringA accept_encoding;
    CelVStringA accept_language;
    CelVStringA access_control_request_headers;
    CelVStringA access_control_request_method;
    CelVStringA allow;
    CelVStringA authorization;
    CelVStringA cache_control;
    CelHttpConnection connection; /* is keepalive */
    CelVStringA content_encoding;
    CelVStringA content_language;
    long long content_length;
    CelVStringA content_location;
    CelVStringA content_md5;
    CelHttpContentRange content_range;
    CelVStringA content_type;
    CelVStringA cookie;
    CelTime date;
    int dnt;
    CelVStringA expect;
    CelTime expires;
    CelVStringA from;
    CelVStringA host;
    CelVStringA if_match;
    CelTime if_modified_since;
    CelVStringA if_none_match;
    CelVStringA if_range;
    CelTime if_unmodified_since;
    CelTime last_modified;
    int max_forwards;
    CelVStringA origin;
    CelVStringA pragma;
    CelVStringA proxy_authorization;
    CelHttpRange range;
    CelVStringA referer;
    CelVStringA te;
    CelVStringA trailer;
    CelHttpTransferEncoding transfer_encoding;
    CelVStringA upgrade;
    int upgrade_insecure_requests;
    CelVStringA user_agent;
    CelVStringA via;
    CelVStringA warning;
    CelVStringA x_auth_token;
    CelVStringA x_forward_for;
    CelVStringA x_forward_host;
    CelVStringA x_forward_proto;
    CelVStringA x_real_ip;
    CelVStringA x_requested_with;
    CelRbTree ext_hdrs;
};

extern CelKeywordA g_requestmethod[];

#ifdef __cplusplus
extern "C" {
#endif

/* Cel http request functions */
int cel_httprequest_init(CelHttpRequest *req);
void cel_httprequest_destroy(CelHttpRequest *req);

CelHttpRequest *cel_httprequest_new(void);
void cel_httprequest_free(CelHttpRequest *req);

void cel_httprequest_clear(CelHttpRequest *req);

#define cel_httprequest_set_body_reading_callback(req, _callback, _user_data) \
    (req)->body_reading_callback = _callback, \
    (req)->body_reading_user_data = _user_data

#define cel_httprequest_set_body_writing_callback(req, _callback, _user_data) \
    (req)->body_writing_callback = _callback,\
    (req)->body_writing_user_data = _user_data

int cel_httprequest_reading(CelHttpRequest *req);
int cel_httprequest_writing(CelHttpRequest *req);

/* CelHttpMethod cel_httprequest_get_method(CelHttpRequest *req); */
#define cel_httprequest_get_method(req) (req)->method
const char *cel_httprequest_get_method_str(CelHttpRequest *req);
#define cel_httprequest_set_method(req, _method) (req)->method = _method

#define cel_httprequest_set_url_scheme(req, _scheme) (req)->url.scheme = scheme
#define cel_httprequest_get_url_scheme(req) \
    cel_vstring_str_a(&((req)->url.scheme))
#define cel_httprequest_set_url_host(req, _host, len) \
    cel_vstring_assign_a(&((req)->url.host), _host, len)
#define cel_httprequest_get_url_host(req) cel_vstring_str_a(&((req)->url.host))
#define cel_httprequest_set_url_port(req, _port) (req)->url.port = _port
#define cel_httprequest_get_url_port(req) (req)->url.port
#define cel_httprequest_set_url_path(req, _path, len) \
    cel_vstring_assign_a(&((req)->url.path), _path, len)
#define cel_httprequest_get_url_path(req) cel_vstring_str_a(&((req)->url.path))
#define cel_httprequest_get_url_path_size(req) \
    cel_vstring_size(&((req)->url.path))
#define cel_httprequest_set_url_query(req, _query, len) \
    cel_vstring_assign_a(&((req)->url.query), _query, len)
#define cel_httprequest_set_url_fragment(req, _fragment, len) \
    cel_vstring_assign_a(&((req)->url.fragment), _fragment, len)

int cel_httprequest_set_url_str(CelHttpRequest *req, const char *url_str);

/* Get and set httprequest query */
static __inline char *cel_httprequest_get_query(CelHttpRequest *req, 
                                                const char *key, 
                                                char *value, size_t *size)
{
    return cel_strgetkeyvalue_a(
        cel_vstring_str_a(&(req->url.query)), '&', '=', key, value, size);
}

static __inline 
int cel_httprequest_set_query(CelHttpRequest *req, 
                              const char *key, const char *value, size_t size)
{
    return cel_httpvstring_set_value(&(req->url.query), '&', '=', key, value, size);
}
#define cel_httprequest_get_query_string(req, key, str, size) \
    cel_keystr((CelKeyGetFunc)cel_httprequest_get_query, req, key, str, size)
#define cel_httprequest_get_query_bool(req, key, b) \
    cel_keybool((CelKeyGetFunc)cel_httprequest_get_query, req, key, b)
#define cel_httprequest_get_query_int(req, key, i) \
    cel_keyint((CelKeyGetFunc)cel_httprequest_get_query, req, key, i)
#define cel_httprequest_get_query_long(req, key, l)\
    cel_keylong((CelKeyGetFunc)cel_httprequest_get_query, req, key, l)
#define cel_httprequest_get_query_double(req, key, d) \
    cel_keydouble((CelKeyGetFunc)cel_httprequest_get_query, req, key, d)
/* Get and set httprequest form */
static __inline 
char *cel_httprequest_get_form(CelHttpRequest *req, const char *key, 
							   char *value, size_t *size)
{
    return cel_strgetkeyvalue_a((char *)cel_stream_get_buffer(&(req->body_cache.buf)),
        '&', '=', key, value, size);
}
#define cel_httprequest_get_form_string(req, key, str, size) \
    cel_keystr((CelKeyGetFunc)cel_httprequest_get_form, req, key, str, size)
#define cel_httprequest_get_form_bool(req, key, b) \
    cel_keybool((CelKeyGetFunc)cel_httprequest_get_form, req, key, b)
#define cel_httprequest_get_form_int(req, key, i) \
    cel_keyint((CelKeyGetFunc)cel_httprequest_get_form, req, key, i)
#define cel_httprequest_get_form_long(req, key, l)\
    cel_keylong((CelKeyGetFunc)cel_httprequest_get_form, req, key, l)
#define cel_httprequest_get_form_double(req, key, d) \
    cel_keydouble((CelKeyGetFunc)cel_httprequest_get_form, req, key, d)
/* Get httprequest params */
char *cel_httprequest_get_params(CelHttpRequest *req, const char *key, 
                                 char *value, size_t *size);
#define cel_httprequest_get_params_string(req, key, str, size) \
    cel_keystr((CelKeyGetFunc)cel_httprequest_get_params, req, key, str, size)
#define cel_httprequest_get_params_bool(req, key, b) \
    cel_keybool((CelKeyGetFunc)cel_httprequest_get_params, req, key, b)
#define cel_httprequest_get_params_int(req, key, i) \
    cel_keyint((CelKeyGetFunc)cel_httprequest_get_params, req, key, i)
#define cel_httprequest_get_params_long(req, key, l)\
    cel_keylong((CelKeyGetFunc)cel_httprequest_get_params, req, key, l)
#define cel_httprequest_get_params_double(req, key, d) \
    cel_keydouble((CelKeyGetFunc)cel_httprequest_get_params, req, key, d)

/* CelHttpVersion cel_httprequest_get_version(CelHttpRequest *req); */
#define cel_httprequest_get_version(req) (req)->ver
static __inline const char *cel_httprequest_get_version_str(CelHttpRequest *req)
{
	return g_httpversion[req->ver].key_word;
}
#define cel_httprequest_set_version(req, version) (req)->ver = version

void *cel_httprequest_get_header(CelHttpRequest *req, CelHttpHeader hdr_index);
int cel_httprequest_set_header(CelHttpRequest *req, CelHttpHeader hdr_index,
                               const void *value, size_t value_len);
static __inline 
char *cel_httprequest_get_ext_header(CelHttpRequest *req, char *hdr_name)
{
    return (char *)cel_rbtree_lookup(&(req->ext_hdrs), hdr_name);
}
static __inline 
void cel_httprequest_set_ext_header(CelHttpRequest *req, 
                                    char *hdr_name, char *hdr_value)
{
    cel_rbtree_insert(&(req->ext_hdrs), 
        cel_strdup(hdr_name), cel_strdup(hdr_value));
}

#define cel_httprequest_get_recv_body_size(req) (req)->reading_body_offset
/* 
 * bytes=1000-2000  first = 1000, last = 2000
 * bytes=1000-      first = 1000, last = 0
 * bytes=-1000      first = -1000, last = 0
 */
static __inline 
long long cel_httprequest_get_body_data(CelHttpRequest *req, 
                                        long long first, long long last,
                                        void *buf, size_t buf_size)
{
    return cel_httpbodycache_read(&(req->body_cache), first, last, buf, buf_size);
}
static __inline 
long long cel_httprequest_save_body_data(CelHttpRequest *req,
                                         long long first, long long last,
                                         const char *file_path) 
{
    return cel_httpbodycache_save_file(&(req->body_cache), first, last, file_path);
}
static __inline int cel_httprequest_move_body_data(CelHttpRequest *req,
                                                   const char *file_path)
{
    return cel_httpbodycache_move_file(&(req->body_cache), file_path);
}

#define cel_httprequest_is_multipart(req) \
    ((req)->body_content_type == CEL_HTTPCONTENTTYPE_MULTIPART)
static __inline 
CelHttpMultipart *cel_httprequest_get_multipart(CelHttpRequest *req)
{
    return &(req->multipart);
}

#define cel_httprequest_get_stream(req) (&(req->hs.s))
void *cel_httprequest_get_send_buffer(CelHttpRequest *req);
size_t cel_httprequest_get_send_buffer_size(CelHttpRequest *req);
void cel_httprequest_seek_send_buffer(CelHttpRequest *req, int offset);
int cel_httprequest_resize_send_buffer(CelHttpRequest *req, size_t resize);

int cel_httprequest_write(CelHttpRequest *req, const void *buf, size_t size);
int cel_httprequest_vprintf(CelHttpRequest *req, 
                            const char *fmt, va_list _args);
int cel_httprequest_printf(CelHttpRequest *req, const char *fmt, ...);
int cel_httprequest_end(CelHttpRequest *req);

int cel_httprequest_send(CelHttpRequest *req, 
                         CelHttpMethod method, const char *url,
                         const void *content, size_t content_len);
/* 
 * bytes=1000-2000  first = 1000, last = 2000
 * bytes=1000-      first = 1000, last = 0
 * bytes=-1000      first = -1000, last = 0
 */
int cel_httprequest_post_file(CelHttpRequest *req, 
                              CelHttpMethod method, const char *url,
                              const char *file_path, 
                              long long first, long long last);

//int cel_httprequest_post_multipart(CelHttpRequest *req, const char *url,
//                                   CelHttpMultipart *multi_part);

#ifdef __cplusplus
}
#endif

#endif
