/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2016 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/crypto/sha4.h"
#include "cel/byteorder.h"
/*
 * Round constants
 */
static const U64 K[80] =
{
    ULL(0x428A2F98D728AE22),  ULL(0x7137449123EF65CD),
    ULL(0xB5C0FBCFEC4D3B2F),  ULL(0xE9B5DBA58189DBBC),
    ULL(0x3956C25BF348B538),  ULL(0x59F111F1B605D019),
    ULL(0x923F82A4AF194F9B),  ULL(0xAB1C5ED5DA6D8118),
    ULL(0xD807AA98A3030242),  ULL(0x12835B0145706FBE),
    ULL(0x243185BE4EE4B28C),  ULL(0x550C7DC3D5FFB4E2),
    ULL(0x72BE5D74F27B896F),  ULL(0x80DEB1FE3B1696B1),
    ULL(0x9BDC06A725C71235),  ULL(0xC19BF174CF692694),
    ULL(0xE49B69C19EF14AD2),  ULL(0xEFBE4786384F25E3),
    ULL(0x0FC19DC68B8CD5B5),  ULL(0x240CA1CC77AC9C65),
    ULL(0x2DE92C6F592B0275),  ULL(0x4A7484AA6EA6E483),
    ULL(0x5CB0A9DCBD41FBD4),  ULL(0x76F988DA831153B5),
    ULL(0x983E5152EE66DFAB),  ULL(0xA831C66D2DB43210),
    ULL(0xB00327C898FB213F),  ULL(0xBF597FC7BEEF0EE4),
    ULL(0xC6E00BF33DA88FC2),  ULL(0xD5A79147930AA725),
    ULL(0x06CA6351E003826F),  ULL(0x142929670A0E6E70),
    ULL(0x27B70A8546D22FFC),  ULL(0x2E1B21385C26C926),
    ULL(0x4D2C6DFC5AC42AED),  ULL(0x53380D139D95B3DF),
    ULL(0x650A73548BAF63DE),  ULL(0x766A0ABB3C77B2A8),
    ULL(0x81C2C92E47EDAEE6),  ULL(0x92722C851482353B),
    ULL(0xA2BFE8A14CF10364),  ULL(0xA81A664BBC423001),
    ULL(0xC24B8B70D0F89791),  ULL(0xC76C51A30654BE30),
    ULL(0xD192E819D6EF5218),  ULL(0xD69906245565A910),
    ULL(0xF40E35855771202A),  ULL(0x106AA07032BBD1B8),
    ULL(0x19A4C116B8D2D0C8),  ULL(0x1E376C085141AB53),
    ULL(0x2748774CDF8EEB99),  ULL(0x34B0BCB5E19B48A8),
    ULL(0x391C0CB3C5C95A63),  ULL(0x4ED8AA4AE3418ACB),
    ULL(0x5B9CCA4F7763E373),  ULL(0x682E6FF3D6B2B8A3),
    ULL(0x748F82EE5DEFB2FC),  ULL(0x78A5636F43172F60),
    ULL(0x84C87814A1F0AB72),  ULL(0x8CC702081A6439EC),
    ULL(0x90BEFFFA23631E28),  ULL(0xA4506CEBDE82BDE9),
    ULL(0xBEF9A3F7B2C67915),  ULL(0xC67178F2E372532B),
    ULL(0xCA273ECEEA26619C),  ULL(0xD186B8C721C0C207),
    ULL(0xEADA7DD6CDE0EB1E),  ULL(0xF57D4F7FEE6ED178),
    ULL(0x06F067AA72176FBA),  ULL(0x0A637DC5A2C898A6),
    ULL(0x113F9804BEF90DAE),  ULL(0x1B710B35131C471B),
    ULL(0x28DB77F523047D84),  ULL(0x32CAAB7B40C72493),
    ULL(0x3C9EBE0A15C9BEBC),  ULL(0x431D67C49C100D4C),
    ULL(0x4CC5D4BECB3E42B6),  ULL(0x597F299CFC657E2A),
    ULL(0x5FCB6FAB3AD6FAEC),  ULL(0x6C44198C4A475817)
};

/*
 * SHA-512 context setup
 */
void cel_sha4_starts(CelSha4Context *ctx, int is384)
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    if (is384 == 0)
    {
        /* SHA-512 */
        ctx->state[0] = ULL(0x6A09E667F3BCC908);
        ctx->state[1] = ULL(0xBB67AE8584CAA73B);
        ctx->state[2] = ULL(0x3C6EF372FE94F82B);
        ctx->state[3] = ULL(0xA54FF53A5F1D36F1);
        ctx->state[4] = ULL(0x510E527FADE682D1);
        ctx->state[5] = ULL(0x9B05688C2B3E6C1F);
        ctx->state[6] = ULL(0x1F83D9ABFB41BD6B);
        ctx->state[7] = ULL(0x5BE0CD19137E2179);
    }
    else
    {
        /* SHA-384 */
        ctx->state[0] = ULL(0xCBBB9D5DC1059ED8);
        ctx->state[1] = ULL(0x629A292A367CD507);
        ctx->state[2] = ULL(0x9159015A3070DD17);
        ctx->state[3] = ULL(0x152FECD8F70E5939);
        ctx->state[4] = ULL(0x67332667FFC00B31);
        ctx->state[5] = ULL(0x8EB44A8768581511);
        ctx->state[6] = ULL(0xDB0C2E0D64F98FA7);
        ctx->state[7] = ULL(0x47B5481DBEFA4FA4);
    }

    ctx->is384 = is384;
}

static void cel_sha4_process(CelSha4Context *ctx, const BYTE data[128])
{
    int i;
    U64 temp1, temp2, W[80];
    U64 A, B, C, D, E, F, G, H;

#define SHR(x ,n) (x >> n)
#define ROTR(x, n) (SHR(x, n) | (x << (64 - n)))

#define S0(x) (ROTR(x, 1)^ ROTR(x, 8)^  SHR(x, 7))
#define S1(x) (ROTR(x,19)^ ROTR(x,61)^  SHR(x, 6))

#define S2(x) (ROTR(x,28)^ ROTR(x,34)^ ROTR(x,39))
#define S3(x) (ROTR(x,14)^ ROTR(x,18)^ ROTR(x,41))

#define F0(x, y, z) ((x & y) | (z & (x | y)))
#define F1(x, y, z) (z ^ (x & (y ^ z)))

#define P(a, b, c, d, e, f, g, h, x, K)  {  \
    temp1 = h + S3(e)+ F1(e,f,g)+ K + x; \
    temp2 = S2(a)+ F0(a,b,c); \
    d += temp1; h = temp1 + temp2;  }

    for(i = 0; i < 16; i++)
    {
        CEL_GET_BE64(W[i], data, i << 3);
    }

    for(; i < 80; i++)
    {
        W[i] = S1(W[i -  2])+ W[i -  7] +
               S0(W[i - 15])+ W[i - 16];
    }

    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];
    E = ctx->state[4];
    F = ctx->state[5];
    G = ctx->state[6];
    H = ctx->state[7];
    i = 0;

    do
    {
        P(A, B, C, D, E, F, G, H, W[i], K[i]); i++;
        P(H, A, B, C, D, E, F, G, W[i], K[i]); i++;
        P(G, H, A, B, C, D, E, F, W[i], K[i]); i++;
        P(F, G, H, A, B, C, D, E, W[i], K[i]); i++;
        P(E, F, G, H, A, B, C, D, W[i], K[i]); i++;
        P(D, E, F, G, H, A, B, C, W[i], K[i]); i++;
        P(C, D, E, F, G, H, A, B, W[i], K[i]); i++;
        P(B, C, D, E, F, G, H, A, W[i], K[i]); i++;
    }while (i < 80);

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
 * SHA-512 process buffer
 */
void cel_sha4_update(CelSha4Context *ctx, const BYTE *input, size_t ilen)
{
    size_t fill;
    unsigned int left;

    if (ilen <= 0)
        return;

    left = (unsigned int) (ctx->total[0] & 0x7F);
    fill = 128 - left;

    ctx->total[0] += (U64)ilen;

    if (ctx->total[0] < (U64)ilen)
        ctx->total[1]++;

    if (left && ilen >= fill)
    {
        memcpy((void *)(ctx->buffer + left),
                (void *)input, fill);
        cel_sha4_process(ctx, ctx->buffer);
        input += fill;
        ilen  -= fill;
        left = 0;
    }

    while (ilen >= 128)
    {
        cel_sha4_process(ctx, input);
        input += 128;
        ilen  -= 128;
    }

    if (ilen > 0)
    {
        memcpy((void *)(ctx->buffer + left), (void *)input, ilen);
    }
}

static const BYTE cel_sha4_padding[128] =
{
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
 * SHA-512 final digest
 */
void cel_sha4_finish(CelSha4Context *ctx, BYTE output[64])
{
    size_t last, padn;
    U64 high, low;
    BYTE msglen[16];

    high = (ctx->total[0] >> 61)
         | (ctx->total[1] <<  3);
    low  = (ctx->total[0] <<  3);

    CEL_PUT_BE64(high, msglen, 0);
    CEL_PUT_BE64(low,  msglen, 8);

    last = (size_t) (ctx->total[0] & 0x7F);
    padn = (last < 112)? (112 - last): (240 - last);

    cel_sha4_update(ctx, (BYTE *)cel_sha4_padding, padn);
    cel_sha4_update(ctx, msglen, 16);

    CEL_PUT_BE64(ctx->state[0], output,  0);
    CEL_PUT_BE64(ctx->state[1], output,  8);
    CEL_PUT_BE64(ctx->state[2], output, 16);
    CEL_PUT_BE64(ctx->state[3], output, 24);
    CEL_PUT_BE64(ctx->state[4], output, 32);
    CEL_PUT_BE64(ctx->state[5], output, 40);

    if (ctx->is384 == 0)
    {
        CEL_PUT_BE64(ctx->state[6], output, 48);
        CEL_PUT_BE64(ctx->state[7], output, 56);
    }
}

/*
 * output = SHA-512(input buffer)
 */
void cel_sha4(const BYTE *input, size_t ilen,
              BYTE output[64], int is384)
{
    CelSha4Context ctx;

    cel_sha4_starts(&ctx, is384);
    cel_sha4_update(&ctx, input, ilen);
    cel_sha4_finish(&ctx, output);

    memset(&ctx, 0, sizeof(CelSha4Context));
}

/*
 * SHA-512 HMAC context setup
 */
void cel_sha4_hmac_starts(CelSha4Context *ctx, const BYTE *key, size_t keylen,
                          int is384)
{
    size_t i;
    BYTE sum[64];

    if (keylen > 128)
    {
        cel_sha4(key, keylen, sum, is384);
        keylen = (is384)? 48 : 64;
        key = sum;
    }

    memset(ctx->ipad, 0x36, 128);
    memset(ctx->opad, 0x5C, 128);

    for (i = 0; i < keylen; i++)
    {
        ctx->ipad[i] = (BYTE) (ctx->ipad[i] ^ key[i]);
        ctx->opad[i] = (BYTE) (ctx->opad[i] ^ key[i]);
    }

    cel_sha4_starts(ctx, is384);
    cel_sha4_update(ctx, ctx->ipad, 128);

    memset(sum, 0, sizeof(sum));
}

/*
 * SHA-512 HMAC process buffer
 */
void cel_sha4_hmac_update(CelSha4Context  *ctx,
                          const BYTE *input, size_t ilen)
{
    cel_sha4_update(ctx, input, ilen);
}

/*
 * SHA-512 HMAC final digest
 */
void cel_sha4_hmac_finish(CelSha4Context *ctx, BYTE output[64])
{
    int is384, hlen;
    BYTE tmpbuf[64];

    is384 = ctx->is384;
    hlen = (is384 == 0)? 64 : 48;

    cel_sha4_finish(ctx, tmpbuf);
    cel_sha4_starts(ctx, is384);
    cel_sha4_update(ctx, ctx->opad, 128);
    cel_sha4_update(ctx, tmpbuf, hlen);
    cel_sha4_finish(ctx, output);

    memset(tmpbuf, 0, sizeof(tmpbuf));
}

/*
 * SHA-512 HMAC context reset
 */
void cel_sha4_hmac_reset(CelSha4Context *ctx)
{
    cel_sha4_starts(ctx, ctx->is384);
    cel_sha4_update(ctx, ctx->ipad, 128);
}

/*
 * output = HMAC-SHA-512(hmac key, input buffer)
 */
void cel_sha4_hmac(const BYTE *key, size_t keylen,
                   const BYTE *input, size_t ilen,
                   BYTE output[64], int is384)
{
    CelSha4Context ctx;

    cel_sha4_hmac_starts(&ctx, key, keylen, is384);
    cel_sha4_hmac_update(&ctx, input, ilen);
    cel_sha4_hmac_finish(&ctx, output);

    memset(&ctx, 0, sizeof(CelSha4Context));
}
