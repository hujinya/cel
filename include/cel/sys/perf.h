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
#ifndef __CEL_SYS_PERF_H__
#define __CEL_SYS_PERF_H__

#include "cel/types.h"
#include "cel/net/if.h" //CEL_IFNLEN

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _CEL_WIN
#pragma comment(lib, "Psapi.lib")    // Added to support GetPerformanceInfo()
#pragma comment(lib, "IPHLPAPI.lib") // Added to support GetIfTable()
#endif

#define CEL_PF_TIMER_HZ                            1000
#define CEL_PF_FLUSH_INTERVAL       1 * CEL_PF_TIMER_HZ
#define CEL_PF_RESET_INTERVAL 60 * 60 * CEL_PF_TIMER_HZ

/* Prosscessors performance */
typedef struct _CelCpuPerf
{
    BYTE num;
    BYTE usage, maxusage;
    unsigned long long usertime;
    unsigned long long systime;
    unsigned long long idletime;
    unsigned long long totaltime;
    struct _CelCpuPerf *cpus[CEL_PRONUM];
}CelCpuPerf;

/* Memory performance */
typedef struct _CelMemPerf
{
    BYTE usage, maxusage;
    /* Range 0 - 2^64 bytes */
    unsigned long long total;
    unsigned long long cached;
    unsigned long long available;
    unsigned long long free;
}CelMemPerf;

/* File system performance */
typedef struct _CelFsPerf
{
    BYTE num;
    TCHAR fsname[CEL_FNLEN];
    TCHAR dir[CEL_DIRLEN];
    BYTE usage, maxusage;
    /* Range 0 - 2^64 bytes */
    unsigned long long total;
    unsigned long long available;
    unsigned long long free;
    struct _CelFsPerf *fss[CEL_HDNUM];
}CelFsPerf;

/* Netdevs performance */
typedef struct _CelNetPerf
{
    BYTE num;
    TCHAR ifname[CEL_IFNLEN];
    unsigned long ticks;
    /* Range 0 - 2^32 bytes */
    unsigned long received;
    unsigned long sent;
    /* Range 0 -  2^32 bytes/s */
    unsigned long receiving, max_receiving;
    unsigned long sending, max_sending;
    struct _CelNetPerf *nets[CEL_NETDEVNUM];
}CelNetPerf;

typedef struct _CelPerf
{
    struct _CelCpuPerf cpu;
    struct _CelMemPerf mem;
    struct _CelFsPerf fs;
    struct _CelNetPerf net;
}CelPerf;

extern CelPerf s_perf;


CelCpuPerf *cel_getcpuperf(void);
CelMemPerf *cel_getmemperf(void);
CelFsPerf *cel_getfsperf(void);
CelNetPerf *cel_getnetperf(void);
CelPerf *cel_getperf(const struct timeval *now, int reset);

int cel_perfserialize(CelPerf *pi, char *buf, size_t size, int indent);
int cel_perfmaxserialize(CelPerf *pi, char *buf, size_t size, int indent);

#ifdef __cplusplus
}
#endif

#endif
