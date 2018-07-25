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
#ifndef __CEL_CONVERT_WIN_H__
#define __CEL_CONVERT_WIN_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (_MSC_VER <= 1800)
long long os_atoll(const char *p);
#define atoll os_atoll
#endif
TCHAR *strsep(TCHAR **stringp, const TCHAR *delim);

/* 
 * int cel_mb2unicode(const CHAR *mbcs, int mbcs_count, 
 *                    WCHAR *wcs, int wcs_count);
 */
#define os_mb2unicode(mbcs, mbcs_count, wcs, wcs_count) \
    MultiByteToWideChar(CP_ACP, 0, mbcs, mbcs_count, wcs, wcs_count)
int os_mb2utf8(const CHAR *mbcs, int mbcs_count, CHAR *utf8, int utf_8count);
int os_utf82mb(const CHAR *utf8, int utf8_count, CHAR *mbcs, int mbcs_count);
/* 
 * int cel_utf82unicode(const CHAR *utf8, int utf8_count,
 *                      WCHAR *wcs, int wcs_count); 
 */
#define os_utf82unicode(utf8, utf8_count, wcs, wcs_count) \
    MultiByteToWideChar(CP_UTF8, 0, utf8, utf8_count, wcs, wcs_count)
/* 
 * int cel_unicode2mb(const WCHAR *wcs, int wcs_count, 
 *                    CHAR *mbcs, int mbcs_count); 
 */
#define os_unicode2mb(wcs, wcs_count, mbcs, mbcs_count) \
    WideCharToMultiByte(CP_OEMCP, 0, wcs, wcs_count, \
    mbcs, mbcs_count, NULL, FALSE)
/* 
 * int cel_mb2unicode(const TCHAR *wcs, int wcs_count, 
 *                    CHAR *utf8, int utf8_count); 
 */
#define os_unicode2utf8(wcs, wcs_count, utf8, utf8_count) \
    WideCharToMultiByte(CP_UTF8, 0, wcs, wcs_count, \
    utf8, utf8_count, NULL, FALSE)

#ifdef __cplusplus
}
#endif

#endif
