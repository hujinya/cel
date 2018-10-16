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
#ifndef __CEL_VERSION_H__
#define __CEL_VERSION_H__

#include "cel/types.h"
#ifdef _CEL_WIN
#pragma comment(lib, "version.lib")
#pragma comment(lib, "user32.lib")
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define CEL_LIB_MAJOR 1
#define CEL_LIB_MINOR 0
#define CEL_LIB_REVISION 9
#define CEL_LIB_BUILD 19
#define CEL_LIB_EXTRA ""

#define CEL_UTS _T(__DATE__) _T(" ") _T(__TIME__)

typedef struct _CelVersion
{
    unsigned short major;
    unsigned short minor;
    unsigned short revision;
    unsigned short build;
    TCHAR *extra;
}CelVersion;

extern CelVersion cel_lib_ver;

#ifdef _CEL_UNIX
#define cel_version_init(ver, file) \
    (ver)->major = MAJOR, \
    (ver)->minor = MINOR, \
    (ver)->revision = REVISION, \
    (ver)->build = BUILD,\
    (ver)->extra = (TCHAR *)EXTRA
#endif
#ifdef _CEL_WIN
int cel_version_init(CelVersion *ver, const TCHAR *file);
#endif
TCHAR *_cel_version_release(CelVersion *ver, const TCHAR *uts);
#define cel_version_release(ver) _cel_version_release(ver, CEL_UTS)
#define cel_lib_realse() _cel_version_release(&cel_lib_ver, NULL)

#ifdef __cplusplus
}   /*  ... extern "C" */
#endif

#endif
