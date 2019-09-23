/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2019 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/crypto/md5.h"
#include "cel/byteorder.h"
/*
 *  The MD5 algorithm was designed by Ron Rivest in 1991.
 *  http://www.ietf.org/rfc/rfc1321.txt
 */

/*
 * MD5 context setup
 */
void cel_md5_starts(CelMd5Context *ctx)
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
}

static void cel_md5_process(CelMd5Context *ctx, const BYTE data[64])
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

#define S(x, n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define P(a, b, c, d, k, s, t)                         \
{                                                       \
    a += F(b, c, d)+ X[k] + t; a = S(a, s)+ b;        \
}

    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];

#define F(x, y, z) (z ^ (x & (y ^ z)))

    P(A, B, C, D,  0,  7, 0xD76AA478);
    P(D, A, B, C,  1, 12, 0xE8C7B756);
    P(C, D, A, B,  2, 17, 0x242070DB);
    P(B, C, D, A,  3, 22, 0xC1BDCEEE);
    P(A, B, C, D,  4,  7, 0xF57C0FAF);
    P(D, A, B, C,  5, 12, 0x4787C62A);
    P(C, D, A, B,  6, 17, 0xA8304613);
    P(B, C, D, A,  7, 22, 0xFD469501);
    P(A, B, C, D,  8,  7, 0x698098D8);
    P(D, A, B, C,  9, 12, 0x8B44F7AF);
    P(C, D, A, B, 10, 17, 0xFFFF5BB1);
    P(B, C, D, A, 11, 22, 0x895CD7BE);
    P(A, B, C, D, 12,  7, 0x6B901122);
    P(D, A, B, C, 13, 12, 0xFD987193);
    P(C, D, A, B, 14, 17, 0xA679438E);
    P(B, C, D, A, 15, 22, 0x49B40821);

#undef F

#define F(x, y, z) (y ^ (z & (x ^ y)))

    P(A, B, C, D,  1,  5, 0xF61E2562);
    P(D, A, B, C,  6,  9, 0xC040B340);
    P(C, D, A, B, 11, 14, 0x265E5A51);
    P(B, C, D, A,  0, 20, 0xE9B6C7AA);
    P(A, B, C, D,  5,  5, 0xD62F105D);
    P(D, A, B, C, 10,  9, 0x02441453);
    P(C, D, A, B, 15, 14, 0xD8A1E681);
    P(B, C, D, A,  4, 20, 0xE7D3FBC8);
    P(A, B, C, D,  9,  5, 0x21E1CDE6);
    P(D, A, B, C, 14,  9, 0xC33707D6);
    P(C, D, A, B,  3, 14, 0xF4D50D87);
    P(B, C, D, A,  8, 20, 0x455A14ED);
    P(A, B, C, D, 13,  5, 0xA9E3E905);
    P(D, A, B, C,  2,  9, 0xFCEFA3F8);
    P(C, D, A, B,  7, 14, 0x676F02D9);
    P(B, C, D, A, 12, 20, 0x8D2A4C8A);

#undef F
    
#define F(x, y, z) (x ^ y ^ z)

    P(A, B, C, D,  5,  4, 0xFFFA3942);
    P(D, A, B, C,  8, 11, 0x8771F681);
    P(C, D, A, B, 11, 16, 0x6D9D6122);
    P(B, C, D, A, 14, 23, 0xFDE5380C);
    P(A, B, C, D,  1,  4, 0xA4BEEA44);
    P(D, A, B, C,  4, 11, 0x4BDECFA9);
    P(C, D, A, B,  7, 16, 0xF6BB4B60);
    P(B, C, D, A, 10, 23, 0xBEBFBC70);
    P(A, B, C, D, 13,  4, 0x289B7EC6);
    P(D, A, B, C,  0, 11, 0xEAA127FA);
    P(C, D, A, B,  3, 16, 0xD4EF3085);
    P(B, C, D, A,  6, 23, 0x04881D05);
    P(A, B, C, D,  9,  4, 0xD9D4D039);
    P(D, A, B, C, 12, 11, 0xE6DB99E5);
    P(C, D, A, B, 15, 16, 0x1FA27CF8);
    P(B, C, D, A,  2, 23, 0xC4AC5665);

#undef F

#define F(x, y, z) (y ^ (x | ~z))

    P(A, B, C, D,  0,  6, 0xF4292244);
    P(D, A, B, C,  7, 10, 0x432AFF97);
    P(C, D, A, B, 14, 15, 0xAB9423A7);
    P(B, C, D, A,  5, 21, 0xFC93A039);
    P(A, B, C, D, 12,  6, 0x655B59C3);
    P(D, A, B, C,  3, 10, 0x8F0CCC92);
    P(C, D, A, B, 10, 15, 0xFFEFF47D);
    P(B, C, D, A,  1, 21, 0x85845DD1);
    P(A, B, C, D,  8,  6, 0x6FA87E4F);
    P(D, A, B, C, 15, 10, 0xFE2CE6E0);
    P(C, D, A, B,  6, 15, 0xA3014314);
    P(B, C, D, A, 13, 21, 0x4E0811A1);
    P(A, B, C, D,  4,  6, 0xF7537E82);
    P(D, A, B, C, 11, 10, 0xBD3AF235);
    P(C, D, A, B,  2, 15, 0x2AD7D2BB);
    P(B, C, D, A,  9, 21, 0xEB86D391);

#undef F

    ctx->state[0] += A;
    ctx->state[1] += B;
    ctx->state[2] += C;
    ctx->state[3] += D;
}

/*
 * MD5 process buffer
 */
void cel_md5_update(CelMd5Context *ctx, const BYTE *input, size_t ilen)
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
        cel_md5_process(ctx, ctx->buffer);
        input += fill;
        ilen  -= fill;
        left = 0;
    }

    while (ilen >= 64)
    {
        cel_md5_process(ctx, input);
        input += 64;
        ilen  -= 64;
    }

    if (ilen > 0)
    {
        memcpy((void *)(ctx->buffer + left), 
                (void *)input, ilen);
    }
}

static const BYTE md5_padding[64] =
{
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
 * MD5 final digest
 */
void cel_md5_finish(CelMd5Context *ctx, BYTE output[16])
{
    unsigned long last, padn;
    unsigned long high, low;
    BYTE msglen[8];

    high = (ctx->total[0] >> 29) | (ctx->total[1] <<  3);
    low  = (ctx->total[0] <<  3);

    CEL_PUT_LE32(low, msglen, 0);
    CEL_PUT_LE32(high, msglen, 4);

    last = ctx->total[0] & 0x3F;
    padn = (last < 56)? (56 - last): (120 - last);

    cel_md5_update(ctx, (BYTE *)md5_padding, padn);
    cel_md5_update(ctx, msglen, 8);

    CEL_PUT_LE32(ctx->state[0], output, 0);
    CEL_PUT_LE32(ctx->state[1], output, 4);
    CEL_PUT_LE32(ctx->state[2], output, 8);
    CEL_PUT_LE32(ctx->state[3], output, 12);
}

/*
 * output = MD5(input buffer)
 */
void cel_md5(const BYTE *input, size_t ilen, BYTE output[16])
{
    CelMd5Context ctx;

    cel_md5_starts(&ctx);
    cel_md5_update(&ctx, input, ilen);
    cel_md5_finish(&ctx, output);

    memset(&ctx, 0, sizeof(CelMd5Context));
}

/*
 * MD5 HMAC context setup
 */
void cel_md5_hmac_starts(CelMd5Context *ctx, const BYTE *key, size_t keylen)
{
    size_t i;
    BYTE sum[16];

    if (keylen > 64)
    {
        cel_md5(key, keylen, sum);
        keylen = 16;
        key = sum;
    }

    memset(ctx->ipad, 0x36, 64);
    memset(ctx->opad, 0x5C, 64);

    for (i = 0; i < keylen; i++)
    {
        ctx->ipad[i] = (BYTE) (ctx->ipad[i] ^ key[i]);
        ctx->opad[i] = (BYTE) (ctx->opad[i] ^ key[i]);
    }

    cel_md5_starts(ctx);
    cel_md5_update(ctx, ctx->ipad, 64);

    //memset(sum, 0, sizeof(sum));
}

/*
 * MD5 HMAC process buffer
 */
void cel_md5_hmac_update(CelMd5Context *ctx, const BYTE *input, size_t ilen)
{
    cel_md5_update(ctx, input, ilen);
}

/*
 * MD5 HMAC final digest
 */
void cel_md5_hmac_finish(CelMd5Context *ctx, BYTE output[16])
{
    BYTE tmpbuf[16];

    cel_md5_finish(ctx, tmpbuf);
    cel_md5_starts(ctx);
    cel_md5_update(ctx, ctx->opad, 64);
    cel_md5_update(ctx, tmpbuf, 16);
    cel_md5_finish(ctx, output);

    //memset(tmpbuf, 0, sizeof(tmpbuf));
}

/*
 * MD5 HMAC context reset
 */
void cel_md5_hmac_reset(CelMd5Context *ctx)
{
    cel_md5_starts(ctx);
    cel_md5_update(ctx, ctx->ipad, 64);
}

/*
 * output = HMAC-MD5(hmac key, input buffer)
 */
void cel_md5_hmac(const BYTE *key, size_t keylen, 
                  const BYTE *input, size_t ilen, 
                  BYTE output[16])
{
    CelMd5Context ctx;

    cel_md5_hmac_starts(&ctx, key, keylen);
    cel_md5_hmac_update(&ctx, input, ilen);
    cel_md5_hmac_finish(&ctx, output);

    memset(&ctx, 0, sizeof(CelMd5Context));
}
