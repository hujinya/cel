/**
 * CEL(C Extension Library)
 * Copyright (C)2008-2017 Hu Jinya(hu_jinya@163.com) 
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
#ifndef __CEL_GUID_H__
#define __CEL_GUID_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

char *cel_guid_rand2str(const U64 bytes[2], char *out);
char *cel_guid_generate(char *out);
BOOL cel_guid_isvalid(const char *guid);
BOOL cel_guid_isvalid_str(const char *guid);

#ifdef __cplusplus
}
#endif

#endif
