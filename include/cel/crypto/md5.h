/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_CRYPTO_MD5_H__
#define __CEL_CRYPTO_MD5_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelMd5Context
{
    unsigned long total[2];  /**< number of bytes processed */
    unsigned long state[4];  /**< intermediate digest state */
    BYTE buffer[64];/**< data block being processed */
    BYTE ipad[64];  /**< HMAC: inner padding  */
    BYTE opad[64];  /**< HMAC: outer padding */
}CelMd5Context;


/**
 * \brief          MD5 context setup
 *
 * \param ctx      context to be initialized
 */
void cel_md5_starts(CelMd5Context *ctx);

/**
 * \brief          MD5 process buffer
 *
 * \param ctx      MD5 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void cel_md5_update(CelMd5Context *ctx, const BYTE *input, size_t ilen);

/**
 * \brief          MD5 final digest
 *
 * \param ctx      MD5 context
 * \param output   MD5 checksum result
 */
void cel_md5_finish(CelMd5Context *ctx, BYTE output[16]);

/**
 * \brief          Output = MD5(input buffer)
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   MD5 checksum result
 */
void cel_md5(const BYTE *input, size_t ilen, BYTE output[16]);


/**
 * \brief          MD5 HMAC context setup
 *
 * \param ctx      HMAC context to be initialized
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 */
void cel_md5_hmac_starts(CelMd5Context *ctx, 
                         const BYTE *key, size_t keylen);

/**
 * \brief          MD5 HMAC process buffer
 *
 * \param ctx      HMAC context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void cel_md5_hmac_update(CelMd5Context *ctx, 
                         const BYTE *input, size_t ilen);

/**
 * \brief          MD5 HMAC final digest
 *
 * \param ctx      HMAC context
 * \param output   MD5 HMAC checksum result
 */
void cel_md5_hmac_finish(CelMd5Context *ctx, BYTE output[16]);

/**
 * \brief          MD5 HMAC context reset
 *
 * \param ctx      HMAC context to be reset
 */
void cel_md5_hmac_reset(CelMd5Context *ctx);

/**
 * \brief          Output = HMAC-MD5(hmac key, input buffer)
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-MD5 result
 */
void cel_md5_hmac(const BYTE *key, size_t keylen, 
                  const BYTE *input, size_t ilen, 
                  BYTE output[16]);

#ifdef __cplusplus
}
#endif

#endif
