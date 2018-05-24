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
#ifndef __CEL_DATETIME_H__
#define __CEL_DATETIME_H__

#include "cel/config.h"
#if defined(_CEL_UNIX)
#include "cel/_unix/_datetime.h"
#elif defined(_CEL_WIN)
#include "cel/_win/_datetime.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Sun, 06 Nov 1994 08:49:37 GMT */
#define CEL_DATETIME_RFC822_LEN   30
/* Sunday, 06-Nov-94 08:49:37 GMT */
#define CEL_DATETIME_RFC850_LEN   31
/* Sun Nov 6 08:49:37 1994 */
#define CEL_DATETIME_ASC_LEN      24

typedef time_t CelDateTime;

#define cel_getticks os_getticks
/**
 * unsigned long cel_diffticks(unsigned long ticks2, unsigned long ticks1);
 *
 * \brief  Get the number of seconds elapsed between tick ticks2 
 *         and tick ticks1, represented as a unsigned long.
 */
#define cel_diffticks(ticks2, ticks1)  CEL_DIFF(ticks2, ticks1)

#define CEL_ISLEAPYEAR(year) \
    ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)

static __inline int cel_datetime_init(CelDateTime *dt)
{
    *(dt)= 0;
    return 0;
}
int cel_datetime_init_value(CelDateTime *dt, int year, int mon, int mday, 
                            int hour, int min, int sec);
static __inline int cel_datetime_init_now(CelDateTime *dt)
{
    *dt = time(NULL);
    return 0;
}
int cel_datetime_init_strtime_a(CelDateTime *dt, const CHAR *strtime);
int cel_datetime_init_strtime_w(CelDateTime *dt, const WCHAR *strtime);

static __inline void cel_datetime_destroy(CelDateTime *dt) 
{
    *(dt)= 0;
}

CelDateTime *cel_datetime_new(void);
CelDateTime *cel_datetime_new_value(int year, int mon, int mday, 
                                    int hour, int min, int sec);
CelDateTime *cel_datetime_new_now(void);
CelDateTime *cel_datetime_new_strtime(const TCHAR *strtime);
void cel_datetime_free(CelDateTime *dt);

/** 
 * Cel datetime get and set functions
 */
void cel_datetime_get_date(CelDateTime *dt, 
                           int *year, int *mon, int *mday, int *wday);
void cel_datetime_get_time(CelDateTime *dt, int *hour, int *min, int *sec);
void cel_datetime_get(CelDateTime *dt, 
                      int *year, int *mon, int *mday, int *wday,
                      int *hour, int *min, int *sec);
void cel_datetime_set_date(CelDateTime *dt, int year, int mon, int mday);
void cel_datetime_set_time(CelDateTime *dt, int hour, int min, int sec);
void cel_datetime_set(CelDateTime *dt, int year, int month, int mday, 
                      int hour, int min, int sec);
/**
 * Cel datetime add and sub functions
 */
void cel_datetime_add_years(CelDateTime *dt, int years);
void cel_datetime_add_months(CelDateTime *dt, int months);
/* void cel_datetime_add_days(CelDateTime *dt, int days) */
#define cel_datetime_add_days(dt, days)  (*(dt))+= ((days) * 24 * 60 * 60)
#define cel_datetime_add_hours(dt, hours)  (*(dt))+= ((hours) * 60 * 60)
#define cel_datetime_add_minutes(dt, minutes)  (*(dt))+= ((minutes) * 60)
#define cel_datetime_add_seconds(dt, seconds)  (*(dt))-= ((seconds))

/** 
 * double cel_datetime_diffseconds(CelDateTime *dt1, CelDateTime *dt2)
 *
 * \brief  The number of seconds elapsed between time time1 and time time2,
 *         represented as a double. 
 */
#define cel_datetime_diffseconds(dt1, dt2) difftime(*(dt1), *(dt2))
#define cel_datetime_difftime cel_datetime_diffseconds

/** 
 * Specifier Replaced by                             Example 
 * %a        Abbreviated weekday name*               Thu 
 * %A        Full weekday name*                      Thursday 
 * %b        Abbreviated month name*                 Aug 
 * %B        Full month name*                        August 
 * %c        Date and time representation*           Thu Aug 23 14:55:02 2001
 * %d        Day of the month (01-31)                23 
 * %H        Hour in 24h format (00-23)              14 
 * %I        Hour in 12h format (01-12)              02 
 * %j        Day of the year (001-366)               235 
 * %m        Month as a decimal number(01-12)        08 
 * %M        Minute (00-59)                          55 
 * %p        AM or PM designation                    PM 
 * %S        Second (00-61)                          02 
 * %U        Week number with the first Sunday as 
 *           the first day of week one(00-53)        33
 * %w        Weekday as a decimal number with Sunday 
 *           as 0 (0-6)                              4 
 * %W        Week number with the first Monday as
 *           the first day of week one(00-53)        34 
 * %x        Date representation*                    08/23/01 
 * %X        Time representation*                    14:55:02 
 * %y        Year, last two digits(00-99)            01 
 * %Y        Year                                    2001 
 * %Z        Timezone name or abbreviation           CDT 
 * %%        A %sign                                 % 
 *
 * (Description is marked with an asterisk(*) are locale-dependent)
 */
size_t cel_datetime_strftime_a(CelDateTime *dt, BOOL local,
                               CHAR *buf, size_t size, const CHAR *fmt);
size_t cel_datetime_strftime_w(CelDateTime *dt, BOOL local,
                               WCHAR *buf, size_t size, const WCHAR *fmt);

#define cel_datetime_strfltime_a(dt, buf, size, fmt) \
    cel_datetime_strftime_a(dt, TRUE, buf, size, fmt)
#define cel_datetime_strfltime_w(dt, buf, size, fmt) \
    cel_datetime_strftime_w(dt, TRUE, buf, size, fmt)

#define cel_datetime_strfgmtime_a(dt, buf, size, fmt) \
    cel_datetime_strftime_a(dt, FALSE, buf, size, fmt)
#define cel_datetime_strfgmtime_w(dt, buf, size, fmt) \
    cel_datetime_strftime_w(dt, FALSE, buf, size, fmt)

/* Sun, 06 Nov 1994 08:49:37 GMT;RFC 822, updated by RFC 1123  */
#define cel_datetime_rfc822_a(dt, buf, size) \
    cel_datetime_strfgmtime_a(dt, buf, size, "%a, %d %b %Y %X GMT")
#define cel_datetime_rfc822_w(dt, buf, size) \
    cel_datetime_strfgmtime_w(dt, buf, size, L"%a, %d %b %Y %X GMT")

/* Sunday, 06-Nov-94 08:49:37 GMT;RFC 850, obsoleted by RFC 1036 */
#define cel_datetime_rfc850_a(dt, buf, size) \
    cel_datetime_strfgmtime_a(dt, buf, size, "%A, %d-%b-%y %X GMT")
#define cel_datetime_rfc850_w(dt, buf, size) \
    cel_datetime_strfgmtime_w(dt, buf, size, L"%A, %d-%b-%y %X GMT")

/* Sun Nov 6 08:49:37 1994;ANSI C's asctime()format */
#define cel_datetime_asc_a(dt, buf, size) \
    cel_datetime_strfltime_a(dt, buf, size, "%c")
#define cel_datetime_asc_w(dt, buf, size) \
    cel_datetime_strfltime_w(dt, buf, size, L"%c")

#ifdef _UNICODE
#define cel_datetime_init_strtime cel_datetime_init_strtime_w
#define cel_datetime_strftime cel_datetime_strftime_w
#define cel_datetime_strfltime cel_datetime_strfltime_w
#define cel_datetime_strfgmtime cel_datetime_strfgmtime_w
#define cel_datetime_rfc822 cel_datetime_rfc822_w
#define cel_datetime_strfgmtime cel_datetime_strfgmtime_w
#define cel_datetime_asc cel_datetime_asc_w
#else
#define cel_datetime_init_strtime cel_datetime_init_strtime_a
#define cel_datetime_strftime cel_datetime_strftime_a
#define cel_datetime_strfltime cel_datetime_strfltime_a
#define cel_datetime_strfgmtime cel_datetime_strfgmtime_a
#define cel_datetime_rfc822 cel_datetime_rfc822_a
#define cel_datetime_strfgmtime cel_datetime_strfgmtime_a
#define cel_datetime_asc cel_datetime_asc_a
#endif

#ifdef __cplusplus
}
#endif

#endif
