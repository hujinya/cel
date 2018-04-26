/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_SYS_LICENSE_H__
#define __CEL_SYS_LICENSE_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define KEY_LEN     1024
#define LICENSE_LEN 1024
#define LICENSE_DELIMITER  _T(':')

typedef struct _CelLicense
{
    TCHAR license[LICENSE_LEN];
    TCHAR key[KEY_LEN];
    long expired_hours;
}CelLicense;

/*
 * ID-Machine Code-Company-Product-
 */


int license_verify(const TCHAR *license_file, const TCHAR *private_file,
                   const TCHAR *key, const TCHAR *license);
long license_expried(const TCHAR *license_file, const TCHAR *private_file,
                     const TCHAR *key, const TCHAR *license);
long license_get_expried(const TCHAR *license_file, const TCHAR *private_file,
                         const TCHAR *key, const TCHAR *license);

#ifdef __cplusplus
}
#endif

#endif
