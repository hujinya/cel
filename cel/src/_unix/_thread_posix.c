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
#include "cel/thread.h"
#ifdef _CEL_UNIX

int os_thread_setaffinity(OsThread *tidp, CelCpuMask *affinity_mask)
{
    return pthread_setaffinity_np(*tidp, sizeof(CelCpuMask), affinity_mask);
}

int os_cond_timedwait(OsCond *cond, OsMutex *mutex, int milliseconds)
{
    struct timeval now;
    struct timespec abstime;

    gettimeofday(&now, NULL);
    abstime.tv_nsec = now.tv_usec * 1000 + (milliseconds % 1000) * 1000000L; 
    abstime.tv_sec = now.tv_sec 
        + milliseconds / 1000L 
        + abstime.tv_nsec / 1000000000L;
    abstime.tv_nsec = abstime.tv_nsec % 1000000000L;
    return pthread_cond_timedwait(cond, mutex, &abstime);
}
#endif
