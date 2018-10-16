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
#ifndef __CEL_CRYPTO_SHA4_H__
#define __CEL_CRYPTO_SHA4_H__

#include "cel/types.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          SHA-512 context structure
 */
typedef struct _CelSha4Context
{
    U64 total[2];                /**< Number of bytes processed  */
    U64 state[8];                /**< Intermediate digest state  */
    BYTE buffer[128];   /**< Data block being processed */

    BYTE ipad[128];     /**< HMAC: inner padding        */
    BYTE opad[128];     /**< HMAC: outer padding        */
    int is384;                   /**< 0 = SHA-512, else SHA-384  */
}CelSha4Context;


/**
 * \brief          SHA-512 context setup
 *
 * \param ctx      context to be initialized
 * \param is384    0 = use SHA512, 1 = use SHA384
 */
void cel_sha4_starts(CelSha4Context *ctx, int is384);

/**
 * \brief          SHA-512 process buffer
 *
 * \param ctx      SHA-512 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void cel_sha4_update(CelSha4Context *ctx, const BYTE *input, size_t ilen);

/**
 * \brief          SHA-512 final digest
 *
 * \param ctx      SHA-512 context
 * \param output   SHA-384/512 checksum result
 */
void cel_sha4_finish(CelSha4Context *ctx, BYTE output[64]);

/**
 * \brief          Output = SHA-512(input buffer)
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   SHA-384/512 checksum result
 * \param is384    0 = use SHA512, 1 = use SHA384
 */
void cel_sha4(const BYTE *input, size_t ilen,
              BYTE output[64], int is384);

/**
 * \brief          SHA-512 HMAC context setup
 *
 * \param ctx      HMAC context to be initialized
 * \param is384    0 = use SHA512, 1 = use SHA384
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 */
void cel_sha4_hmac_starts(CelSha4Context *ctx, const BYTE *key, size_t keylen,
                          int is384);

/**
 * \brief          SHA-512 HMAC process buffer
 *
 * \param ctx      HMAC context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void cel_sha4_hmac_update(CelSha4Context *ctx, const BYTE *input, size_t ilen);

/**
 * \brief          SHA-512 HMAC final digest
 *
 * \param ctx      HMAC context
 * \param output   SHA-384/512 HMAC checksum result
 */
void cel_sha4_hmac_finish(CelSha4Context *ctx, BYTE output[64]);

/**
 * \brief          SHA-512 HMAC context reset
 *
 * \param ctx      HMAC context to be reset
 */
void cel_sha4_hmac_reset(CelSha4Context *ctx);

/**
 * \brief          Output = HMAC-SHA-512(hmac key, input buffer)
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-SHA-384/512 result
 * \param is384    0 = use SHA512, 1 = use SHA384
 */
void cel_sha4_hmac(const BYTE *key, size_t keylen,
                   const BYTE *input, size_t ilen,
                   BYTE output[64], int is384);

#ifdef __cplusplus
}
#endif

#endif
