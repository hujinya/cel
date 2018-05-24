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
