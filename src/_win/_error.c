#include "cel/error.h"
#ifdef _CEL_WIN

CHAR *os_strerror_a(int err_no)
{
    CelErrBuffer *ptr = _cel_err_buffer();
    CHAR *lpMsgBuf = ptr->a_buffer[ptr->i];
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
    CelErrBuffer *ptr = _cel_err_buffer();
    WCHAR *lpMsgBuf = ptr->w_buffer[ptr->i];
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