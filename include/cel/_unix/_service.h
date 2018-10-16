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
#ifndef __CEL_SYS_SERVICE_UNIX_H__
#define __CEL_SYS_SERVICE_UNIX_H__

#include "cel/types.h"

#define SERVICE_BOOT_START             0x00000000
#define SERVICE_SYSTEM_START           0x00000001
#define SERVICE_AUTO_START             0x00000002
#define SERVICE_DEMAND_START           0x00000003
#define SERVICE_DISABLED               0x00000004

#define SERVICE_STOPPED                0x00000001
#define SERVICE_START_PENDING          0x00000002
#define SERVICE_STOP_PENDING           0x00000003
#define SERVICE_RUNNING                0x00000004
#define SERVICE_CONTINUE_PENDING       0x00000005
#define SERVICE_PAUSE_PENDING          0x00000006
#define SERVICE_PAUSED                 0x00000007

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _OsServiceEntry
{
    CelMainFunc on_start;
    CelVoidFunc on_stop;
}OsServiceEntry;

OsServiceEntry *os_service_entry_create(const TCHAR *name,
                                        CelMainFunc on_start, 
                                        CelVoidFunc on_stop);

int _os_service_entry_dispatch(OsServiceEntry *sc_entry,
                               BOOL is_foreground,
                               int argc, char **argv);
#define os_service_entry_dispatch(sc_entry, is_foreground) \
    _os_service_entry_dispatch(sc_entry, is_foreground, argc, argv)

BOOL os_service_is_running(const TCHAR *name);
BOOL os_service_stop(const TCHAR *name);
BOOL os_service_reload(const TCHAR *name);

#ifdef __cplusplus
}
#endif

#endif
