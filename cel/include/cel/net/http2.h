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
#ifndef __CEL_NET_HTTP2_H__
#define __CEL_NET_HTTP2_H__

#include "cel/types.h"
#include "cel/net/http.h"

/*
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|            Length (24)                        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Type (8)    |   Flags (8)   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|R|              Stream Identifier (31)                         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                Frame Payload (0...) ...                       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

#pragma pack(1)

#define CEL_HTTP2_DEFAULT_FRAME_SIZE           (1 << 14)
#define CEL_HTTP2_MAX_WINDOW                   ((1U << 31) - 1)
#define CEL_HTTP2_DEFAULT_WINDOW               65535

typedef enum _CelHttp2FrameType
{
    CEL_HTTP2_FRAME_DATA = 0x00,
    CEL_HTTP2_FRAME_HEADERS = 0x01,
    CEL_HTTP2_FRAME_PRIORITY = 0x02,
    CEL_HTTP2_FRAME_RST_STREAM = 0x03,
    CEL_HTTP2_FRAME_SETTINGS = 0x04,
    CEL_HTTP2_FRAME_PUSH_PROMISE = 0x05,
    CEL_HTTP2_FRAME_PING = 0x06,
    CEL_HTTP2_FRAME_GOAWAY = 0x07,
    CEL_HTTP2_FRAME_WINDOW_UPDATE = 0x08
}CelHttp2FarmeType;

typedef enum CelHttp2FrameFlag 
{
    CEL_HTTP2_END_STREAM = 0x01,   /* DATA, HEADERS */
    CEL_HTTP2_ACK = 0x01,          /* SETTINGS, PING */
    CEL_HTTP2_END_HEADERS = 0x04,  /* HEADERS, PUSH_PROMISE, CONTINUATION */
    CEL_HTTP2_PADDED = 0x08,       /* DATA, HEADERS, PUSH_PROMISE */
    CEL_HTTP2_PRIORITY = 0x20,     /* HEADERS */
}CelHttp2FrameFlag;

typedef enum _CelHttp2SettingsParameter
{
    CEL_HTTP2_HEADER_TABLE_SIZE = 0x1,
    CEL_HTTP2_ENABLE_PUSH = 0x2,
    CEL_HTTP2_MAX_CONCURRENT_STREAMS = 0x3,
    CEL_HTTP2_INITIAL_WINDOW_SIZE = 0x4,
    CEL_HTTP2_MAX_FRAME_SIZE = 0x5,
    CEL_HTTP2_MAX_HEADER_LIST_SIZE = 0x6,
}CelHttp2SettingsParameter;

typedef enum _CelHttp2Error
{
    CEL_HTTP2_NO_ERROR = 0x0,
    CEL_HTTP2_PROTOCOL_ERROR = 0x1,
    CEL_HTTP2_INTERNAL_ERROR = 0x2,
    CEL_HTTP2_FLOW_CONTROL_ERROR = 0x3,
    CEL_HTTP2_SETTINGS_TIMEOUT = 0x4,
    CEL_HTTP2_STREAM_CLOSED = 0x5,
    CEL_HTTP2_FRAME_SIZE_ERROR = 0x6,
    CEL_HTTP2_REFUSED_STREAM = 0x7,
    CEL_HTTP2_CANCEL = 0x8,
    CEL_HTTP2_COMPRESSION_ERROR = 0x9,
    CEL_HTTP2_CONNECT_ERROR = 0xa,
    CEL_HTTP2_ENHANCE_YOUR_CALM = 0xb,
    CEL_HTTP2_INADEQUATE_SECURITY = 0xc,
    CEL_HTTP2_HTTP_1_1_REQUIRED = 0xd
}CelHttp2Error;

typedef struct _CelHttp2FrameHeader
{
    U8 len[3];
    U8 type;
    U8 flags;
    union {
        struct {
            U32 id:31;
            U32 r:1;
        };
        U32 _id;
    };
}CelHttp2FrameHeader;

typedef struct _CelHttp2Priority
{
    CelHttp2FrameHeader hdr;
    union {
        struct {
            U32 stream_dependency:31;
            U32 e:1;
        };
        U32 stream_dependency1;
    };
    U8 weight;
}CelHttp2Priority;

typedef struct _CelHttp2FrameRstStream
{
    CelHttp2FrameHeader hdr;
    U32 error;
}CelHttp2FrameRstStream;

typedef struct _CelHttp2FrameSettings
{
    U16 setting_id;
    U32 value;
}CelHttp2FrameSettings;

typedef struct _CelHttp2FramePing
{
    CelHttp2FrameHeader hdr;
    U8 opaque_data[8];
}CelHttp2FramePing;

typedef struct _CelHttp2FrameGoAway
{
    CelHttp2FrameHeader hdr;
    union {
        struct {
            U32 last_stream_id:31;
            U32 r:1;
        };
        U32 last_stream__id;
    };
    U32 error;
}CelHttp2FrameGoAway;

typedef struct _CelHttp2WindowUpdate
{
    union {
        struct {
            U32 window_size_increment:31;
            U32 r:1;
        };
        U32 window_size_increment1;
    };
}CelHttp2WindowUpdate;

typedef union _CelHttp2Frame
{
    CelHttp2FrameHeader hdr;
    CelHttp2Priority priority;
    CelHttp2FrameRstStream rst_stream;
    CelHttp2FrameSettings settings;
    CelHttp2FramePing ping;
    CelHttp2FrameGoAway goaway;
    CelHttp2WindowUpdate window_update;
}CelHttp2Frame;

#define CEL_HTTP2_FRAME_HDR_SIZE     sizeof(CelHttp2FrameHeader)
#define CEL_HTTP2_PRIORITY_SIZE      sizeof(CelHttp2Priority) - CEL_HTTP2_FRAME_HDR_SIZE
#define CEL_HTTP2_RST_STREAM_SIZE    sizeof(CelHttp2FrameRstStream) - CEL_HTTP2_FRAME_HDR_SIZE
#define CEL_HTTP2_SETTINGS_SIZE      sizeof(CelHttp2FrameSettings) - CEL_HTTP2_FRAME_HDR_SIZE
#define CEL_HTTP2_PING_SIZE          sizeof(CelHttp2FramePing) - CEL_HTTP2_FRAME_HDR_SIZE
#define CEL_HTTP2_GOAWAY_SIZE        sizeof(CelHttp2FrameGoAway) - CEL_HTTP2_FRAME_HDR_SIZE
#define CEL_HTTP2_WINDOW_UPDATE_SIZE sizeof(CelHttp2WindowUpdate) - CEL_HTTP2_FRAME_HDR_SIZE

#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif

static __inline U32 cel_http2frameheader_get_len(CelHttp2FrameHeader *hdr)
{
    return (ntohl(*((U32 *)(hdr->len))) >> 8);
}

static __inline void cel_http2frameheader_set_len(CelHttp2FrameHeader *hdr, U32 len)
{
    len = htonl(len << 8);
    memcpy(hdr->len, &len, 3);
}

static __inline U32 cel_httpframeheader_get_stream_id(CelHttp2FrameHeader *hdr)
{
    return (ntohl(hdr->_id) & 0x7FFFFFFF);
}

static __inline void cel_httpframeheader_set_stream_id(CelHttp2FrameHeader *hdr, U32 id)
{
    hdr->id = id;
    hdr->r = 0;
    hdr->_id = htonl(hdr->_id);
}

#ifdef __cplusplus
}
#endif

#endif
