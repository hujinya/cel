/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
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
