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
#ifndef __CEL_NET_SOCKADDR_UNIX_H__
#define __CEL_NET_SOCKADDR_UNIX_H__

#include "cel/types.h"
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <linux/netlink.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GetAddrInfo getaddrinfo
#define FreeAddrInfo freeaddrinfo
typedef struct addrinfo ADDRINFOT;

typedef union _CelSockAddr
{
    sa_family_t sa_family;
    struct sockaddr_un addr_un;
    struct sockaddr_nl addr_nl;
    struct sockaddr addr;
    struct sockaddr_in addr_in;
    struct sockaddr_in6 addr_in6;
}CelSockAddr;

/* Socket address len table */
extern const int c_addrlen[];

int cel_sockaddr_init_unix(CelSockAddr *addr, const TCHAR *path);
int cel_sockaddr_init_nl(CelSockAddr *addr, 
                         unsigned long pid, unsigned long groups);

CelSockAddr *cel_sockaddr_new_unix(const TCHAR *path);
CelSockAddr *cel_sockaddr_new_nl(unsigned long pid, unsigned long groups);

#ifdef __cplusplus
}
#endif

#endif
