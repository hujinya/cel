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
#ifndef __CEL_SYS_SERVICE_WIN_H__
#define __CEL_SYS_SERVICE_WIN_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _OsServiceEntry
{
    TCHAR service_name[CEL_SNLEN];
    SERVICE_TABLE_ENTRY scEntry[2];
    SERVICE_STATUS_HANDLE scStautsHandle;
    SERVICE_STATUS scStatus;
    CelMainFunc on_start;
    CelVoidFunc on_stop;
}OsServiceEntry;

//BOOL os_service_get_starttype(TCHAR *name);
//BOOL os_service_set_starttype(TCHAR *name, DWORD dwStartType);
//
//BOOL os_service_start(TCHAR *name,
//                      DWORD dwNumServiceArgs = 0,
//                      TCHAR *lpszServiceArgVectors = NULL);
BOOL os_service_is_running(const TCHAR *name);
BOOL os_service_stop(const TCHAR *name);
BOOL os_service_reload(const TCHAR *name);

OsServiceEntry *os_service_entry_create(const TCHAR *name, 
                                        CelMainFunc on_start, 
                                        CelVoidFunc on_stop);

#define os_service_entry_dispatch(sc_entry, is_foreground) \
    (is_foreground \
    ? sc_entry->on_start(argc, argv) \
    : StartServiceCtrlDispatcher(sc_entry->scEntry) ? 0 : -1)

#ifdef __cplusplus
}
#endif

#endif
