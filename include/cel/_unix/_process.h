/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
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
