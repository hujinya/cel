/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __OS_ERROR_UNIX_H__
#define __OS_ERROR_UNIX_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define os_seterrno(err_no) SetLastError(err_no)
#define os_geterrno() GetLastError()
#define os_setwsaerrno(err_no) WSASetLastError(err_no)
#define os_getwsaerrno() WSAGetLastError()
CHAR *os_strerror_a(int err_no);
WCHAR *os_strerror_w(int err_no);

#ifdef __cplusplus
}
#endif


#endif
