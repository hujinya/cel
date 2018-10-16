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
#ifndef __CEL_BYTEORDER_H__
#define __CEL_BYTEORDER_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

//#define _CEL_BIGEDIAN

typedef union _CelEndianTest
{ 
    char c[4];
    unsigned long l;
}CelEndianTest;

extern const CelEndianTest g_endiantest;

#ifdef _CEL_BIGEDIAN

//#define CEL_ARCH_SWAB16
//#define CEL_ARCH_SWAB32
//#define CEL_ARCH_SWAB64
//#define CEL_ARCH_SWAHW32
//#define CEL_ARCH_SWAHB32

#define CEL_GET_LE16(n, b, i)  (n)= cel_swab16(*(U16 *)(&((b)[(i)])))
#define CEL_GET_BE16(n, b, i)  (n)= (*(U16 *)(&((b)[(i)])))
#define CEL_PUT_LE16(n, b, i)  (*(U16 *)(&((b)[(i)])))= cel_swab16(n)
#define CEL_PUT_BE16(n, b, i)  (*(U16 *)(&((b)[(i)])))= (n)

#define CEL_GET_LE32(n, b, i)  (n)= cel_swab32(*(U32 *)(&((b)[(i)])))
#define CEL_GET_BE32(n, b, i)  (n)= (*(U32 *)(&((b)[(i)])))
#define CEL_PUT_LE32(n, b, i)  (*(U32 *)(&((b)[(i)])))= cel_swab32(n)
#define CEL_PUT_BE32(n, b, i)  (*(U32 *)(&((b)[(i)])))= (n)

#define CEL_GET_LE64(n, b, i)  (n)= cel_swab64(*(U64 *)(&((b)[(i)])))
#define CEL_GET_BE64(n, b, i)  (n)= (*(U64 *)(&((b)[(i)])))
#define CEL_PUT_LE64(n, b, i)  (*(U64 *)(&((b)[(i)])))= cel_swab64(n)
#define CEL_PUT_BE64(n, b, i)  (*(U64 *)(&((b)[(i)])))= (n)

#else

#ifdef _CEL_UNIX
#include <arpa/inet.h>
#endif
#ifdef _CEL_WIN
#include <Winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#endif

#define CEL_ARCH_SWAB16
#define arch_swab16(val) htons(val)
#define CEL_ARCH_SWAB32
#define arch_swab32(val) htonl(val)
//#define CEL_ARCH_SWAB64
//#define CEL_ARCH_SWAHW32
//#define CEL_ARCH_SWAHB32

#define CEL_GET_LE16(n, b, i)  (n)= (*(U16 *)(&((b)[(i)])))
#define CEL_GET_BE16(n, b, i)  (n)= cel_swab16(*(U16 *)(&((b)[(i)])))
#define CEL_PUT_LE16(n, b, i)  (*(U16 *)(&((b)[(i)])))= (n)
#define CEL_PUT_BE16(n, b, i)  (*(U16 *)(&((b)[(i)])))= cel_swab16(n)

#define CEL_GET_LE32(n, b, i)  (n)= (*(U32 *)(&((b)[(i)])))
#define CEL_GET_BE32(n, b, i)  (n)= cel_swab32(*(U32 *)(&((b)[(i)])))
#define CEL_PUT_LE32(n, b, i)  (*(U32 *)(&((b)[(i)])))= (n)
#define CEL_PUT_BE32(n, b, i)  (*(U32 *)(&((b)[(i)])))= cel_swab32(n)

#define CEL_GET_LE64(n, b, i)  (n)= (*(U64 *)(&((b)[(i)])))
#define CEL_GET_BE64(n, b, i)  (n)= cel_swab64(*(U64 *)(&((b)[(i)])))
#define CEL_PUT_LE64(n, b, i)  (*(U64 *)(&((b)[(i)])))= (n)
#define CEL_PUT_BE64(n, b, i)  (*(U64 *)(&((b)[(i)])))= cel_swab64(n)

#endif

#define CEL_CONSTANT_SWAB16(x) ((U16) (    \
    (((U16) (x) & U(0x00ff)) << 8) | (((U16) (x) & U(0xff00)) >> 8)))

#define CEL_CONSTANT_SWAB32(x) ((U32) (    \
    (((U32) (x) & UL(0x000000ff)) << 24) | \
    (((U32) (x) & UL(0x0000ff00)) <<  8) | \
    (((U32) (x) & UL(0x00ff0000)) >>  8) | \
    (((U32) (x) & UL(0xff000000)) >> 24)))

#define CEL_CONSTANT_SWAB64(x) ((U64) (    \
    (((U64) (x) & ULL(0x00000000000000ff)) << 56) | \
    (((U64) (x) & ULL(0x000000000000ff00)) << 40) | \
    (((U64) (x) & ULL(0x0000000000ff0000)) << 24) | \
    (((U64) (x) & ULL(0x00000000ff000000)) <<  8) | \
    (((U64) (x) & ULL(0x000000ff00000000)) >>  8) | \
    (((U64) (x) & ULL(0x0000ff0000000000)) >> 24) | \
    (((U64) (x) & ULL(0x00ff000000000000)) >> 40) | \
    (((U64) (x) & ULL(0xff00000000000000)) >> 56)))

#define CEL_CONSTANT_SWAHW32(x) ((U32)  ( \
    (((U32) (x) & UL(0x0000ffff)) << 16) \
    | (((U32) (x) & UL(0xffff0000)) >> 16)))

#define CEL_CONSTANT_SWAHB32(x) ((U32)  ( \
    (((U32) (x) & UL(0x00ff00ff)) << 8) | (((U32) (x) & UL(0xff00ff00)) >> 8)))

/* swab16 - return a byteswapped 16-bit value */
static __inline U16 cel_swab16(U16 val)
{
#ifdef CEL_ARCH_SWAB16
    return arch_swab16(val);
#else
    return CEL_CONSTANT_SWAB16(val);
#endif
}    

/* swab32 - return a byteswapped 32-bit value */
static __inline U32 cel_swab32(U32 val)
{
#ifdef CEL_ARCH_SWAB32
    return arch_swab32(val);
#else
    return CEL_CONSTANT_SWAB32(val);
#endif
}

/* swab64 - return a byteswapped 64-bit value */
static __inline U64 cel_swab64(U64 val)
{
#ifdef CEL_ARCH_SWAB64
    return arch_swab64(val);
#else
    return CEL_CONSTANT_SWAB64(val);
#endif
}

/* swahw32 - return a word-swapped 32-bit value */
static __inline U32 cel_swahw32(U32 val)
{
#ifdef CEL_ARCH_SWAHW32
    return arch_swahw32(val);
#else
    return CEL_CONSTANT_SWAHW32(val);
#endif
}

/* swahb32 - return a high and low byte-swapped 32-bit value */
static __inline U32 cel_swahb32(U32 val)
{
#ifdef CEL_ARCH_SWAHB32
    return arch_swahb32(val);
#else
    return CEL_CONSTANT_SWAHB32(val);
#endif
}    

#ifdef __cplusplus
}
#endif

#endif
