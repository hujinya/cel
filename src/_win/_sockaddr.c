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
#ifdef _CEL_WIN
#include "cel/net/if.h"

const int c_addrlen[] = 
{ 
    0, 0, sizeof(struct sockaddr_in), 0, 0, /* 0 - 4*/
    0, 0, 0, 0, 0, /* 5 - 9 */
    0, 0, 0, 0, 0, /* 10 - 14 */
    0, 0, 0, 0, 0, /* 15 - 19 */
    0, 0, 0, sizeof(struct sockaddr_in6), 0 /* 20 - 24 */
};

int sethostname(const char *name, size_t len)
{
    LSTATUS ls;
    HKEY hkResult;

    if ((ls = RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
        "SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName", 
        0, KEY_ALL_ACCESS|KEY_WOW64_64KEY, &hkResult)) != ERROR_SUCCESS)
        return -1;
    if ((ls = RegSetValueExA(hkResult, "ComputerName",
        0, REG_SZ, (LPBYTE)name, (DWORD)len)) != ERROR_SUCCESS)
    {
        RegCloseKey(hkResult);
        return -1;
    }
    if ((ls = RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
        "SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ComputerName", 
        0, KEY_ALL_ACCESS|KEY_WOW64_64KEY, &hkResult)) != ERROR_SUCCESS)
        return -1;
    if ((ls = RegSetValueExA(hkResult, "ComputerName",
        0, REG_SZ, (LPBYTE)name, (DWORD)len)) != ERROR_SUCCESS)
    {
        RegCloseKey(hkResult);
        return -1;
    }
    if ((ls = RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
        "System\\CurrentControlSet\\Services\\Tcpip\\Parameters", 
        0, KEY_ALL_ACCESS|KEY_WOW64_64KEY, &hkResult)) != ERROR_SUCCESS)
        return -1;
    if ((ls = RegSetValueExA(hkResult, "NV Hostname",
        0, REG_SZ, (LPBYTE)name, (DWORD)len)) != ERROR_SUCCESS)
    {
        RegCloseKey(hkResult);
        return -1;
    }
    if ((ls = RegSetValueExA(hkResult, "Hostname",
        0, REG_SZ, (LPBYTE)name, (DWORD)len)) != ERROR_SUCCESS)
    {
        RegCloseKey(hkResult);
        return -1;
    }
    RegCloseKey(hkResult);

    return 0;
}

#endif
