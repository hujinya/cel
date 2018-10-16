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
#ifndef __CEL_NET_IF_UNIX_H__
#define __CEL_NET_IF_UNIX_H__

#include "cel/types.h"
#include "cel/_unix/_netlink.h"
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif

#define InetNtop inet_ntop
#define InetPton inet_pton

typedef struct in_addr CelIpAddr, CelIpNetmask;
typedef struct in6_addr CelIp6Addr, CelIp6Netmask;


int cel_if_getindex(const TCHAR *if_name, int *if_index);
int cel_if_gethrdaddr(const TCHAR *if_name, CelHrdAddr *hrdaddr);
int cel_if_getipaddr(const TCHAR *if_name, CelIpAddr *ipaddr);
int cel_if_getip6addr(const TCHAR *if_name, CelIp6Addr *ip6addr);

int cel_if_sethrdaddr(const TCHAR *if_name, CelHrdAddr *hrdaddr);
int cel_if_setipaddr(const TCHAR *if_name, CelIpAddr *ipaddr);
int cel_if_setip6addr(const TCHAR *if_name, CelIp6Addr *ip6addr);

BOOL cel_if_isup(const TCHAR *if_name);

int cel_ipaddr_op(int if_index, CelIpAddr *addr, const TCHAR *label, int type);
/* 
 * int cel_if_newipaddr(int if_index, CelIpAddr *ipaddr)
 * int cel_if_delipaddr(int if_index, CelIpAddr *ipaddr)
 * int cel_if_getipaddr(int if_index, CelIpAddr *ipaddr) 
 */
#define cel_if_newipaddr(if_index, ipaddr, label) \
    cel_ipaddr_op(if_index, ipaddr, label, RTM_NEWADDR)
#define cel_if_delipaddr(if_index, ipaddr) \
    cel_ipaddr_op(if_index, ipaddr, NULL, RTM_DELADDR)
//#define cel_if_getipaddr(if_index, ipaddr)

#ifdef __cplusplus
}
#endif

#endif
