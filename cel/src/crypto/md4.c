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
#include "cel/crypto/md4.h"
#include "cel/byteorder.h"

/*
 * MD4 context setup
 */
void cel_md4_starts(CelMd4Context *ctx)
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
}

static void cel_md4_process(CelMd4Context *ctx, const BYTE data[64])
{
    unsigned long X[16], A, B, C, D;

    CEL_GET_LE32(X[ 0], data,  0);
    CEL_GET_LE32(X[ 1], data,  4);
    CEL_GET_LE32(X[ 2], data,  8);
    CEL_GET_LE32(X[ 3], data, 12);
    CEL_GET_LE32(X[ 4], data, 16);
    CEL_GET_LE32(X[ 5], data, 20);
    CEL_GET_LE32(X[ 6], data, 24);
    CEL_GET_LE32(X[ 7], data, 28);
    CEL_GET_LE32(X[ 8], data, 32);
    CEL_GET_LE32(X[ 9], data, 36);
    CEL_GET_LE32(X[10], data, 40);
    CEL_GET_LE32(X[11], data, 44);
    CEL_GET_LE32(X[12], data, 48);
    CEL_GET_LE32(X[13], data, 52);
    CEL_GET_LE32(X[14], data, 56);
    CEL_GET_LE32(X[15], data, 60);

#define S(x,n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];

#define F(x, y, z) ((x & y) | ((~x) & z))
#define P(a,b,c,d,x,s){ a += F(b,c,d)+ x; a = S(a,s); }

    P(A, B, C, D, X[ 0],  3);
    P(D, A, B, C, X[ 1],  7);
    P(C, D, A, B, X[ 2], 11);
    P(B, C, D, A, X[ 3], 19);
    P(A, B, C, D, X[ 4],  3);
    P(D, A, B, C, X[ 5],  7);
    P(C, D, A, B, X[ 6], 11);
    P(B, C, D, A, X[ 7], 19);
    P(A, B, C, D, X[ 8],  3);
    P(D, A, B, C, X[ 9],  7);
    P(C, D, A, B, X[10], 11);
    P(B, C, D, A, X[11], 19);
    P(A, B, C, D, X[12],  3);
    P(D, A, B, C, X[13],  7);
    P(C, D, A, B, X[14], 11);
    P(B, C, D, A, X[15], 19);

#undef P
#undef F

#define F(x,y,z) ((x & y) | (x & z) | (y & z))
#define P(a,b,c,d,x,s){ a += F(b,c,d)+ x + 0x5A827999; a = S(a,s); }

    P(A, B, C, D, X[ 0],  3);
    P(D, A, B, C, X[ 4],  5);
    P(C, D, A, B, X[ 8],  9);
    P(B, C, D, A, X[12], 13);
    P(A, B, C, D, X[ 1],  3);
    P(D, A, B, C, X[ 5],  5);
    P(C, D, A, B, X[ 9],  9);
    P(B, C, D, A, X[13], 13);
    P(A, B, C, D, X[ 2],  3);
    P(D, A, B, C, X[ 6],  5);
    P(C, D, A, B, X[10],  9);
    P(B, C, D, A, X[14], 13);
    P(A, B, C, D, X[ 3],  3);
    P(D, A, B, C, X[ 7],  5);
    P(C, D, A, B, X[11],  9);
    P(B, C, D, A, X[15], 13);

#undef P
#undef F

#define F(x,y,z) (x ^ y ^ z)
#define P(a,b,c,d,x,s){ a += F(b,c,d)+ x + 0x6ED9EBA1; a = S(a,s); }

    P(A, B, C, D, X[ 0],  3);
    P(D, A, B, C, X[ 8],  9);
    P(C, D, A, B, X[ 4], 11);
    P(B, C, D, A, X[12], 15);
    P(A, B, C, D, X[ 2],  3);
    P(D, A, B, C, X[10],  9);
    P(C, D, A, B, X[ 6], 11);
    P(B, C, D, A, X[14], 15);
    P(A, B, C, D, X[ 1],  3);
    P(D, A, B, C, X[ 9],  9);
    P(C, D, A, B, X[ 5], 11);
    P(B, C, D, A, X[13], 15);
    P(A, B, C, D, X[ 3],  3);
    P(D, A, B, C, X[11],  9);
    P(C, D, A, B, X[ 7], 11);
    P(B, C, D, A, X[15], 15);

#undef F
#undef P

    ctx->state[0] += A;
    ctx->state[1] += B;
    ctx->state[2] += C;
    ctx->state[3] += D;
}

/*
 * MD4 process buffer
 */
void cel_md4_update(CelMd4Context *ctx, const BYTE *input, size_t ilen)
{
    size_t fill;
    unsigned long left;

    if (ilen <= 0)
        return;

    left = ctx->total[0] & 0x3F;
    fill = 64 - left;

    ctx->total[0] += (unsigned long)ilen;
    ctx->total[0] &= 0xFFFFFFFF;

    if (ctx->total[0] < (unsigned long)ilen)
        ctx->total[1]++;

    if (left && ilen >= fill)
    {
        memcpy((void *)(ctx->buffer + left),
                (void *)input, fill);
        cel_md4_process(ctx, ctx->buffer);
        input += fill;
        ilen  -= fill;
        left = 0;
    }

    while (ilen >= 64)
    {
        cel_md4_process(ctx, input);
        input += 64;
        ilen  -= 64;
    }

    if (ilen > 0)
    {
        memcpy((void *)(ctx->buffer + left),
                (void *)input, ilen);
    }
}

static const BYTE md4_padding[64] =
{
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
 * MD4 final digest
 */
void cel_md4_finish(CelMd4Context *ctx, BYTE output[16])
{
    unsigned long last, padn;
    unsigned long high, low;
    BYTE msglen[8];

    high = (ctx->total[0] >> 29)
         | (ctx->total[1] <<  3);
    low  = (ctx->total[0] <<  3);

    CEL_PUT_LE32(low,  msglen, 0);
    CEL_PUT_LE32(high, msglen, 4);

    last = ctx->total[0] & 0x3F;
    padn = (last < 56)? (56 - last): (120 - last);

    cel_md4_update(ctx, (BYTE *)md4_padding, padn);
    cel_md4_update(ctx, msglen, 8);

    CEL_PUT_LE32(ctx->state[0], output,  0);
    CEL_PUT_LE32(ctx->state[1], output,  4);
    CEL_PUT_LE32(ctx->state[2], output,  8);
    CEL_PUT_LE32(ctx->state[3], output, 12);
}

/*
 * output = MD4(input buffer)
 */
void cel_md4(const BYTE *input, size_t ilen, BYTE output[16])
{
    CelMd4Context ctx;

    cel_md4_starts(&ctx);
    cel_md4_update(&ctx, input, ilen);
    cel_md4_finish(&ctx, output);

    memset(&ctx, 0, sizeof(CelMd4Context));
}

/*
 * MD4 HMAC context setup
 */
void cel_md4_hmac_starts(CelMd4Context *ctx, const BYTE *key, size_t keylen)
{
    size_t i;
    BYTE sum[16];

    if (keylen > 64)
    {
        cel_md4(key, keylen, sum);
        keylen = 16;
        key = sum;
    }

    memset(ctx->ipad, 0x36, 64);
    memset(ctx->opad, 0x5C, 64);

    for(i = 0; i < keylen; i++)
    {
        ctx->ipad[i] = (BYTE) (ctx->ipad[i] ^ key[i]);
        ctx->opad[i] = (BYTE) (ctx->opad[i] ^ key[i]);
    }

    cel_md4_starts(ctx);
    cel_md4_update(ctx, ctx->ipad, 64);

    memset(sum, 0, sizeof(sum));
}

/*
 * MD4 HMAC process buffer
 */
void cel_md4_hmac_update(CelMd4Context *ctx, const BYTE *input, size_t ilen)
{
    cel_md4_update(ctx, input, ilen);
}

/*
 * MD4 HMAC final digest
 */
void cel_md4_hmac_finish(CelMd4Context *ctx, BYTE output[16])
{
    BYTE tmpbuf[16];

    cel_md4_finish(ctx, tmpbuf);
    cel_md4_starts(ctx);
    cel_md4_update(ctx, ctx->opad, 64);
    cel_md4_update(ctx, tmpbuf, 16);
    cel_md4_finish(ctx, output);

    memset(tmpbuf, 0, sizeof(tmpbuf));
}

/*
 * MD4 HMAC context reset
 */
void cel_md4_hmac_reset(CelMd4Context *ctx)
{
    cel_md4_starts(ctx);
    cel_md4_update(ctx, ctx->ipad, 64);
}

/*
 * output = HMAC-MD4(hmac key, input buffer)
 */
void cel_md4_hmac(const BYTE *key, size_t keylen,
                  const BYTE *input, size_t ilen,
                  BYTE output[16])
{
    CelMd4Context ctx;

    cel_md4_hmac_starts(&ctx, key, keylen);
    cel_md4_hmac_update(&ctx, input, ilen);
    cel_md4_hmac_finish(&ctx, output);

    memset(&ctx, 0, sizeof(CelMd4Context));
}
