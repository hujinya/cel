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
#ifndef __CEL_SYS_ENVINFO_H__
#define __CEL_SYS_ENVINFO_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _CEL_WIN
#define CelCpuMask DWORD_PTR
#define cel_setcpumask(mask, i) ((*mask) = 1 << (i))
#endif
#ifdef _CEL_UNIX
#include <sched.h>
#define CelCpuMask cpu_set_t
#endif

typedef struct _CelSystemInfo
{
    TCHAR name[128]; /**< canonicalized sysname */
    TCHAR version[128]; /**< utsname.release */
    TCHAR arch[128];
    TCHAR machine[128];
    TCHAR description[128];
    TCHAR patch_level[128];
    TCHAR vendor[128];
    TCHAR vendor_version[128];
    TCHAR vendor_name[128];  /**< utsname.sysname */
    TCHAR vendor_code_name[128];
}CelSystemInfo;

typedef struct _CelCpuInfo
{
    TCHAR vendor[128];
    TCHAR model[128];
    int sockets;
    int cores;
    int cores_per_socket;
    int mhz;
    unsigned long long cache;
}CelCpuInfo;

static __inline unsigned long cel_getnprocs()
{
#ifdef _CEL_WIN
    SYSTEM_INFO info; 
    GetSystemInfo(&info); 
    return info.dwNumberOfProcessors; 
#endif
#ifdef _CEL_UNIX
     return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

#ifdef _CEL_UNIX
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>

typedef struct _CelSignalss
{
    int si_no;
    char *si_name;
    void (*handler) (int si_no);
}CelSignals;

typedef struct _CelResourceLimits
{
    rlim_t core;
    rlim_t nofile;
    rlim_t stack;
}CelResourceLimits;

int cel_setcpumask(CelCpuMask *mask, int i);
int cel_signals_init(CelSignals *signals);
int cel_resourcelimits_get(CelResourceLimits *rlimits);
int cel_resourcelimits_set(CelResourceLimits *rlimits);

#endif


#ifdef __cplusplus
}
#endif

#endif
