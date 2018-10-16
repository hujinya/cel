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
#ifndef __CEL_CRYPTO_MD2_H__
#define __CEL_CRYPTO_MD2_H__

#include "cel/types.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          MD2 context structure
 */
typedef struct _CelMd5Context
{
    BYTE cksum[16];    /**< checksum of the data block */
    BYTE state[48];    /**< intermediate digest state  */
    BYTE buffer[16];   /**< data block being processed */

    BYTE ipad[16];     /**< HMAC: inner padding        */
    BYTE opad[16];     /**< HMAC: outer padding        */
    size_t left;                /**< amount of data in buffer   */
}CelMd5Context;


/**
 * \brief          MD2 context setup
 *
 * \param ctx      context to be initialized
 */
void cel_md2_starts(CelMd5Context *ctx);

/**
 * \brief          MD2 process buffer
 *
 * \param ctx      MD2 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void cel_md2_update(CelMd5Context *ctx, const BYTE *input, size_t ilen);

/**
 * \brief          MD2 final digest
 *
 * \param ctx      MD2 context
 * \param output   MD2 checksum result
 */
void cel_md2_finish(CelMd5Context *ctx, BYTE output[16]);

/**
 * \brief          Output = MD2(input buffer)
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   MD2 checksum result
 */
void cel_md2(const BYTE *input, size_t ilen, BYTE output[16]);

/**
 * \brief          MD2 HMAC context setup
 *
 * \param ctx      HMAC context to be initialized
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 */
void cel_md2_hmac_starts(CelMd5Context *ctx, const BYTE *key, size_t keylen);

/**
 * \brief          MD2 HMAC process buffer
 *
 * \param ctx      HMAC context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void cel_md2_hmac_update(CelMd5Context *ctx, const BYTE *input, size_t ilen);

/**
 * \brief          MD2 HMAC final digest
 *
 * \param ctx      HMAC context
 * \param output   MD2 HMAC checksum result
 */
void cel_md2_hmac_finish(CelMd5Context *ctx, BYTE output[16]);

/**
 * \brief          MD2 HMAC context reset
 *
 * \param ctx      HMAC context to be reset
 */
void cel_md2_hmac_reset(CelMd5Context *ctx);

/**
 * \brief          Output = HMAC-MD2(hmac key, input buffer)
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-MD2 result
 */
void cel_md2_hmac(const BYTE *key, size_t keylen,
                  const BYTE *input, size_t ilen,
                  BYTE output[16]);


#ifdef __cplusplus
}
#endif

#endif
