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
#ifndef __CEL_TYPES_UNIX_H__
#define __CEL_TYPES_UNIX_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "cel/tchars.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef char CHAR;
typedef wchar_t WCHAR;
typedef unsigned char BYTE;
typedef int HANDLE;

/* Carriage Return/Line Feed */
#ifdef  CEL_CRLF
#undef  CEL_CRLF
#endif
#define CEL_CRLF_A "\n"
#define CEL_CRLF_W L"\n"
#ifdef _UNICODE
#define CEL_CRLF CEL_CRLF_W
#else
#define CEL_CRLF CEL_CRLF_A
#endif
#define CEL_CRLFLEN 1
/* Path Separator */
#ifdef  CEL_PS
#undef  CEL_PS
#endif
#define CEL_PS_A   '/'
#define CEL_PS_W  L'/'
#ifdef _UNICODE
#define CEL_PS CEL_PS_W
#else
#define CEL_PS CEL_PS_A
#endif

#include <stdint.h>
typedef int8_t              S8;
typedef uint8_t             U8;
typedef int16_t            S16;
typedef uint16_t           U16;
typedef int32_t            S32;
typedef uint32_t           U32;
typedef int64_t            S64;
typedef uint64_t           U64;
typedef float              F32;
typedef double             D64;

#ifdef __cplusplus
}
#endif

#endif
