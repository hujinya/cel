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
#ifndef __CEL_CRYPTO_DES_H__
#define __CEL_CRYPTO_DES_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CEL_DES_ENCRYPT     1
#define CEL_DES_DECRYPT     0

#define CEL_DES_KEY_SIZE    8 /* 64 bits */ 

/**
 * \brief          DES context structure
 */
typedef struct _CelDesContext
{
    int mode;             /**<  encrypt/decrypt   */
    unsigned long sk[32]; /**<  DES subkeys       */
}CelDesContext;

/**
 * \brief          Triple-DES context structure
 */
typedef struct _CelDes3Context
{
    int mode;             /**<  encrypt/decrypt   */
    unsigned long sk[96]; /**<  3DES subkeys      */
}CelDes3Context;

/**
 * \brief          Set key parity on the given key to odd.
 *
 *                 DES keys are 56 bits long, but each byte is padded with
 *                 a parity bit to allow verification.
 *
 * \param key      8-byte secret key
 */
void cel_des_key_set_parity(BYTE key[CEL_DES_KEY_SIZE]);

/**
 * \brief          Check that key parity on the given key is odd.
 *
 *                 DES keys are 56 bits long, but each byte is padded with
 *                 a parity bit to allow verification.
 *
 * \param key      8-byte secret key
 *
 * \return         0 is parity was ok, 1 if parity was not correct.
 */
int cel_des_key_check_key_parity(const BYTE key[CEL_DES_KEY_SIZE]);

/**
 * \brief          Check that key is not a weak or semi-weak DES key
 *
 * \param key      8-byte secret key
 *
 * \return         0 if no weak key was found, 1 if a weak key was identified.
 */
int cel_des_key_check_weak(const BYTE key[CEL_DES_KEY_SIZE]);

/**
 * \brief          DES key schedule (56-bit, encryption)
 *
 * \param ctx      DES context to be initialized
 * \param key      8-byte secret key
 *
 * \return         0
 */
int cel_des_setkey_enc(CelDesContext *ctx, const BYTE key[CEL_DES_KEY_SIZE]);

/**
 * \brief          DES key schedule (56-bit, decryption)
 *
 * \param ctx      DES context to be initialized
 * \param key      8-byte secret key
 *
 * \return         0
 */
int cel_des_setkey_dec(CelDesContext *ctx, const BYTE key[CEL_DES_KEY_SIZE]);

/**
 * \brief          Triple-DES key schedule (112-bit, encryption)
 *
 * \param ctx      3DES context to be initialized
 * \param key      16-byte secret key
 *
 * \return         0
 */
int cel_des3_set2key_enc(CelDes3Context *ctx, 
                         const BYTE key[CEL_DES_KEY_SIZE * 2]);

/**
 * \brief          Triple-DES key schedule (112-bit, decryption)
 *
 * \param ctx      3DES context to be initialized
 * \param key      16-byte secret key
 *
 * \return         0
 */
int cel_des3_set2key_dec(CelDes3Context *ctx, 
                         const BYTE key[CEL_DES_KEY_SIZE * 2]);

/**
 * \brief          Triple-DES key schedule (168-bit, encryption)
 *
 * \param ctx      3DES context to be initialized
 * \param key      24-byte secret key
 *
 * \return         0
 */
int cel_des3_set3key_enc(CelDes3Context *ctx, 
                         const BYTE key[CEL_DES_KEY_SIZE * 3]);

/**
 * \brief          Triple-DES key schedule (168-bit, decryption)
 *
 * \param ctx      3DES context to be initialized
 * \param key      24-byte secret key
 *
 * \return         0
 */
int cel_des3_set3key_dec(CelDes3Context *ctx, 
                         const BYTE key[CEL_DES_KEY_SIZE * 3]);

/**
 * \brief          DES-ECB block encryption/decryption
 *
 * \param ctx      DES context
 * \param input    64-bit input block
 * \param output   64-bit output block
 *
 * \return         0 if successful
 */
int cel_des_crypt_ecb(CelDesContext *ctx, const BYTE input[8], BYTE output[8]);

/**
 * \brief          DES-CBC buffer encryption/decryption
 *
 * \param ctx      DES context
 * \param mode     CEL_DES_ENCRYPT or CEL_DES_DECRYPT
 * \param length   length of the input data
 * \param iv       initialization array (updated after use)
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 */
int cel_des_crypt_cbc(CelDesContext *ctx, int mode, size_t length, 
                      BYTE iv[8], const BYTE *input, 
                      BYTE *output);

/**
 * \brief          3DES-ECB block encryption/decryption
 *
 * \param ctx      3DES context
 * \param input    64-bit input block
 * \param output   64-bit output block
 *
 * \return         0 if successful
 */
int cel_des3_crypt_ecb(CelDes3Context *ctx, const BYTE input[8], 
                       BYTE output[8]);

/**
 * \brief          3DES-CBC buffer encryption/decryption
 *
 * \param ctx      3DES context
 * \param mode     CEL_DES_ENCRYPT or CEL_DES_DECRYPT
 * \param length   length of the input data
 * \param iv       initialization array (updated after use)
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 *
 * \return         0 if successful, or POLARSSL_ERR_DES_INVALID_INPUT_LENGTH
 */
int cel_des3_crypt_cbc(CelDes3Context *ctx, int mode, size_t length, 
                       BYTE iv[8], const BYTE *input, 
                       BYTE *output);


#ifdef __cplusplus
}
#endif

#endif
