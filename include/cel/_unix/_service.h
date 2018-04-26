/**
 * CEL(C Extension Library)
 * Copyright (C)2011 - 2012 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __OS_SYS_SERVICE_UNIX_H__
#define __OS_SYS_SERVICE_UNIX_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _OsServiceEntry
{
    CelMainFunc on_start;
    CelVoidFunc on_stop;
}OsServiceEntry;

OsServiceEntry *os_service_entry_create(TCHAR *name, CelMainFunc on_start, 
                                          CelVoidFunc on_stop);

int _os_service_entry_dispatch(OsServiceEntry *sc_entry, 
                               int argc, char **argv);
#define os_service_entry_dispatch(sc_entry) \
    _os_service_entry_dispatch(sc_entry, argc, argv)

BOOL os_service_is_running(TCHAR *name);
BOOL os_service_stop(TCHAR *name);
BOOL os_service_reload(TCHAR *name);

#ifdef __cplusplus
}
#endif

#endif
