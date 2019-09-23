/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2019 Hu Jinya(hu_jinya@163.com) 
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

typedef struct _CelErrBuffer
{
	int err_no;
	union {
		CHAR a_buffer[CEL_ERRSLEN];
		WCHAR w_buffer[CEL_ERRSLEN];
	};
}CelErrBuffer;

typedef struct _CelErrItem
{
	CelListItem item;
	CelErrBuffer buf;
}CelErrItem;

typedef struct _CelErr
{
	CelErrItem *items;
	BOOL stack_on;
	size_t stack_used, stack_max;
	CelList stack;
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
void cel_clearerr();

int cel_geterrno(void);
CHAR *cel_geterrstr_a(void);
WCHAR *cel_geterrstr_w(void);

void cel_seterr_a(int err_no, const CHAR *fmt, ...);
void cel_seterr_w(int err_no, const WCHAR *fmt, ...);
void cel_seterr_ex_a(int err_no, const CHAR *file, int line, 
					 const CHAR *func, CHAR *err_str);
void cel_seterr_ex_w(int err_no, const WCHAR *file, int line, 
					 const WCHAR *func, WCHAR *err_str);

#ifdef _UNICODE
#define cel_sys_strerror cel_sys_strerror_w
#define cel_geterrstr cel_geterrstr_w
#define cel_seterr cel_seterr_w
#define cel_seterr_ex cel_seterr_ex_w
#else
#define cel_sys_strerror cel_sys_strerror_a
#define cel_geterrstr cel_geterrstr_a
#define cel_seterr cel_seterr_a
#define cel_seterr_ex cel_seterr_ex_a
#endif

CelErrBuffer *cel_seterr_stic(int err_no, const TCHAR *fmt, ...);
void cel_seterr_stic_ex(const TCHAR *file, int line, 
						const TCHAR *func, CelErrBuffer *stic);

#define CEL_SETERR(args) cel_seterr args
/* 
cel_seterr_stic_ex(_T(__FILE__), __LINE__, _T(__FUNCTION__), cel_seterr_stic args)
*/

#ifdef __cplusplus
}
#endif

#endif
