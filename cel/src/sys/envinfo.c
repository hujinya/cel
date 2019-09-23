/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2019 Hu Jinya(hu_jinya@163.com) 
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
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "cel/sys/envinfo.h"
#include "cel/allocator.h"

#ifdef _CEL_UNIX
int cel_setcpumask(CelCpuMask *mask, int i)
{
    return CPU_SET(i, mask);
}

int cel_signals_init(CelSignals *signals)
{
    struct sigaction sa;

    while (signals->si_no != 0)
    {
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = signals->handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        if (sigaction(signals->si_no, &sa, NULL) != 0)
            return -1;
        signals++;
    }
    return 0;
}

int cel_resourcelimits_get(CelResourceLimits *rlimits)
{
    struct rlimit rlim;

    if (getrlimit(RLIMIT_CORE, &rlim) == -1)
        return -1;
    rlimits->core = rlim.rlim_cur;
    if (getrlimit(RLIMIT_NOFILE, &rlim) == -1)
        return -1;
    rlimits->nofile = rlim.rlim_cur;
    if (getrlimit(RLIMIT_STACK, &rlim) == -1)
        return -1;
    rlimits->stack = rlim.rlim_cur;

    return 0;
}

int cel_resourcelimits_set(CelResourceLimits *rlimits)
{
    struct rlimit rlim;

    if (getrlimit(RLIMIT_CORE, &rlim) == -1)
        return -1;
    if (rlim.rlim_cur < rlimits->core) rlim.rlim_cur = rlimits->core;
    if (rlim.rlim_max < rlimits->core) rlim.rlim_max = rlimits->core;
    if (setrlimit(RLIMIT_CORE, &rlim) == -1)
        return -1;
    if (getrlimit(RLIMIT_NOFILE, &rlim) == -1)
        return -1;
    if (rlim.rlim_cur < rlimits->nofile) rlim.rlim_cur = rlimits->nofile;
    if (rlim.rlim_max < rlimits->nofile) rlim.rlim_max = rlimits->nofile;
    if (setrlimit(RLIMIT_NOFILE, &rlim) == -1)
        return -1;
    if (getrlimit(RLIMIT_STACK, &rlim) == -1)
        return -1;
    if (rlim.rlim_cur < rlimits->stack) rlim.rlim_cur = rlimits->stack;
    if (rlim.rlim_max < rlimits->stack) rlim.rlim_max = rlimits->stack;
    if (setrlimit(RLIMIT_STACK, &rlim) == -1)
        return -1;

    return 0;
}
#endif

