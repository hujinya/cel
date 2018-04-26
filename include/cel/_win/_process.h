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

typedef DWORD CelProcessId;
typedef HANDLE CelProcess;

#define cel_process_getid() GetCurrentProcessId()
#define cel_process_setaffinity(tidp, affinity_mask) \
    (SetProcessAffinityMask(tidp, *(affinity_mask)) == 0 ? -1 : 0)

#ifdef __cplusplus
}
#endif

#endif
