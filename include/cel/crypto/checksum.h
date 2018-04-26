/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_CRYPTO_CHECKSUM_H__
#define __CEL_CRYPTO_CHECKSUM_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

U16 cel_checksum(U16 *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif
