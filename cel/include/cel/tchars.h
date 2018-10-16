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
#ifndef __CEL_TCHARS_H__
#define __CEL_TCHARS_H__

#ifdef _CEL_WIN
#include <tchar.h>
#define _TCHAR_DEFINED
#endif
#ifdef _CEL_UNIX
#include <wchar.h>
#endif

#ifndef _TCHAR_DEFINED

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef _UNICODE
typedef wchar_t TCHAR;
typedef wchar_t TBYTE;
#define _T(x)  L ## x

/* Formatted i/o */
#define _tprintf        wprintf
#define _ftprintf       fwprintf
//#define _stprintf       sprintf
#define _sntprintf      swprintf
#define _vstprintf      vwprintf
//#define _vsntprintf     vwprintf
#define _vftprintf      vfwprintf
#define _tscanf         wscanf
#define _stscanf        swscanf

/* Unformatted i/o */
#define _gettc          getwc
#define _puttc          putwc
#define _getts          getws
#define _putts          putws
#define _puttchar       putwchar
#define _fgettc         fgetwc
#define _fputtc         fputwc
#define _fgetts         fgetws
#define _fputts         fputws

/* String conversion functions */
//#define _tstoi          atoi
#define _tstol          wcstol
#define _tcstoul        wcstoul
//#define _tstof          atof
//#define _tcstod         wcstod
//#define _ttoi           atoi
#define _ttol           wcstol
//#define _ttof           atof
//#define _itot           itoa
//#define _ltot           itoa

/* String functions */
#define _tcscat         wcscat
#define _tcscmp         wcscmp
#define _tcscpy         wcscpy
#define _tcsdup         wcsdup
#define _tcslen         wcslen
#define _tcsncat        wcsncat
#define _tcsnccmp       wcsncmp
#define _tcsncmp        wcsncmp
#define _tcsncpy        wcsncpy
#define _tcsrchr        wcschr
#define _tcsstr         wcsstr

/* Execute functions */
//#define _tsystem        system

/* Time functions */
#define _tcsftime       wcsftime
//#define _tasctime       wasctime

/* Stdio functions */
//#define _tfopen         wfopen

/* String functions */
//#define _tcserror       wcserror

/* Stat functions */
//#define _tstat          stat

#else
#include <string.h>

typedef char TCHAR;
typedef unsigned char TBYTE;
#define _T(x) x

/* Formatted i/o */
#define _tprintf        printf
#define _ftprintf       fprintf
#define _stprintf       sprintf
#define _sntprintf      snprintf
#define _vstprintf      vprintf
#define _vsntprintf     vsnprintf
#define _vftprintf      vprintf
#define _tscanf         scanf
#define _stscanf        sscanf

/* Unformatted i/o */
#define _gettc          getc
#define _puttc          putc
#define _getts          gets
#define _putts          puts
#define _puttchar       putchar
#define _fgettc         fgetc
#define _fputtc         fputc
#define _fgetts         fgets
#define _fputts         fputs

/* String conversion functions */
#define _tstoi          atoi
#define _tstol          atol
#define _tcstoul        atoul
#define _tstof          atof
//#define _tcstod         wcstod
#define _ttoi           atoi
#define _ttol           atol
#define _ttof           atof
#define _itot           itoa
#define _ltot           itoa

/* String functions */
#define _tcscat         strcat
#define _tcscmp         strcmp
#define _tcscpy         strcpy
#define _tcsdup         strdup
#define _tcslen         strlen
#define _tcsncat        strncat
#define _tcsnccmp       strncmp
#define _tcsncmp        strncmp
#define _tcsncpy        strncpy
#define _tcsrchr        strrchr
#define _tcsstr         strstr


/* Execute functions */
#define _tsystem        system
#define _tmain          main

/* Time functions */
#define _tcsftime       strftime
//#define _tasctime       asctime

/* Stdio functions */
#define _tfopen         fopen

/* String functions */
//#define _tcserror       strerror

/* Stat functions */
#define _tstat          stat

#endif

#ifdef __cplusplus
}
#endif
#endif
#endif
