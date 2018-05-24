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
#include "cel/sys/perf.h"
#ifdef _CEL_WIN
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"
#include "cel/timer.h"
#include "cel/convert.h"
#include <pdh.h>
#include <Winternl.h>
#include <Psapi.h>    /* GetPerformanceInfo() */
#include <Iphlpapi.h> /* GetIfTable() */

/* Debug defines */
#define Debug(args)   /*cel_log_debug args*/
#define Warning(args) CEL_SETERRSTR(args)/*cel_log_warning args*/
#define Err(args)   CEL_SETERRSTR(args)/*cel_log_err args*/

CelPerf s_perf = { { 0 }, { 0 }, { 0 }, { 0 }};

typedef struct _CelPdh
{
    HKEY hKey;
    DWORD dwTotal, dwBytes;
    PPERF_DATA_BLOCK pdb; 
}CelPdh;

typedef struct _CelPdhXx
{
    TCHAR *CounterNameTitle;
    DWORD CounterNameTitleIndex;
    DWORD CounterOffset; 
}CelPdhXx;

#define PdhFirstObject(pdh) ((PPERF_OBJECT_TYPE)((PBYTE)pdh + pdh->HeaderLength))
#define PdhNextObject(obj) ((PPERF_OBJECT_TYPE)((PBYTE)obj + obj->TotalByteLength))
#define PdhFirstCounter(obj) \
    ((PPERF_COUNTER_DEFINITION)((PBYTE)obj + obj->HeaderLength))
#define PdhNextCounter(cnt) ((PPERF_COUNTER_DEFINITION)((PBYTE)cnt + cnt->ByteLength))
#define PdhGetCounterBlock(inst) \
    ((PPERF_COUNTER_BLOCK)((PBYTE)inst + inst->ByteLength))
#define PdhFirstInstance(obj) \
    ((PPERF_INSTANCE_DEFINITION)((PBYTE)obj + obj->DefinitionLength))
#define PdhNextInstance(inst) \
    ((PPERF_INSTANCE_DEFINITION)((PBYTE)inst + inst->ByteLength \
    + PdhGetCounterBlock(inst)->ByteLength))
#define PdhInstanceName(inst) ((wchar_t *)((BYTE *)inst + inst->NameOffset))

void PdhClose(CelPdh *pdh)
{
    RegCloseKey(pdh->hKey);
    if (pdh->pdb != NULL)
    {
        cel_free(pdh->pdb);
        pdh->pdb = NULL;
    }
    pdh->dwTotal = pdh->dwBytes = 0;
}

LSTATUS PdhOpen(CelPdh *pdh, LPTSTR lpMachineName, LPTSTR lpPDBName)
{
    LSTATUS lStatus;
    DWORD dwType;

    if ((lStatus = RegConnectRegistry(
        lpMachineName, HKEY_PERFORMANCE_DATA, &(pdh->hKey))) != ERROR_SUCCESS)
    {
        _tprintf(_T("RegConnectRegistry failed.\r\n"));
        return -1;
    }
    pdh->dwTotal = 4096;
    if ((pdh->pdb = cel_calloc(1, 4096)) == NULL)
    {
        PdhClose(pdh);
        return -1;
    }
    while ((lStatus = RegQueryValueEx(pdh->hKey, lpPDBName, NULL, &dwType, 
        (LPBYTE)pdh->pdb, &(pdh->dwBytes))) != ERROR_SUCCESS)
    {
        if (lStatus != ERROR_MORE_DATA)
        {
            _tprintf(_T("RegQueryValueEx failed.\r\n"));
            PdhClose(pdh);
            return -1;
        }
        else 
        {
            if ((pdh->pdb = cel_realloc(pdh->pdb, pdh->dwBytes + 4096)) == NULL)
            {
                PdhClose(pdh);
                return -1;
            }
            pdh->dwBytes += 4096;
        }
    }
    return 0;
}

#define PDH_CPU_BLOCK_KEY   _T("238")

static CelPdhXx s_pdhcpu[] = { 
    { _T(""), 142,  0 }, //user
    { _T(""), 1746, 0 }, //idle
    { _T(""), 144,  0 }, //system
    //{ _T(""), 698,  0 }  //irq
};

//CelCpuPerf *cel_getcpuperf(void)
//{
//    CelPdh pdh;
//    PERF_OBJECT_TYPE *obj;
//    PERF_COUNTER_DEFINITION *cnt;
//    PERF_INSTANCE_DEFINITION *inst;
//    PERF_COUNTER_BLOCK *cnt_blk;
//    int i, j;
//    unsigned long long idletime, totaltime;
//    unsigned long long t_idletime, t_totaltime;
//    CelCpuPerf *s_ci = &(s_perf.cpu), *c_ci;
//
//    if (PdhOpen(&pdh, NULL, PDH_CPU_BLOCK_KEY) != ERROR_SUCCESS)
//        return NULL;
//    if (pdh.pdb->NumObjectTypes == 0)
//        return NULL;
//    obj = PdhFirstObject(pdh.pdb);
//    if (obj->NumCounters == 0)
//        return NULL;
//    i = 0;
//    cnt = PdhFirstCounter(obj);
//    while (i++ < (int)obj->NumCounters)
//    {
//        j = 0;
//        while (j < sizeof(s_pdhcpu) / sizeof(CelPdhXx))
//        {
//            if (cnt->CounterNameTitleIndex == s_pdhcpu[j].CounterNameTitleIndex)
//            {
//                //_tprintf(_T("%d\n"), s_pdhcpu[j].CounterOffset);
//                s_pdhcpu[j].CounterOffset = cnt->CounterOffset;
//                break;
//            }
//            j++;
//        }
//        cnt = PdhNextCounter(cnt);
//    }
//    if (obj->NumInstances == 0)
//        return NULL;
//    t_idletime = t_totaltime = 0;
//    s_ci->num = 0;
//    inst = PdhFirstInstance(obj);
//    inst = PdhNextInstance(inst);
//    //_tprintf(_T("%d\n"), obj->NumInstances);
//    while (s_ci->num < (int)obj->NumInstances)
//    {
//        cnt_blk = PdhGetCounterBlock(inst);
//        if ((c_ci = s_ci->cpus[s_ci->num]) == NULL
//            && (c_ci = s_ci->cpus[s_ci->num] = 
//            (CelCpuPerf *)cel_calloc(1, sizeof(CelCpuPerf))) == NULL)
//            break;
//        t_idletime += (idletime 
//            = *((DWORD *)((BYTE *)cnt_blk + s_pdhcpu[1].CounterOffset)));
//        s_ci->systime += (c_ci->systime 
//            = *((DWORD *)((BYTE *)cnt_blk + s_pdhcpu[2].CounterOffset)));
//        s_ci->usertime += (c_ci->usertime 
//            = *((DWORD *)((BYTE *)cnt_blk + s_pdhcpu[0].CounterOffset)));
//        t_totaltime += (totaltime = c_ci->usertime + c_ci->systime + idletime);
//        /*
//        _tprintf(_T("idle   %I64d\r\nkernel %I64d\r\nuser   %I64d\r\n"),
//            idletime, s_ci->systime, s_ci->usertime);
//            */
//        if (totaltime != c_ci->totaltime)
//        {
//            c_ci->usage = 100 - (BYTE)((idletime - c_ci->idletime) * 100 
//                /(totaltime - c_ci->totaltime));
//            CEL_SETMAX(c_ci->maxusage, c_ci->usage);
//        }
//        c_ci->idletime = idletime;
//        c_ci->totaltime = totaltime;
//        s_ci->num++;
//        inst = PdhNextInstance(inst);
//    }
//    PdhClose(&pdh);
//    if (t_totaltime != s_ci->totaltime)
//    {
//        s_ci->usage = 100 - (BYTE)((t_idletime - s_ci->idletime) * 100 
//            / (t_totaltime - s_ci->totaltime));
//        CEL_SETMAX(s_ci->maxusage, s_ci->usage);
//    }
//    s_ci->idletime = t_idletime;
//    s_ci->totaltime = t_totaltime;
//
//    //_tprintf(_T("xx\r\n"));
//
//    return NULL;
//}

//#define Li2Double(x) ((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))
typedef LONG (WINAPI *PROCNTQSI) (UINT, PVOID, ULONG, PULONG);

static PROCNTQSI PNtQuerySystemInformation = NULL;

CelCpuPerf *cel_getcpuperf(void)
{
    CCHAR i;
    ULONG ulLen;
    NTSTATUS lStatus;
    SYSTEM_BASIC_INFORMATION sbi;  
    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION *sppi, *c_sppi;
    unsigned long long idletime, totaltime;
    unsigned long long t_idletime, t_totaltime;
    CelCpuPerf *s_ci = &(s_perf.cpu), *c_ci;

    // Load NtQuerySystemInformation function 
    if (PNtQuerySystemInformation == NULL 
        && (PNtQuerySystemInformation = (PROCNTQSI) GetProcAddress(
        GetModuleHandle(_T("ntdll")), "NtQuerySystemInformation")) == NULL)
    {
        Err((_T("Load NtQuerySystemInformation failed.(GetProcAddress():%s)"), 
            cel_geterrstr(cel_sys_geterrno())));
        return NULL;
    }
    if ((lStatus = PNtQuerySystemInformation(
        SystemBasicInformation, &sbi, sizeof(sbi), &ulLen)) != NO_ERROR)
    {
        Err((_T("SystemBasicInformation():%s."), cel_geterrstr(cel_sys_geterrno())));
        return NULL;
    }
    // sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) = 44(32bit)/48(32bit)
    ulLen = 48 * sbi.NumberOfProcessors;
    if ((sppi = 
        (SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION *)cel_malloc(ulLen)) == NULL)
    {
        Err((_T("%s"), cel_geterrstr(cel_sys_geterrno())));
        return NULL;
    }
    if ((lStatus = PNtQuerySystemInformation(SystemProcessorPerformanceInformation,
        sppi, ulLen, &ulLen)) != NO_ERROR)
    {
        Err((_T("SystemProcessorPerformanceInformation():%s."), cel_geterrstr(cel_sys_geterrno())));
        cel_free(sppi);
        return NULL;
    }
    ulLen = ulLen / sbi.NumberOfProcessors;
    c_sppi = sppi;
    t_idletime  = t_totaltime = 0;
    s_ci->num = 0;
    for (i = 0; i < sbi.NumberOfProcessors; i++)
    {
        if ((c_ci = s_ci->cpus[s_ci->num]) == NULL
            && (c_ci = s_ci->cpus[s_ci->num] = 
            (CelCpuPerf *)cel_calloc(1, sizeof(CelCpuPerf))) == NULL)
            break;
        idletime = c_sppi->IdleTime.QuadPart;
        c_ci->systime = c_sppi->KernelTime.QuadPart - idletime;
        c_ci->usertime = c_sppi->UserTime.QuadPart;
        totaltime = c_ci->usertime + c_ci->systime + idletime;
        //_tprintf(_T("idle   %I64d\r\nkernel %I64d\r\nuser   %I64d\r\n"),
        //    idletime, c_ci->systime, c_ci->usertime);
        if (totaltime != c_ci->totaltime)
        {
            c_ci->usage = 100 - (BYTE)((idletime - c_ci->idletime) * 100 
                /(totaltime - c_ci->totaltime));
            if (c_ci->maxusage < c_ci->usage)
                c_ci->maxusage = c_ci->usage;
        }
        t_idletime += (c_ci->idletime = idletime);
        s_ci->systime += c_ci->systime;
        s_ci->usertime += c_ci->usertime;
        t_totaltime += (c_ci->totaltime = totaltime);
        (char *)c_sppi += ulLen;
        if ((++s_ci->num) > CEL_PRONUM) break;
    }
    cel_free(sppi);
    if (t_totaltime != s_ci->totaltime)
    {
        s_ci->usage = 100 - (BYTE)((t_idletime - s_ci->idletime) * 100 
            / (t_totaltime - s_ci->totaltime));
        if (s_ci->maxusage < s_ci->usage) 
            s_ci->maxusage = s_ci->usage;
    }
    s_ci->idletime = t_idletime;
    s_ci->totaltime = t_totaltime;
    
    return s_ci;
}

CelMemPerf *cel_getmemperf(void)
{
    CelMemPerf *s_mi = &(s_perf.mem);
    PERFORMANCE_INFORMATION pi;

    if (!GetPerformanceInfo(&pi, sizeof(pi)))
    {
        //_putts(cel_geterrstr(cel_sys_geterrno()));
        return NULL;
    }
    if ((s_mi->total = (U64)pi.PhysicalTotal * pi.PageSize) != 0)
    {
        //_tprintf(_T("Kernel %ld\r\n"), pi.KernelPaged * pi.PageSize >> 10);
        s_mi->available = (U64)pi.PhysicalAvailable * pi.PageSize;
        s_mi->cached = (U64)pi.SystemCache * pi.PageSize;
        s_mi->free = (s_mi->available < s_mi->cached 
            ? s_mi->available : (s_mi->available - s_mi->cached));
        s_mi->usage = (BYTE)
            ((s_mi->total - s_mi->available) * 100 / s_mi->total);
        if (s_mi->maxusage < s_mi->usage)
            s_mi->maxusage = s_mi->usage;
    } 
    return s_mi;
}

CelFsPerf *cel_getfsperf(void)
{
    TCHAR chBuffer[CEL_LINELEN];
    unsigned long ulLen, i;
    ULARGE_INTEGER ulTotal, ulAvail, ulFree;
    CelFsPerf *s_hi = &(s_perf.fs), *c_hi;

    //DWORD *ppDisks;
    //GetAllPresentDisks(&ppDisks);
    s_hi->num = 0;
    s_hi->total = s_hi->available = s_hi->free = 0;
    ulLen = GetLogicalDriveStrings(CEL_LINELEN, chBuffer) / 4;
    for (i = 0 ; i < ulLen ; i++)
    {
        /* If not hard disk, continue */
        if (GetDriveType(&chBuffer[i * 4]) != DRIVE_FIXED
            || !GetDiskFreeSpaceEx(&chBuffer[i * 4], &ulAvail, &ulTotal, &ulFree)) 
            continue;
        //GetPhysicalDriveFromPartitionLetter
        if ((c_hi = s_hi->fss[s_hi->num]) == NULL
            && ((c_hi = s_hi->fss[s_hi->num] = 
            (CelFsPerf *)cel_calloc(1, sizeof(CelFsPerf)))) == NULL)
            break;
        _tcsncpy(c_hi->fsname, &chBuffer[i * 4], CEL_DIRLEN);
        _tcsncpy(c_hi->dir, &chBuffer[i * 4], CEL_DIRLEN);
        c_hi->total = ulTotal.QuadPart;
        c_hi->available = ulAvail.QuadPart;
        c_hi->free = ulFree.QuadPart;
        if (c_hi->total != 0)
        {
            c_hi->usage = (BYTE)
                ((c_hi->total - c_hi->free) * 100 / c_hi->total);
            if (c_hi->maxusage < c_hi->usage)
                c_hi->maxusage = c_hi->usage;
            s_hi->total += c_hi->total;
            s_hi->available += c_hi->available;
            s_hi->free += c_hi->free;
        }
        if ((++s_hi->num) >= CEL_HDNUM) break;
    }
    if (s_hi->total != 0)
    {
        if (s_hi->maxusage < (s_hi->usage = (BYTE)
            ((s_hi->total - s_hi->free) * 100 / s_hi->total)))
            s_hi->maxusage = s_hi->usage;
    }

    return s_hi;
}

CelNetPerf *cel_getnetperf(void)
{
    ULONG i, ulLen, dwResult;
    MIB_IFROW *ifrow;
    MIB_IFTABLE *iftb;
    DWORD received, sent, t_received, t_sent;
    unsigned long ticks;
    CelNetPerf *s_ni = &(s_perf.net), *c_ni;
    
    ulLen = sizeof(MIB_IFTABLE);
    if ((iftb = cel_malloc(sizeof(MIB_IFTABLE))) == NULL) 
        return NULL;
    if ((dwResult = GetIfTable(iftb, &ulLen, TRUE)) != NO_ERROR)
    {
        if (dwResult != ERROR_INSUFFICIENT_BUFFER)
        {
            Err((_T("GetIfTable():%s."), cel_geterrstr(cel_sys_geterrno())));
            cel_free(iftb);
            return NULL;
        }
        cel_free(iftb);
    }  
    if ((iftb = cel_malloc(ulLen)) == NULL) 
        return NULL;
    if ((dwResult = GetIfTable(iftb, &ulLen, FALSE)) != NO_ERROR)
    {
        cel_free(iftb);
        Err((_T("GetIfTable():%s."), cel_geterrstr(cel_sys_geterrno())));
        return NULL;
    }
    t_received = t_sent = 0;
    ticks = cel_getticks();
    s_ni->num = 0;
    for (i = 0; i < iftb->dwNumEntries; i++)
    {
        ifrow = &iftb->table[i];
        if (ifrow->dwType == IF_TYPE_SOFTWARE_LOOPBACK
            || ifrow->dwOperStatus != IF_OPER_STATUS_OPERATIONAL
            || strstr(ifrow->bDescr, "WAN") != NULL
            || strstr(ifrow->bDescr, "QoS") != NULL
            || strstr(ifrow->bDescr, "WFP") != NULL)
        {
            continue;
        }
        if ((c_ni = s_ni->nets[s_ni->num]) == NULL
            && (c_ni = s_ni->nets[s_ni->num] 
        = cel_calloc(1, sizeof(CelNetPerf))) == NULL)
            break;
        //puts(ifrow->bDescr);
#ifdef _UNICODE
        cel_mb2unicode(ifrow->bDescr, ifrow->dwDescrLen,
            c_ni->ifname, CEL_IFNLEN);
        //_tprintf(_T("%s\r\n"), pniLast->ifname);
#else
         _tcsncpy(c_ni->ifname, ifrow->bDescr, CEL_IFNLEN);
#endif
        received = ifrow->dwInOctets;
        sent = ifrow->dwOutOctets;
        if (ticks != c_ni->ticks)
        {
            c_ni->receiving = CEL_DIFF(received, c_ni->received) * ULL(1000) 
                / CEL_DIFF(ticks, c_ni->ticks);
            c_ni->sending = CEL_DIFF(sent, c_ni->sent) * ULL(1000) 
                / CEL_DIFF(ticks, c_ni->ticks);
            if (c_ni->max_receiving < c_ni->receiving)
                c_ni->max_receiving = c_ni->receiving;
            if (c_ni->max_sending < c_ni->sending)
                c_ni->max_sending = c_ni->sending; 
            //printf("%ul %ul %ul\r\n", 
            //    sent, c_ni->sent, c_ni->sending);
        }
        c_ni->ticks = ticks;
        t_received += (c_ni->received = received);
        t_sent += (c_ni->sent = sent);
        if ((++s_ni->num) >= CEL_NETDEVNUM) break;
    }
    cel_free(iftb);
    if (ticks != s_ni->ticks)
    {
        s_ni->receiving = CEL_DIFF(t_received, s_ni->received) * ULL(1000) 
            / CEL_DIFF(ticks, s_ni->ticks);
        s_ni->sending = CEL_DIFF(t_sent, s_ni->sent) * ULL(1000) 
            / CEL_DIFF(ticks, s_ni->ticks);
        if (s_ni->max_receiving < s_ni->receiving)
            s_ni->max_receiving = s_ni->receiving;
        if (s_ni->max_sending < s_ni->sending)
            s_ni->max_sending = s_ni->sending; 
    }
    s_ni->ticks = ticks;
    s_ni->received = t_received;
    s_ni->sent = t_sent;
    return s_ni;
}

#endif
