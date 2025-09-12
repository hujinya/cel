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


// * ? 00:00:00-23:59:59  按每天
// ? Working-Day 00:00:00-23:59:59 按星期
// 1,2,3 ? 00:00:00-23:59:59  按具体天
int cel_timepolicy_init(CelTimePolicy *time_policy, const TCHAR *policy_str)
{
	int i = 0, j;
	int mday;
	char buf[20];
	CelTimeRange *time_range;

	time_policy->is_everyday = FALSE;
	time_policy->is_weekly = FALSE;
	memset(time_policy->mdays, 0, sizeof(time_policy->mdays));
	memset(time_policy->wdays, 0, sizeof(time_policy->wdays));
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
					if (buf[1] == _T('U')) //SUN
						time_policy->wdays[0] = TRUE;
					else if (buf[1] == _T('A')) //SAT
						time_policy->wdays[6] = TRUE;
				}
				else if (buf[0] == _T('M')) //MON
				{
					time_policy->wdays[1] = TRUE;
				}
				else if (buf[0] == _T('T'))
				{
					if (buf[1] == _T('U')) //TUE
						time_policy->wdays[2] = TRUE;
					else if (buf[1] == _T('H')) //THU
						time_policy->wdays[4] = TRUE;
				}
				else if (buf[0] == _T('W')) //WED
				{
					if (buf[1] == _T('o')) //Working-Day
					{
						time_policy->wdays[1] = TRUE;
						time_policy->wdays[2] = TRUE;
						time_policy->wdays[3] = TRUE;
						time_policy->wdays[4] = TRUE;
						time_policy->wdays[5] = TRUE;
					}
					else if (buf[1] == _T('E'))
						time_policy->wdays[3] = TRUE;
				}
				else if (buf[0] == _T('F')) //FRI
				{
					time_policy->wdays[5] = TRUE;
				}
				else if (buf[0] == _T('O')) //Off-Day
				{
					time_policy->wdays[6] = TRUE;
					time_policy->wdays[0] = TRUE;
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
			if (j > 11) {
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
				time_policy->mdays[mday - 1] = TRUE;
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
			if (j > 2) {
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
		if (j > 17) {
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
	memset(time_policy->mdays, 0, sizeof(time_policy->mdays));
	memset(time_policy->wdays, 0, sizeof(time_policy->wdays));
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
		if (time_policy->is_weekly) {
			if (!time_policy->wdays[wday]) {
				//puts("WDay not matched");
				return FALSE;
			}
		}
		else 
		{
			if (!time_policy->mdays[mday - 1]) {
				//puts("MDay not matched");
				return FALSE;
			}
		}
	}
	n = cel_arraylist_get_size(&(time_policy->time_ranges));
	if ( n == 0)
		return TRUE;
	for (i = 0; i < n; n++)
	{
		time_range = (CelTimeRange *)cel_arraylist_get_by_index(
			&(time_policy->time_ranges), i);
		memcpy(&ltime, time, sizeof(CelTime));
		cel_time_set_time(&ltime, 
			time_range->start_hour,
			time_range->star_min,
			time_range->start_sec);
		//if time < ltime rturn false
		if (cel_time_compare(time, &ltime) < 0 ) {
			//puts("Start not matched");
			return FALSE;
		}
		cel_time_set_time(&ltime, 
			time_range->end_hour,
			time_range->end_min,
			time_range->end_sec);
		//if time > ltime rturn false
		if (cel_time_compare(time, &ltime) > 0) {
			//puts("End not matched");
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}
