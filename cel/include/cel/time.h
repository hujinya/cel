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
#ifndef __CEL_TIME_H__
#define __CEL_TIME_H__

#include "cel/config.h"
#if defined(_CEL_UNIX)
#include "cel/_unix/_time.h"
#elif defined(_CEL_WIN)
#include "cel/_win/_time.h"
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

typedef struct timeval CelTime;

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

#define CEL_TIME_SLOTS 64

void cel_time_update(BOOL is_sigsafe);


static __inline int cel_time_init(CelTime *dt)
{
	dt->tv_sec = 0;
	dt->tv_usec = 0;
    return 0;
}
int cel_time_init_datatime(CelTime *dt, int year, int mon, int mday, 
						   int hour, int min, int sec);
int cel_time_init_now(CelTime *dt);

int cel_time_init_strtime_a(CelTime *dt, const CHAR *strtime);
int cel_time_init_strtime_w(CelTime *dt, const WCHAR *strtime);

static __inline void cel_time_destroy(CelTime *dt) 
{
    dt->tv_sec = 0;
	dt->tv_usec = 0;
}

CelTime *cel_time_new(void);
CelTime *cel_time_new_datetime(int year, int mon, int mday, 
							   int hour, int min, int sec);
CelTime *cel_time_new_now(void);
CelTime *cel_time_new_strtime(const TCHAR *strtime);
void cel_time_free(CelTime *dt);


#define CEL_TIME_NOW(now) \
    CelTime ltime; \
    if (now == NULL) { cel_time_init_now(&ltime); now = &ltime;}

/* void cel_time_clear(CelTime *dt); */
#define cel_time_clear(tv) (tv)->tv_sec = 0; (tv)->tv_usec = 0
int cel_time_compare(const CelTime *dt2, const CelTime *dt1);
/* BOOL cel_time_is_expired(CelTime *dt, CelTime *now); */
#define cel_time_is_expired(tv, now) (cel_time_compare((tv), now) <= 0)


/** 
 * Cel datetime get and set functions
 */
void cel_time_get_date(CelTime *dt, 
					   int *year, int *mon, int *mday, int *wday);
void cel_time_get_time(CelTime *dt, int *hour, int *min, int *sec);
void cel_time_get(CelTime *dt, 
				  int *year, int *mon, int *mday, int *wday,
				  int *hour, int *min, int *sec);
void cel_time_set_date(CelTime *dt, int year, int mon, int mday);
void cel_time_set_time(CelTime *dt, int hour, int min, int sec);
void cel_time_set_datetime(CelTime *dt, int year, int mon, int mday, 
						   int hour, int min, int sec);
void cel_time_set_milliseconds(CelTime *dt, const CelTime *now, long milliseconds);
/**
 * Cel datetime add and sub functions
 */
void cel_time_add_years(CelTime *dt, int years);
void cel_time_add_months(CelTime *dt, int months);
/* void cel_time_add_days(CelTime *dt, int days) */
#define cel_time_add_days(dt, days)  ((dt)->tv_sec) += ((days) * 24 * 60 * 60)
#define cel_time_add_hours(dt, hours)  ((dt)->tv_sec) += ((hours) * 60 * 60)
#define cel_time_add_minutes(dt, minutes)  ((dt)->tv_sec) += ((minutes) * 60)
#define cel_time_add_seconds(dt, seconds)  ((dt)->tv_sec) -= ((seconds))

/** 
 * double cel_time_diffseconds(CelTime *dt1, CelTime *dt2)
 *
 * \brief  The number of seconds elapsed between time time1 and time time2,
 *         represented as a double. 
 */
#define cel_time_diffseconds(dt1, dt2) difftime((dt1)->tv_sec, (dt2)->tv_sec)
long long cel_time_diffmicroseconds(const CelTime *dt2, const CelTime *dt1);
long cel_time_diffmilliseconds(const CelTime *dt2, const CelTime *dt1);

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
size_t cel_time_strftime_a(CelTime *dt, BOOL local,
						   CHAR *buf, size_t size, const CHAR *fmt);
size_t cel_time_strftime_w(CelTime *dt, BOOL local,
						   WCHAR *buf, size_t size, const WCHAR *fmt);

#define cel_time_strfltime_a(dt, buf, size, fmt) \
    cel_time_strftime_a(dt, TRUE, buf, size, fmt)
#define cel_time_strfltime_w(dt, buf, size, fmt) \
    cel_time_strftime_w(dt, TRUE, buf, size, fmt)

#define cel_time_strfgmtime_a(dt, buf, size, fmt) \
    cel_time_strftime_a(dt, FALSE, buf, size, fmt)
#define cel_time_strfgmtime_w(dt, buf, size, fmt) \
    cel_time_strftime_w(dt, FALSE, buf, size, fmt)

/* Sun, 06 Nov 1994 08:49:37 GMT;RFC 822, updated by RFC 1123  */
#define cel_time_rfc822_a(dt, buf, size) \
    cel_time_strfgmtime_a(dt, buf, size, "%a, %d %b %Y %X GMT")
#define cel_time_rfc822_w(dt, buf, size) \
    cel_time_strfgmtime_w(dt, buf, size, L"%a, %d %b %Y %X GMT")

/* Sunday, 06-Nov-94 08:49:37 GMT;RFC 850, obsoleted by RFC 1036 */
#define cel_time_rfc850_a(dt, buf, size) \
    cel_time_strfgmtime_a(dt, buf, size, "%A, %d-%b-%y %X GMT")
#define cel_time_rfc850_w(dt, buf, size) \
    cel_time_strfgmtime_w(dt, buf, size, L"%A, %d-%b-%y %X GMT")

/* Sun Nov 6 08:49:37 1994;ANSI C's asctime()format */
#define cel_time_asc_a(dt, buf, size) \
    cel_time_strfltime_a(dt, buf, size, "%c")
#define cel_time_asc_w(dt, buf, size) \
    cel_time_strfltime_w(dt, buf, size, L"%c")

#ifdef _UNICODE
#define cel_time_init_strtime cel_time_init_strtime_w
#define cel_time_strftime cel_time_strftime_w
#define cel_time_strfltime cel_time_strfltime_w
#define cel_time_strfgmtime cel_time_strfgmtime_w
#define cel_time_rfc822 cel_time_rfc822_w
#define cel_time_strfgmtime cel_time_strfgmtime_w
#define cel_time_asc cel_time_asc_w
#else
#define cel_time_init_strtime cel_time_init_strtime_a
#define cel_time_strftime cel_time_strftime_a
#define cel_time_strfltime cel_time_strfltime_a
#define cel_time_strfgmtime cel_time_strfgmtime_a
#define cel_time_rfc822 cel_time_rfc822_a
#define cel_time_strfgmtime cel_time_strfgmtime_a
#define cel_time_asc cel_time_asc_a
#endif

#ifdef __cplusplus
}
#endif

#endif
