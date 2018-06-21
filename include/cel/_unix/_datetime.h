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
#ifndef __CEL_DATETIME_UNIX_H__
#define __CEL_DATETIME_UNIX_H__

#include "cel/types.h"
#include "cel/allocator.h"
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>

#ifdef __cplusplus
extern "C" {
#endif

#define mkgmtime timegm

//static __inline time_t mkgmtime(struct tm *tm)
//{
    //time_t ret;
    //char *tz = cel_strdup(getenv("TZ"));
    //setenv("TZ", "GMT-8", 1);
    //tzset();
    //ret = mktime(tm);
    //if (tz != NULL) 
    //{
    //    setenv("TZ", tz, 1);
    //    cel_free(tz);
    //} 
    //else
    //{
    //    unsetenv("TZ");
    //}
    //tzset();
    //return ret;
//}

static __inline unsigned long os_getticks(void)
{
    struct tms tm;
    return (times(&tm) * (1000 / sysconf(_SC_CLK_TCK)));
}

#ifdef __cplusplus
}
#endif

#endif