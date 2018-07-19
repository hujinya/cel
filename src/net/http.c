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
#include "cel/net/http.h"
#include "cel/error.h"
#include "cel/log.h"
#include "cel/datetime.h"
#include "cel/convert.h"
#include "cel/file.h"

/* Debug defines */
#define Debug(args)   /* cel_log_debug args */
#define Warning(args) CEL_SETERRSTR(args)/* cel_log_warning args */
#define Err(args)   CEL_SETERRSTR(args)/* cel_log_err args */

CelKeywordA g_httpversion[] =
{
    { sizeof("HTTP/0.9") - 1, "HTTP/0.9"},
    { sizeof("HTTP/1.0") - 1, "HTTP/1.0"},
    { sizeof("HTTP/1.1") - 1, "HTTP/1.1"},
    { sizeof("HTTP/2.0") - 1, "HTTP/2.0"},
};

CelHttpHeaderHandler g_httpvstring_handler = {
    sizeof(CelVString),
    (CelHttpHeaderInitFunc)cel_vstring_init_a,
    (CelHttpHeaderDestroyFunc)cel_vstring_destroy_a,
    (CelHttpHeaderGetFunc)cel_vstring_str_a,
    (CelHttpHeaderSetFunc)cel_vstring_assign_a,
    (CelHttpHeaderReadingFunc)cel_httpvstring_reading, 
    (CelHttpHeaderWritingFunc)cel_httpvstring_writing
};
CelHttpHeaderHandler g_httpvdatetime_handler = {
    sizeof(CelDateTime),
    (CelHttpHeaderInitFunc)cel_datetime_init,
    (CelHttpHeaderDestroyFunc)cel_datetime_destroy,
    (CelHttpHeaderGetFunc)NULL,
    (CelHttpHeaderSetFunc)memcpy,
    (CelHttpHeaderReadingFunc)cel_httpdatetime_reading, 
    (CelHttpHeaderWritingFunc)cel_httpdatetime_writing
};
CelHttpHeaderHandler g_httpint_handler = {
    sizeof(int),
    NULL,
    NULL,
    NULL,
    (CelHttpHeaderSetFunc)memcpy,
    (CelHttpHeaderReadingFunc)cel_httpint_reading,
    (CelHttpHeaderWritingFunc)cel_httpint_writing
};
CelHttpHeaderHandler g_httpl_handler = {
    sizeof(long),
    NULL,
    NULL,
    NULL,
    (CelHttpHeaderSetFunc)memcpy,
    (CelHttpHeaderReadingFunc)cel_httpl_reading, 
    (CelHttpHeaderWritingFunc)cel_httpl_writing
};
CelHttpHeaderHandler g_httpll_handler = {
    sizeof(long long),
    NULL,
    NULL,
    NULL,
    (CelHttpHeaderSetFunc)memcpy,
    (CelHttpHeaderReadingFunc)cel_httpll_reading, 
    (CelHttpHeaderWritingFunc)cel_httpll_writing
};
CelHttpHeaderHandler g_httpconnection_handler = {
    sizeof(CelHttpConnection),
    NULL,
    NULL,
    NULL,
    (CelHttpHeaderSetFunc)memcpy,
    (CelHttpHeaderReadingFunc)cel_httpconnection_reading, 
    (CelHttpHeaderWritingFunc)cel_httpconnection_writing
};
CelHttpHeaderHandler g_httpcontentrange_handler = {
    sizeof(CelHttpContentRange),
    NULL,
    NULL,
    NULL,
    (CelHttpHeaderSetFunc)memcpy,
    (CelHttpHeaderReadingFunc)cel_httpcontentrange_reading,
    (CelHttpHeaderWritingFunc)cel_httpcontentrange_writing
};

CelHttpHeaderHandler g_httpcookie_handler = {
    sizeof(CelHttpCookie),
    (CelHttpHeaderInitFunc)cel_httpcookie_init,
    (CelHttpHeaderDestroyFunc)cel_httpcookie_destroy,
    (CelHttpHeaderGetFunc)NULL,
    (CelHttpHeaderSetFunc)cel_httpcookie_set,
    (CelHttpHeaderReadingFunc)cel_httpcookie_reading, 
    (CelHttpHeaderWritingFunc)cel_httpcookie_writing
};

CelHttpHeaderHandler g_httprange_handler = {
    sizeof(CelHttpRange),
    NULL,
    NULL,
    NULL,
    (CelHttpHeaderSetFunc)memcpy,
    (CelHttpHeaderReadingFunc)cel_httprange_reading, 
    (CelHttpHeaderWritingFunc)cel_httprange_writing
};

CelHttpHeaderHandler g_httptransferencoding_handler = {
    sizeof(CelHttpTransferEncoding),
    NULL,
    NULL,
    NULL,
    (CelHttpHeaderSetFunc)memcpy,
    (CelHttpHeaderReadingFunc)cel_httptransferencoding_reading,
    (CelHttpHeaderWritingFunc)cel_httptransferencoding_writing
};

CelKeywordA g_case_httphdr[] = 
{
    { sizeof("accept") - 1, 
    "accept", "Accept", &g_httpvstring_handler },                       /* rfc2616.14.1 */
    { sizeof("accept-charset") - 1, 
    "accept-charset", "Accept-Charset", &g_httpvstring_handler },       /* rfc2616.14.2 */
    { sizeof("accept-encoding") - 1,
    "accept-encoding", "Accept-Encoding", &g_httpvstring_handler },     /* rfc2616.14.3 */
    { sizeof("accept-language") - 1,
    "accept-language", "Accept-Language", &g_httpvstring_handler },     /* rfc2616.14.4 */
    { sizeof("accept-ranges") - 1, 
    "accept-ranges", "Accept-Ranges", &g_httpvstring_handler },         /* rfc2616.14.5 */

    { sizeof("access-control-allow-credentials") - 1, 
    "access-control-allow-credentials", "Access-Control-Allow-Credentials", 
    &g_httpvstring_handler },
    { sizeof("access-control-allow-headers") - 1,
    "access-control-allow-headers", "Access-Control-Allow-Headers", 
    &g_httpvstring_handler },
    { sizeof("access-control-allow-methods") - 1, 
    "access-control-allow-methods", "Access-Control-Allow-Methods", 
    &g_httpvstring_handler },
    { sizeof("access-control-allow-origin") - 1, 
    "access-control-allow-origin", "Access-Control-Allow-Origin", 
    &g_httpvstring_handler },
    { sizeof("access-control-expose-headers") - 1, 
    "access-control-expose-headers", "Access-Control-Expose-Headers", 
    &g_httpvstring_handler },

    { sizeof("access-control-max-age") - 1, 
    "access-control-max-age", "Access-Control-Max-Age", &g_httpl_handler },
    { sizeof("access-control-request-headers") - 1, 
    "access-control-request-headers", "Access-Control-Request-Headers", 
    &g_httpvstring_handler },
    { sizeof("access-control-request-method") - 1,
    "access-control-request-method", "Access-Control-Request-Method", 
    &g_httpvstring_handler },
    { sizeof("age") - 1, "Age", "Age", &g_httpl_handler },              /* rfc2616.14.6 */
    { sizeof("allow") - 1,"Allow", "Allow", &g_httpvstring_handler },   /* rfc2616.14.7 */

    { sizeof("authorization") - 1, 
    "authorization", "Authorization", &g_httpvstring_handler },         /* rfc2616.14.8 */
    { sizeof("cache-control") - 1, 
    "cache-control", "Cache-Control", &g_httpvstring_handler },         /* rfc2616.14.9 */
    { sizeof("connection") - 1, 
    "connection", "Connection", &g_httpconnection_handler },            /* rfc2616.14.10 */
    { sizeof("content-disposition") - 1, 
    "content-disposition", "Content-Disposition", &g_httpvstring_handler }, 
    { sizeof("content-encoding") - 1, 
    "content-encoding", "Content-Encoding", &g_httpvstring_handler },   /* rfc2616.14.11 */

    { sizeof("content-language") - 1, 
    "content-language", "Content-Language", &g_httpvstring_handler },   /* rfc2616.14.12 */
    { sizeof("content-length") - 1, 
    "content-length", "Content-Length", &g_httpll_handler },            /* rfc2616.14.13 */
    { sizeof("content-location") - 1, 
    "content-location", "Content-Location", &g_httpvstring_handler },   /* rfc2616.14.14 */
    { sizeof("content-md5") - 1, 
    "content-md5", "Content-MD5", &g_httpvstring_handler },             /* rfc2616.14.15 */
    { sizeof("content-range") - 1, 
    "content-range", "Content-Range", &g_httpcontentrange_handler },    /* rfc2616.14.16 */

    { sizeof("content-type") - 1, 
    "content-type", "Content-Type", &g_httpvstring_handler },           /* rfc2616.14.17 */
    { sizeof("cookie") - 1, 
    "cookie", "Cookie", &g_httpvstring_handler },                       /* rfc6265 */
    { sizeof("date") - 1, 
    "date", "Date", &g_httpvdatetime_handler },                         /* rfc2616.14.18 */
    { sizeof("dnt") - 1, 
    "dnt", "DNT", &g_httpint_handler },                                 /* do not track */
                                                                        /* mime */
    { sizeof("etag") - 1, 
    "etag", "Etag", &g_httpvstring_handler },                           /* rfc2616.14.19 */

    { sizeof("expect") - 1, 
    "expect", "Expect", &g_httpvstring_handler },                       /* rfc2616.14.20 */
    { sizeof("expires") - 1, 
    "expires", "Expires", &g_httpvdatetime_handler },                   /* rfc2616.14.21 */
    { sizeof("from") - 1, 
    "from", "From", &g_httpvstring_handler },                           /* rfc2616.14.22 */
    { sizeof("host") - 1, 
    "host", "Host", &g_httpvstring_handler },                           /* rfc2616.14.23 */
    { sizeof("if-match") - 1, 
    "if-match", "If-Match", &g_httpvstring_handler },                   /* rfc2616.14.24 */

    { sizeof("if-modified-since") - 1, 
    "if-modified-since", "If-Modified-Since", &g_httpvdatetime_handler }, 
                                                                        /* rfc2616.14.25 */
    { sizeof("if-none-match") - 1, 
    "if-none-match", "If-None-Match", &g_httpvstring_handler },         /* rfc2616.14.26 */
    { sizeof("if-range") - 1, 
    "if-range", "If-Range", &g_httpvstring_handler },                   /* rfc2616.14.27 */
    { sizeof("if-unmodified-since") - 1, 
    "if-unmodified-since", "If-Unmodified-Since", &g_httpvdatetime_handler }, 
                                                                        /* rfc2616.14.28 */
    { sizeof("last-modified") - 1,
    "last-modified", "Last-Modified", &g_httpvdatetime_handler },       /* rfc2616.14.29 */

    { sizeof("location") - 1, 
    "location", "Location", &g_httpvstring_handler },                   /* rfc2616.14.30 */
    { sizeof("max-forwards") - 1, 
    "max-forwards", "Max-Forwards", &g_httpint_handler },               /* rfc2616.14.31 */
    { sizeof("origin") - 1, 
    "origin", "Origin", &g_httpvstring_handler }, 
    { sizeof("pragma") - 1, 
    "pragma", "Pragma", &g_httpvstring_handler },                       /* rfc2616.14.32 */
    { sizeof("proxy-authenticate") - 1, 
    "proxy-authenticate", "Proxy-Authenticate", &g_httpvstring_handler },
                                                                        /* rfc2616.14.33 */

    { sizeof("proxy-authorization") - 1, 
    "proxy-authorization", "Proxy-Authorization", &g_httpvstring_handler }, 
                                                                        /* rfc2616.14.34 */
    { sizeof("range") - 1, 
    "range", "Range", &g_httprange_handler },                           /* rfc2616.14.35 */
    { sizeof("referer") - 1, 
    "referer", "Referer", &g_httpvstring_handler },                     /* rfc2616.14.36 */
    { sizeof("retry-after") - 1, 
    "retry-after", "Retry-After", &g_httpvstring_handler },             /* rfc2616.14.37 */
    { sizeof("server") - 1, 
    "server", "Server", &g_httpvstring_handler },                       /* rfc2616.14.38 */

    { sizeof("set-cookie") - 1, 
    "set-cookie", "Set-Cookie", &g_httpcookie_handler },
    { sizeof("te") - 1, 
    "te", "TE", &g_httpvstring_handler },                              /* rfc2616.14.39 */
    { sizeof("trailer") - 1, 
    "trailer", "Trailer", &g_httpvstring_handler },                    /* rfc2616.14.40 */
    { sizeof("transfer-encoding") - 1,
    "transfer-encoding", "Transfer-Encoding", &g_httptransferencoding_handler }, 
                                                                       /* rfc2616.14.41 */
    { sizeof("upgrade") - 1,
    "upgrade", "Upgrade", &g_httpvstring_handler },                    /* rfc2616.14.42 */

    { sizeof("upgrade-insecure-requests") - 1, 
    "upgrade-insecure-requests", "Upgrade-Insecure-Requests", &g_httpint_handler },
    { sizeof("user-agent") - 1, 
    "user-agent", "User-Agent", &g_httpvstring_handler  },             /* rfc2616.14.43 */
    { sizeof("vary") - 1, 
    "vary", "Vary", &g_httpvstring_handler },                          /* rfc2616.14.44 */
    { sizeof("via") - 1,
    "via", "Via", &g_httpvstring_handler },                            /* rfc2616.14.45 */
    { sizeof("warning") - 1, 
    "warning", "Warning", &g_httpvstring_handler },                    /* rfc2616.14.46 */

    { sizeof("www-authenticate") - 1, 
    "www-authenticate", "WWW-Authenticate", &g_httpvstring_handler },  /* rfc2616.14.47 */
    { sizeof("x-forwarded-for") - 1, 
    "x-forwarded-for", "X-Forwarded-For", &g_httpvstring_handler },
    { sizeof("x-forwarded-host") - 1, 
    "x-forwarded-host", "X-Forwarded-Host", &g_httpvstring_handler },
    { sizeof("x-forwarded-proto") - 1, 
    "x-forwarded-proto", "X-Forwarded-Proto", &g_httpvstring_handler },
    { sizeof("x-forwarded-server") - 1, 
    "x-forwarded-server", "X-Forwarded-Server", &g_httpvstring_handler },

    { sizeof("x-powered-by") - 1, 
    "x-powered-by", "X-Powered-By", &g_httpvstring_handler },
    { sizeof("x-real-ip") - 1, 
    "x-real-ip", "X-Real-IP", &g_httpvstring_handler },
    { sizeof("x-requested-with") - 1, 
    "x-requested-with", "X-Requested-With", &g_httpvstring_handler }
};

static unsigned char hexchars[] = "0123456789ABCDEF";

static int cel_http_htoi(const char *s)  
{
    int value;
    int c;

    c = ((unsigned char *)s)[0];
    if (CEL_ISUPPER_A(c))
        c = CEL_TOLOWER_A(c);
    value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

    c = ((unsigned char *)s)[1];
    if (CEL_ISUPPER_A(c))
        c = CEL_TOLOWER_A(c);
    value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

    return (value);
}

char *cel_http_url_encode(char *_dst, size_t *dest_size,
                          const char *_src, size_t src_size)
{  
    register unsigned char c;
    unsigned char *to, *start;
    unsigned char const *from, *end;

    from = (unsigned char *)_src;
    end  = (unsigned char *)_src + src_size;
    start = to = (unsigned char *)_dst;

    while (from < end)
    {
        c = *from++;

        if (c == ' ')
        {
            *to++ = '+';
        }
        else if ((c < '0' && c != '-' && c != '.') 
            || (c < 'A' && c > '9') 
            || (c > 'Z' && c < 'a' && c != '_') 
            || (c > 'z'))
        {  
            to[0] = '%';
            to[1] = hexchars[c >> 4];
            to[2] = hexchars[c & 15];
            to += 3;
        }
        else
        {  
            *to++ = c;
        }
    }
    *to = 0;
    if (dest_size)
    {
        *dest_size = to - start;
    }
    return (char *) start;
}

char *cel_http_url_decode(char *_dst, size_t *dest_size,
                          const char *_src, size_t src_size)
{
    char *dest = _dst;
    const char *data = _src;

    while (src_size--)
    {
        if (*data == '+')
        {
            *dest = ' ';
        }
        else if (*data == '%' 
            && src_size >= 2
            && CEL_ISXDIGIT_A((int) *(data + 1)) 
            && CEL_ISXDIGIT_A((int) *(data + 2)))
        {
            *dest = (char)cel_http_htoi(data + 1);
            data += 2;
            src_size -= 2;
        }
        else
        {
            *dest = *data;
        }
        data++;
        dest++;
    }
    *dest = '\0';
    *dest_size = dest - _dst;
    return _dst;
}

int cel_httpvstring_set_value(CelVString *vstr, 
                              char key_delimiter, char value_delimiter,
                              const char *key,
                              const char *value, size_t value_size)
{
    size_t key_start = 0, key_end;
    size_t value_start = 0, value_end;
    size_t i = 0, old_len, key_len;
    char *old_values;

    key_len = strlen(key);
    if ((old_len = cel_vstring_len(vstr)) > 0)
    {
        old_values = cel_strdup(cel_vstring_str_a(vstr));
        cel_vstring_clear(vstr);
        while (TRUE)
        {
            if (old_values[i] == '=')
            {
                key_end = i;
                value_start = i + 1;
            }
            if (old_values[i] == ';' || old_values[i] == '\0')
            {
                value_end = i;
                if (key_len == (key_end - key_start)
                    && memcmp(&old_values[key_start], key, key_len) == 0)
                    break;
                _cel_vstring_nappend_a(vstr, 
                    &old_values[key_start], value_end - key_start);
                cel_vstring_append_a(vstr, ';');
                if (old_values[i] == '\0')
                    break;
                key_start = value_start = i + 1;
            }
            i++;
        }
        _cel_vstring_nappend_a(vstr, key, key_len);
        cel_vstring_append_a(vstr, '=');
        _cel_vstring_nappend_a(vstr, value, value_size);
        if ((key_start = i + 1) < old_len)
            _cel_vstring_nappend_a(vstr, 
            &old_values[key_start], old_len - key_start);
        cel_free(old_values);
    }
    else
    {
         _cel_vstring_nappend_a(vstr, key, key_len);
        cel_vstring_append_a(vstr, '=');
        _cel_vstring_nappend_a(vstr, value, value_size);
    }

    return 0;
}

int cel_httpvstring_reading(CelVString *vstr, const char *value, size_t size)
{
    //printf("cel_httpvstring_reading %p %s %d\r\n", vstr, value, size);
    cel_vstring_assign_a(vstr, value, size);
    //puts(vstr->str);
    return 0;
}

int cel_httpvstring_writing(const char *hdr_name, 
                            CelVString *vstr, CelStream *s)
{
    //printf("%s size = %d\r\n", vstr->str, vstr->size);
    if (cel_vstring_len(vstr) > 0)
    {
        cel_stream_printf(s, "%s: %s\r\n", hdr_name, cel_vstring_str_a(vstr));
    }
    return 0;
}

int cel_httpdatetime_reading(CelDateTime *dt, const char *value, size_t size)
{
    return cel_datetime_init_strtime_a(dt, value);;
}

int cel_httpdatetime_writing(const char *hdr_name,
                             CelDateTime *dt, CelStream *s)
{
    int size;
    if (*dt != 0)
    {
        cel_stream_printf(s, "%s: ", hdr_name);
        if ((size = cel_datetime_strfgmtime_a(dt, 
            (char *)cel_stream_get_pointer(s), 
            cel_stream_get_remaining_capacity(s), 
            "%a, %d %b %Y %X GMT\r\n")) > 0)
            cel_stream_seek(s, size);
    }
    return  0;
}

int cel_httpint_reading(int *i, const char *value, size_t size)
{
    return (*i = atoi(value)) >= 0 ? 0 : -1;
}

int cel_httpint_writing(const char *hdr_name, int *i, CelStream *s)
{
    cel_stream_printf(s, "%s: %d\r\n", hdr_name, *i);
    return 0;
}

int cel_httpl_reading(long *l, const char *value, size_t size)
{
    return (*l = atol(value)) >= 0 ? 0 : -1;
}

int cel_httpl_writing(const char *hdr_name, long *l, CelStream *s)
{
    cel_stream_printf(s, "%s: %ld\r\n", hdr_name, *l);
    return 0;
}

int cel_httpll_reading(long long *ll, const char *value, size_t size)
{
    return (*ll = atoll(value)) >= 0 ? 0 : -1;
}

int cel_httpll_writing(const char *hdr_name, long long *ll, CelStream *s)
{
    cel_stream_printf(s, "%s: %lld\r\n", hdr_name, *ll);
    return 0;
}

int cel_httpconnection_reading(CelHttpConnection *connection, 
                               const char *value, size_t size)
{
    *connection = (memcmp(value, "close", size) == 0 
        ? CEL_HTTPCON_CLOSE : CEL_HTTPCON_KEEPALIVE);
    //_tprintf("*connection = %d\r\n", *connection);
    return 0;
}

int cel_httpconnection_writing(const char *hdr_name,
                               CelHttpConnection *connection, 
                               CelStream *s)
{
    if (*connection == CEL_HTTPCON_KEEPALIVE)
        cel_stream_printf(s, "%s: Keep-Alive\r\n", hdr_name);
    else if (*connection == CEL_HTTPCON_CLOSE)
        cel_stream_printf(s, "%s: close\r\n", hdr_name);
    return 0;
}

int cel_httpcontentrange_reading(CelHttpContentRange *content_range, 
                                 const char *value, size_t size)
{
    size_t i = 0;

    while (i < size && value[i++] == '=');
    content_range->first = atoll(&value[i]);
    while (i < size && value[i++] == '-');
    content_range->last = atoll(&value[i]);
    while (i < size && value[i++] == '/');
    content_range->total = atoll(&value[i]);

    return 0;
}

int cel_httpcontentrange_writing(const char *hdr_name, 
                                 CelHttpContentRange *content_range, 
                                 CelStream *s)
{
    cel_stream_printf(s, "%s: bytes=%lld-%lld/%lld\r\n", 
        hdr_name, 
        content_range->first, content_range->last, content_range->total);
    return 0;
}

int cel_httpcookie_init(CelHttpCookie *cookie)
{
    //puts("cel_httpcookie_init");
    cel_vstring_init_a(&(cookie->values));
    cel_datetime_init(&(cookie->expires));
    cookie->max_age = 0;
    cel_vstring_init_a(&(cookie->domain));
    cel_vstring_init_a(&(cookie->path));
    cookie->secure = FALSE;
    cookie->httponly = FALSE;

    return 0;
}

void cel_httpcookie_destroy(CelHttpCookie *cookie)
{
    cel_vstring_destroy_a(&(cookie->values));
    cel_datetime_destroy(&(cookie->expires));
    cookie->max_age = 0;
    cel_vstring_destroy_a(&(cookie->domain));
    cel_vstring_destroy_a(&(cookie->path));
    cookie->secure = FALSE;
    cookie->httponly = FALSE;
}

int cel_httpcookie_set_attribute(CelHttpCookie *cookie, 
                                 CelDateTime *expires, long max_age,
                                 const char *domain, const char *path, 
                                 BOOL secure, BOOL httponly)
{
    if (expires != NULL)
        memcpy(&(cookie->expires), expires, sizeof(CelDateTime));
    if (max_age > 0)
        cookie->max_age = max_age;
    if (domain != NULL)
        cel_vstring_assign_a(&(cookie->domain), domain, strlen(domain));
    if (path != NULL)
        cel_vstring_assign_a(&(cookie->path), path, strlen(path));
    cookie->secure = secure;
    cookie->httponly = httponly;

    return 0;
}

void *cel_httpcookie_set(CelHttpCookie *cookie1, 
                         CelHttpCookie *cookie2, size_t len)
{
    if (sizeof(CelHttpCookie) != len)
        return NULL;
    if (cel_vstring_len(&(cookie2->values)) > 0)
        cel_vstring_assign_a(&(cookie1->values), 
        cel_vstring_str_a(&(cookie2->values)), 
        cel_vstring_len(&(cookie2->values)));
    memcpy(&(cookie1->expires), 
        &(cookie2->expires), sizeof(CelDateTime));
    cookie1->max_age = cookie2->max_age;
    if (cel_vstring_len(&(cookie2->domain)) > 0)
        cel_vstring_assign_a(&(cookie1->domain), 
        cel_vstring_str_a(&(cookie2->domain)), 
        cel_vstring_len(&(cookie2->domain)));
    if (cel_vstring_len(&(cookie2->path)) > 0)
        cel_vstring_assign_a(&(cookie1->path), 
        cel_vstring_str_a(&(cookie2->path)), 
        cel_vstring_len(&(cookie2->path)));
    cookie1->secure = cookie2->secure;
    cookie1->httponly = cookie2->httponly;

    return cookie1;
}

int cel_httpcookie_reading(CelHttpCookie *cookie, 
                           const char *value, size_t size)
{
    size_t key_start = 0, key_end;
    size_t value_start = 0, value_end;
    size_t i = 0;

    while (i < size)
    {
        if (value[i] == '=')
        {
            key_end = i;
            value_start = i + 1;
        }
        if (value[i] == ';')
        {
            value_end = i;
            if (key_end - key_start == 7
                && memcmp(&value[key_start], 
                "Expires", key_end - key_start) == 0)
                cel_datetime_init_strtime_a(&(cookie->expires),
                &value[value_start]);
            else if (key_end - key_start == 6
                && memcmp(&value[key_start], 
                "Domain", key_end - key_start) == 0)
                cel_vstring_assign_a(&(cookie->domain),
                &value[value_start], value_end - value_start);
            else if (key_end - key_start == 4
                && memcmp(&value[key_start], 
                "Path", key_end - key_start) == 0)
                cel_vstring_assign_a(&(cookie->path), 
                &value[value_start], value_end - value_start);
            else if (key_end - key_start == 6
                && memcmp(&value[key_start], 
                "Secure", key_end - key_start) == 0)
                cookie->secure = TRUE;
            else if (key_end - key_start == 8
                && memcmp(&value[key_start], 
                "Httponly", key_end - key_start) == 0)
                cookie->httponly = TRUE;
            else
                cel_vstring_assign_a(&(cookie->values),
                &value[key_start], value_end - key_start);
            key_start = value_start = i + 1;
        }
        i++;
    }
    return 0;
}

int cel_httpcookie_writing(const char *hdr_name, 
                           CelHttpCookie *cookie, CelStream *s)
{
    int size;

    if (cel_vstring_len(&(cookie->values)) > 0)
    {
        cel_stream_printf(s, "%s: %s", 
            hdr_name, cel_vstring_str_a(&(cookie->values)));
        if (cookie->expires != 0)
        {
            if ((size = cel_datetime_strfgmtime_a(&(cookie->expires), 
                (char *)cel_stream_get_pointer(s), 
                cel_stream_get_remaining_capacity(s), 
                ";Expires=%a, %d %b %Y %X GMT")) > 0)
                cel_stream_seek(s, size);
        }
        if (cel_vstring_len(&(cookie->domain)) > 0)
            cel_stream_printf(s, ";Domain=%s", 
            cel_vstring_str_a(&(cookie->domain)));
        if (cel_vstring_len(&(cookie->path)) > 0)
            cel_stream_printf(s, ";Path=%s", 
            cel_vstring_str_a(&(cookie->path)));
        if (cookie->secure)
            cel_stream_printf(s, ";Secure");
        if (cookie->httponly)
            cel_stream_printf(s, ";Httponly");
        cel_stream_write(s, "\r\n", 2);
    }
    return 0;
}

int cel_httprange_reading(CelHttpRange *range, const char *value, size_t size)
{
    size_t i = 0;

    range->first = range->last = 0;
    while (i < size)
    {
        if (value[i] == '=')
            range->first = atoll(&value[++i]);
        else if (value[i] == '-')
            range->last = atoll(&value[++i]);
        else
            i++;
    }
    return 0;
}

int cel_httprange_writing(const char *hdr_name, 
                          CelHttpRange *range, CelStream *s)
{
    if (range->first < 0 && range->last == 0)
        cel_stream_printf(s, "%s: bytes=%lld\r\n", 
        hdr_name, range->last);
    if (range->first > 0 &&  range->last == 0)
        cel_stream_printf(s, "%s: bytes=%lld-\r\n", 
        hdr_name, range->first);
    else
        cel_stream_printf(s, "%s: bytes=%lld-%lld\r\n", 
        hdr_name, range->first, range->last);
    return 0;
}


int cel_httptransferencoding_reading(CelHttpTransferEncoding *transfer_encoding,
                                     const char *value, size_t size)
{
    if (memcmp(value, "identity", size) == 0)
        *transfer_encoding = CEL_HTTPTE_IDENTITY;
    else
        *transfer_encoding = CEL_HTTPTE_CHUNKED;
    return 0;
}

int cel_httptransferencoding_writing(const char *hdr_name, 
                                     CelHttpTransferEncoding *transfer_encoding,
                                     CelStream *s)
{
    if (*transfer_encoding == CEL_HTTPTE_IDENTITY)
        cel_stream_printf(s, "%s: identity\r\n", hdr_name);
    else if (*transfer_encoding == CEL_HTTPTE_CHUNKED)
        cel_stream_printf(s, "%s: chunked\r\n", hdr_name);
    return 0;
}

int cel_httpextheader_writing(char *hdr_name, char *value, CelStream *s)
{
    cel_stream_printf(s, "%s: %s\r\n", hdr_name, value);
    return 0;
}

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
long cel_http_chunked_reading(CelStream *s)
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
        Err((_T("Http chunk decode error.")));
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
        Err((_T("Http chunk decode error.")));
        return -2;
    }
}

long cel_http_chunked_last(CelHttpChunked *chunked, CelStream *s)
{
    cel_stream_set_position(s, chunked->start);
    if (chunked->size > 0)
    {
        sprintf((char *)cel_stream_get_pointer(s), 
            "%07x", chunked->size);
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

int cel_httpbodycache_init(CelHttpBodyCache *cache, size_t buf_max_size)
{
    cache->size = 0;
    cache->buf_max_size = buf_max_size;
    cel_stream_init(&(cache->buf));
    cache->clear_file = FALSE;
    cache->fp = NULL;
    cel_vstring_init(&(cache->file_path));
    
    return 0;
}

void cel_httpbodycache_destroy(CelHttpBodyCache *cache)
{
    char *file_path;

    cache->buf_max_size = 0;
    cache->size = 0;
    cel_stream_destroy(&(cache->buf));
    if (cache->fp != NULL)
    {
        cel_fclose(cache->fp);
        cache->fp = NULL;
    }
    if (cache->clear_file
        && (file_path = cel_vstring_str_a(&(cache->file_path))) != NULL)
        cel_fremove(file_path);
    cache->clear_file = FALSE;
    cel_vstring_destroy_a(&(cache->file_path));
   
    //puts("cel_httpbodycache_destroy");
}

void cel_httpbodycache_clear(CelHttpBodyCache *cache)
{
    char *file_path;

    //cache->buf_max_size = 0;
    cache->size = 0;
    cel_stream_clear(&(cache->buf));
    if (cache->fp != NULL)
    {
        cel_fclose(cache->fp);
        cache->fp = NULL;
    }
    if (cache->clear_file
        && (file_path = cel_vstring_str_a(&(cache->file_path))) != NULL)
        cel_fremove(file_path);
    cache->clear_file = FALSE;
    cel_vstring_clear_a(&(cache->file_path));
    //puts("cel_httpbodycache_clear");
}

int cel_httpbodycache_reading(CelHttpBodyCache *cache, 
                              const char *value, size_t size)
{
    size_t _size, w_size;
    CelDateTime dt;
    char dt_filename[15];

    //_size = cel_stream_get_length(&(cache->buf));
    if (cache->size + size > cache->buf_max_size)
    {
        if (cache->fp == NULL)
        {
            cel_datetime_init_now(&dt);
            cel_datetime_strfltime(&dt, dt_filename, 15, _T("%Y%m%d%H%M%S"));
            cel_vstring_resize_a(&(cache->file_path), CEL_PATHLEN);
            cache->file_path.size = snprintf(cache->file_path.str, CEL_PATHLEN,
                "%s%s_%ld.bdy", 
                cel_fullpath_a(CEL_HTTPBODY_CACHE_PATH), 
                dt_filename, cel_getticks());
            //puts(cel_vstring_str_a(&(cache->file_path)));
            if ((cache->fp = fopen(
                cel_vstring_str_a(&(cache->file_path)), "wb+")) == NULL
                && (cel_mkdirs_a(cel_filedir_a(
                cel_vstring_str_a(&(cache->file_path))), S_IRUSR|S_IWUSR) == -1 
                || (cache->fp = fopen(
                cel_vstring_str_a(&(cache->file_path)), "wb+")) == NULL))
            {
                puts("Open body cache file failed.");
                return -1;
            }
            cache->clear_file = TRUE;
            if ((_size = cel_stream_get_length(&(cache->buf))) > 0)
            {
                if ((_size = fwrite(
                    cel_stream_get_buffer(&(cache->buf)), 1, _size, cache->fp)) <= 0)
                    return -1;
                cel_stream_clear(&(cache->buf));
                //printf("copy write %d\r\n", _size);
            }
        }
        w_size = fwrite(value, 1, size, cache->fp);
        //printf("file write %d\r\n", w_size);
    }
    else
    {
        cel_stream_resize(&(cache->buf), cache->buf_max_size);
        w_size = cel_stream_write(&(cache->buf), value, size);
        *((char *)cache->buf.pointer) = '\0';
        cel_stream_seal_length(&(cache->buf));
        //printf("buf write %d\r\n", w_size);
    }
    cache->size += w_size;

    return (int)w_size;
}

int cel_httpbodycache_writing(CelHttpBodyCache *cache, CelStream *s)
{
    return 0;
}

long long cel_httpbodycache_read(CelHttpBodyCache *cache, 
                                 long long first, long long last,
                                 void *buf, size_t buf_size)
{
    long long size = 0;

    if (first < 0 && last == 0)
    {
        first = cache->size + first;
        last = cache->size - 1;
    }
    else if (first >= 0 && last == 0)
    {
        last = cache->size - 1;
    }
    if (buf_size < last - first)
        last = first + buf_size;
    if (cache->fp != NULL)
    {
        _fseeki64(cache->fp, first, SEEK_SET);
        size = fread(buf, 1, buf_size, cache->fp);
    }
    else 
    {
        memcpy(buf, cel_stream_get_buffer(&(cache->buf)) + first, buf_size);
    }

    return size;
}

long long cel_httpbodycache_save_file(CelHttpBodyCache *cache,
                                      long long first, long long last,
                                      const char *file_path)
{
    FILE *fp;
    size_t _size;
    long long size = 0;

    if (first < 0 && last == 0)
    {
        first = cache->size - first;
        last = cache->size - 1;
    }
    else if (first >= 0 && last == 0)
    {
        last = cache->size - 1;
    }
    if (first >= cache->size 
        || last >= cache->size)
    {
        printf("first %lld or last %lld offset %lld.\r\n", 
            first, last, cache->size);
        return -1;
    }
    if ((fp = fopen(file_path, "wb+")) == NULL 
        && (cel_mkdirs_a(cel_filedir_a(file_path), S_IRUSR|S_IWUSR) == -1
        || (fp = fopen(file_path, "wb+")) == NULL))
    {
        puts("cel_httprequest_save_body_data failed");
        return -1;
    }
    if (cache->fp != NULL)
    {
        _fseeki64(cache->fp, first, SEEK_SET);
        cel_stream_resize(&(cache->buf), cache->buf_max_size);
        while (first < last)
        {
            _size = fread(cache->buf.buffer,
                1, cache->buf_max_size, cache->fp);
            _size = fwrite(cache->buf.buffer, 1, _size, fp);
            first += _size;
            size += _size;
        }
    }
    else
    {
        size = fwrite(cel_stream_get_buffer(&(cache->buf)) + first,
            1, (size_t)(last - first), fp);
    }
    fclose(fp);

    return size;
}

int cel_httpbodycache_move_file(CelHttpBodyCache *cache, const char *file_path)
{
    if (cache->fp == NULL)
    {
        return (cel_httpbodycache_save_file(cache, 0, 0, file_path) != -1 
            ? 0 : -1);
    }
    else
    {
        /*_tprintf("cel_httpbodycache_move_file %s %s\r\n", 
            cel_vstring_str_a(&(cache->file_path)),
            cel_fullpath_a(file_path));*/
        cel_fclose(cache->fp);
        cache->fp = NULL;
        cache->clear_file = FALSE;
        if (cel_fmove(cel_vstring_str_a(&(cache->file_path)),
            cel_fullpath_a(file_path)) == 0)
        {
            cel_vstring_clear(&(cache->file_path));
            cache->size = 0;
            return 0;
        }
        puts(cel_geterrstr(cel_sys_geterrno()));
        return -1;
    }
}
