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
#ifndef __CEL_PROCESS_UNIX_H__
#define __CEL_PROCESS_UNIX_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef pid_t CelProcessId;
typedef unsigned long CelProcess;

#define cel_process_getid() getpid()
#define cel_process_setaffinity(tidp, affinity_mask) \
    sched_setaffinity(tidp, sizeof(CelCpuMask), affinity_mask)

#ifdef __cplusplus
}
#endif

#endif
