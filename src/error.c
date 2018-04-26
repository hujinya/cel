#include "cel/error.h"
#include "cel/allocator.h"
#include "cel/multithread.h"
#include "cel/convert.h"
#include "cel/net/ssl.h"
#include <stdarg.h>

CelErrBuffer *_cel_err_buffer()
{
    CelErrBuffer *ptr;
    
    if ((ptr = (CelErrBuffer *)
        cel_multithread_get_keyvalue(CEL_MT_KEY_ERROR)) == NULL)
    {
        if ((ptr = (CelErrBuffer *)
            _cel_sys_malloc(sizeof(CelErrBuffer))) != NULL)
        {
            //_tprintf(_T("new %p\r\n"), ptr);
            ptr->i = 0;
            ptr->a_buffer[0][0] = '\0';
            ptr->a_buffer[1][0] = '\0';
            ptr->w_buffer[0][0] = L'\0';
            ptr->w_buffer[1][0] = L'\0';
            if (cel_multithread_set_keyvalue(CEL_MT_KEY_ERROR, ptr) != -1
                && cel_multithread_set_keydestructor(
                CEL_MT_KEY_ERROR, _cel_sys_free) != -1)
                return ptr;
            _cel_sys_free(ptr);
        }
        return NULL;
    }
    return ptr;
}

CHAR *cel_geterrstr_a(int err_no)
{
    CelErrBuffer *ptr;
    
    if ((err_no & CEL_ERR_FLAG) == CEL_ERR_FLAG)
    {
        ptr = _cel_err_buffer(); 
#ifdef _UNICODE
        cel_unicode2mb(ptr->w_buffer[ptr->i], -1, 
            ptr->a_buffer[ptr->i], CEL_ERRSLEN);
#endif
        return ptr->a_buffer[ptr->i];
    }
    else
        return cel_sys_strerror_a(err_no);
}

WCHAR *cel_geterrstr_w(int err_no)
{
    CelErrBuffer *ptr;

    if ((err_no & CEL_ERR_FLAG) == CEL_ERR_FLAG)
    {
        ptr = _cel_err_buffer(); 
#ifndef _UNICODE
        cel_mb2unicode(ptr->a_buffer[ptr->i], -1,
            ptr->w_buffer[ptr->i], CEL_ERRSLEN);
#endif
        return ptr->w_buffer[ptr->i];
    }
    else
        return cel_sys_strerror_w(err_no);
}

CHAR *cel_seterrstr_a(const CHAR *fmt, ...)
{
    CelErrBuffer *ptr = _cel_err_buffer();
    CHAR *err_ptr = ptr->a_buffer[(++(ptr->i))];
    va_list ap;

    //_tprintf(_T("%d %p\r\n"), ptr->i, ptr);
    va_start(ap, fmt);
    vsnprintf(err_ptr, CEL_ERRSLEN, fmt, ap);
    va_end(ap);
#ifdef _UNICODE
    cel_mb2unicode(err_ptr, -1, ptr->w_buffer[ptr->i], CEL_ERRSLEN);
#endif
    cel_seterrno(CEL_ERR_FLAG);

    return err_ptr;
}

WCHAR *cel_seterrstr_w(const WCHAR *fmt, ...)
{
    CelErrBuffer *ptr = _cel_err_buffer();
    WCHAR *err_ptr = ptr->w_buffer[(++(ptr->i))];
    va_list ap;

    //_tprintf(_T("%d %p\r\n"), ptr->i, ptr);
    va_start(ap, fmt);
    vswprintf(err_ptr, CEL_ERRSLEN, fmt, ap);
    va_end(ap);
#ifndef _UNICODE
    cel_unicode2mb(err_ptr, -1, ptr->a_buffer[ptr->i], CEL_ERRSLEN);
#endif
    cel_seterrno(CEL_ERR_FLAG);

    return err_ptr;
}

CHAR *cel_seterrstr_ex_a(const CHAR *file, int line, 
                         const CHAR *func, CHAR *err_str)
{
    CelErrBuffer *ptr = _cel_err_buffer();
    CHAR *err_ptr = ptr->a_buffer[(++(ptr->i))];

    snprintf(err_ptr, CEL_ERRSLEN , "%s(%d)-%s():(%s)", 
        file, line, func, err_str);
#ifdef _UNICODE
    cel_mb2unicode(err_ptr, -1, ptr->w_buffer[ptr->i], CEL_ERRSLEN);
#endif

    return err_ptr;
}

WCHAR *cel_seterrstr_ex_w(const WCHAR *file, int line, 
                          const WCHAR *func, WCHAR *err_str)
{
    CelErrBuffer *ptr = _cel_err_buffer();
    WCHAR *err_ptr = ptr->w_buffer[(++(ptr->i))];

    snwprintf(err_ptr, CEL_ERRSLEN , L"%s(%d)-%s():(%s)", 
        file, line, func, err_str);
#ifndef _UNICODE
    cel_unicode2mb(err_ptr, -1, ptr->a_buffer[ptr->i], CEL_ERRSLEN);
#endif

    return err_ptr;
}

CHAR *cel_seterr_a(int err_no, const CHAR *fmt, ...)
{
    CelErrBuffer *ptr = _cel_err_buffer();
    CHAR *err_ptr = ptr->a_buffer[(++(ptr->i))];
    va_list ap;

    //_tprintf(_T("%d %p\r\n"), ptr->i, ptr);
    va_start(ap, fmt);
    vsnprintf(err_ptr, CEL_ERRSLEN, fmt, ap);
    va_end(ap);
#ifdef _UNICODE
    cel_mb2unicode(err_ptr, -1, ptr->w_buffer[ptr->i], CEL_ERRSLEN);
#endif
    cel_seterrno(err_no);

    return err_ptr;
}

WCHAR *cel_seterr_w(int err_no, const WCHAR *fmt, ...)
{
    CelErrBuffer *ptr = _cel_err_buffer();
    WCHAR *err_ptr = ptr->w_buffer[(++(ptr->i))];
    va_list ap;

    //_tprintf(_T("%d %p\r\n"), ptr->i, ptr);
    va_start(ap, fmt);
    vswprintf(err_ptr, CEL_ERRSLEN, fmt, ap);
    va_end(ap);
#ifndef _UNICODE
    cel_unicode2mb(err_ptr, -1, ptr->a_buffer[ptr->i], CEL_ERRSLEN);
#endif
    cel_seterrno(err_no);

    return err_ptr;
}

CHAR *cel_seterr_ex_a(int err_no, const CHAR *file, int line, 
                       const CHAR *func, CHAR *err_str)
{
    CelErrBuffer *ptr = _cel_err_buffer();
    CHAR *err_ptr = ptr->a_buffer[(++(ptr->i))];

    snprintf(err_ptr, CEL_ERRSLEN , "%s(%d)-%s()-%s", 
        file, line, func, err_str);
#ifdef _UNICODE
    cel_mb2unicode(err_ptr, -1, ptr->w_buffer[ptr->i], CEL_ERRSLEN);
#endif
    cel_seterrno(err_no);

    return err_ptr;
}

WCHAR *cel_seterr_ex_w(int err_no, const WCHAR *file, int line, 
                       const WCHAR *func, WCHAR *err_str)
{
    CelErrBuffer *ptr = _cel_err_buffer();
    WCHAR *err_ptr = ptr->w_buffer[(++(ptr->i))];

    snwprintf(err_ptr, CEL_ERRSLEN , L"%s(%d)-%s()-%s", 
        file, line, func, err_str);
#ifndef _UNICODE
    cel_unicode2mb(err_ptr, -1, ptr->a_buffer[ptr->i], CEL_ERRSLEN);
#endif
    cel_seterrno(err_no);

    return err_ptr;
}
