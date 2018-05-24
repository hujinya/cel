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
#ifndef __CEL_CRYPTO_MD4_H__
#define __CEL_CRYPTO_MD4_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          MD4 context structure
 */
typedef struct _CelMd4Context
{
    unsigned long total[2];     /**< Number of bytes processed  */
    unsigned long state[4];     /**< Intermediate digest state  */
    BYTE buffer[64];   /**< Data block being processed */

    BYTE ipad[64];     /**< HMAC: inner padding        */
    BYTE opad[64];     /**< HMAC: outer padding        */
}CelMd4Context;

/**
 * \brief          MD4 context setup
 *
 * \param ctx      context to be initialized
 */
void cel_md4_starts(CelMd4Context *ctx);

/**
 * \brief          MD4 process buffer
 *
 * \param ctx      MD4 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void cel_md4_update(CelMd4Context *ctx, const BYTE *input, size_t ilen);

/**
 * \brief          MD4 final digest
 *
 * \param ctx      MD4 context
 * \param output   MD4 checksum result
 */
void cel_md4_finish(CelMd4Context *ctx, BYTE output[16]);

/**
 * \brief          Output = MD4(input buffer)
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   MD4 checksum result
 */
void cel_md4(const BYTE *input, size_t ilen, BYTE output[16]);

/**
 * \brief          MD4 HMAC context setup
 *
 * \param ctx      HMAC context to be initialized
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 */
void cel_md4_hmac_starts(CelMd4Context *ctx, const BYTE *key, size_t keylen);

/**
 * \brief          MD4 HMAC process buffer
 *
 * \param ctx      HMAC context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void cel_md4_hmac_update(CelMd4Context *ctx, const BYTE *input, size_t ilen);

/**
 * \brief          MD4 HMAC final digest
 *
 * \param ctx      HMAC context
 * \param output   MD4 HMAC checksum result
 */
void cel_md4_hmac_finish(CelMd4Context *ctx, BYTE output[16]);

/**
 * \brief          MD4 HMAC context reset
 *
 * \param ctx      HMAC context to be reset
 */
void cel_md4_hmac_reset(CelMd4Context *ctx);

/**
 * \brief          Output = HMAC-MD4(hmac key, input buffer)
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-MD4 result
 */
void cel_md4_hmac(const BYTE *key, size_t keylen,
                  const BYTE *input, size_t ilen,
                  BYTE output[16]);

#ifdef __cplusplus
}
#endif

#endif
