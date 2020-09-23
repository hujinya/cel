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
#include "cel/crypto/sha2.h"
#include "cel/byteorder.h"

/*
 * SHA-256 context setup
 */
void cel_sha2_starts(CelSha2Context *ctx, int is224)
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    if (is224 == 0)
    {
        /* SHA-256 */
        ctx->state[0] = 0x6A09E667;
        ctx->state[1] = 0xBB67AE85;
        ctx->state[2] = 0x3C6EF372;
        ctx->state[3] = 0xA54FF53A;
        ctx->state[4] = 0x510E527F;
        ctx->state[5] = 0x9B05688C;
        ctx->state[6] = 0x1F83D9AB;
        ctx->state[7] = 0x5BE0CD19;
    }
    else
    {
        /* SHA-224 */
        ctx->state[0] = 0xC1059ED8;
        ctx->state[1] = 0x367CD507;
        ctx->state[2] = 0x3070DD17;
        ctx->state[3] = 0xF70E5939;
        ctx->state[4] = 0xFFC00B31;
        ctx->state[5] = 0x68581511;
        ctx->state[6] = 0x64F98FA7;
        ctx->state[7] = 0xBEFA4FA4;
    }

    ctx->is224 = is224;
}

static void cel_sha2_process(CelSha2Context *ctx, const BYTE data[64])
{
    unsigned long temp1, temp2, W[64];
    unsigned long A, B, C, D, E, F, G, H;

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

#define SHR(x,n) ((x & 0xFFFFFFFF) >> n)
#define ROTR(x,n) (SHR(x,n) | (x << (32 - n)))

#define S0(x) (ROTR(x, 7)^ ROTR(x,18)^  SHR(x, 3))
#define S1(x) (ROTR(x,17)^ ROTR(x,19)^  SHR(x,10))

#define S2(x) (ROTR(x, 2)^ ROTR(x,13)^ ROTR(x,22))
#define S3(x) (ROTR(x, 6)^ ROTR(x,11)^ ROTR(x,25))

#define F0(x,y,z) ((x & y) | (z & (x | y)))
#define F1(x,y,z) (z ^ (x & (y ^ z)))

#define R(t)                                   \
(                                               \
    W[t] = S1(W[t -  2])+ W[t -  7] +          \
           S0(W[t - 15])+ W[t - 16]            \
)

#define P(a,b,c,d,e,f,g,h,x,K)                 \
{                                               \
    temp1 = h + S3(e)+ F1(e,f,g)+ K + x;      \
    temp2 = S2(a)+ F0(a,b,c);                  \
    d += temp1; h = temp1 + temp2;              \
}

    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];
    E = ctx->state[4];
    F = ctx->state[5];
    G = ctx->state[6];
    H = ctx->state[7];

    P(A, B, C, D, E, F, G, H, W[ 0], 0x428A2F98);
    P(H, A, B, C, D, E, F, G, W[ 1], 0x71374491);
    P(G, H, A, B, C, D, E, F, W[ 2], 0xB5C0FBCF);
    P(F, G, H, A, B, C, D, E, W[ 3], 0xE9B5DBA5);
    P(E, F, G, H, A, B, C, D, W[ 4], 0x3956C25B);
    P(D, E, F, G, H, A, B, C, W[ 5], 0x59F111F1);
    P(C, D, E, F, G, H, A, B, W[ 6], 0x923F82A4);
    P(B, C, D, E, F, G, H, A, W[ 7], 0xAB1C5ED5);
    P(A, B, C, D, E, F, G, H, W[ 8], 0xD807AA98);
    P(H, A, B, C, D, E, F, G, W[ 9], 0x12835B01);
    P(G, H, A, B, C, D, E, F, W[10], 0x243185BE);
    P(F, G, H, A, B, C, D, E, W[11], 0x550C7DC3);
    P(E, F, G, H, A, B, C, D, W[12], 0x72BE5D74);
    P(D, E, F, G, H, A, B, C, W[13], 0x80DEB1FE);
    P(C, D, E, F, G, H, A, B, W[14], 0x9BDC06A7);
    P(B, C, D, E, F, G, H, A, W[15], 0xC19BF174);
    P(A, B, C, D, E, F, G, H, R(16), 0xE49B69C1);
    P(H, A, B, C, D, E, F, G, R(17), 0xEFBE4786);
    P(G, H, A, B, C, D, E, F, R(18), 0x0FC19DC6);
    P(F, G, H, A, B, C, D, E, R(19), 0x240CA1CC);
    P(E, F, G, H, A, B, C, D, R(20), 0x2DE92C6F);
    P(D, E, F, G, H, A, B, C, R(21), 0x4A7484AA);
    P(C, D, E, F, G, H, A, B, R(22), 0x5CB0A9DC);
    P(B, C, D, E, F, G, H, A, R(23), 0x76F988DA);
    P(A, B, C, D, E, F, G, H, R(24), 0x983E5152);
    P(H, A, B, C, D, E, F, G, R(25), 0xA831C66D);
    P(G, H, A, B, C, D, E, F, R(26), 0xB00327C8);
    P(F, G, H, A, B, C, D, E, R(27), 0xBF597FC7);
    P(E, F, G, H, A, B, C, D, R(28), 0xC6E00BF3);
    P(D, E, F, G, H, A, B, C, R(29), 0xD5A79147);
    P(C, D, E, F, G, H, A, B, R(30), 0x06CA6351);
    P(B, C, D, E, F, G, H, A, R(31), 0x14292967);
    P(A, B, C, D, E, F, G, H, R(32), 0x27B70A85);
    P(H, A, B, C, D, E, F, G, R(33), 0x2E1B2138);
    P(G, H, A, B, C, D, E, F, R(34), 0x4D2C6DFC);
    P(F, G, H, A, B, C, D, E, R(35), 0x53380D13);
    P(E, F, G, H, A, B, C, D, R(36), 0x650A7354);
    P(D, E, F, G, H, A, B, C, R(37), 0x766A0ABB);
    P(C, D, E, F, G, H, A, B, R(38), 0x81C2C92E);
    P(B, C, D, E, F, G, H, A, R(39), 0x92722C85);
    P(A, B, C, D, E, F, G, H, R(40), 0xA2BFE8A1);
    P(H, A, B, C, D, E, F, G, R(41), 0xA81A664B);
    P(G, H, A, B, C, D, E, F, R(42), 0xC24B8B70);
    P(F, G, H, A, B, C, D, E, R(43), 0xC76C51A3);
    P(E, F, G, H, A, B, C, D, R(44), 0xD192E819);
    P(D, E, F, G, H, A, B, C, R(45), 0xD6990624);
    P(C, D, E, F, G, H, A, B, R(46), 0xF40E3585);
    P(B, C, D, E, F, G, H, A, R(47), 0x106AA070);
    P(A, B, C, D, E, F, G, H, R(48), 0x19A4C116);
    P(H, A, B, C, D, E, F, G, R(49), 0x1E376C08);
    P(G, H, A, B, C, D, E, F, R(50), 0x2748774C);
    P(F, G, H, A, B, C, D, E, R(51), 0x34B0BCB5);
    P(E, F, G, H, A, B, C, D, R(52), 0x391C0CB3);
    P(D, E, F, G, H, A, B, C, R(53), 0x4ED8AA4A);
    P(C, D, E, F, G, H, A, B, R(54), 0x5B9CCA4F);
    P(B, C, D, E, F, G, H, A, R(55), 0x682E6FF3);
    P(A, B, C, D, E, F, G, H, R(56), 0x748F82EE);
    P(H, A, B, C, D, E, F, G, R(57), 0x78A5636F);
    P(G, H, A, B, C, D, E, F, R(58), 0x84C87814);
    P(F, G, H, A, B, C, D, E, R(59), 0x8CC70208);
    P(E, F, G, H, A, B, C, D, R(60), 0x90BEFFFA);
    P(D, E, F, G, H, A, B, C, R(61), 0xA4506CEB);
    P(C, D, E, F, G, H, A, B, R(62), 0xBEF9A3F7);
    P(B, C, D, E, F, G, H, A, R(63), 0xC67178F2);

    ctx->state[0] += A;
    ctx->state[1] += B;
    ctx->state[2] += C;
    ctx->state[3] += D;
    ctx->state[4] += E;
    ctx->state[5] += F;
    ctx->state[6] += G;
    ctx->state[7] += H;
}

/*
 * SHA-256 process buffer
 */
void cel_sha2_update(CelSha2Context *ctx, const BYTE *input, size_t ilen)
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
        memcpy((void *)(ctx->buffer + left), (void *)input, fill);
        cel_sha2_process(ctx, ctx->buffer);
        input += fill;
        ilen  -= fill;
        left = 0;
    }

    while (ilen >= 64)
    {
        cel_sha2_process(ctx, input);
        input += 64;
        ilen  -= 64;
    }

    if (ilen > 0)
    {
        memcpy((void *)(ctx->buffer + left), (void *)input, ilen);
    }
}

static const BYTE sha2_padding[64] =
{
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
 * SHA-256 final digest
 */
void cel_sha2_finish(CelSha2Context *ctx, BYTE output[32])
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

    cel_sha2_update(ctx, (BYTE *)sha2_padding, padn);
    cel_sha2_update(ctx, msglen, 8);

    CEL_PUT_BE32(ctx->state[0], output,  0);
    CEL_PUT_BE32(ctx->state[1], output,  4);
    CEL_PUT_BE32(ctx->state[2], output,  8);
    CEL_PUT_BE32(ctx->state[3], output, 12);
    CEL_PUT_BE32(ctx->state[4], output, 16);
    CEL_PUT_BE32(ctx->state[5], output, 20);
    CEL_PUT_BE32(ctx->state[6], output, 24);

    if (ctx->is224 == 0)
        CEL_PUT_BE32(ctx->state[7], output, 28);
}

/*
 * output = SHA-256(input buffer)
 */
void cel_sha2(const BYTE *input, size_t ilen,
              BYTE output[32], int is224)
{
    CelSha2Context ctx;

    cel_sha2_starts(&ctx, is224);
    cel_sha2_update(&ctx, input, ilen);
    cel_sha2_finish(&ctx, output);

    memset(&ctx, 0, sizeof(CelSha2Context));
}

/*
 * SHA-256 HMAC context setup
 */
void cel_sha2_hmac_starts(CelSha2Context *ctx, const BYTE *key, size_t keylen,
                          int is224)
{
    size_t i;
    BYTE sum[32];

    if (keylen > 64)
    {
        cel_sha2(key, keylen, sum, is224);
        keylen = (is224)? 28 : 32;
        key = sum;
    }

    memset(ctx->ipad, 0x36, 64);
    memset(ctx->opad, 0x5C, 64);

    for(i = 0; i < keylen; i++)
    {
        ctx->ipad[i] = (BYTE) (ctx->ipad[i] ^ key[i]);
        ctx->opad[i] = (BYTE) (ctx->opad[i] ^ key[i]);
    }

    cel_sha2_starts(ctx, is224);
    cel_sha2_update(ctx, ctx->ipad, 64);

    //memset(sum, 0, sizeof(sum));
}

/*
 * SHA-256 HMAC process buffer
 */
void cel_sha2_hmac_update(CelSha2Context *ctx, const BYTE *input, size_t ilen)
{
    cel_sha2_update(ctx, input, ilen);
}

/*
 * SHA-256 HMAC final digest
 */
void cel_sha2_hmac_finish(CelSha2Context *ctx, BYTE output[32])
{
    int is224, hlen;
    BYTE tmpbuf[32];

    is224 = ctx->is224;
    hlen = (is224 == 0)? 32 : 28;

    cel_sha2_finish(ctx, tmpbuf);
    cel_sha2_starts(ctx, is224);
    cel_sha2_update(ctx, ctx->opad, 64);
    cel_sha2_update(ctx, tmpbuf, hlen);
    cel_sha2_finish(ctx, output);

    //memset(tmpbuf, 0, sizeof(tmpbuf));
}

/*
 * SHA-256 HMAC context reset
 */
void cel_sha2_hmac_reset(CelSha2Context *ctx)
{
    cel_sha2_starts(ctx, ctx->is224);
    cel_sha2_update(ctx, ctx->ipad, 64);
}

/*
 * output = HMAC-SHA-256(hmac key, input buffer)
 */
void cel_sha2_hmac(const BYTE *key, size_t keylen,
                   const BYTE *input, size_t ilen,
                   BYTE output[32], int is224)
{
    CelSha2Context ctx;

    cel_sha2_hmac_starts(&ctx, key, keylen, is224);
    cel_sha2_hmac_update(&ctx, input, ilen);
    cel_sha2_hmac_finish(&ctx, output);

    memset(&ctx, 0, sizeof(CelSha2Context));
}
