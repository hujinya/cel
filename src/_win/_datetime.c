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
#include "cel/datetime.h"
#ifdef _CEL_WIN
/**
 * Number of micro-seconds between the beginning of the Windows epoch 
 * (Jan. 1, 1601)and the Unix epoch (Jan. 1, 1970). 
 * 
 * This assumes all Win32 compilers have 64-bit support. 
 *
 * Number of micro-seconds between the beginning of the Windows epoch
 * (Jan. 1, 1601)and the Unix epoch (Jan. 1, 1970).
 *
 * This assumes all Win32 compilers have 64-bit support.
 */
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS) || defined(__WATCOMC__)
 #define DELTA_EPOCH_IN_USEC 11644473600000000Ui64
#else
 #define DELTA_EPOCH_IN_USEC 11644473600000000ULL
#endif

static __inline unsigned __int64 filetime_to_unix_epoch(const FILETIME *ft)
{
    unsigned __int64 res = (unsigned __int64)ft->dwHighDateTime << 32; 

    res |= ft->dwLowDateTime;
    res /= 10;                  /**< From 100 nano-sec periods to usec */
    res -= DELTA_EPOCH_IN_USEC; /**< From Win epoch to Unix epoch */

    return (res);
} 

int gettimeofday(struct timeval *tv, void *tz)
{
    FILETIME ft;
    unsigned __int64 tim; 

    if (!tv)
    {
        errno = EINVAL;
        return (-1);
    }
    GetSystemTimeAsFileTime(&ft);
    tim = filetime_to_unix_epoch(&ft);
    tv->tv_sec = (long) (tim / 1000000L);
    tv->tv_usec = (long) (tim % 1000000L);

    return (0);
}
#endif
