/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_CONVERT_UNIX_H__
#define __CEL_CONVERT_UNIX_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

char *itoa(int num, char *str, int radix);
unsigned _rotl(unsigned val,int shift);
unsigned _rotr (unsigned val,int shift);
int snwprintf(WCHAR *str, size_t size, const WCHAR *format, ...);

int os_mb2unicode(const CHAR *mbcs, int mbcs_count, WCHAR *wcs, int wcs_count);
int os_unicode2mb(const WCHAR *wcs, int wcs_count, CHAR *mbcs, int mbcs_count);
int os_utf82unicode(const CHAR *utf8, int utf8_count, 
                    WCHAR *wcs, int wcs_count);

#ifdef __cplusplus
}
#endif

#endif