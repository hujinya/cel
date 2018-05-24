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
#ifndef __CEL_NET_SMTP_H__
#define __CEL_NET_SMTP_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CEL_SMTP_PORT        25

/* Command stage */
#define CEL_SMTP_HELO         4
#define CEL_SMTP_MAIL         5
#define CEL_SMTP_RCPT         6
#define CEL_SMTP_DATA         7
#define CEL_SMTP_BODY         8
#define CEL_SMTP_QUIT         9
#define CEL_SMTP_END         10
#define CEL_SMTP_ERROR       11

typedef struct _CelSmtp
{
    int x;
}CelSmtp;

#ifdef __cplusplus
}
#endif

#endif