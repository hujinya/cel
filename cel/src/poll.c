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
#include "cel/poll.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

CelPoll *cel_poll_new(int max_threads, int max_fileds)
{
    CelPoll *poll;

    if ((poll = (CelPoll *)cel_malloc(sizeof(CelPoll))) != NULL)
    {
        if (cel_poll_init(poll, max_threads, max_fileds) == 0)
            return poll;
        cel_free(poll);
    }
    return NULL;
}

void cel_poll_free(CelPoll *poll)
{
    cel_poll_destroy(poll);
    cel_free(poll);
}

