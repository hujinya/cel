/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
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

#define CEL_MAJOR 1
#define CEL_MINOR 0
#define CEL_REVISION 6
#define CEL_BUILD 100
#define CEL_EXTRA _T("")
#define CEL_UTS _T(__DATE__) _T(" ") _T(__TIME__)

typedef struct _CelVersion
{
    unsigned short major;
    unsigned short minor;
    unsigned short revision;
    unsigned short build;
    TCHAR *extra;
}CelVersion;

extern CelVersion cel_ver;

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
#define cel_realse() _cel_version_release(&cel_ver, NULL)

#ifdef __cplusplus
}   /*  ... extern "C" */
#endif

#endif
