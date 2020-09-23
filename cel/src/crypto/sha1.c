/**
 * CEL(C Extension Library)
 * Copyright (C)2008 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/crypto/sha1.h"
#include "cel/byteorder.h"

/*
 * SHA-1 context setup
 */
void cel_sha1_starts(CelSha1Context *ctx)
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xC3D2E1F0;
}

static void cel_sha1_process(CelSha1Context *ctx, const BYTE data[64])
{
    unsigned long temp, W[16], A, B, C, D, E;

    CEL_GET_BE32(W[ 0], data,  0);
    CEL_GET_BE32(W[ 1], data,  4);
    CEL_GET_BE32(W[ 2], data,  8);
    CEL_GET_BE32(W[ 3], data, 12);
    CEL_GET_BE32(W[ 4], data, 16);
    CEL_GET_BE32(W[ 5], data, 20);
    CEL_GET_BE32(W[ 6], data, 24);
    CEL_GET_BE32(W[ 7], data, 28);
    CEL_GET_BE32(W[ 8], data, 32);
    CEL_GET_BE32(W[ 9], data, 36);
    CEL_GET_BE32(W[10], data, 40);
    CEL_GET_BE32(W[11], data, 44);
    CEL_GET_BE32(W[12], data, 48);
    CEL_GET_BE32(W[13], data, 52);
    CEL_GET_BE32(W[14], data, 56);
    CEL_GET_BE32(W[15], data, 60);

#define S(x,n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define R(t)                                           \
(                                                    \
    temp = W[(t -  3) & 0x0F] ^ W[(t - 8) & 0x0F] ^     \
           W[(t - 14) & 0x0F] ^ W[ t      & 0x0F],      \
    (W[t & 0x0F] = S(temp,1))                        \
)

#define P(a,b,c,d,e,x)                                 \
{                                                       \
    e += S(a,5)+ F(b,c,d)+ K + x; b = S(b,30);        \
}

    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];
    E = ctx->state[4];

#define F(x,y,z) (z ^ (x & (y ^ z)))
#define K 0x5A827999

    P(A, B, C, D, E, W[0]);
    P(E, A, B, C, D, W[1]);
    P(D, E, A, B, C, W[2]);
    P(C, D, E, A, B, W[3]);
    P(B, C, D, E, A, W[4]);
    P(A, B, C, D, E, W[5]);
    P(E, A, B, C, D, W[6]);
    P(D, E, A, B, C, W[7]);
    P(C, D, E, A, B, W[8]);
    P(B, C, D, E, A, W[9]);
    P(A, B, C, D, E, W[10]);
    P(E, A, B, C, D, W[11]);
    P(D, E, A, B, C, W[12]);
    P(C, D, E, A, B, W[13]);
    P(B, C, D, E, A, W[14]);
    P(A, B, C, D, E, W[15]);
    P(E, A, B, C, D, R(16));
    P(D, E, A, B, C, R(17));
    P(C, D, E, A, B, R(18));
    P(B, C, D, E, A, R(19));

#undef K
#undef F

#define F(x,y,z) (x ^ y ^ z)
#define K 0x6ED9EBA1

    P(A, B, C, D, E, R(20));
    P(E, A, B, C, D, R(21));
    P(D, E, A, B, C, R(22));
    P(C, D, E, A, B, R(23));
    P(B, C, D, E, A, R(24));
    P(A, B, C, D, E, R(25));
    P(E, A, B, C, D, R(26));
    P(D, E, A, B, C, R(27));
    P(C, D, E, A, B, R(28));
    P(B, C, D, E, A, R(29));
    P(A, B, C, D, E, R(30));
    P(E, A, B, C, D, R(31));
    P(D, E, A, B, C, R(32));
    P(C, D, E, A, B, R(33));
    P(B, C, D, E, A, R(34));
    P(A, B, C, D, E, R(35));
    P(E, A, B, C, D, R(36));
    P(D, E, A, B, C, R(37));
    P(C, D, E, A, B, R(38));
    P(B, C, D, E, A, R(39));

#undef K
#undef F

#define F(x,y,z) ((x & y) | (z & (x | y)))
#define K 0x8F1BBCDC

    P(A, B, C, D, E, R(40));
    P(E, A, B, C, D, R(41));
    P(D, E, A, B, C, R(42));
    P(C, D, E, A, B, R(43));
    P(B, C, D, E, A, R(44));
    P(A, B, C, D, E, R(45));
    P(E, A, B, C, D, R(46));
    P(D, E, A, B, C, R(47));
    P(C, D, E, A, B, R(48));
    P(B, C, D, E, A, R(49));
    P(A, B, C, D, E, R(50));
    P(E, A, B, C, D, R(51));
    P(D, E, A, B, C, R(52));
    P(C, D, E, A, B, R(53));
    P(B, C, D, E, A, R(54));
    P(A, B, C, D, E, R(55));
    P(E, A, B, C, D, R(56));
    P(D, E, A, B, C, R(57));
    P(C, D, E, A, B, R(58));
    P(B, C, D, E, A, R(59));

#undef K
#undef F

#define F(x,y,z) (x ^ y ^ z)
#define K 0xCA62C1D6

    P(A, B, C, D, E, R(60));
    P(E, A, B, C, D, R(61));
    P(D, E, A, B, C, R(62));
    P(C, D, E, A, B, R(63));
    P(B, C, D, E, A, R(64));
    P(A, B, C, D, E, R(65));
    P(E, A, B, C, D, R(66));
    P(D, E, A, B, C, R(67));
    P(C, D, E, A, B, R(68));
    P(B, C, D, E, A, R(69));
    P(A, B, C, D, E, R(70));
    P(E, A, B, C, D, R(71));
    P(D, E, A, B, C, R(72));
    P(C, D, E, A, B, R(73));
    P(B, C, D, E, A, R(74));
    P(A, B, C, D, E, R(75));
    P(E, A, B, C, D, R(76));
    P(D, E, A, B, C, R(77));
    P(C, D, E, A, B, R(78));
    P(B, C, D, E, A, R(79));

#undef K
#undef F

    ctx->state[0] += A;
    ctx->state[1] += B;
    ctx->state[2] += C;
    ctx->state[3] += D;
    ctx->state[4] += E;
}

/*
 * SHA-1 process buffer
 */
void cel_sha1_update(CelSha1Context *ctx, const BYTE *input, size_t ilen)
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
        cel_sha1_process(ctx, ctx->buffer);
        input += fill;
        ilen  -= fill;
        left = 0;
    }

    while (ilen >= 64)
    {
        cel_sha1_process(ctx, input);
        input += 64;
        ilen  -= 64;
    }

    if (ilen > 0)
    {
        memcpy((void *)(ctx->buffer + left),
                (void *)input, ilen);
    }
}

static const BYTE sha1_padding[64] =
{
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
 * SHA-1 final digest
 */
void cel_sha1_finish(CelSha1Context *ctx, BYTE output[20])
{
    unsigned long last, padn;
    unsigned long high, low;
    BYTE msglen[8];

    high = (ctx->total[0] >> 29)
         | (ctx->total[1] <<  3);
    low  = (ctx->total[0] <<  3);

    CEL_PUT_BE32(high, msglen, 0);
    CEL_PUT_BE32(low,  msglen, 4);

    last = ctx->total[0] & 0x3F;
    padn = (last < 56)? (56 - last): (120 - last);

    cel_sha1_update(ctx, (BYTE *)sha1_padding, padn);
    cel_sha1_update(ctx, msglen, 8);

    CEL_PUT_BE32(ctx->state[0], output,  0);
    CEL_PUT_BE32(ctx->state[1], output,  4);
    CEL_PUT_BE32(ctx->state[2], output,  8);
    CEL_PUT_BE32(ctx->state[3], output, 12);
    CEL_PUT_BE32(ctx->state[4], output, 16);
}

/*
 * output = SHA-1(input buffer)
 */
void cel_sha1(const BYTE *input, size_t ilen, BYTE output[20])
{
    CelSha1Context ctx;

    cel_sha1_starts(&ctx);
    cel_sha1_update(&ctx, input, ilen);
    cel_sha1_finish(&ctx, output);

    memset(&ctx, 0, sizeof(CelSha1Context));
}

/*
 * SHA-1 HMAC context setup
 */
void cel_sha1_hmac_starts(CelSha1Context *ctx, const BYTE *key, size_t keylen)
{
    size_t i;
    BYTE sum[20];

    if (keylen > 64)
    {
        cel_sha1(key, keylen, sum);
        keylen = 20;
        key = sum;
    }

    memset(ctx->ipad, 0x36, 64);
    memset(ctx->opad, 0x5C, 64);

    for(i = 0; i < keylen; i++)
    {
        ctx->ipad[i] = (BYTE) (ctx->ipad[i] ^ key[i]);
        ctx->opad[i] = (BYTE) (ctx->opad[i] ^ key[i]);
    }

    cel_sha1_starts(ctx);
    cel_sha1_update(ctx, ctx->ipad, 64);

    memset(sum, 0, sizeof(sum));
}

/*
 * SHA-1 HMAC process buffer
 */
void cel_sha1_hmac_update(CelSha1Context *ctx, const BYTE *input, size_t ilen)
{
    cel_sha1_update(ctx, input, ilen);
}

/*
 * SHA-1 HMAC final digest
 */
void cel_sha1_hmac_finish(CelSha1Context *ctx, BYTE output[20])
{
    BYTE tmpbuf[20];

    cel_sha1_finish(ctx, tmpbuf);
    cel_sha1_starts(ctx);
    cel_sha1_update(ctx, ctx->opad, 64);
    cel_sha1_update(ctx, tmpbuf, 20);
    cel_sha1_finish(ctx, output);

    memset(tmpbuf, 0, sizeof(tmpbuf));
}

/*
 * SHA1 HMAC context reset
 */
void cel_sha1_hmac_reset(CelSha1Context *ctx)
{
    cel_sha1_starts(ctx);
    cel_sha1_update(ctx, ctx->ipad, 64);
}

/*
 * output = HMAC-SHA-1(hmac key, input buffer)
 */
void cel_sha1_hmac(const BYTE *key, size_t keylen,
                   const BYTE *input, size_t ilen,
                   BYTE output[20])
{
    CelSha1Context ctx;

    cel_sha1_hmac_starts(&ctx, key, keylen);
    cel_sha1_hmac_update(&ctx, input, ilen);
    cel_sha1_hmac_finish(&ctx, output);

    memset(&ctx, 0, sizeof(CelSha1Context));
}
