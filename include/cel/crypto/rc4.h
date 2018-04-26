/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_CRYPTO_RC4_H__
#define __CEL_CRYPTO_RC4_H__

#include "cel/types.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelRc4Context
{
    int x, y;
    BYTE m[256];
}CelRc4Context;


/**
 * \brief          RC4 key schedule
 *
 * \param ctx      RC4 context to be initialized
 * \param key      the secret key
 * \param keylen   length of the key
 */
void cel_arc4_setup(CelRc4Context *ctx, const BYTE *key, unsigned int keylen);

/**
 * \brief          RC4 cipher function
 *
 * \param ctx      RC4 context
 * \param length   length of the input data
 * \param input    buffer holding the input data
 * \param output   buffer for the output data
 *
 * \return         0 if successful
 */
int cel_arc4_crypt(CelRc4Context *ctx, size_t len, 
                   const BYTE *input, BYTE *output);

#ifdef __cplusplus
}
#endif

#endif
