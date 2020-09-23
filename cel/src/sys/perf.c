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
#include "cel/sys/perf.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"
#include "cel/timer.h"
#include "cel/multithread.h"
#include "cel/convert.h"


CelPerf *cel_getperf(const CelTime *now, int reset)
{
    static CelTime flush_timeval= { 0, 0 };
    static CelTime reset_timeval= { 0, 0 };
    int i;

    CEL_TIME_NOW(now);
    if (cel_time_is_expired(&reset_timeval, now))
    {
        cel_multithread_mutex_lock(CEL_MT_MUTEX_PERFOINFO);
        cel_time_set_milliseconds(&reset_timeval, now, CEL_PF_RESET_INTERVAL);
        s_perf.cpu.maxusage = 0;
        for (i = 0; i < s_perf.cpu.num && s_perf.cpu.cpus[i] != NULL; i++)
            s_perf.cpu.cpus[i]->maxusage = 0;
        s_perf.mem.maxusage = 0;
        for (i = 0; i < s_perf.fs.num && s_perf.fs.fss[i] != NULL; i++)
            s_perf.fs.fss[i]->maxusage = 0;
        s_perf.fs.maxusage = 0;
        s_perf.net.max_receiving = 0;
        s_perf.net.max_sending = 0;
        for (i = 0; i < s_perf.net.num && s_perf.net.nets[i] != NULL; i++)
        {
            s_perf.net.nets[i]->max_receiving = 0;
            s_perf.net.nets[i]->max_sending = 0;
        }
        cel_multithread_mutex_unlock(CEL_MT_MUTEX_PERFOINFO);
    }
    if (reset == 1
        || cel_time_is_expired(&flush_timeval, now))
    {
        cel_multithread_mutex_lock(CEL_MT_MUTEX_PERFOINFO);
        cel_time_set_milliseconds(&flush_timeval, now, CEL_PF_FLUSH_INTERVAL);
        cel_getcpuperf();
        cel_getmemperf();
        cel_getfsperf();
        cel_getnetperf();
        cel_multithread_mutex_unlock(CEL_MT_MUTEX_PERFOINFO);
    }
    
    return &s_perf;
}

int cel_perfserialize(CelPerf *pi, char *buf, size_t size, int indent)
{
    int len, i;

#ifdef _CEL_UNIX
    if (indent == 0)
    {
#endif
        len = snprintf(buf, size, 
            "{\"Performance\":{\"cpu\":{\"usage\":%d,\"details\":[", 
            pi->cpu.usage);
        for (i = 0; i < pi->cpu.num; i++)
        {
            len += snprintf(buf + len,size - len, 
                "{\"id\":%d,\"usage\":%d},", i, pi->cpu.cpus[i]->usage);
        }
        len--;
        len += snprintf(buf + len, size - len, "]},\"memory\":"
            "{\"total\":%lld,\"available\":%lld,\"cached\":%lld,\"free\":%lld,\"usage\":%d},",
            pi->mem.total, pi->mem.available, pi->mem.cached, pi->mem.free, pi->mem.usage);
        len += snprintf(buf + len, size - len, "\"harddisk\":"
            "{\"total\":%lld,\"available\":%lld,\"free\":%lld,\"usage\":%d,\"details\":[",
            pi->fs.total, pi->fs.available, pi->fs.free, pi->fs.usage);
        for (i = 0; i < pi->fs.num; i++)
        {
#ifdef _UNICODE
            cel_unicode2mb(pi->fs.fss[i]->dir, -1, (char *)pi->fs.dir, CEL_DIRLEN);
#endif
            len += snprintf(buf + len, size - len, 
                "{\"dir\":\"%s\","
                "\"total\":%lld,\"available\":%lld,\"free\":%lld,\"usage\":%d},", 
#ifdef _UNICODE
                (char *)pi->fs.dir,
#else
                pi->fs.fss[i]->dir, 
#endif
                pi->fs.fss[i]->total, pi->fs.fss[i]->available, pi->fs.fss[i]->free, 
                pi->fs.fss[i]->usage);
        }
        len--;
        len += snprintf(buf + len, size - len, "]},\"net\":{"
            "\"received\":%ld,\"sent\":%ld,\"sending\":%ld,\"receiving\":%ld,"
            "\"details\":[",
            pi->net.received, pi->net.sent, pi->net.sending, pi->net.receiving);
        for (i = 0; i < pi->net.num; i++)
        {
#ifdef _UNICODE
            cel_unicode2mb(pi->net.nets[i]->ifname, -1, (char *)pi->net.ifname, CEL_IFNLEN),
#endif
            len += snprintf(buf + len, size - len, 
                "{\"ifname\":\"%s\",\"received\":%ld,\"sent\":%ld,"
                "\"sending\":%ld,\"receiving\":%ld},", 
#ifdef _UNICODE
            (char *)pi->net.ifname,
#else
            pi->net.nets[i]->ifname, 
#endif
                pi->net.nets[i]->received, pi->net.nets[i]->sent,
                pi->net.nets[i]->sending, pi->net.nets[i]->receiving);
        }
        len--;
        len += snprintf(buf + len, size - len, "]}}}");
#ifdef _CEL_UNIX
    }
    else
    {
        len = snprintf(buf, size, 
            "{"
            CEL_CRLF CEL_INDENT"\"Performance\":{"
            CEL_CRLF CEL_INDENT CEL_INDENT"\"cpu\":{"
            CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT"\"usage\":%d,"
            CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT"\"details\":[", pi->cpu.usage);
        for (i = 0; i < pi->cpu.num; i++)
        {
            len += snprintf(buf + len,size - len, 
                CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT CEL_INDENT
                "{\"id\":%d,\"usage\":%d},", 
                i, pi->cpu.cpus[i]->usage);
        }
        len--;
        len += snprintf(buf + len, size - len, 
            CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT"]"
            CEL_CRLF CEL_INDENT CEL_INDENT"},"
            CEL_CRLF CEL_INDENT CEL_INDENT"\"memory\":{"
            CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT"\"total\":%lld,"
            CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT"\"available\":%lld,"
            CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT"\"cached\":%lld,"
            CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT"\"free\":%lld,"
            CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT"\"usage\":%d"
            CEL_CRLF CEL_INDENT CEL_INDENT"},",
            pi->mem.total, pi->mem.available, pi->mem.cached, pi->mem.free, pi->mem.usage);
        len += snprintf(buf + len, size - len, 
            CEL_CRLF CEL_INDENT CEL_INDENT"\"harddisk\":{"
            CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT"\"total\":%lld,"
            CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT"\"available\":%lld,"
            CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT"\"free\":%lld,"
            CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT"\"usage\":%d,"
            CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT"\"details\":[",
            pi->fs.total, pi->fs.available, pi->fs.free, pi->fs.usage);
        for (i = 0; i < pi->fs.num; i++)
        {
            len += snprintf(buf + len, size - len, 
                CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT CEL_INDENT
                "{\"dir\":\"%s\","
                "\"total\":%lld,\"available\":%lld,\"free\":%lld,\"usage\":%d},", 
                pi->fs.fss[i]->dir,
                pi->fs.fss[i]->total, pi->fs.fss[i]->available, pi->fs.fss[i]->free, 
                pi->fs.fss[i]->usage);
        }
        len--;
        len += snprintf(buf + len, size - len, 
            CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT"]"
            CEL_CRLF CEL_INDENT CEL_INDENT"},"
            CEL_CRLF CEL_INDENT CEL_INDENT"\"net\":{"
            CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT"\"received\":%ld,"
            CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT"\"sent\":%ld,"
            CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT"\"sending\":%ld,"
            CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT"\"receiving\":%ld,"
            CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT"\"details\":[",
            pi->net.received, pi->net.sent, pi->net.sending, pi->net.receiving);
        for (i = 0; i < pi->net.num; i++)
        {
            len += snprintf(buf + len, size - len, 
                CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT CEL_INDENT
                "{\"ifname\":\"%s\",\"received\":%ld,\"sent\":%ld,"
                "\"sending\":%ld,\"receiving\":%ld},", 
                pi->net.nets[i]->ifname, 
                pi->net.nets[i]->received, pi->net.nets[i]->sent,
                pi->net.nets[i]->sending, pi->net.nets[i]->receiving);
        }
        len--;
        len += snprintf(buf + len, size - len, 
            CEL_CRLF CEL_INDENT CEL_INDENT CEL_INDENT"]"
            CEL_CRLF CEL_INDENT CEL_INDENT"}"
            CEL_CRLF CEL_INDENT"}"
            CEL_CRLF "}"CEL_CRLF);
    }
#endif

    return len;
}

int cel_perfmaxserialize(CelPerf *pi, char *buf, size_t size, int indent)
{
    int len, i;
    CelTime dt;
    TCHAR dtstr[32];

    cel_time_init_now(&dt);
    cel_time_strfltime(&dt, dtstr, 32, _T("%Y-%m-%d %X"));
    len = snprintf(buf, size, 
        "{\"datetime\":\"%s\",\"cpu\":{\"maxusage\":%d,\"details\":[", 
        dtstr, pi->cpu.maxusage);
    for (i = 0; i < pi->cpu.num; i++)
    {
        len += snprintf(buf + len,size - len, 
            "{\"id\":%d,\"maxusage\":%d},", i, pi->cpu.cpus[i]->maxusage);
    }
    len--;
    len += snprintf(buf + len, size - len, 
        "]},\"memory\":{\"maxusage\":%d},", pi->mem.maxusage);
    len += snprintf(buf + len, size - len, "\"harddisk\":"
        "{\"maxusage\":%d,\"details\":[", pi->fs.usage);
    for (i = 0; i < pi->fs.num; i++)
    {
        len += snprintf(buf + len, size - len, 
            "{\"dir\":\"%s\",\"maxusage\":%d},", 
#ifdef _UNICODE
                cel_unicode2mb(pi->fs.fss[i]->dir, -1, (char *)pi->fs.dir, CEL_DIRLEN),
#else
                pi->fs.fss[i]->dir, 
#endif
            pi->fs.fss[i]->usage);
    }
    len--;
    len += snprintf(buf + len, size - len, "]},\"net\":{"
        "\"maxsending\":%ld,\"maxreceiving\":%ld,"
        "\"details\":[",
        pi->net.max_sending, pi->net.max_receiving);
    for (i = 0; i < pi->net.num; i++)
    {
        len += snprintf(buf + len, size - len, 
            "{\"ifname\":\"%s\", \"maxsending\":%ld,\"maxreceiving\":%ld},", 
#ifdef _UNICODE
            cel_unicode2mb(pi->net.nets[i]->ifname, -1, (char *)pi->net.ifname, CEL_IFNLEN),
#else
            pi->net.nets[i]->ifname, 
#endif
            pi->net.nets[i]->max_sending, pi->net.nets[i]->max_receiving);
    }
    len--;
    len += snprintf(buf + len, size - len, "]}}");

    return len;
}
