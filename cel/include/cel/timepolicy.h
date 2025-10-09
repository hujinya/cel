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
#ifndef __CEL_TIME_POLICY_H__
#define __CEL_TIME_POLICY_H__

#include "cel/types.h"
#include "cel/time.h"
#include "cel/arraylist.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelTimeRange
{
	int start_hour,star_min,start_sec;
	int end_hour, end_min, end_sec;
}CelTimeRange;

typedef struct _CelTimePolicy
{
	BOOL is_everyday;
	BOOL is_weekly;
	unsigned int mdays;  /* 0 - 30 */
	unsigned int wdays;  /* 0- 6 */
	CelArrayList time_ranges;
}CelTimePolicy;

int cel_timepolicy_init(CelTimePolicy *time_policy, const TCHAR *policy_str);
void cel_timepolicy_destroy(CelTimePolicy *time_policy);

CelTimePolicy *cel_timepolicy_new(const char *policy_str);
void cel_timepolicy_free(CelTimePolicy *time_policy);

BOOL cel_timepolicy_is_allow(CelTimePolicy *time_policy, CelTime *time);

#ifdef __cplusplus
}
#endif

#endif
