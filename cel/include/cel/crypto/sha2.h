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
#ifndef __CEL_CRYPTO_SHA2_H__
#define __CEL_CRYPTO_SHA2_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * \brief          SHA-256 context structure
 */
typedef struct _CelSha2Context
{
    unsigned long total[2];     /**< Number of bytes processed  */
    unsigned long state[8];     /**< Intermediate digest state  */
    BYTE buffer[64];   /**< Data block being processed */

    BYTE ipad[64];     /**< HMAC: inner padding        */
    BYTE opad[64];     /**< HMAC: outer padding        */
    int is224;                  /**< 0 =>SHA-256, else SHA-224 */
}CelSha2Context;


/**
 * \brief          SHA-256 context setup
 *
 * \param ctx      context to be initialized
 * \param is224    0 = use SHA256, 1 = use SHA224
 */
void cel_sha2_starts(CelSha2Context *ctx, int is224);

/**
 * \brief          SHA-256 process buffer
 *
 * \param ctx      SHA-256 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void cel_sha2_update(CelSha2Context *ctx, const BYTE *input, size_t ilen);

/**
 * \brief          SHA-256 final digest
 *
 * \param ctx      SHA-256 context
 * \param output   SHA-224/256 checksum result
 */
void cel_sha2_finish(CelSha2Context *ctx, BYTE output[32]);

/**
 * \brief          Output = SHA-256(input buffer)
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   SHA-224/256 checksum result
 * \param is224    0 = use SHA256, 1 = use SHA224
 */
void cel_sha2(const BYTE *input, size_t ilen,
              BYTE output[32], int is224);

/**
 * \brief          SHA-256 HMAC context setup
 *
 * \param ctx      HMAC context to be initialized
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param is224    0 = use SHA256, 1 = use SHA224
 */
void cel_sha2_hmac_starts(CelSha2Context *ctx, const BYTE *key, size_t keylen,
                          int is224);

/**
 * \brief          SHA-256 HMAC process buffer
 *
 * \param ctx      HMAC context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void cel_sha2_hmac_update(CelSha2Context *ctx, const BYTE *input, size_t ilen);

/**
 * \brief          SHA-256 HMAC final digest
 *
 * \param ctx      HMAC context
 * \param output   SHA-224/256 HMAC checksum result
 */
void cel_sha2_hmac_finish(CelSha2Context *ctx, BYTE output[32]);

/**
 * \brief          SHA-256 HMAC context reset
 *
 * \param ctx      HMAC context to be reset
 */
void cel_sha2_hmac_reset(CelSha2Context *ctx);

/**
 * \brief          Output = HMAC-SHA-256(hmac key, input buffer)
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-SHA-224/256 result
 * \param is224    0 = use SHA256, 1 = use SHA224
 */
void cel_sha2_hmac(const BYTE *key, size_t keylen,
                   const BYTE *input, size_t ilen,
                   BYTE output[32], int is224);

#ifdef __cplusplus
}
#endif

#endif
