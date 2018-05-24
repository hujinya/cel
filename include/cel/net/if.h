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
#ifndef __CEL_NET_IF_H__
#define __CEL_NET_IF_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CEL_HRDADDRLEN        6    /* MAC address len, 6 bytes */
#define CEL_HRDSTRLEN        18    /* e.g. 01-23-45-67-89-ab */

typedef U8 CelHrdAddr[CEL_HRDADDRLEN];

#if defined(_CEL_UNIX)
#include "cel/_unix/_if.h"
#elif defined(_CEL_WIN)
#include "cel/_win/_if.h"
#endif

#define CEL_IPADDRLEN        sizeof(CelIpAddr)
#define CEL_IPSTRLEN         16    /* e.g. 192.168.100.254 */
#define CEL_IP6LEN           sizeof(CelIp6Addr)
#define CEL_IP6STRLEN        40    /* e.g. fe80:0000:0000:0000:aaaa:0000:00c2:0002 */
#define CEL_IPNETMASKBITS    sizeof(CelIpNetmask) * 8
#define CEL_IP6NETMASKLEN    sizeof(CelIp6Netmask)
#define CEL_IP6NETMASKBITS   sizeof(CelIp6Netmask) * 8

const TCHAR *cel_hrdaddr_get_addrs_r(CelHrdAddr *hrdaddr, 
                                     TCHAR *buf, int size);
const TCHAR *cel_ipaddr_get_addrs_r(CelIpAddr *ipaddr, TCHAR *buf, int size);
const TCHAR *cel_ip6addr_get_addrs_r(CelIp6Addr *ip6addr, 
                                     TCHAR *buf, int size);
/*
 * int cel_hrdaddr_pton(const TCHAR *hrdstr, CelHrdAddr *hrdaddr);
 * const TCHAR *cel_hrdaddr_notp(CelHrdAddr *hrdaddr);
 * const TCHAR *cel_hrdaddr_notp_r(CelHrdAddr *hrdaddr, 
 *                                 TCHAR *buf, size_t size);
 */
int cel_hrdaddr_pton(const TCHAR *hrdstr, CelHrdAddr *hrdaddr);
#define cel_hrdaddr_notp(hrdaddr) cel_hrdaddr_ntop_r(hrdaddr, NULL, 0)
#define cel_hrdaddr_ntop_r cel_hrdaddr_get_addrs_r

/*
 * int cel_ipaddr_pton(const TCHAR *ipv4str, CelIpAddr *ipaddr);
 * const TCHAR *cel_ipaddr_ntop(CelIpAddr *ipaddr);
 * const TCHAR *cel_ipaddr_ntop_r(CelIpAddr *ipaddr, TCHAR *buf, size_t size);
 */
#define cel_ipaddr_ntop_r cel_ipaddr_get_addrs_r
#define cel_ipaddr_pton(ipv4str, ipaddr) InetPton(AF_INET, ipv4str, ipaddr)
#define cel_ipaddr_ntop(ipaddr) cel_ipaddr_ntop_r(ipaddr, NULL, 0)

/*
 * int cel_ip6addr_pton(const TCHAR *ip6str, CelIp6Addr *ip6addr);
 * const TCHAR *cel_ip6addr_ntop(CelIp6Addr *ip6addr);
 * const TCHAR *cel_ip6addr_notp_r(CelIp6Addr *ip6addr, 
 *                                 TCHAR *buf, size_t size);
 */
#define cel_ip6addr_ntop_r cel_ip6addr_get_addrs_r
#define cel_ip6addr_pton(ip6str, ip6addr) InetPton(AF_INET6, ip6str, ip6addr)
#define cel_ip6addr_ntop(ip6addr) cel_ip6addr_ntop_r(ip6addr, NULL, 0)

int cel_netmask_dton(int prefixlen, void *netmask, int nm_len);
int cel_netmask_ntod(void *netmask, int nm_len);
/*
 * int cel_ipnetmask_dton(int prefixlen, CelIpNetmask *ipnetmask);
 * int cel_ipnetmask_ntod(CelIpNetmask *ipnetmask);
 */
#define cel_ipnetmask_dton(prefixlen, ipnetmask) \
    cel_netmask_dton(prefixlen, ipnetmask, sizeof(CelIpNetmask))
#define cel_ipnetmask_ntod(ipnetmask) \
    cel_netmask_ntod(ipnetmask, sizeof(CelIpNetmask))
/*
 * int cel_ip6netmask_dton(int prefixlen, CelIp6Netmask *ip6netmask);
 * int cel_ip6netmask_ntod(CelIp6Netmask *ip6netmask);
 */
#define cel_ip6netmask_dton(prefixlen, ip6netmask) \
    cel_netmask_dton(prefixlen, ip6netmask, sizeof(CelIp6Netmask))
#define cel_ip6netmask_ntod(ip6netmask) \
    cel_netmask_ntod(ip6netmask, sizeof(CelIp6Netmask))

#ifdef __cplusplus
}
#endif

#endif