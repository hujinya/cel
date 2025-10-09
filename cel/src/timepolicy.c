/**
 * CEL(C Extension Library)
 * Copyright (C)2008 Hu Jinya(hu_jinya@163.com)
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
#include "cel/allocator.h"
#include "cel/log.h"
#include "cel/error.h"
#include "cel/timepolicy.h"
#include "cel/convert.h"

// * ? 00:00:00-23:59:59
// ? Working-Day 00:00:00-23:59:59
// 1,2,3 ? 00:00:00-23:59:59
int cel_timepolicy_init(CelTimePolicy *time_policy, const TCHAR *policy_str)
{
	int i = 0, j;
	int mday;
	char buf[20];
	CelTimeRange *time_range;

	time_policy->is_everyday = FALSE;
	time_policy->is_weekly = FALSE;
	CEL_ZEROFLAG(time_policy->mdays);
	CEL_ZEROFLAG(time_policy->wdays);
	cel_arraylist_init(&(time_policy->time_ranges), cel_free);
	// every day
	if (policy_str[0] == _T('*'))
	{
		time_policy->is_everyday = TRUE;
		if (policy_str[++i] != _T(' ') || policy_str[++i] != _T('?') || policy_str[++i] != _T(' '))
		{
			CEL_SETERR((CEL_ERR_LIB, _T("Policy \"%s\" mday[%d] format error."), policy_str, i));
			return -1;
		}
		i++;
	}
	// parse wday
	else if (policy_str[0] == _T('?'))
	{
		if (policy_str[++i] != _T(' '))
		{
			CEL_SETERR((CEL_ERR_LIB, _T("Policy \"%s\" wday[%d] format error."), policy_str, i));
			return -1;
		}
		time_policy->is_weekly = TRUE;
		i++;
		j = 0;
		for (; policy_str[i] != _T('\0');)
		{
			buf[j++] = policy_str[i++];
			if (policy_str[i] == _T(',') || policy_str[i] == _T(' '))
			{
				buf[j] = '\0';
				if (buf[0] == _T('S'))
				{
					if (buf[1] == _T('U')) // SUN
						CEL_SETFLAG(time_policy->wdays, (U(1) << 0));
					else if (buf[1] == _T('A')) // SAT
						CEL_SETFLAG(time_policy->wdays, (U(1) << 6));
				}
				else if (buf[0] == _T('M')) // MON
				{
					CEL_SETFLAG(time_policy->wdays, (U(1) << 1));
				}
				else if (buf[0] == _T('T'))
				{
					if (buf[1] == _T('U')) // TUE
						CEL_SETFLAG(time_policy->wdays, (U(1) << 2));
					else if (buf[1] == _T('H')) // THU
						CEL_SETFLAG(time_policy->wdays, (U(1) << 4));
				}
				else if (buf[0] == _T('W')) // WED
				{
					if (buf[1] == _T('o')) // Working-Day
					{
						/* 1-5 B111110 */
						CEL_SETFLAG(time_policy->wdays, 0x3E);
					}
					else if (buf[1] == _T('E'))
						CEL_SETFLAG(time_policy->wdays, (U(1) << 3));
				}
				else if (buf[0] == _T('F')) // FRI
				{
					CEL_SETFLAG(time_policy->wdays, (U(1) << 5));
				}
				else if (buf[0] == _T('O')) // Off-Day
				{
					/* 6,0 B1000001 */
					CEL_SETFLAG(time_policy->wdays, 0x41);
				}
				else
				{
					CEL_SETERR((CEL_ERR_LIB, _T("Policy \"%s\" wday \"%s\" format error."), policy_str, buf));
					return -1;
				}
				j = 0;
				if (policy_str[i++] == _T(' '))
				{
					break;
				}
			}
			if (j > 11)
			{
				CEL_SETERR((CEL_ERR_LIB, _T("Policy \"%s\" wday \"%s\" length error."), policy_str, buf));
				return -1;
			}
		}
	}
	// parse mday
	else
	{
		time_policy->is_weekly = FALSE;
		j = 0;
		for (; policy_str[i] != _T('\0');)
		{
			buf[j++] = policy_str[i++];
			if (policy_str[i] == _T(',') || policy_str[i] == _T(' '))
			{
				buf[j] = '\0';
				mday = atoi(buf);
				if (mday < 1 || mday > 31)
					return -1;
				CEL_SETFLAG(time_policy->mdays, (U(1) << (mday - 1)));
				j = 0;
				if (policy_str[i++] == _T(' '))
				{
					if (policy_str[i++] == _T('?') && policy_str[i++] == _T(' '))
					{
						break;
					}
					CEL_SETERR((CEL_ERR_LIB, _T("Policy \"%s\" mday[%d] format error."), policy_str, i));
					return -1;
				}
			}
			if (j > 2)
			{
				CEL_SETERR((CEL_ERR_LIB, _T("Policy \"%s\" mday \"%s\" length error."), policy_str, buf));
				return -1;
			}
		}
	}
	// parse time range
	if (policy_str[i] == _T('*'))
		return 0;
	j = 0;
	for (; policy_str[i] != _T('\0');)
	{
		buf[j++] = policy_str[i++];
		if (policy_str[i] == _T(',') || policy_str[i] == _T('\0'))
		{
			buf[j] = '\0';
			time_range = (CelTimeRange *)cel_malloc(sizeof(CelTimeRange));
			if (_stscanf(buf, "%d:%d:%d-%d:%d:%d",
						 &(time_range->start_hour), &(time_range->star_min), &(time_range->start_sec),
						 &(time_range->end_hour), &(time_range->end_min), &(time_range->end_sec)) != 6)
			{
				CEL_SETERR((CEL_ERR_LIB, _T("Policy \"%s\" time-range \"%s\" format error."), policy_str, buf));
				return -1;
			}
			cel_arraylist_push_back(&(time_policy->time_ranges), time_range);
			j = 0;
			if (policy_str[i] != _T('\0'))
				i++;
		}
		if (j > 17)
		{
			CEL_SETERR((CEL_ERR_LIB, _T("Policy \"%s\" time-range \"%s\" length error."), policy_str, buf));
			return -1;
		}
	}
	return 0;
}

void cel_timepolicy_destroy(CelTimePolicy *time_policy)
{
	time_policy->is_everyday = FALSE;
	time_policy->is_weekly = FALSE;
	CEL_ZEROFLAG(time_policy->mdays);
	CEL_ZEROFLAG(time_policy->wdays);
	cel_arraylist_destroy(&(time_policy->time_ranges));
}

CelTimePolicy *cel_timepolicy_new(const char *policy_str)
{
	CelTimePolicy *time_policy;

	if ((time_policy = (CelTimePolicy *)cel_malloc(sizeof(CelTimePolicy))) != NULL)
	{
		if (cel_timepolicy_init(time_policy, policy_str) == 0)
			return time_policy;
		cel_free(time_policy);
	}

	return NULL;
}

void cel_timepolicy_free(CelTimePolicy *time_policy)
{
	//_tprintf(_T("CelTimePolicy %p free.\r\n"), time_policy);
	cel_timepolicy_destroy(time_policy);
	cel_free(time_policy);
}

BOOL cel_timepolicy_is_allow(CelTimePolicy *time_policy, CelTime *time)
{
	int i, n;
	int mday, wday;
	CelTimeRange *time_range;
	CelTime ltime;

	if (!time_policy->is_everyday)
	{
		cel_time_get_date(time, NULL, NULL, &mday, &wday);
		if (time_policy->is_weekly)
		{
			if (!CEL_CHECKFLAG(time_policy->wdays, (U(1) << wday)))
			{
				// puts("WDay not matched");
				return FALSE;
			}
		}
		else
		{
			if (!CEL_CHECKFLAG(time_policy->mdays, (U(1) << (mday - 1))))
			{
				// puts("MDay not matched");
				return FALSE;
			}
		}
	}
	n = cel_arraylist_get_size(&(time_policy->time_ranges));
	if (n == 0)
		return TRUE;
	for (i = 0; i < n; i++)
	{
		time_range = (CelTimeRange *)cel_arraylist_get_by_index(
			&(time_policy->time_ranges), i);
		memcpy(&ltime, time, sizeof(CelTime));
		cel_time_set_time(&ltime,
						  time_range->start_hour,
						  time_range->star_min,
						  time_range->start_sec);
		// if time < ltime rturn false
		if (cel_time_compare(time, &ltime) < 0)
		{
			// puts("Start not matched");
			continue;
		}
		cel_time_set_time(&ltime,
						  time_range->end_hour,
						  time_range->end_min,
						  time_range->end_sec);
		// if time > ltime rturn false
		if (cel_time_compare(time, &ltime) > 0)
		{
			// puts("End not matched");
			continue;
		}
		return TRUE;
	}
	return FALSE;
}
