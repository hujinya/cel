/**
 * OS(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __OS_DATETIME_WIN_H__
#define __OS_DATETIME_WIN_H__

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
