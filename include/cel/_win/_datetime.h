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
#ifndef __CEL_DATETIME_WIN_H__
#define __CEL_DATETIME_WIN_H__

#include "cel/types.h"
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define mkgmtime(tm) _mkgmtime(tm)
/* struct tm *gmtime_r(const time_t *timep, struct tm *result); */
#define gmtime_r(timep, result) \
    (gmtime_s(result, timep) == 0 ? (result) : NULL)
/* struct tm *localtime_r(const time_t *timep, struct tm *result); */
#define localtime_r(timep, result) \
    (localtime_s(result, timep) == 0 ? (result) : NULL)

static __inline unsigned long os_getticks(void)
{
    return GetTickCount();
}

int gettimeofday (struct timeval *tv, void *tz);

#ifdef __cplusplus
}
#endif

#endif
