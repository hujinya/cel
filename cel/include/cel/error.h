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
#ifndef __CEL_ERROR_H__
#define __CEL_ERROR_H__

#include "cel/config.h"
#if defined(_CEL_UNIX)
#include "cel/_unix/_error.h"
#elif defined(_CEL_WIN)
#include "cel/_win/_error.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Bit 29 is reserved for application-defined error codes;
 * no system error code has this bit set.
 */
#define CEL_ERR_NO                 0x00000000
#define CEL_ERR_FLAG               (0x1 << 28)
#define CEL_ERR_OPENSSL            ((0x1 << 24) | CEL_ERR_FLAG)
#define CEL_ERR_MYSQL              ((0x2 << 24) | CEL_ERR_FLAG)
#define CEL_ERR_USER               ((0xF << 24) | CEL_ERR_FLAG)

#ifndef CEL_ERRSLEN
#define CEL_ERRSLEN                512
#endif

/* Double error string buffer */
typedef struct _CelErrBuffer
{
    unsigned int i:1;
    int err_no;
    CHAR a_buffer[2][CEL_ERRSLEN];
    WCHAR w_buffer[2][CEL_ERRSLEN];
}CelErrBuffer;

#define cel_sys_seterrno os_seterrno
#define cel_sys_geterrno os_geterrno
#define cel_sys_setwsaerrno os_setwsaerrno
#define cel_sys_getwsaerrno os_getwsaerrno
#define cel_sys_strerror_a os_strerror_a
#define cel_sys_strerror_w os_strerror_w

CelErrBuffer *_cel_err_buffer();
static __inline void cel_seterrno(int err_no)
{
    _cel_err_buffer()->err_no = err_no;
}
static __inline int cel_geterrno(void)
{
    return _cel_err_buffer()->err_no;
}
CHAR *cel_geterrstr_a(int err_no);
WCHAR *cel_geterrstr_w(int err_no);

CHAR *cel_seterrstr_a(const CHAR *fmt, ...);
WCHAR *cel_seterrstr_w(const WCHAR *fmt, ...);
CHAR *cel_seterrstr_ex_a(const CHAR *file, int line, 
                         const CHAR *func, CHAR *err_str);
WCHAR *cel_seterrstr_ex_w(const WCHAR *file, int line, 
                          const WCHAR *func, WCHAR *err_str);

CHAR *cel_seterr_a(int err_no, const CHAR *fmt, ...);
WCHAR *cel_seterr_w(int err_no, const WCHAR *fmt, ...);
CHAR *cel_seterr_ex_a(int err_no, const CHAR *file, int line, 
                      const CHAR *func, CHAR *err_str);
WCHAR *cel_seterr_ex_w(int err_no, const WCHAR *file, int line, 
                       const WCHAR *func, WCHAR *err_str);

#define CEL_SETERRNO(errno) cel_seterrno(errno)
#define CEL_SETERRSTR(args) cel_seterrstr args
/* 
cel_seterrstr_ex(_T(__FILE__), __LINE__, _T(__FUNCTION__), cel_seterrstr args)
*/
#define CEL_SETERR(args) cel_seterr args
/* 
cel_seterrstr_ex(_T(__FILE__), __LINE__, _T(__FUNCTION__), cel_seterr args) 
*/

#ifdef _UNICODE
#define cel_geterrstr cel_geterrstr_w
#define cel_seterrstr cel_seterrstr_w
#define cel_seterrstr_ex cel_seterrstr_ex_w
#define cel_seterr cel_seterr_w
#define cel_seterr_ex cel_seterr_ex_w
#else
#define cel_geterrstr cel_geterrstr_a
#define cel_seterrstr cel_seterrstr_a
#define cel_seterrstr_ex cel_seterrstr_ex_a
#define cel_seterr cel_seterr_a
#define cel_seterr_ex cel_seterr_ex_a
#endif

#ifdef __cplusplus
}
#endif

#endif
