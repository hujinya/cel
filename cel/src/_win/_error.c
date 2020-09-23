/**
 * CEL(C Extension Library)
 * Copyright (C)2008 Hu Jinya(hu_jinya@163.com) 
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
#ifdef _CEL_WIN

CHAR *os_strerror_a(int err_no)
{
    CelErr *ptr = _cel_err();
    CHAR *lpMsgBuf = ptr->stic.a_buffer;
    size_t len;

    FormatMessageA(/* FORMAT_MESSAGE_ALLOCATE_BUFFER
        |*/FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
        NULL, 
        (DWORD)err_no, 
        /* MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)*/
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), 
        (LPSTR)lpMsgBuf, 
        CEL_ERRSLEN, 
        NULL);
    if ((len = strlen(lpMsgBuf)) > 1) lpMsgBuf[len - 2] = '\0';
    //LocalFree(lpMsgBuf);
    return lpMsgBuf;
}

WCHAR *os_strerror_w(int err_no)
{
    CelErr *ptr = _cel_err();
    WCHAR *lpMsgBuf = ptr->stic.w_buffer;
    size_t len;

    FormatMessageW(/* FORMAT_MESSAGE_ALLOCATE_BUFFER
        |*/FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
        NULL, 
        (DWORD)err_no, 
        /* MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)*/
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), 
        (LPWSTR)lpMsgBuf, 
        CEL_ERRSLEN, 
        NULL);
    if ((len = wcslen(lpMsgBuf)) > 1) lpMsgBuf[len - 2] = L'\0';
    //LocalFree(lpMsgBuf);
    return lpMsgBuf;
}

#endif