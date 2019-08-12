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
#include "cel/crypto/base64.h"
#include "cel/error.h"
#include "cel/log.h"

static const BYTE base64_enc_map[64] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'
};

static const BYTE base64_dec_map[128] =
{
    127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 
    127, 127, 127,  62, 127, 127, 127,  63,
     52,  53,  54,  55,  56,  57,  58,  59,
     60,  61, 127, 127, 127,  64, 127, 127,
    127,   0,   1,   2,   3,   4,   5,   6,
      7,   8,   9,  10,  11,  12,  13,  14, 
     15,  16,  17,  18,  19,  20,  21,  22,
     23,  24,  25, 127, 127, 127, 127, 127,
    127,  26,  27,  28,  29,  30,  31,  32,
     33,  34,  35,  36,  37,  38,  39,  40,
     41,  42,  43,  44,  45,  46,  47,  48, 
     49,  50,  51, 127, 127, 127, 127, 127
};

/*
 * Encode a buffer into base64 format
 */
int cel_base64_encode(BYTE *dst, size_t *dlen, 
                      const BYTE *src, size_t slen)
{
    size_t i, n;
    int c1, c2, c3;
    BYTE *p;

    if (slen == 0)
        return (0);

    n = (slen << 3)/ 6;

    switch ((slen << 3)- (n * 6))
    {
    case  2: n += 3; break;
    case  4: n += 2; break;
    default: break;
    }

    if (*dlen < n + 1)
    {
        *dlen = n + 1;
        cel_seterr(CEL_ERR_LIB, _T("Destination buffer too small."));
        return -1;
    }

    n = (slen / 3)* 3;

    for (i = 0, p = dst; i < n; i += 3)
    {
        c1 = *src++;
        c2 = *src++;
        c3 = *src++;

        *p++ = base64_enc_map[(c1 >> 2)& 0x3F];
        *p++ = base64_enc_map[(((c1 &  3) << 4)+ (c2 >> 4)) & 0x3F];
        *p++ = base64_enc_map[(((c2 & 15) << 2)+ (c3 >> 6)) & 0x3F];
        *p++ = base64_enc_map[c3 & 0x3F];
    }

    if (i < slen)
    {
        c1 = *src++;
        c2 = ((i + 1) < slen)? *src++ : 0;

        *p++ = base64_enc_map[(c1 >> 2) & 0x3F];
        *p++ = base64_enc_map[(((c1 & 3) << 4)+ (c2 >> 4))& 0x3F];

        if ((i + 1) < slen)
            *p++ = base64_enc_map[((c2 & 15) << 2)& 0x3F];
        else *p++ = '=';

        *p++ = '=';
    }

    *dlen = p - dst;
    *p = 0;

    return 0;
}

/*
 * Decode a base64-formatted buffer
 */
int cel_base64_decode(BYTE *dst, size_t *dlen, 
                      const BYTE *src, size_t slen)
{
    size_t i, j, n;
    unsigned long x;
    BYTE *p;

    for (i = j = n = 0; i < slen; i++)
    {
        if (src[i] == '\n'
            || ((slen - i) >= 2 && src[i] == '\r' && src[i + 1] == '\n'))
            continue;

        if (src[i] == '=' && ++j > 2)
        {
            cel_seterr(CEL_ERR_LIB, _T("Invalid character."));
            return  -1;
        }

        if (src[i] > 127 || base64_dec_map[src[i]] == 127)
        {
            cel_seterr(CEL_ERR_LIB, _T("Invalid character."));
            return -1;
        }

        if (base64_dec_map[src[i]] < 64 && j != 0)
        {
            cel_seterr(CEL_ERR_LIB, _T("Invalid character."));
            return -1;
        }

        n++;
    }
    if (n == 0)
        return 0;
    n = ((n * 6)+ 7) >> 3;
    if (*dlen < n)
    {
        *dlen = n;
        cel_seterr(CEL_ERR_LIB, _T("Destination buffer too small."));
        return -1;
    }
   for (j = 3, n = x = 0, p = dst; i > 0; i--, src++)
   {
        if (*src == '\r' || *src == '\n')
            continue;

        j -= (base64_dec_map[*src] == 64);
        x  = (x << 6) | (base64_dec_map[*src] & 0x3F);

        if (++n == 4)
        {
            n = 0;
            if (j > 0) *p++ = (BYTE)(x >> 16);
            if (j > 1) *p++ = (BYTE)(x >>  8);
            if (j > 2) *p++ = (BYTE)(x);
        }
    }

    *dlen = p - dst;

    return 0;
}

int cel_base64url_encode(BYTE *dst, size_t *dlen, 
                         const BYTE *src, size_t slen)
{
    size_t i, j;
    
    if (cel_base64_encode(dst, dlen, src, slen) == -1)
        return -1;
    for (i = j = 0; i < *dlen; i++) 
    {
        switch (dst[i]) 
        {
        case '+':
            dst[j++] = '-';
            break;
        case '/':
            dst[j++] = '_';
            break;
        case '=':
            break;
        default:
            dst[j++] = dst[i];
        }
    }
    dst[j] = '\0';
    *dlen = j;
    return 0;
}

int cel_base64url_decode(BYTE *dst, size_t *dlen, 
                         const BYTE *src, size_t slen)
{
    size_t i, z;
    BYTE *new_mem; 
    
    new_mem = (BYTE *)alloca(slen + 4);
    for (i = 0; i < slen; i++) 
    {
        switch (src[i]) 
        {
        case '-':
            new_mem[i] = '+';
            break;
        case '_':
            new_mem[i] = '/';
            break;
        default:
            new_mem[i] = src[i];
        }
    }
    z = 4 - (i % 4);
    if (z < 4) 
    {
        while (z--)
            new_mem[i++] = '=';
    }
    new_mem[i] = '\0';
    return cel_base64_decode(dst, dlen, new_mem, i);
}
