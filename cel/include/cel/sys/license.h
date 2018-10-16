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
