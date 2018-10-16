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
#ifndef __CEL_NET_SOCKS5_H__
#define __CEL_NET_SOCKS5_H__

#include "cel/types.h"

#pragma pack(1)

#define CEL_SOCKS5_VERSION 0x05

typedef enum _CelScoks5Method
{
    CEL_SOCKS5_METHOD_NO_AUTH = 0x00,
    CEL_SOCKS5_METHOD_GSSAPI = 0x01,
    CEL_SOCKS5_METHOD_USERNAME_PASSWORD = 0x02,
    CEL_SOCKS5_METHOD_INNA = 0x03,
    CEL_SOCKS5_METHOD_PRIVATE = 0x80,
    CEL_SOCKS5_METHOD_NO_ACCEPTABLE = 0xFF
}CelSocks5Method;

typedef struct _CelSocks5NegReq
{
    U8 ver;
    U8 n_methods;
    U8 methods;
}CelSocks5NegReq;

typedef struct _CelSocks5NegRsp
{
    U8 ver;
    U8 method;
}CelSocks5NegRsp;

typedef enum _CelSocks5Cmd
{
    CEL_SOCKS5_CMD_CONNECT = 0x01,
    CEL_SOCKS5_CMD_BIND = 0x02,
    CEL_SOCKS5_CMD_UDP = 0x03
}CelSocks5Cmd;

typedef enum _CelSocks5Atyp
{
    CEL_SOCKS5_ATYP_IPV4 = 0x01,
    CEL_SOCKS5_ATYP_DOMAIN = 0x03,
    CEL_SOCKS5_ATYP_IPV6 = 0x04
}CelSocks5Atyp;

typedef struct _CelSocks5Request
{
    U8 ver;
    U8 cmd;
    U8 rsv;
    U8 atyp;
}CelSocksrRequest;

typedef struct _CelSocks5Ipv4
{
    U32 ip;
    U16 port;
}CelSocks5Ipv4;

typedef struct _CelSocks5Domain
{
    U8 len;
    U8 domain;
}CelSocks5Domain;

typedef struct _CelSocks5Ipv6
{
    U8 ip[16];
    U16 port;
}CelSocks5Ipv6;

typedef enum _CelSocks5Rep
{
    CEL_SOCKS5_REP_SUCCEEDED = 0x00,
    CEL_SOCKS5_REP_SOCKS_SERVER_FAILURE = 0x01,
    CEL_SOCKS5_REP_NOT_ALLOWED = 0x02,
    CEL_SOCKS5_REP_NETWORK_UNREACHABLE = 0x03,
    CEL_SOCKS5_REP_HOST_UNREACHABLE = 0x04,
    CEL_SOCKS5_REP_CONNECTION_REFUSED = 0x05,
    CEL_SOCKS5_REP_TTL_EXPIRED = 0x06,
    CEL_SOCKS5_REP_CMD_NOT_SUPPORTED = 0x07,
    CEL_SOCKS5_REP_ATYP_NOT_SUPPORTED = 0x08,
    CEL_SOCKS5_REP_UNASSIGNED = 0x09
}CelSocks5Rep;

typedef struct _CelSocks5Response
{
    U8 ver;
    U8 rep;
    U8 rsv;
    U8 atyp;
}CelSocks5Response;

typedef struct _CelSocks5UdpHdr
{
    U16 rsv;
    U8 flag;
    U8 atyp;
}CelSocks5UdpHdr;

#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif
