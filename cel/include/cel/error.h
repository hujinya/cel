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
#include "cel/list.h"

/*
 * Bit 29 is reserved for application-defined error codes;
 * no system error code has this bit set.
 */
#define CEL_ERR_NO                 0x00000000
#define CEL_ERR_FLAG               (0x1 << 28)
#define CEL_ERR_LIB                ((0x1 << 24) | CEL_ERR_FLAG)
#define CEL_ERR_OPENSSL            ((0x2 << 24) | CEL_ERR_FLAG)
#define CEL_ERR_MYSQL              ((0x3 << 24) | CEL_ERR_FLAG)

#define CEL_ERR_USER               ((0xF << 24) | CEL_ERR_FLAG)

#ifndef CEL_ERRSLEN
#define CEL_ERRSLEN                512
#endif

typedef union _CelErrBuffer
{
	CHAR a_buffer[CEL_ERRSLEN];
	WCHAR w_buffer[CEL_ERRSLEN];
}CelErrBuffer;

typedef struct _CelErrItem
{
	int err_no;
	CelErrBuffer buf;
}CelErrItem;

typedef struct _CelErr
{
	BOOL stack_on;
	size_t stack_max_size;
	CelList stack;
	CelErrItem *current;
	CelErrBuffer stic;
}CelErr;

#define cel_sys_seterrno os_seterrno
#define cel_sys_geterrno os_geterrno
#define cel_sys_setwsaerrno os_setwsaerrno
#define cel_sys_getwsaerrno os_getwsaerrno
#define cel_sys_strerror_a os_strerror_a
#define cel_sys_strerror_w os_strerror_w

#ifdef __cplusplus
extern "C" {
#endif

CelErr *_cel_err();
CelErrItem *_cel_err_buffer_current(BOOL is_set);
void cel_clearerr();

static __inline int cel_geterrno(void)
{
    CelErr *err = _cel_err();
	return (err->current == NULL ? 0 : err->current->err_no);
}
CHAR *cel_geterrstr_a(int err_no);
WCHAR *cel_geterrstr_w(int err_no);


void cel_seterr_a(int err_no, const CHAR *fmt, ...);
void cel_seterr_w(int err_no, const WCHAR *fmt, ...);
void cel_seterr_ex_a(int err_no, const CHAR *file, int line, 
					 const CHAR *func, CHAR *err_str);
void cel_seterr_ex_w(int err_no, const WCHAR *file, int line, 
					 const WCHAR *func, WCHAR *err_str);

#define CEL_SETERR(args) cel_seterr args
/* 
cel_seterrstr_ex(_T(__FILE__), __LINE__, _T(__FUNCTION__), cel_seterr args) 
*/

#ifdef _UNICODE
#define cel_geterrstr cel_geterrstr_w
#define cel_seterr cel_seterr_w
#define cel_seterr_ex cel_seterr_ex_w
#else
#define cel_geterrstr cel_geterrstr_a
#define cel_seterr cel_seterr_a
#define cel_seterr_ex cel_seterr_ex_a
#endif

#ifdef __cplusplus
}
#endif

#endif
