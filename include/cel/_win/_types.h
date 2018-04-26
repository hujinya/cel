/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_TYPES_WIN_H__
#define __CEL_TYPES_WIN_H__

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
//#ifndef WIN32_LEAN_AND_MEAN
//#define WIN32_LEAN_AND_MEAN
//#endif  /* WIN32_LEAN_AND_MEAN */
#include <WinSock2.h>
#include <windows.h>
#include "cel/tchars.h"
#include <lm.h>

#define snprintf  _snprintf
#define snwprintf _snwprintf
#define strcasecmp stricmp
#define strncasecmp  strnicmp

#define usleep(x) Sleep(x / 1000)
#define sleep(x)  Sleep(x * 1000)

/* Carriage Return/Line Feed */
#ifdef  CEL_CRLF
#undef  CEL_CRLF
#endif
#define CEL_CRLF_A "\r\n"
#define CEL_CRLF_W L"\r\n"
#ifdef _UNICODE
#define CEL_CRLF CEL_CRLF_W
#else
#define CEL_CRLF CEL_CRLF_A
#endif
#define CEL_CRLFLEN 2
/* Path Separator */
#ifdef  CEL_PS
#undef  CEL_PS
#endif
#define CEL_PS_A   '\\'
#define CEL_PS_W  L'\\'
#ifdef _UNICODE
#define CEL_PS CEL_PS_W
#else
#define CEL_PS CEL_PS_A
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef __int8              S8;
typedef unsigned __int8     U8;
typedef __int16            S16;
typedef unsigned __int16   U16;
typedef __int32            S32;
typedef unsigned __int32   U32;
typedef __int64            S64;
typedef unsigned __int64   U64;
typedef float              F32;
typedef double             D64;

#ifdef __cplusplus
}
#endif

#endif