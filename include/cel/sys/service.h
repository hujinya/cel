/**
 * CEL(C Extension Library)
 * Copyright (C)2011 - 2012 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_SYS_SERVICE_H__
#define __CEL_SYS_SERVICE_H__

#include "cel/config.h"
#if defined(_CEL_UNIX)
#include "cel/_unix/_service.h"
#elif defined(_CEL_WIN)
#include "cel/_win/_service.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef OsServiceEntry CelServiceEntry;

/* 
CelServiceEntry *cel_service_entry_create(TCHAR *name, CelMainFunc on_start, 
                                          CelVoidFunc on_stop); 
*/
#define cel_service_entry_create os_service_entry_create
/* int cel_service_entry_dispatch(CelServiceEntry *sc_entry); */
#define cel_service_entry_dispatch os_service_entry_dispatch

/* BOOL cel_service_is_running(TCHAR *name); */
#define cel_service_is_running os_service_is_running
#define cel_service_stop os_service_stop
#define cel_service_reload os_service_reload

#ifdef __cplusplus
}
#endif

#endif
