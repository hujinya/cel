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
#include "cel/sys/service.h"
#ifdef _CEL_WIN

static OsServiceEntry sc_entry;

BOOL os_service_is_running(const TCHAR *name)
{
    return FALSE;
}

BOOL os_service_stop(const TCHAR *name)
{
    return FALSE;
}

BOOL os_service_reload(const TCHAR *name)
{
    return FALSE;
}

static void WINAPI ServiceControl(DWORD dwCode)
{
    switch (dwCode)
    {
    case SERVICE_CONTROL_STOP:
        sc_entry.scStatus.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(sc_entry.scStautsHandle, &sc_entry.scStatus);
        if (sc_entry.on_stop != NULL)
            sc_entry.on_stop();
        break;
    default:
        break;
    }
}

void WINAPI ServiceStart(DWORD dwArgc, LPTSTR *lpArgv)
{
    sc_entry.scStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    sc_entry.scStatus.dwCurrentState = SERVICE_START_PENDING;
    sc_entry.scStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    sc_entry.scStatus.dwServiceSpecificExitCode = 0;
    sc_entry.scStatus.dwWin32ExitCode = 0;
    sc_entry.scStatus.dwCheckPoint = 0;
    sc_entry.scStatus.dwWaitHint = 0;

    /* Register service control handler */
    if (sc_entry.scEntry[0].lpServiceName == NULL
        || (sc_entry.scStautsHandle = RegisterServiceCtrlHandler(
        sc_entry.scEntry[0].lpServiceName, ServiceControl)) == 0)
        return ;

    /* Set service status */
    sc_entry.scStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(sc_entry.scStautsHandle, &(sc_entry.scStatus));

    /* Running main loop */
    if (sc_entry.on_start != NULL)
        sc_entry.on_start(dwArgc, lpArgv);
    /* If main return, set service status stoped */
    sc_entry.scStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(sc_entry.scStautsHandle, &(sc_entry.scStatus));
}

OsServiceEntry *os_service_entry_create(const TCHAR *name, 
                                        CelMainFunc on_start, 
                                        CelVoidFunc on_stop)
{
    _tcsncpy(sc_entry.service_name, name, CEL_SNLEN);
    sc_entry.scEntry[0].lpServiceName = sc_entry.service_name;
    sc_entry.scEntry[0].lpServiceProc = ServiceStart;
    sc_entry.scEntry[1].lpServiceName = NULL;
    sc_entry.scEntry[1].lpServiceProc = NULL;
    sc_entry.on_start = on_start;
    sc_entry.on_stop = on_stop;

    return &sc_entry;
}
#endif
