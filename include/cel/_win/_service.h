/**
 * CEL(C Extension Library)
 * Copyright (C)2011 - 2012 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_SYS_SERVICE_WIN_H__
#define __CEL_SYS_SERVICE_WIN_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _OsServiceEntry
{
    SERVICE_TABLE_ENTRY scEntry[2];
    SERVICE_STATUS_HANDLE scStautsHandle;
    SERVICE_STATUS scStatus;
    CelMainFunc on_start;
    CelVoidFunc on_stop;
}OsServiceEntry;

BOOL os_service_is_running(TCHAR *name);
BOOL os_service_stop(TCHAR *name);
BOOL os_service_reload(TCHAR *name);

OsServiceEntry *os_service_entry_create(TCHAR *name, CelMainFunc on_start, 
                                        CelVoidFunc on_stop);

#define os_service_entry_dispatch(sc_entry) \
    (StartServiceCtrlDispatcher(sc_entry->scEntry) ? 0 : -1)

#ifdef __cplusplus
}
#endif

#endif
