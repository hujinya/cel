/**
 * CEL(C Extension Library)
 * Copyright (C)2008-2017 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_GUID_H__
#define __CEL_GUID_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

const char *cel_guid_generate();
BOOL IsValidGUID(char *guid);
BOOL IsValidGUIDOutputString(char *guid);
char *RandomDataToGUIDString(const U64 bytes[2]);

#ifdef __cplusplus
}
#endif

#endif
