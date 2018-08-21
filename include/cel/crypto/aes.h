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
#ifndef __CEL_CRYPTO_AES_H__
#define __CEL_CRYPTO_AES_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _CEL_AES_MODE_CFB
#define _CEL_AES_MODE_CTR
#define _CEL_AES_ROM_TABLES

#define CEL_AES_ENCRYPT     1
#define CEL_AES_DECRYPT     0

/* AES context structure */
typedef struct _CelAesContext
{
    int nr;                     /**< number of rounds */
    unsigned long *rk;          /**< AES round keys   */
    unsigned long buf[68];      /**< unaligned data   */
}CelAesContext;

/**
 * \brief          AES key schedule (encryption)
 *
 * \param ctx      AES context to be initialized
 * \param key      encryption key
 * \param keysize  must be 128, 192 or 256
 *
 * \return         0 if successful, or -1
 */
int cel_aes_setkey_enc(CelAesContext *ctx, 
                       const BYTE *key, unsigned int keysize);

/**
 * \brief          AES key schedule (decryption)
 *
 * \param ctx      AES context to be initialized
 * \param key      decryption key
 * \param keysize  must be 128, 192 or 256
 *
 * \return         0 if successful, or -1
 */
int cel_aes_setkey_dec(CelAesContext *ctx, 
                       const BYTE *key, unsigned int keysize);

/**
 * \brief          AES-ECB block encryption/decryption
 *
 * \param ctx      AES context
 * \param mode     CEL_AES_ENCRYPT or CEL_AES_DECRYPT
 * \param input    16-byte input block
 * \param output   16-byte output block
 *
 * \return         0 if successful
 */
int cel_aes_crypt_ecb(CelAesContext *ctx, int mode, 
                      const BYTE input[16], BYTE output[16]);

/**
 * \brief          AES-CBC buffer encryption/decryption Length should be a multiple
 *                 of the block size (16 bytes)
 *
 * \param ctx      AES context
 * \param mode     CEL_AES_ENCRYPT or CEL_AES_DECRYPT
 * \param length   length of the input data
 * \param iv       initialization vector (updated after use)
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 *
 * \return         0 if successful, or -1
 */
int cel_aes_crypt_cbc(CelAesContext *ctx, int mode, size_t length, 
                      BYTE iv[16],
                      const BYTE *input, BYTE *output);

/**
 * \brief          AES-CFB128 buffer encryption/decryption.
 *
 * Note: Due to the nature of CFB you should use the same key schedule for
 * both encryption and decryption. So a context initialized with
 * aes_setkey_enc()for both CEL_AES_ENCRYPT and CEL_AES_DECRYPT.
 *
 * \param ctx      AES context
 * \param mode     CEL_AES_ENCRYPT or CEL_AES_DECRYPT
 * \param length   length of the input data
 * \param iv_off   offset in IV (updated after use)
 * \param iv       initialization vector (updated after use)
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 *
 * \return         0 if successful
 */
int cel_aes_crypt_cfb128(CelAesContext *ctx, int mode,
                         size_t length, size_t *iv_off, BYTE iv[16],
                         const BYTE *input, BYTE *output);

/**
 * \brief               AES-CTR buffer encryption/decryption
 *
 * CEL_WARNING: You have to keep the maximum use of your counter in mind!
 *
 * Note: Due to the nature of CTR you should use the same key schedule for
 * both encryption and decryption. So a context initialized with
 * aes_setkey_enc()for both CEL_AES_ENCRYPT and CEL_AES_DECRYPT.
 *
 * \param length        The length of the data
 * \param nc_off        The offset in the current stream_block (for resuming
 *                      within current cipher stream). The offset pointer to
 *                      should be 0 at the start of a stream.
 * \param nonce_counter The 128-bit nonce and counter.
 * \param stream_block  The saved stream-block for resuming. Is overwritten
 *                      by the function.
 * \param input         The input data stream
 * \param output        The output data stream
 *
 * \return              0 if successful
 */
int cel_aes_crypt_ctr(CelAesContext *ctx, size_t length, size_t *nc_off,
                      BYTE nonce_counter[16], BYTE stream_block[16],
                      const BYTE *input, BYTE *output);

#ifdef __cplusplus
}
#endif

#endif /* aes.h */
