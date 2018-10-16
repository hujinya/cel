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
#ifndef __CEL_NET_SOCKADDR_H__
#define __CEL_NET_SOCKADDR_H__

#include "cel/config.h"
#ifdef _CEL_UNIX
#include "cel/_unix/_sockaddr.h"
#elif defined(_CEL_WIN)
#include "cel/_win/_sockaddr.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

const CHAR *cel_gethostname_r_a(CHAR *buf, size_t size);
const WCHAR *cel_gethostname_r_w(WCHAR *buf, size_t size);
#define cel_gethostname_a() cel_gethostname_r_a(NULL, 0)
#define cel_gethostname_w() cel_gethostname_r_w(NULL, 0)

const CHAR *cel_getipstr_full(const CHAR *hostname, CHAR *ipstr, int family);
BOOL cel_isipstr_full(const CHAR *hostname, const CHAR *ipstr, int family);

int cel_sockaddr_str_split(TCHAR *str, int *family, 
                           TCHAR **paddr, TCHAR **pport);

int cel_sockaddr_init(CelSockAddr *addr);
int cel_sockaddr_init_ip(CelSockAddr *addr, 
                         const struct in_addr *ip, unsigned short port);
int cel_sockaddr_init_ipstr(CelSockAddr *addr, 
                            const TCHAR *ipstr, unsigned short port);
int cel_sockaddr_init_ip6(CelSockAddr *addr, 
                          const struct in6_addr *ip6str, unsigned short port);
int cel_sockaddr_init_ip6str(CelSockAddr *addr, 
                             const TCHAR *ip6str, unsigned short port);
int cel_sockaddr_init_host(CelSockAddr *addr, 
                           const TCHAR *node, const TCHAR *service);
/* String format - <ipv4|ipv6|unix>@<ipv4addr|[ipv6addr]>:<port> */
int cel_sockaddr_init_str(CelSockAddr *addr, const TCHAR *str);

void cel_sockaddr_destroy(CelSockAddr *addr);

CelSockAddr *cel_sockaddr_new(void);
CelSockAddr *cel_sockaddr_new_ip(const struct in_addr *ip, 
                                 unsigned short port);
CelSockAddr *cel_sockaddr_new_ipstr(const TCHAR *ipstr, unsigned short port);
CelSockAddr *cel_sockaddr_new_ip6(const struct in6_addr *ip6str, 
                                  unsigned short port);
CelSockAddr *cel_sockaddr_new_ip6str(const TCHAR *ip6str, unsigned short port);
CelSockAddr *cel_sockaddr_new_host(const TCHAR *node, const TCHAR *service);
CelSockAddr *cel_sockaddr_new_str(const TCHAR *str);

void cel_sockaddr_free(CelSockAddr *addr);

TCHAR *cel_sockaddr_get_addrs_r(CelSockAddr *addr, TCHAR *buf, size_t size);
const TCHAR *cel_sockaddr_get_ipstr_r(CelSockAddr *addr, 
                                      TCHAR *buf, size_t size);
#define cel_sockaddr_get_ipstr(addr) cel_sockaddr_get_ipstr_r(addr, NULL, 0)
/* unsigned short cel_sockaddr_get_port(CelSockAddr *addr); */
#define cel_sockaddr_get_port(addr) ntohs((addr)->addr_in.sin_port)
/* size_t cel_sockaddr_get_len(CelSockAddr *addr) */
#define cel_sockaddr_get_len(addr) \
    (((addr)->sa_family>= 0 && (addr)->sa_family <= 24) \
    ? c_addrlen[(addr)->sa_family] : 0)

/* 
 * const TCHAR *cel_sockaddr_ntop(CelSockAddr *addr); 
 * const TCHAR *cel_sockaddr_ntop_r(CelSockAddr *addr, TCHAE *buf, size_t size); 
 */
#define cel_sockaddr_ntop(addr) cel_sockaddr_get_addrs_r(addr, NULL, 0)
#define cel_sockaddr_ntop_r cel_sockaddr_get_addrs_r

#ifndef _UNICODE
#define cel_gethostname_r cel_gethostname_r_a
#define cel_gethostname cel_gethostname_a
#else
#define cel_gethostname_r cel_gethostname_r_w
#define cel_gethostname cel_gethostname_w
#endif

#ifdef __cplusplus
}
#endif

#endif