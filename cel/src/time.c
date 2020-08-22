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
#include "cel/time.h"
#include "cel/allocator.h"
#include "cel/log.h"
#include "cel/error.h"

static long timezone_diff_secs = -1;

long cel_timezone_diff_secs()
{
    struct tm *timeinfo;
    time_t secs, local_secs, gmt_secs;

    time(&secs);
    timeinfo = localtime(&secs);
    local_secs = mktime(timeinfo);
    timeinfo = gmtime(&secs);
    gmt_secs = mktime(timeinfo);

    return (timezone_diff_secs = (long)(local_secs - gmt_secs));
}

static unsigned int slot = 0;
static CelAtomic time_lock;
static volatile CelTime *cached_time = NULL;
static CelTime cached_times[CEL_TIME_SLOTS];

void cel_time_update(BOOL is_sigsafe)
{
	;
}

int cel_time_init_now(CelTime *dt)
{
	return gettimeofday(dt, NULL);
}

int cel_time_init_datatime(CelTime *dt, int year, int mon, int mday, 
						   int hour, int min, int sec)
{
    struct tm tms;

    tms.tm_year = year - 1900;
    tms.tm_mon = mon;
    tms.tm_mday = mday;
    tms.tm_hour = hour;
    tms.tm_min = min;
    tms.tm_sec = sec;
    tms.tm_isdst = -1;
	dt->tv_sec = (long)mktime(&tms);
	dt->tv_usec = 0;

    return 0;
}

static int cel_time_mstr2i_a(const CHAR *month)
{
    switch (*month)
    {
    case 'A':
        return (*++month == 'p' ? 3 : 7);
    case 'D':
        return (11);
    case 'F':
        return (1);
    case 'J':
        if (*(++month) == 'a')
            return (0);
        return (*(++month) == 'n' ? 5 : 6);
    case 'M':
        return (*(month + 2) == 'r' ? 2 : 4);
    case 'N':
        return (10);
    case 'O':
        return (9);
    case 'S':
        return (8);
    default:
        CEL_SETERR((CEL_ERR_LIB, _T("Month \"%s\" format invalid."), month));
        return (-1);
    }
}

static int cel_time_mstr2i_w(const WCHAR *month)
{
    switch (*month)
    {
    case L'A':
        return (*++month == L'p' ? 3 : 7);
    case L'D':
        return (11);
    case L'F':
        return (1);
    case L'J':
        if (*(++month) == L'a')
            return (0);
        return (*(++month) == L'n' ? 5 : 6);
    case L'M':
        return (*(month + 2) == L'r' ? 2 : 4);
    case L'N':
        return (10);
    case L'O':
        return (9);
    case L'S':
        return (8);
    default:
        CEL_SETERR((CEL_ERR_LIB, _T("Month \"%s\" format invalid."), month));
        return (-1);
    }
}

int cel_time_init_strtime_a(CelTime *dt, const CHAR *strtime)
{
    struct tm _tm;
    CHAR month[11] = { '\0'};
    CHAR timezone[4] = { '\0'};

     /* Sun, 06 Nov 1994 08:49:37 GMT;RFC 822, updated by RFC 1123 */
    if (sscanf(strtime, "%*s%d %3s %d %d:%d:%d %3s",
        &(_tm.tm_mday), month, &(_tm.tm_year),
        &(_tm.tm_hour), &(_tm.tm_min), &(_tm.tm_sec), timezone) == 7);
    /* Sunday, 06-Nov-94 08:49:37 GMT;RFC 850, obsoleted by RFC 1036 */
    else if (sscanf(strtime, "%*s%d-%3s-%d %d:%d:%d %3s", 
        &(_tm.tm_mday), month, &(_tm.tm_year),
        &(_tm.tm_hour), &(_tm.tm_min), &(_tm.tm_sec), timezone) == 7);
    /* Sun Nov 6 08:49:37 1994;ANSI C's asctime()format */
    else if (sscanf(strtime, "%*s%3s %d %d:%d:%d %d",
        month, &(_tm.tm_mday), 
        &(_tm.tm_hour), &(_tm.tm_min), &(_tm.tm_sec), &(_tm.tm_year)) == 6);
    else
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Unable to resolve the time format \"%s\"."), strtime));
        return -1;
    }
    if ((_tm.tm_mon = cel_time_mstr2i_a(month)) == -1)
    {
        return -1;
    }
    _tm.tm_year -= 1900;
    _tm.tm_isdst = -1;
    /*printf("%d-%d-%d %d:%d:%d\r\n", 
        _tm.tm_year, _tm.tm_mon, _tm.tm_mday,
        _tm.tm_hour, _tm.tm_min, _tm.tm_sec);*/
	dt->tv_usec = 0;
    if (timezone[0] == '\0')
		dt->tv_sec = (long)mktime(&_tm);
    else if (strcmp(timezone, "GMT") == 0)
        dt->tv_sec = (long)mkgmtime(&_tm); 
    else
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Invalid time zone format \"%s\""), timezone));
        return -1;
    }
    //perror("xxx");
    return (dt->tv_sec == -1 ? -1 : 0);
}

int cel_time_init_strtime_w(CelTime *dt, const WCHAR *strtime)
{
    struct tm _tm;
    WCHAR month[11] = { L'\0'};
    WCHAR timezone[4] = { L'\0'};

     /* Sun, 06 Nov 1994 08:49:37 GMT;RFC 822, updated by RFC 1123 */
    if (swscanf(strtime, L"%*s%d %3s %d %d:%d:%d %3s",
        &(_tm.tm_mday), month, &(_tm.tm_year),
        &(_tm.tm_hour), &(_tm.tm_min), &(_tm.tm_sec), timezone) == 7);
    /* Sunday, 06-Nov-94 08:49:37 GMT;RFC 850, obsoleted by RFC 1036 */
    else if (swscanf(strtime, L"%*s%d-%3s-%d %d:%d:%d %3s", 
        &(_tm.tm_mday), month, &(_tm.tm_year),
        &(_tm.tm_hour), &(_tm.tm_min), &(_tm.tm_sec), timezone) == 7);
    /* Sun Nov 6 08:49:37 1994;ANSI C's asctime()format */
    else if (swscanf(strtime, L"%*s%3s %d %d:%d:%d %d",
        month, &(_tm.tm_mday), 
        &(_tm.tm_hour), &(_tm.tm_min), &(_tm.tm_sec), &(_tm.tm_year)) == 6);
    else
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Unable to resolve the time format \"%s\"."), strtime));
        return -1;
    }
    if ((_tm.tm_mon = cel_time_mstr2i_w(month)) == -1)
    {
        return -1;
    }
	dt->tv_usec = 0;
    if (timezone[0] == L'\0')
		dt->tv_sec = (long)mktime(&_tm);
    else if (wcscmp(timezone, L"GMT") == 0)
		dt->tv_usec = (long)mkgmtime(&_tm);
    else
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Invalid time zone format \"%s\""), timezone));
        return -1;
    }

    return (dt->tv_sec == -1 ? -1 : 0);
}

CelTime *cel_time_new(void)
{
    CelTime *dt;

    if ((dt = (CelTime *)cel_malloc(sizeof(CelTime))) != NULL)
    {
        if (cel_time_init(dt) == 0)
            return dt;
        cel_free(dt);
    }
    return NULL;
}

CelTime *cel_time_new_datetime(int year, int mon, int mday, 
							   int hour, int min, int sec)
{
    CelTime *dt;

    if ((dt = (CelTime *)cel_malloc(sizeof(CelTime))) != NULL)
    {
        if (cel_time_init_datatime(dt, year, mon, mday, hour, min, sec) == 0)
            return dt;
        cel_free(dt);
    }
    return NULL;
}

CelTime *cel_time_new_now(void)
{
    CelTime *dt;

    if ((dt = cel_time_new()) != NULL)
        gettimeofday(dt, NULL);
    return dt;
}

CelTime *cel_time_new_strtime(const TCHAR *strtime)
{
    CelTime *dt;
    if ((dt = cel_time_new()) == NULL 
        || cel_time_init_strtime(dt, strtime) == -1)
        return NULL;

    return dt;
}

void cel_time_free(CelTime *dt)
{
    cel_time_destroy(dt); 
    cel_free(dt);
}

void cel_time_set_milliseconds(CelTime *dt, 
							   const CelTime *now, long milliseconds)
{
    CEL_TIME_NOW(now);
    dt->tv_sec = now->tv_sec + (milliseconds / 1000L);
    if ((dt->tv_usec = now->tv_usec + ((milliseconds % 1000L) * 1000L)) > 1000000L)
    {
        dt->tv_sec += (dt->tv_usec / 1000000L);
        dt->tv_usec = (dt->tv_usec % 1000000L);
    }
    //_tprintf(_T("%ld - %ld %ld\n"), tv->tv_sec, tv->tv_usec, milliseconds);
}

int cel_time_compare(const CelTime *dt2, const CelTime *dt1)
{
    CEL_TIME_NOW(dt1);

    /*_tprintf(_T("%ld %ld - %ld %ld\n"), 
               dt2->tv_sec, dt2->tv_usec, dt1->tv_sec, dt1->tv_usec);*/
    if (dt2->tv_sec > dt1->tv_sec)
        return 1;
    else if (dt2->tv_sec < dt1->tv_sec)
        return -1;
    else if (dt2->tv_usec > dt1->tv_usec)
        return 1;
    else if (dt2->tv_usec < dt1->tv_usec)
        return -1;
    return 0;
}

long long cel_time_diffmicroseconds(const CelTime *dt2, const CelTime *dt1)
{
    long long diff_microseconds;

    CEL_TIME_NOW(dt1);
    diff_microseconds = dt2->tv_usec - dt1->tv_usec;
    diff_microseconds += (dt2->tv_sec - dt1->tv_sec) * LL(1000000);

    return diff_microseconds;
}

long cel_time_diffmilliseconds(const CelTime *dt2, const CelTime *dt1)
{
    long diff_microseconds, diff_milliseconds;

    CEL_TIME_NOW(dt1);
    diff_milliseconds = (dt2->tv_sec - dt1->tv_sec) * L(1000);
    diff_milliseconds += 
        (diff_microseconds = dt2->tv_usec - dt1->tv_usec) / L(1000);
    if (diff_microseconds % L(1000) != 0)
    {
        if (diff_milliseconds >= 0)
        {
            if (diff_microseconds > 0)
                diff_milliseconds += 1;
        }
        else if (diff_microseconds < 0)
            diff_milliseconds += -1;
    }
    /*
    _tprintf(_T("%ld %ld - %ld %ld\n"), 
        dt2->tv_sec, dt2->tv_usec, dt1->tv_sec, dt1->tv_usec);
        */

    return diff_milliseconds;
}

void cel_time_get_date(CelTime *dt, int *year, int *mon, int *mday, int *wday)
{
	struct tm tms;

	if (dt == NULL)
		gettimeofday(dt, NULL);

	localtime_r((time_t *)&(dt->tv_sec), &tms);
	if (year != NULL) *year = tms.tm_year + 1900;
	if (mon != NULL) *mon = tms.tm_mon;
	if (mday != NULL) *mday = tms.tm_mday;
	if (wday != NULL) *wday = tms.tm_wday;
}

void cel_time_get_time(CelTime *dt, int *hour, int *min, int *sec)
{
    struct tm tms;

    if (dt == NULL)
		gettimeofday(dt, NULL);

	localtime_r((time_t *)&(dt->tv_sec), &tms);
    if (hour != NULL) *hour = tms.tm_hour;
    if (min != NULL) *min = tms.tm_min;
    if (sec!= NULL) *sec = tms.tm_sec;
}

void cel_time_get(CelTime *dt, int *year, 
				  int *mon, int *mday, int *wday,
				  int *hour, int *min, int *sec)
{
    struct tm tms;

    if (dt == NULL)
		gettimeofday(dt, NULL);

	localtime_r((time_t *)&(dt->tv_sec), &tms);
    if (year != NULL) *year = tms.tm_year + 1900;
    if (mon != NULL) *mon = tms.tm_mon;
    if (mday != NULL) *mday = tms.tm_mday;
    if (wday != NULL) *wday = tms.tm_wday;
    if (hour != NULL) *hour = tms.tm_hour;
    if (min != NULL) *min = tms.tm_min;
    if (sec != NULL) *sec = tms.tm_sec;
}

void cel_time_set_date(CelTime *dt, int year, int mon, int mday)
{
    struct tm tms;

    localtime_r((time_t *)&(dt->tv_sec), &tms);
    tms.tm_year = year - 1900;
    tms.tm_mon = mon;
    tms.tm_mday = mday;
	dt->tv_sec = (long)mktime(&tms);
	dt->tv_usec = 0;
}

void cel_time_set_time(CelTime *dt, int hour, int min, int sec)
{
    struct tm tms;

    localtime_r((time_t *)&(dt->tv_sec), &tms);
    tms.tm_hour = hour;
    tms.tm_min = min;
    tms.tm_sec = sec;
    dt->tv_sec = (long)mktime(&tms);
	dt->tv_usec = 0;
}

void cel_time_set_datetime(CelTime *dt, int year, int mon, int mday, 
						   int hour, int min, int sec)
{
    struct tm tms;

    localtime_r((time_t *)&(dt->tv_sec), &tms);
    tms.tm_year = year - 1900;
    tms.tm_mon = mon;
    tms.tm_mday = mday;
    tms.tm_hour = hour;
    tms.tm_min = min;
    tms.tm_sec = sec;
    dt->tv_sec = (long)mktime(&tms);
	dt->tv_usec = 0;
}

void cel_time_add_months(CelTime *dt, int months)
{
    int n_months, n_years;
    struct tm tms;

    n_years  = months / 12;
    n_months = months % 12;
    localtime_r((time_t *)&(dt->tv_sec), &tms);
    tms.tm_year += n_years;
    tms.tm_mon += n_months;
    if (tms.tm_mon > 11)
    {
        tms.tm_year += 1;
        tms.tm_mon -= 12;
    }
    else if (tms.tm_mon < 0)
    {
        tms.tm_year -= 1;
        tms.tm_mon += 12;
    }
    if ((tms.tm_mon == 1 && tms.tm_mday == 29 
        && !CEL_ISLEAPYEAR(tms.tm_year + 1900))
        || (tms.tm_mday == 31 && (tms.tm_mon & 1)))
        tms.tm_mday--;
    dt->tv_sec = (long)mktime(&tms);
}

void cel_time_add_years(CelTime *dt, int years)
{
    struct tm tms;

    localtime_r((time_t *)&(dt->tv_sec), &tms);
    tms.tm_year += years;
    if (tms.tm_mon == 1 && tms.tm_mday == 29 
        && !CEL_ISLEAPYEAR(tms.tm_year + 1900))
        tms.tm_mday--;
    dt->tv_sec = (long)mktime(&tms);
}

size_t cel_time_strftime_a(CelTime *dt, BOOL local,
						   CHAR *buf, size_t size, const CHAR *fmt)
{
    struct tm tms;

    if (dt == NULL)
		gettimeofday(dt, NULL);
    return strftime(buf, size, fmt, 
		(local ? localtime_r((time_t *)&(dt->tv_sec), &tms) 
		: gmtime_r((time_t *)&(dt->tv_sec), &tms)));
}

#ifdef _CEL_WIN
size_t cel_time_strftime_w(CelTime *dt, BOOL local,
						   WCHAR *buf, size_t size, const WCHAR *fmt)
{
    struct tm tms;

    if (dt == NULL)
		gettimeofday(dt, NULL);
    return wcsftime(buf, size, fmt, 
        (local ? localtime_r((time_t *)&(dt->tv_sec), &tms) 
		: gmtime_r((time_t *)&(dt->tv_sec), &tms)));
}
#endif
