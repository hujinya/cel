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
#include "cel/net/if.h"
#ifdef _CEL_WIN
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

const TCHAR *InetNtop(int af, const void *src, TCHAR *dst, socklen_t size)
{
    switch (af)
    {
    case AF_INET:
        _sntprintf(dst, size, _T("%d.%d.%d.%d"), 
            ((CelIpAddr *)src)->S_un.S_un_b.s_b1, 
            ((CelIpAddr *)src)->S_un.S_un_b.s_b2, 
            ((CelIpAddr *)src)->S_un.S_un_b.s_b3, 
            ((CelIpAddr *)src)->S_un.S_un_b.s_b4);   
        return dst;  
    case AF_INET6:
        _sntprintf(dst, size, _T("%x:%x:%x:%x:%x:%x:%x:%x"), 
            ntohs(((CelIp6Addr *)src)->u.Word[0]),
            ntohs(((CelIp6Addr *)src)->u.Word[1]),
            ntohs(((CelIp6Addr *)src)->u.Word[2]),
            ntohs(((CelIp6Addr *)src)->u.Word[3]),
            ntohs(((CelIp6Addr *)src)->u.Word[4]),
            ntohs(((CelIp6Addr *)src)->u.Word[5]),
            ntohs(((CelIp6Addr *)src)->u.Word[6]),
            ntohs(((CelIp6Addr *)src)->u.Word[7])); 
        return dst;
    default:
        cel_sys_setwsaerrno(WSAEAFNOSUPPORT);
        return NULL;
    }
    cel_sys_setwsaerrno(ERROR_INVALID_PARAMETER);
    return 0;
}

int InetPton(int af, const TCHAR *src, void *dst)
{
    int i;
    TCHAR *p = NULL;

    switch (af) 
    {  
    case AF_INET: 
        if (_stscanf(src, _T("%d.%d.%d.%d"), 
            &(((CelIpAddr *)dst)->S_un.S_un_b.s_b1), 
            &(((CelIpAddr *)dst)->S_un.S_un_b.s_b2),
            &(((CelIpAddr *)dst)->S_un.S_un_b.s_b3), 
            &(((CelIpAddr *)dst)->S_un.S_un_b.s_b4)) == 4)
            return (1);
        break;
    case AF_INET6:
        i = 0;
        do 
        {
            //_putts(src);
            if (*src != _T('\0'))
            {
                ((CelIp6Addr *)dst)->u.Word[i] 
                = htons((unsigned short)_tcstoul(src, &p, 16));
                if (*(src = p) != _T('\0'))
                    src++;
            }
            else
            {
                ((CelIp6Addr *)dst)->u.Word[i] = 0;
            }
        }while ((++i) < 8);
        if (i == 8)
            return (1);
    default:
        cel_sys_setwsaerrno(WSAEAFNOSUPPORT);
        return (-1);  
    }
    cel_sys_setwsaerrno(WSAEFAULT);
    return 0;
}

#endif
