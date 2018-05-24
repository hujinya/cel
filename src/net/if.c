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
#include "cel/net/if.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

/* Debug defines */
#define Debug(args)   /* cel_log_debug args */
#define Warning(args) CEL_SETERRSTR(args)/* cel_log_warning args */
#define Err(args)   CEL_SETERRSTR(args)/* cel_log_err args */


int cel_hrdaddr_pton(const TCHAR *hrdstr, CelHrdAddr *hrdaddr)
{
    return (_stscanf(hrdstr, _T("%02x:%02x:%02x:%02x:%02x:%02x"), 
        (int *)&(*hrdaddr)[0], (int *)&(*hrdaddr)[1], 
        (int *)&(*hrdaddr)[2], (int *)&(*hrdaddr)[3], 
        (int *)&(*hrdaddr)[4], (int *)&(*hrdaddr)[5]) != 6 ? -1 : 0);
}

const TCHAR *cel_hrdaddr_get_addrs_r(CelHrdAddr *hrdaddr, TCHAR *buf, int size)
{
    static volatile int i = 0;
    static TCHAR s_buf[CEL_BUFNUM][CEL_HRDSTRLEN] = { {_T('\0')}, {_T('\0')} };

    if (buf == NULL)
    {
        buf = s_buf[((++i) % (int)CEL_BUFNUM)];
        size = CEL_HRDSTRLEN;
    }
    _sntprintf(buf, size, _T("%02x:%02x:%02x:%02x:%02x:%02x"), 
        (*hrdaddr)[0], (*hrdaddr)[1], (*hrdaddr)[2], 
        (*hrdaddr)[3], (*hrdaddr)[4], (*hrdaddr)[5]);

    return buf;
}

const TCHAR *cel_ipaddr_get_addrs_r(CelIpAddr *ipaddr, TCHAR *buf, int size)
{
    static volatile int i = 0;
    static TCHAR s_buf[CEL_BUFNUM][CEL_IPSTRLEN] = { {_T('\0')}, {_T('\0')} };

    if (buf == NULL)
    {
        buf = s_buf[((++i) % (int)CEL_BUFNUM)];
        size = CEL_IPSTRLEN;
    }
    return InetNtop(AF_INET, ipaddr, buf, size);
}

const TCHAR *cel_ip6addr_get_addrs_r(CelIp6Addr *ip6addr, TCHAR *buf, int size)
{
    static volatile int i = 0;
    static TCHAR s_buf[CEL_BUFNUM][CEL_IP6STRLEN] = { {_T('\0')}, {_T('\0')} };

    if (buf == NULL)
    {
        buf = s_buf[((++i) % (int)CEL_BUFNUM)];
        size = CEL_IP6STRLEN;
    }
    return InetNtop(AF_INET6, ip6addr, buf, size);
}

int cel_netmask_dton(int prefixlen, void *netmask, int nm_len)
{
    int i;

    nm_len /= 4;
    for (i = 0; i < nm_len; i++)
    {
        if (prefixlen >= 32)
        {
            ((U32 *)netmask)[i] = U(0xFFFFFFFF);
            prefixlen -= 32;
        }
        else if (prefixlen > 0)
        {
            ((U32 *)netmask)[i] = htonl((U(0xFFFFFFFF) << (32 - prefixlen)));
            //_tprintf(_T("%x %d\r\n"), ((U32 *)netmask)[i], prefixlen);
            prefixlen = 0;
        }
        else
        {
            ((U32 *)netmask)[i] = U(0x00000000); 
        }
    }
    return prefixlen == 0 ? 0 : -1;
}

int cel_netmask_ntod(void *netmask, int nm_len)
{
    U8 u_b;
    int i = nm_len, prefixlen = 0;

    while ((--i) >= 0)
    {
        if (((TCHAR *)netmask)[i] == 0x00)
            prefixlen += 8;
        else if (((TCHAR *)netmask)[i] == 0xFF)
            break;
        else
        {
            u_b = (U8)((TCHAR *)netmask)[i];
            while ((u_b & 0x01) == U(0))
            {
                prefixlen++;
                u_b >>= 1;
                //_tprintf(_T("prefixlen = %d\r\n"), prefixlen);
            }
        }
    }
    return nm_len * 8 - prefixlen;
}
