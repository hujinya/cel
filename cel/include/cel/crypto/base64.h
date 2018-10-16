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
#ifndef __CEL_CRYPTO_BASE64_H__
#define __CEL_CRYPTO_BASE64_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define cel_base64_encode_size(n) ((n + 2) / 3 * 4 + 4)
#define cel_base64_decode_size(n) ((n + 3) / 4 * 3 + 4)

int cel_base64_encode(BYTE *dst, size_t *dlen, 
                      const BYTE *src, size_t slen);
int cel_base64_decode(BYTE *dst, size_t *dlen, 
                      const BYTE *src, size_t slen);

#define cel_base64url_encode_size(n) ((n + 2) / 3 * 4 + 4)
#define cel_base64url_decode_size(n) ((n + 3) / 4 * 3 + 4 + 4)

int cel_base64url_encode(BYTE *dst, size_t *dlen, 
                         const BYTE *src, size_t slen);
int cel_base64url_decode(BYTE *dst, size_t *dlen, 
                         const BYTE *src, size_t slen);

#ifdef __cplusplus
}
#endif

#endif
