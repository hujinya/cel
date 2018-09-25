/**
 * CEL(C Extension Library)
 * Copyright (C)2011 - 2012 Hu Jinya(hu_jinya@163.com) 
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
CelServiceEntry *cel_service_entry_create(const TCHAR *name, CelMainFunc on_start, 
                                          CelVoidFunc on_stop); 
*/
#define cel_service_entry_create os_service_entry_create
/* int cel_service_entry_dispatch(CelServiceEntry *sc_entry); */
#define cel_service_entry_dispatch os_service_entry_dispatch

/* BOOL cel_service_is_running(const TCHAR *name); */
#define cel_service_is_running os_service_is_running
#define cel_service_stop os_service_stop
#define cel_service_reload os_service_reload

#ifdef __cplusplus
}
#endif

#endif
