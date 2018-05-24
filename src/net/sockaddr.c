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
#include "cel/net/sockaddr.h"
#include "cel/net/if.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"
#include "cel/convert.h"

/* Debug defines */
#define Debug(args)   /* cel_log_debug args */
#define Warning(args) CEL_SETERRSTR(args)/* cel_log_warning args */
#define Err(args)   CEL_SETERRSTR(args)/* cel_log_err args */

int cel_sockaddr_init(CelSockAddr *addr)
{
    memset(addr, 0, sizeof(CelSockAddr));

    return 0;
}

int cel_sockaddr_init_ip(CelSockAddr *addr, 
                         const struct in_addr *ip, unsigned short port)
{
    addr->addr_in.sin_family = AF_INET;
    addr->addr_in.sin_port = htons(port);
    addr->addr_in.sin_addr = (*ip);

    return 0;
}

int cel_sockaddr_init_ipstr(CelSockAddr *addr, 
                            const TCHAR *ipstr, unsigned short port)
{
    addr->addr_in.sin_family = AF_INET;
    addr->addr_in.sin_port = htons(port);

    return (cel_ipaddr_pton(ipstr, &(addr->addr_in.sin_addr.s_addr)) <= 0 ? -1 : 0);
}

int cel_sockaddr_init_ip6(CelSockAddr *addr, 
                          const struct in6_addr *ip6str, unsigned short port)
{
    addr->addr_in6.sin6_family = AF_INET6;
    addr->addr_in6.sin6_port = htons(port);
    addr->addr_in6.sin6_addr = (*ip6str);

    return 0;
}

int cel_sockaddr_init_ip6str(CelSockAddr *addr, 
                             const TCHAR *ip6str, unsigned short port)
{
    addr->addr_in6.sin6_family = AF_INET6;
    addr->addr_in6.sin6_port = htons(port);

    return (cel_ip6addr_pton(ip6str, &(addr->addr_in6.sin6_addr)) <= 0 ? -1 : 0);
}

int cel_sockaddr_init_host(CelSockAddr *addr, const TCHAR *node, const TCHAR *service)
{ 
    int ret;
    ADDRINFOT *addr_info, hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = addr->sa_family;
    hints.ai_socktype = 0;
    hints.ai_protocol = 0;
    if ((ret = GetAddrInfo(node, service, &hints, &addr_info)) != 0)
    {
        Err((_T("GetAddrInfo():%s."), gai_strerror(ret)));
        return -1;
    }
    if (addr_info != NULL)
    {
        memcpy(addr, (CelSockAddr *)addr_info->ai_addr, addr_info->ai_addrlen);
        FreeAddrInfo(addr_info);
        return 0;
    }

    return -1;
}

int cel_sockaddr_str_split(TCHAR *str, int *family, TCHAR **paddr, TCHAR **pport)
{
    TCHAR *p;

    p = *paddr = str;
    *pport = NULL;
    *family = AF_INET;
    while (*p != _T('\0'))
    {
        if (*p == _T('@'))
        {
            if (memcmp(paddr, _T("ipv4"), 4 * sizeof(TCHAR)) == 0)
                *family = AF_INET;
            else if (memcmp(paddr, _T("ipv6"), 4 * sizeof(TCHAR)) == 0)
                *family = AF_INET6;
#ifdef _CEL_UNIX
            else if (memcmp(paddr, _T("unix"), 4 * sizeof(TCHAR)) == 0)
            {
                *family = AF_UNIX;
                *paddr = p;
                //strncpy(addr->addr_un.sun_path, ++p, sizeof(addr->addr_un.sun_path));
                return 0;
            }
#endif
            else
            {
                *p = _T('\0');
                Err((_T("Socket address family %s invalid."), paddr));
                return -1;
            }
            p++;
            *paddr = p;
        }
        else if (*p == _T('['))
        {
            *family = AF_INET6;
            *paddr = (++p);
            while (*p != _T('\0'))
            {
                if (*p == _T(']'))
                    *p = _T('\0');
                else if (*p == _T(':'))
                {
                    if ((++p) != _T('\0'))
                        *pport = p;
                    return 0;
                }
            }
            break;
        }
        else if (*p == _T(':'))
        {
            *p = _T('\0');
            if ((++p) != _T('\0'))
                *pport = p;
            return 0;
        }
#ifdef _CEL_UNIX
        else if (*str == _T('/'))
        {
            *family = AF_UNIX;
            *paddr = p;
            //strncpy(addr->addr_un.sun_path, p, sizeof(addr->addr_un.sun_path));
            return 0;
        }
#endif
        else
            p++;
    }
    return 0;
}

int cel_sockaddr_init_str(CelSockAddr *addr, const TCHAR *str)
{
    TCHAR *paddr, *pport;
    TCHAR addrs[CEL_ADDRLEN];

    _tcsncpy(addrs, str, CEL_ADDRLEN - 1);
    addrs[CEL_ADDRLEN - 1] = _T('\0');
    return (cel_sockaddr_str_split(addrs, (int *)&addr->sa_family, &paddr, &pport) == 0
        && cel_sockaddr_init_host(addr, paddr, pport) == 0) ? 0 : -1;
}

void cel_sockaddr_destroy(CelSockAddr *addr)
{
    memset(addr, 0, sizeof(CelSockAddr));
}

CelSockAddr *cel_sockaddr_new()
{
    CelSockAddr *addr;

    if ((addr = (CelSockAddr *)cel_malloc(sizeof(CelSockAddr))) != NULL)
    {   
        if (cel_sockaddr_init(addr) == 0)
            return addr;
        cel_free(addr);
    }
    return addr;
}

CelSockAddr *cel_sockaddr_new_ip(const struct in_addr *ip, unsigned short port)
{
    CelSockAddr *addr;

    if ((addr = (CelSockAddr *)cel_malloc(sizeof(CelSockAddr))) != NULL)
    {   
        if (cel_sockaddr_init_ip(addr, ip, port) == 0)
            return addr;
        cel_free(addr);
    }

    return addr;
}

CelSockAddr *cel_sockaddr_new_ipstr(const TCHAR *ipstr, unsigned short port)
{
    CelSockAddr *addr;

    if ((addr = (CelSockAddr *)cel_malloc(sizeof(CelSockAddr))) != NULL)
    {   
        if (cel_sockaddr_init_ipstr(addr, ipstr, port) == 0)
            return addr;
        cel_free(addr);
    }

    return addr;
}

CelSockAddr *cel_sockaddr_new_ip6(const struct in6_addr *ip6str, unsigned short port)
{
    CelSockAddr *addr;

    if ((addr = (CelSockAddr *)cel_malloc(sizeof(CelSockAddr))) != NULL)
    {   
        if (cel_sockaddr_init_ip6(addr, ip6str, port) == 0)
            return addr;
        cel_free(addr);
    }

    return addr;
}

CelSockAddr *cel_sockaddr_new_ip6str(const TCHAR *ip6str, unsigned short port)
{
    CelSockAddr *addr;

    if ((addr = (CelSockAddr *)cel_malloc(sizeof(CelSockAddr))) != NULL)
    {   
        if (cel_sockaddr_init_ip6str(addr, ip6str, port) == 0)
            return addr;
        cel_free(addr);
    }

    return addr;
}

CelSockAddr *cel_sockaddr_new_host(const TCHAR *node, const TCHAR *service)
{
    CelSockAddr *addr;

    if ((addr = (CelSockAddr *)cel_malloc(sizeof(CelSockAddr))) != NULL)
    {   
        addr->sa_family = 0;
        if (cel_sockaddr_init_host(addr, node, service) == 0)
            return addr;
        cel_free(addr);
    }

    return addr;
}

CelSockAddr *cel_sockaddr_new_str(const TCHAR *str)
{
    CelSockAddr *addr;

    if ((addr = (CelSockAddr *)cel_malloc(sizeof(CelSockAddr))) != NULL)
    {   
        if (cel_sockaddr_init_str(addr, str) == 0)
            return addr;
        cel_free(addr);
    }

    return addr;
}

void cel_sockaddr_free(CelSockAddr *addr)
{
    cel_sockaddr_destroy(addr);
    cel_free(addr);
}

const TCHAR *cel_sockaddr_get_ipstr(CelSockAddr *addr)
{
    switch (addr->sa_family)
    {
    case AF_INET:
        return cel_ipaddr_ntop(&(addr->addr_in.sin_addr));
    case AF_INET6:
        return cel_ip6addr_ntop(&(addr->addr_in6.sin6_addr));
#ifdef _CEL_UNIX
    case AF_UNIX:
        return addr->addr_un.sun_path;
#endif
    default:
        Err((_T("Socket address family \"%d\" undefined¡£"), 
            addr->sa_family));
        return NULL;
    }

    return NULL;
}

TCHAR *cel_sockaddr_get_addrs_r(CelSockAddr *addr, TCHAR *buf, size_t size)
{
    static volatile int i = 0;
    static TCHAR s_buf[CEL_BUFNUM][CEL_ADDRLEN] = { {_T('\0')}, {_T('\0')} };

    if (buf == NULL)
    {
        buf = s_buf[((++i) % (int)CEL_BUFNUM)];
        size = CEL_ADDRLEN;
    }
    switch (addr->sa_family)
    {
    case AF_INET:
        _sntprintf(buf, size, _T("%s:%d"), 
            cel_ipaddr_ntop(&(addr->addr_in.sin_addr)), 
            ntohs(addr->addr_in.sin_port));
        break;
    case AF_INET6:
        _sntprintf(buf, size, _T("%s:%d"), 
            cel_ip6addr_ntop(&(addr->addr_in6.sin6_addr)), 
            ntohs(addr->addr_in.sin_port));
        break;
#ifdef _CEL_UNIX
    case AF_UNIX:
        _sntprintf(buf, size, _T("%s"), addr->addr_un.sun_path);
        break;
    case AF_NETLINK:
        _sntprintf(buf, size, _T("%d-%d"), 
            addr->addr_nl.nl_pid, addr->addr_nl.nl_groups);
        break;
#endif
    default:
        Err((_T("Socket address family \"%d\" undefined¡£"), 
            addr->sa_family));
        return NULL;
    }

    return buf;
}

const CHAR *cel_gethostname_r_a(CHAR *buf, size_t size)
{
    static CHAR s_buf[CEL_HNLEN] = { _T('\0')};

    if (s_buf[0] == _T('\0'))
    {
        if (gethostname(s_buf, CEL_HNLEN) != 0)
            return NULL;
    }
    return (buf == NULL ? s_buf : strncpy(buf, s_buf, size));
}

const WCHAR *cel_gethostname_r_w(WCHAR *buf, size_t size)
{
    static WCHAR s_buf[CEL_HNLEN] = { _T('\0')};
    char local_buf[CEL_HNLEN];

    if (s_buf[0] == _T('\0'))
    {
        if (gethostname(local_buf, CEL_HNLEN) != 0
            || cel_mb2unicode(local_buf, -1, s_buf, CEL_HNLEN) <= 0)
            return NULL;
    }
    return (buf == NULL ? s_buf : wcsncpy(buf, s_buf, size));
}

const CHAR *cel_getipstr_full(const CHAR *hostname, CHAR *ipstr, int family)
{
    int ret;
    char hname[128];
    ADDRINFOT *addr_info, *result, hints;
    const char *_ipstr;

    if (hostname == NULL)
    {
        gethostname(hname, sizeof(hname));
        hostname = hname;
    }
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = family;
    hints.ai_socktype = 0;
    hints.ai_protocol = 0;
    if ((ret = GetAddrInfo(hostname, NULL, &hints, &addr_info)) != 0)
    {
        Err((_T("GetAddrInfo():%s."), gai_strerror(ret)));
        return NULL;
    }
    result = addr_info;
    while (addr_info != NULL)
    {
        if ((_ipstr = cel_sockaddr_get_ipstr(
            (CelSockAddr *)addr_info->ai_addr)) != NULL
            && strncmp(_ipstr, "127.0.0.1", CEL_IP6STRLEN) != 0)
        {
            //printf("get %s\r\n", _ipstr);
            strncpy(ipstr, _ipstr, CEL_IP6STRLEN);
            FreeAddrInfo(result);
            return ipstr;
        }
        //printf("##%s\r\n", _ipstr);
        addr_info = addr_info->ai_next;
    }
    FreeAddrInfo(result);
    return NULL;
}

BOOL cel_isipstr_full(const CHAR *hostname, const CHAR *ipstr, int family)
{
    int ret;
    char hname[128];
    ADDRINFOT *addr_info, *result, hints;
    const char *_ipstr;

    if (hostname == NULL)
    {
        gethostname(hname, sizeof(hname));
        hostname = hname;
    }
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = family;
    hints.ai_socktype = 0;
    hints.ai_protocol = 0;
    if ((ret = GetAddrInfo(hostname, NULL, &hints, &addr_info)) != 0)
    {
        Err((_T("GetAddrInfo():%s."), gai_strerror(ret)));
        return FALSE;
    }
    result = addr_info;
    while (addr_info != NULL)
    {
        if ((_ipstr = cel_sockaddr_get_ipstr(
            (CelSockAddr *)addr_info->ai_addr)) != NULL
            && strncmp(ipstr, _ipstr, CEL_IP6STRLEN) == 0
            && strncmp(ipstr, "127.0.0.1", CEL_IP6STRLEN) != 0)
        {
            FreeAddrInfo(result);
            return TRUE;
        }
        //printf("@@%s\r\n", _ipstr);
        addr_info = addr_info->ai_next;
    }
    FreeAddrInfo(result);
    return FALSE;
}
