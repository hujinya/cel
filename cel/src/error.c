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
#include "cel/error.h"
#include "cel/allocator.h"
#include "cel/multithread.h"
#include "cel/convert.h"
#include "cel/net/ssl.h"
#include <stdarg.h>

void _cel_err_free(CelErr *err)
{
	cel_list_destroy(&(err->stack));
	_cel_sys_free(err);
}

CelErr *_cel_err()
{
    CelErr *err;
    
    if ((err = (CelErr *)
        cel_multithread_get_keyvalue(CEL_MT_KEY_ERROR)) == NULL)
    {
        if ((err = (CelErr *)_cel_sys_malloc(sizeof(CelErr))) != NULL)
        {
            //_tprintf(_T("new %p\r\n"), err);
			err->stack_on = TRUE;
			err->stack_max_size = 32;
			cel_list_init(&(err->stack), cel_free);

			err->stic.a_buffer[0] = '\0';
			err->stic.w_buffer[0] = L'\0';
			err->current = NULL;
            if (cel_multithread_set_keyvalue(CEL_MT_KEY_ERROR, err) != -1
                && cel_multithread_set_keydestructor(
                CEL_MT_KEY_ERROR, (CelDestroyFunc)_cel_err_free) != -1)
                return err;
            _cel_sys_free(err);
        }
        return NULL;
    }
    return err;
}

CelErrItem *_cel_err_item_current(CelErr *err)
{
	if (err->stack_on)
	{
		if (err->current != NULL)
		{
			cel_list_push_back(&(err->stack), (CelListItem *)(err->current));
			err->current = NULL;
		}
	}
	if (err->current == NULL)
	{
		if (cel_list_get_size(&(err->stack)) < err->stack_max_size)
			err->current = (CelErrItem *)cel_malloc(sizeof(CelErrItem));
		else
			err->current = (CelErrItem *)cel_list_pop_front(&(err->stack));
		err->current->buf.a_buffer[0] = '\0';
		err->current->buf.w_buffer[0] = L'\0';
	}
	return _cel_err()->current;
}

void cel_clearerr()
{
	CelErr *err = _cel_err(); 

	if (err->current != NULL)
	{
		cel_free(err->current);
		err->current = NULL;
	}
	if (cel_list_get_size(&(err->stack)) > 0)
		cel_list_clear(&(err->stack));
}

CHAR *cel_geterrstr_a(int err_no)
{
	CelErr *err;

	if ((err_no & CEL_ERR_FLAG) == CEL_ERR_FLAG)
	{
		err = _cel_err(); 
		if (err->current == NULL)
			return NULL;
#ifdef _UNICODE
		cel_unicode2mb(err->current->buf.w_buffer, -1, 
			err->stic.a_buffer, CEL_ERRSLEN);
		return err->stic.a_buffer;
#else
		return err->current->buf.a_buffer;
#endif
	}
	else
	{
		return cel_sys_strerror_a(err_no);
	}
}

WCHAR *cel_geterrstr_w(int err_no)
{
	CelErr *err;

	if ((err_no & CEL_ERR_FLAG) == CEL_ERR_FLAG)
	{
		err = _cel_err(); 
		if (err->current == NULL)
			return NULL;
#ifdef _UNICODE
		return err->current->buf.w_buffer;
#else
		cel_mb2unicode(err->current->buf.a_buffer, -1, 
			err->stic.w_buffer, CEL_ERRSLEN);
		return err->stic.w_buffer;
#endif
	}
	else
	{
		return cel_sys_strerror_w(err_no);
	}
}

void cel_seterr_a(int err_no, const CHAR *fmt, ...)
{
    CelErr *err = _cel_err();
	CelErrItem *err_item = _cel_err_item_current(err);
    va_list ap;

    //_tprintf(_T("%d %p\r\n"), ptr->i, ptr);
#ifdef _UNICODE
	va_start(ap, fmt);
	vsnprintf(err->stic.a_buffer, CEL_ERRSLEN, fmt, ap);
	va_end(ap);
	cel_mb2unicode(err->stic.a_buffer, -1, err_item->buf.w_buffer, CEL_ERRSLEN);
#else
	va_start(ap, fmt);
	vsnprintf(err_item->buf.a_buffer, CEL_ERRSLEN, fmt, ap);
	va_end(ap);
#endif
	err_item->err_no = err_no;
}

void cel_seterr_w(int err_no, const WCHAR *fmt, ...)
{
    CelErr *err = _cel_err();
	CelErrItem *err_item = _cel_err_item_current(err);
    va_list ap;

    //_tprintf(_T("%d %p\r\n"), ptr->i, ptr);
#ifdef _UNICODE
	va_start(ap, fmt);
	vswprintf(err_item->buf.w_buffer, CEL_ERRSLEN, fmt, ap);
	va_end(ap);
#else
	va_start(ap, fmt);
	vswprintf(err->stic.w_buffer, CEL_ERRSLEN, fmt, ap);
	va_end(ap);
	cel_unicode2mb(err->stic.w_buffer, -1, err_item->buf.a_buffer, CEL_ERRSLEN);
#endif
	err_item->err_no = err_no;
}

void cel_seterr_ex_a(int err_no, const CHAR *file, int line, 
					 const CHAR *func, CHAR *err_str)
{
    CelErr *err = _cel_err();
	CelErrItem *err_item = _cel_err_item_current(err);
#ifdef _UNICODE
    snprintf(err->stic.a_buffer, CEL_ERRSLEN , "%s(%d)-%s()-%s", 
        file, line, func, err_str);
    cel_mb2unicode(err->stic.a_buffer, -1, err_item->buf.w_buffer, CEL_ERRSLEN);
#else
	snprintf(err_item->buf.a_buffer, CEL_ERRSLEN , "%s(%d)-%s()-%s", 
        file, line, func, err_str);
#endif
    err_item->err_no = err_no;
}

void cel_seterr_ex_w(int err_no, const WCHAR *file, int line, 
					 const WCHAR *func, WCHAR *err_str)
{
	CelErr *err = _cel_err();
	CelErrItem *err_item = _cel_err_item_current(err);

#ifndef _UNICODE
	snwprintf(err_item->buf.w_buffer, CEL_ERRSLEN , L"%s(%d)-%s()-%s", 
		file, line, func, err_str);
#else
	snwprintf(err->stic.w_buffer, CEL_ERRSLEN , L"%s(%d)-%s()-%s", 
		file, line, func, err_str);
	cel_unicode2mb(err->stic.w_buffer, -1, err_item->buf.a_buffer, CEL_ERRSLEN);
#endif
	err_item->err_no = err_no;
}
