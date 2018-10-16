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
#ifndef __CEL_CRYPTO_SH1_H__
#define __CEL_CRYPTO_SH1_H__

#include "cel/types.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          SHA-1 context structure
 */
typedef struct _CelSha1Context
{
    unsigned long total[2];     /**< Number of bytes processed  */
    unsigned long state[5];     /**< Intermediate digest state  */
    BYTE buffer[64];   /**< Data block being processed */

    BYTE ipad[64];     /**< HMAC: inner padding        */
    BYTE opad[64];     /**< HMAC: outer padding        */
}CelSha1Context;


/**
 * \brief          SHA-1 context setup
 *
 * \param ctx      context to be initialized
 */
void cel_sha1_starts(CelSha1Context *ctx);

/**
 * \brief          SHA-1 process buffer
 *
 * \param ctx      SHA-1 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void cel_sha1_update(CelSha1Context *ctx, const BYTE *input, size_t ilen);

/**
 * \brief          SHA-1 final digest
 *
 * \param ctx      SHA-1 context
 * \param output   SHA-1 checksum result
 */
void cel_sha1_finish(CelSha1Context *ctx, BYTE output[20]);

/**
 * \brief          Output = SHA-1(input buffer)
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   SHA-1 checksum result
 */
void cel_sha1(const BYTE *input, size_t ilen, BYTE output[20]);

/**
 * \brief          SHA-1 HMAC context setup
 *
 * \param ctx      HMAC context to be initialized
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 */
void cel_sha1_hmac_starts(CelSha1Context *ctx, const BYTE *key, size_t keylen);

/**
 * \brief          SHA-1 HMAC process buffer
 *
 * \param ctx      HMAC context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void cel_sha1_hmac_update(CelSha1Context *ctx, const BYTE *input, size_t ilen);

/**
 * \brief          SHA-1 HMAC final digest
 *
 * \param ctx      HMAC context
 * \param output   SHA-1 HMAC checksum result
 */
void cel_sha1_hmac_finish(CelSha1Context *ctx, BYTE output[20]);

/**
 * \brief          SHA-1 HMAC context reset
 *
 * \param ctx      HMAC context to be reset
 */
void cel_sha1_hmac_reset(CelSha1Context *ctx);

/**
 * \brief          Output = HMAC-SHA-1(hmac key, input buffer)
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-SHA-1 result
 */
void cel_sha1_hmac(const BYTE *key, size_t keylen,
                   const BYTE *input, size_t ilen,
                   BYTE output[20]);

#ifdef __cplusplus
}
#endif

#endif
