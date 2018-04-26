/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_ERROR_UNIX_H__
#define __CEL_ERROR_UNIX_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define os_seterrno(err_no) errno = err_no
#define os_geterrno() errno
#define os_setwsaerrno(err_no) errno = err_no
#define os_getwsaerrno() errno
#define os_strerror_a(errno) strerror(errno)
WCHAR *os_strerror_w(int err_no);

#ifdef __cplusplus
}
#endif

#endif
