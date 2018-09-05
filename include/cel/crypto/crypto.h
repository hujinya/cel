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
#ifndef __CEL_CRYPTO_CRYPTO_H__
#define __CEL_CRYPTO_CRYPTO_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAVE_OPENSSL_CRYPTO_H

#include "cel/ssl/rsa.h"
typedef struct _CelPkey
{
    int i;
}CelPkey;

#include "cel/ssl/md2.h"
#include "cel/ssl/md4.h"
#include "cel/ssl/md5.h"
#include "cel/ssl/sha1.h"
#include "cel/ssl/sha2.h"
#include "cel/ssl/sha4.h"

typedef enum _CelMdType
{
    CEL_MD_NONE = 0,
    CEL_MD_MD2,
    CEL_MD_MD4,
    CEL_MD_MD5,
    /* SHA1 */
    CEL_MD_SHA1,
    /* SHA2 */
    CEL_MD_SHA224,
    CEL_MD_SHA256,
    /* SHA4 */
    CEL_MD_SHA384,
    CEL_MD_SHA512,
}CelMdType;

typedef struct _CelMd
{
    CelMdType type;

    /* Digest initialisation function */
    void (*starts_func) (void *ctx);
    /* Digest update function */
    void (*update_func) (void *ctx, const BYTE *input, size_t ilen);
    /* Digest finalisation function */
    void (*finish_func) (void *ctx, BYTE *output);

    /* Generic digest function */
    void (*digest_func) (const BYTE *input, size_t ilen,
        BYTE *output);

    /* HMAC Initialisation function */
    void (*hmac_starts_func) (void *ctx, const BYTE *key, size_t keylen);
    /* HMAC update function */
    void (*hmac_update_func) (void *ctx, const BYTE *input, size_t ilen);
    /* HMAC finalisation function */
    void (*hmac_finish_func) (void *ctx, BYTE *output);
    /* HMAC context reset function */
    void (*hmac_reset_func) (void *ctx);

    /* Generic HMAC function */
    void (*hmac_func) (const BYTE *key, size_t keylen,
        const BYTE *input, size_t ilen,
        BYTE *output);
}CelMd;

typedef struct _CelMdContext
{
    CelMd *md;
}CelMdContext;

int  cel_mdcontext_init(CelMdContext *ctx, CelMd *md);
void cel_mdcontext_destroy(CelMdContext *ctx);

int cel_mdcontext_update(CelMdContext *ctx, const void *input, size_t ilen);
int cel_mdcontext_finish(CelMdContext *ctx, BYTE *output, size_t *olen);

int cel_mdcontext_md(CelMd *md, const void *input, size_t ilen, 
                     BYTE *output, size_t *olen);

int  cel_mdcontext_hmac_init(CelMdContext *ctx, CelMd *md, 
                             const BYTE *key, size_t keyl);
void cel_mdcontext_hmac_destroy(CelMdContext *ctx);

int cel_mdcontext_hmac_update(CelMdContext *ctx, 
                              const void *input, size_t ilen);
int cel_mdcontext_hmac_finish(CelMdContext *ctx, BYTE *output, size_t *olen);

int cel_mdcontext_hmac_md(CelMd *md, const BYTE *key, size_t keyl,
                          const void *input, size_t ilen, 
                          BYTE *output, size_t *olen);

#include "cel/ssl/des.h"
#include "cel/ssl/aes.h"
#include "cel/ssl/rc4.h"

typedef enum _CelCipherId
{
    CEL_CIPHER_ID_NONE = 0,
    CEL_CIPHER_ID_AES,
    CEL_CIPHER_ID_DES,
    CEL_CIPHER_ID_3DES,
    CEL_CIPHER_ID_RC4
}CelCipherId;

typedef enum _CelCipherMode
{
    CEL_CIPHER_MODE_NONE = 0,
    CEL_CIPHER_MODE_CBC,
    CEL_CIPHER_MODE_CFB,
    CEL_CIPHER_MODE_OFB,
    CEL_CIPHER_MODE_CTR
}CelCipherMode;

typedef struct _CelCipher
{
    CelCipherId id;
    CelCipherMode mode;
}CelCipher;

typedef struct _CelCipherContext
{
    CelCipher *cipher;
}CelCipherContext;

int cel_ciphercontext_init(CelCipherContext *ctx, CelCipher *cipher, 
                           const BYTE *key, const BYTE *iv, 
                           int enc);
void cel_ciphercontext_destroy(CelCipherContext *ctx);
int cel_ciphercontext_update(CelCipherContext *ctx, BYTE *input, size_t ilen, 
                             BYTE *output, size_t *olen);
int cel_ciphercontext_finish(CelCipherContext *ctx, 
                             BYTE *output, size_t *olen);

#else
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/rc4.h>
#include <openssl/x509v3.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/crypto.h>

typedef unsigned long (* CelCryptoThreadIdFunc) (void);
typedef void (* CelCryptoThreadLockingFunc) (int mode, int type, 
                                             const char *file, int line);

/* 
 * \brief   Get openssl library mutex numbers
 */
#define cel_cryptomutex_num() CRYPTO_num_locks()

int cel_cryptomutex_register(CelCryptoThreadIdFunc threadidfunc, 
                             CelCryptoThreadLockingFunc threadlockingfunc);
void cel_cryptomutex_unregister(void);


typedef EVP_PKEY CelPkey;


typedef EVP_MD CelMd;
typedef EVP_MD_CTX CelMdContext;

/* int  cel_mdcontext_init(CelMdContext *ctx, CelMd *md)*/
#define cel_mdcontext_init(ctx, md) \
    EVP_MD_CTX_init(ctx); EVP_DigestInit(ctx, md)
/* void cel_mdcontext_destroy(CelMdContext *ctx)*/
#define cel_mdcontext_destroy(ctx) EVP_MD_CTX_destroy(ctx)

/* 
 * int cel_mdcontext_update(CelMdContext *ctx, const void *input, size_t ilen)
 */
#define cel_mdcontext_update(ctx, input, ilen) \
    EVP_DigestUpdate(ctx, input, ilen)
/* int cel_mdcontext_finish(CelMdContext *ctx, BYTE *output, size_t *olen)*/
#define cel_mdcontext_finish(ctx, output, olen) \
    EVP_DigestFinal(ctx, output, olen)

/* 
 * int cel_mdcontext_md(CelMd *md, const void *input, size_t ilen, 
 *                      BYTE *output, size_t *olen)
 */
#define cel_mdcontext_md(md, input, ilen, output, olen) \
    EVP_Digest(input, ilen, output, olen, md, NULL)

/*
 * int cel_mdcontext_hmac_init(CelMdContext *ctx, CelMd *md, 
 *                             const BYTE *key, size_t keyl)
 */
#define cel_mdcontext_hmac_init(ctx, md, key, keyl)
/*
 * void cel_mdcontext_hmac_destroy(CelMdContext *ctx)
 */
#define cel_mdcontext_hmac_destroy(ctx) EVP_MD_CTX_destroy(ctx)

/* 
 * int cel_mdcontext_hmac_update(CelMdContext *ctx, 
 *                               const void *input, size_t ilen)
 */
#define cel_mdcontext_hmac_update(ctx, input, ilen)
/* int cel_mdcontext_hmac_finish(CelMdContext *ctx, 
 *                               BYTE *output, size_t *olen)
 */
#define cel_mdcontext_hmac_finish(ctx, output, olen)

/*
 * int cel_mdcontext_hmac_md(CelMd *md, const BYTE *key, size_t keyl,
 *                           const void *input, size_t ilen, 
 *                           BYTE *output, size_t *olen)
 */
#define cel_mdcontext_hmac_md(md, key, keyl, input, ilen, output, olen)

typedef EVP_CIPHER CelCipher;
typedef EVP_CIPHER_CTX  CelCipherContext;
/*
 * int cel_ciphercontext_init(CelCipherContext *ctx, CelCipher *cipher, 
 *                            const BYTE *key, const BYTE *iv, 
 *                            int enc);
 */
#define cel_ciphercontext_init(ctx, cipher, key, iv, enc) \
    EVP_CIPHER_CTX_init(ctx); EVP_CipherInit(ctx, cipher, key, iv, enc)
/*
 * void cel_ciphercontext_destroy(CelCipherContext *ctx);
 */
#define cel_ciphercontext_destroy(ctx) EVP_CIPHER_CTX_cleanup(ctx)

/*
 * int cel_ciphercontext_update(CelCipherContext *ctx, 
 *                              BYTE *input, size_t ilen, 
 *                              BYTE *output, size_t *olen)
 */
#define cel_ciphercontext_update(ctx, input, ilen, output, olen) \
    EVP_CipherUpdate(ctx, output, olen, input, ilen)
/*
 * int cel_ciphercontext_finish(CelCipherContext *ctx, 
 *                              BYTE *output, size_t *olen)
 */
#define cel_ciphercontext_finish(ctx, output, olen) \
    EVP_CipherFinal(ctx, output, olen);

#endif

#define cel_sign_init(ctx, md, key, keyl)
#define cel_sign_update(ctx, input, ilen)
#define cel_sign_finish(ctx, output, olen)

#define cel_verify_init(ctx, md, key, keyl)
#define cel_verify_update(ctx, input, ilen)
#define cel_verify_finish(ctx, output, olen)

#define cel_encrypt_init(ctx, cipher, key, iv) \
    cel_ciphercontext_init(ctx, cipher, key, iv, 1)
#define cel_encrypt_destroy(ctx) cel_ciphercontext_destroy(ctx)
#define cel_encrypt_update(ctx, input, ilen, output, olen) \
    cel_ciphercontext_update(ctx, input, ilen, output, olen)
#define cel_encrypt_finish(ctx, output, olen) \
    cel_ciphercontext_finish(ctx, output, olen)

#define cel_decrypt_init(ctx, cipher, key, iv) \
    cel_ciphercontext_init(ctx, cipher, key, iv, 0)
#define cel_decrypt_destroy(ctx) cel_ciphercontext_destroy(ctx)
#define cel_decrypt_update(ctx, input, ilen, output, olen) \
    cel_ciphercontext_update(ctx, input, ilen, output, olen)
#define cel_decrypt_finish(ctx, output, olen) \
    cel_ciphercontext_finish(ctx, output, olen)

#ifdef __cplusplus
}
#endif

#endif