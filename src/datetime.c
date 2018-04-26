#include "cel/datetime.h"
#include "cel/allocator.h"
#include "cel/log.h"
#include "cel/error.h"

/* Debug defines */
#define Debug(args)   /* cel_log_debug args */
#define Warning(args) CEL_SETERRSTR(args)/* cel_log_warning args */
#define Err(args)   CEL_SETERRSTR(args)/* cel_log_err args */

static long timezone_diff_secs = -1;

long cel_timezone_diff_secs()
{
    struct tm * timeinfo;
    time_t secs, local_secs, gmt_secs;

    time(&secs);
    timeinfo = localtime(&secs);
    local_secs = mktime(timeinfo);
    timeinfo = gmtime(&secs);
    gmt_secs = mktime(timeinfo);
    return (timezone_diff_secs = (long)(local_secs - gmt_secs));
}

int cel_datetime_init_value(CelDateTime *dt, int year, int mon, int mday, 
                            int hour, int min, int sec)
{
    struct tm tms;

    *dt = 0;
    localtime_r(dt, &tms);
    tms.tm_year = year - 1900;
    tms.tm_mon = mon;
    tms.tm_mday = mday;
    tms.tm_hour = hour;
    tms.tm_min = min;
    tms.tm_sec = sec;
    tms.tm_isdst = -1;
    *dt = mktime(&tms);

    return 0;
}

static int cel_datetime_mstr2i_a(const CHAR *month)
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
        Err((_T("Month \"%s\" format invalid."), month));
        return (-1);
    }
}

static int cel_datetime_mstr2i_w(const WCHAR *month)
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
        Err((_T("Month \"%s\" format invalid."), month));
        return (-1);
    }
}

int cel_datetime_init_strtime_a(CelDateTime *dt, const CHAR *strtime)
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
        Err((_T("Unable to resolve the time format \"%s\"."), strtime));
        return -1;
    }
    if ((_tm.tm_mon = cel_datetime_mstr2i_a(month)) == -1)
    {
        return -1;
    }
    _tm.tm_year -= 1900;
    _tm.tm_isdst = -1;
    /*printf("%d-%d-%d %d:%d:%d\r\n", 
        _tm.tm_year, _tm.tm_mon, _tm.tm_mday,
        _tm.tm_hour, _tm.tm_min, _tm.tm_sec);*/
    if (timezone[0] == '\0')
        *dt = mktime(&_tm);
    else if (strcmp(timezone, "GMT") == 0)
        *dt = mkgmtime(&_tm); 
    else
    {
        Err((_T("Invalid time zone format \"%s\""), timezone));
        return -1;
    }
    //perror("xxx");
    return (*dt == -1 ? -1 : 0);
}

int cel_datetime_init_strtime_w(CelDateTime *dt, const WCHAR *strtime)
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
        Err((_T("Unable to resolve the time format \"%s\"."), strtime));
        return -1;
    }
    if ((_tm.tm_mon = cel_datetime_mstr2i_w(month)) == -1)
    {
        return -1;
    }
    if (timezone[0] == L'\0')
        *dt = mktime(&_tm);
    else if (wcscmp(timezone, L"GMT") == 0)
        *dt = mkgmtime(&_tm);
    else
    {
        Err((_T("Invalid time zone format \"%s\""), timezone));
        return -1;
    }

    return 0;
}

CelDateTime *cel_datetime_new(void)
{
    CelDateTime *dt;

    if ((dt = (CelDateTime *)cel_malloc(sizeof(CelDateTime))) != NULL)
    {
        if (cel_datetime_init(dt) == 0)
            return dt;
        cel_free(dt);
    }
    return NULL;
}

CelDateTime *cel_datetime_new_value(int year, int mon, int mday, 
                                    int hour, int min, int sec)
{
    CelDateTime *dt;

    if ((dt = (CelDateTime *)cel_malloc(sizeof(CelDateTime))) != NULL)
    {
        if (cel_datetime_init_value(dt, year, mon, mday, hour, min, sec) == 0)
            return dt;
        cel_free(dt);
    }
    return NULL;
}

CelDateTime *cel_datetime_new_now(void)
{
    CelDateTime *dt;

    if ((dt = cel_datetime_new()) != NULL)
        *dt = time(NULL);
    return dt;
}

CelDateTime *cel_datetime_new_strtime(const TCHAR *strtime)
{
    CelDateTime *dt;
    if ((dt = cel_datetime_new()) == NULL 
        || cel_datetime_init_strtime(dt, strtime) == -1)
        return NULL;

    return dt;
}

void cel_datetime_free(CelDateTime *dt)
{
    cel_datetime_destroy(dt); 
    cel_free(dt);
}

void cel_datetime_get_date(CelDateTime *dt, 
                           int *year, int *mon, int *mday, int *wday)
{
    time_t now;
    struct tm tms;

    if (dt == NULL)
    {
        now = time(NULL);
        dt = &now;
    }
    localtime_r(dt, &tms);
    if (year != NULL) *year = tms.tm_year + 1900;
    if (mon != NULL) *mon = tms.tm_mon;
    if (mday != NULL) *mday = tms.tm_mday;
    if (wday != NULL) *wday = tms.tm_wday;
}

void cel_datetime_get_time(CelDateTime *dt, int *hour, int *min, int *sec)
{
    time_t now;
    struct tm tms;

    if (dt == NULL)
    {
        now = time(NULL);
        dt = &now;
    }
    localtime_r(dt, &tms);
    if (hour != NULL) *hour = tms.tm_hour;
    if (min != NULL) *min = tms.tm_min;
    if (sec!= NULL) *sec = tms.tm_sec;
}

void cel_datetime_get(CelDateTime *dt, int *year, 
                      int *mon, int *mday, int *wday,
                      int *hour, int *min, int *sec)
{
    time_t now;
    struct tm tms;

    if (dt == NULL)
    {
        now = time(NULL);
        dt = &now;
    }
    localtime_r(dt, &tms);
    if (year != NULL) *year = tms.tm_year + 1900;
    if (mon != NULL) *mon = tms.tm_mon;
    if (mday != NULL) *mday = tms.tm_mday;
    if (wday != NULL) *wday = tms.tm_wday;
    if (hour != NULL) *hour = tms.tm_hour;
    if (min != NULL) *min = tms.tm_min;
    if (sec != NULL) *sec = tms.tm_sec;
}

void cel_datetime_set_date(CelDateTime *dt, int year, int mon, int mday)
{
    struct tm tms;

    localtime_r(dt, &tms);
    tms.tm_year = year - 1900;
    tms.tm_mon = mon;
    tms.tm_mday = mday;
    *dt = mktime(&tms);
}

void cel_datetime_set_time(CelDateTime *dt, int hour, int min, int sec)
{
    struct tm tms;

    localtime_r(dt, &tms);
    tms.tm_hour = hour;
    tms.tm_min = min;
    tms.tm_sec = sec;
    *dt = mktime(&tms);
}

void cel_datetime_set(CelDateTime *dt, int year, int mon, int mday, 
                      int hour, int min, int sec)
{
    struct tm tms;

    localtime_r(dt, &tms);
    tms.tm_year = year - 1900;
    tms.tm_mon = mon;
    tms.tm_mday = mday;
    tms.tm_hour = hour;
    tms.tm_min = min;
    tms.tm_sec = sec;
    *dt = mktime(&tms);
}

void cel_datetime_add_months(CelDateTime *dt, int months)
{
    int n_months, n_years;
    struct tm tms;

    n_years  = months / 12;
    n_months = months % 12;
    localtime_r(dt, &tms);
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
    *dt = mktime(&tms);
}

void cel_datetime_add_years(CelDateTime *dt, int years)
{
    struct tm tms;

    localtime_r(dt, &tms);
    tms.tm_year += years;
    if (tms.tm_mon == 1 && tms.tm_mday == 29 
        && !CEL_ISLEAPYEAR(tms.tm_year + 1900))
        tms.tm_mday--;
    *dt = mktime(&tms);
}

size_t cel_datetime_strftime_a(CelDateTime *dt, BOOL local,
                               CHAR *buf, size_t size, const CHAR *fmt)
{
    time_t now;
    struct tm tms;

    if (dt == NULL)
    {
        now = time(NULL);
        dt = &now;
    }
    return strftime(buf, size, fmt, 
        (local ? localtime_r(dt, &tms) : gmtime_r(dt, &tms)));
}

#ifdef _CEL_WIN
size_t cel_datetime_strftime_w(CelDateTime *dt, BOOL local,
                               WCHAR *buf, size_t size, const WCHAR *fmt)
{
    time_t now;
    struct tm tms;

    if (dt == NULL)
    {
        now = time(NULL);
        dt = &now;
    }
    return wcsftime(buf, size, fmt, 
        (local ? localtime_r(dt, &tms) : gmtime_r(dt, &tms)));
}
#endif
