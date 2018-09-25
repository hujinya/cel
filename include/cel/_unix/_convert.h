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
#ifndef __CEL_CONVERT_UNIX_H__
#define __CEL_CONVERT_UNIX_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define stricmp strcasecmp

char *itoa(int num, char *str, int radix);
#include <xmmintrin.h>
//unsigned _rotl(unsigned val,int shift);
//unsigned _rotr (unsigned val,int shift);
int snwprintf(WCHAR *str, size_t size, const WCHAR *format, ...);

int os_mb2unicode(const CHAR *mbcs, int mbcs_count, WCHAR *wcs, int wcs_count);
int os_unicode2mb(const WCHAR *wcs, int wcs_count, CHAR *mbcs, int mbcs_count);
int os_utf82unicode(const CHAR *utf8, int utf8_count, 
                    WCHAR *wcs, int wcs_count);

#ifdef __cplusplus
}
#endif

#endif