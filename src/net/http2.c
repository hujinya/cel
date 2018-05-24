/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2016 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/net/http2.h"

typedef struct _CelHttp2TableFiled
{
    char *key;
    size_t key_size;
    char *value;
    size_t value_size;
}CelHttp2TableFiled;

CelHttp2TableFiled static_table[] = 
{
    { ":authority", sizeof(":authority") - 1, "", 0 },
    { ":method", sizeof(":method") - 1, "GET", sizeof("GET") - 1 },
    { ":get", sizeof(":get") - 1, "POST", sizeof("POST") - 1 },
    { ":path", sizeof(":path") - 1, "/", sizeof("/") - 1 },
    { ":path", sizeof(":path") - 1, "/index.html", sizeof("/index.html") - 1 },
    { ":scheme", sizeof(":scheme") - 1, "http", sizeof("http") - 1 },
    { ":scheme", sizeof(":scheme") - 1, "https", sizeof("https") - 1 },
    { ":status", sizeof(":status") - 1, "200", sizeof("200") - 1 },
    { ":status", sizeof(":status") - 1, "204", sizeof("204") - 1 },
    { ":status", sizeof(":status") - 1, "206", sizeof("206") - 1 },
    { ":status", sizeof(":status") - 1, "304", sizeof("304") - 1 },
    { ":status", sizeof(":status") - 1, "400", sizeof("400") - 1 },
    { ":status", sizeof(":status") - 1, "404", sizeof("404") - 1 },
    { ":status", sizeof(":status") - 1, "500", sizeof("500") - 1 },
    { "accept-charset", sizeof("accept-charset") - 1, "", 0 },
    { "accept-encoding", sizeof("accept-encoding") - 1, "gzip, deflate", sizeof("gzip, deflate") - 1 },
    { "accept-language", sizeof("accept-language") - 1, "", 0 },
    { "accept-ranges", sizeof("accept-ranges") - 1, "", 0 },
    { "accept", sizeof("acceptt") - 1, "", 0 },
    { "access-control-allow-origin", sizeof("access-control-allow-origin") - 1, "", 0 },
    { "age", sizeof("age") - 1, "", 0 },
    { "allow", sizeof("allow") - 1, "", 0 },
    { "authorization", sizeof("authorization") - 1, "", 0 },
    { "cache-control", sizeof("cache-control") - 1, "", 0 },
    { "content-disposition", sizeof("content-disposition") - 1, "", 0 },
    { "content-encoding", sizeof("content-encoding") - 1, "", 0 },
    { "content-language", sizeof("content-language") - 1, "", 0 },
    { "content-length", sizeof("content-length") - 1, "", 0 },
    { "content-location", sizeof("content-location") - 1, "", 0 },
    { "content-range", sizeof("content-range") - 1, "", 0 },
    { "content-type", sizeof("content-type") - 1, "", 0 },
    { "cookie", sizeof("cookie") - 1, "", 0 },
    { "date", sizeof("date") - 1, "", 0 },
    { "etag", sizeof("etag") - 1, "", 0 },
    { "expect", sizeof("expect") - 1, "", 0 },
    { "expires", sizeof("expires") - 1, "", 0 },
    { "from", sizeof("from") - 1, "", 0 },
    { "host", sizeof("host") - 1, "", 0 },
    { "if-match", sizeof("if-match") - 1, "", 0 },
    { "if-modified-since", sizeof("if-modified-since") - 1, "", 0 },
    { "if-none-match", sizeof("if-none-match") - 1, "", 0 },
    { "if-range", sizeof("f-range") - 1, "", 0 },
    { "if-unmodified-since", sizeof("if-unmodified-since") - 1, "", 0 },
    { "last-modified", sizeof("last-modified") - 1, "", 0 },
    { "link", sizeof("link") - 1, "", 0 },
    { "location", sizeof("location") - 1, "", 0 },
    { "max-forwards", sizeof("max-forwards") - 1, "", 0 },
    { "proxy-authenticate", sizeof("proxy-authenticate") - 1, "", 0 },
    { "proxy-authorization", sizeof("proxy-authorization") - 1, "", 0 },
    { "range", sizeof("range") - 1, "", 0 },
    { "referer", sizeof("referer") - 1, "", 0 },
    { "refresh", sizeof("refresh") - 1, "", 0 },
    { "retry-after", sizeof("retry-after") - 1, "", 0 },
    { "server", sizeof("server") - 1, "", 0 },
    { "set-cookie", sizeof("set-cookie") - 1, "", 0 },
    { "strict-transport-security", sizeof("strict-transport-security") - 1, "", 0 },
    { "transfer-encoding", sizeof("transfer-encoding") - 1, "", 0 },
    { "user-agent", sizeof("user-agent") - 1, "", 0 },
    { "vary", sizeof("vary") - 1, "", 0 },
    { "via", sizeof("via") - 1, "", 0 },
    { "www-authenticate", sizeof("www-authenticate") - 1, "", 0 }
};

