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
#include "cel/timer.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

int cel_timer_init(CelTimer *timer, long milliseconds, int repeat,
                   CelTimerCallbackFunc call_back, void *user_data)
{
    timer->evt_type = CEL_EVENT_TIMER;
    timer->repeat = repeat;
    timer->timer_id = timer;
    timer->interval = milliseconds;
    timer->elapsed = 0;
    timer->call_back = call_back;
    timer->user_data = user_data;
    cel_timer_start(timer, NULL);

    return 0;
}

void cel_timer_destroy(CelTimer *timer)
{
    cel_time_clear(&(timer->expired));
    timer->repeat = 0;
    timer->timer_id = 0;
    timer->interval = 0;
    timer->elapsed = 0;
    timer->call_back = NULL;
    timer->user_data = NULL;
}

CelTimer *cel_timer_new(long microseconds, int repeat,
                        CelTimerCallbackFunc call_back, void *user_data)
{
    CelTimer *timer;

    if ((timer = (CelTimer *)cel_malloc(sizeof(CelTimer))) != NULL)
    {
        if (cel_timer_init(
            timer, microseconds, repeat, call_back, user_data) == 0)
            return timer;
        cel_free(timer);
    }

    return NULL;
}

void cel_timer_free(CelTimer *timer)
{
    //_tprintf(_T("Timer %p free.\r\n"), timer);
    cel_timer_destroy(timer); 
    cel_free(timer);
}
