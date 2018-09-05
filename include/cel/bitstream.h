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
#ifndef __CEL_BITSTREAM_H__
#define __CEL_BITSTREAM_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelBitStream
{
    const BYTE* buffer;
    BYTE *pointer;
    U32 position;
    U32 length;
    U32 capacity;
    U32 mask;
    U32 offset;
    U32 prefetch;
    U32 accumulator;
}CelBitStream;

#define BITDUMP_MSB_FIRST   0x00000001
#define BITDUMP_STDERR      0x00000002


CelBitStream *cel_bitstream_new();
void cel_bitstream_free(CelBitStream* bs);

#define cel_bitstream_prefetch(_bs) do { \
    (_bs->prefetch) = 0; \
    if (((U32) (_bs->pointer - _bs->buffer) + 4) < (_bs->capacity)) \
        (_bs->prefetch) |= (*(_bs->pointer + 4) << 24); \
    if (((U32) (_bs->pointer - _bs->buffer) + 5) < (_bs->capacity)) \
       (_bs->prefetch) |= (*(_bs->pointer + 5) << 16); \
    if (((U32) (_bs->pointer - _bs->buffer) + 6) < (_bs->capacity)) \
       (_bs->prefetch) |= (*(_bs->pointer + 6) << 8); \
    if (((U32) (_bs->pointer - _bs->buffer) + 7) < (_bs->capacity)) \
       (_bs->prefetch) |= (*(_bs->pointer + 7) << 0); \
} while(0)

#define cel_bitstream_fetch(_bs) do { \
    (_bs->accumulator) = 0; \
    if (((U32) (_bs->pointer - _bs->buffer) + 0) < (_bs->capacity)) \
        (_bs->accumulator) |= (*(_bs->pointer + 0) << 24); \
    if (((U32) (_bs->pointer - _bs->buffer) + 1) < (_bs->capacity)) \
        (_bs->accumulator) |= (*(_bs->pointer + 1) << 16); \
    if (((U32) (_bs->pointer - _bs->buffer) + 2) < (_bs->capacity)) \
       (_bs->accumulator) |= (*(_bs->pointer + 2) << 8); \
    if (((U32) (_bs->pointer - _bs->buffer) + 3) <(_bs->capacity)) \
       (_bs->accumulator) |= (*(_bs->pointer + 3) << 0); \
    cel_bitstream_prefetch(_bs); \
} while(0)

#define cel_bitstream_flush(_bs) do { \
    if (((U32) (_bs->pointer - _bs->buffer) + 0) < (_bs->capacity)) \
       *(_bs->pointer + 0) = (_bs->accumulator >> 24); \
    if (((U32) (_bs->pointer - _bs->buffer) + 1) < (_bs->capacity)) \
       *(_bs->pointer + 1) = (_bs->accumulator >> 16); \
    if (((U32) (_bs->pointer - _bs->buffer) + 2) < (_bs->capacity)) \
       *(_bs->pointer + 2) = (_bs->accumulator >> 8); \
    if (((U32) (_bs->pointer - _bs->buffer) + 3) < (_bs->capacity)) \
       *(_bs->pointer + 3) = (_bs->accumulator >> 0); \
} while(0)

#define cel_bitstream_shift(_bs, _nbits) do { \
    if ((_nbits > 0) && (_nbits < 32)) { \
        _bs->accumulator <<= _nbits; \
        _bs->position += _nbits; \
        _bs->offset += _nbits; \
        if (_bs->offset < 32) { \
            _bs->mask = ((1 << _nbits) - 1); \
            _bs->accumulator |= \
            ((_bs->prefetch >> (32 - _nbits)) & _bs->mask); \
            _bs->prefetch <<= _nbits; \
        } \
        else { \
            _bs->mask = ((1 << _nbits) - 1); \
            _bs->accumulator |= \
            ((_bs->prefetch >> (32 - _nbits)) & _bs->mask); \
            _bs->prefetch <<= _nbits; \
            _bs->offset -= 32; \
            _bs->pointer += 4; \
            cel_bitstream_prefetch(_bs); \
            if (_bs->offset) { \
               _bs->mask = ((1 << _bs->offset) - 1); \
               _bs->accumulator |= \
               ((_bs->prefetch >> (32 - _bs->offset)) & _bs->mask); \
               _bs->prefetch <<= _bs->offset; \
            } \
        } \
   }\
   else { \
       CEL_ERR((_T("warning: cel_bitstream_shift(%d)"), _nbits)); \
   } \
} while(0)

#define cel_bitstream_shift32(_bs) do { \
    cel_bitstream_shift(_bs, 16); \
    cel_bitstream_shift(_bs, 16); \
} while(0)

#define cel_bitstream_write_bits(_bs, _bits, _nbits) do { \
    _bs->position += _nbits; \
    _bs->offset += _nbits; \
    if (_bs->offset < 32) { \
        _bs->accumulator |= (_bits << (32 - _bs->offset)); \
    } \
    else { \
        _bs->offset -= 32; \
        _bs->mask = ((1 << (_nbits - _bs->offset)) - 1); \
        _bs->accumulator |= ((_bits >> _bs->offset) & _bs->mask); \
        cel_bitstream_flush(bs); \
        _bs->accumulator = 0; \
        _bs->pointer += 4; \
        if (_bs->offset) { \
            _bs->mask = ((1 << _bs->offset) - 1); \
            _bs->accumulator |= ((_bits & _bs->mask) << (32 - _bs->offset)); \
        } \
    } \
} while(0)

#define cel_bitstream_get_remaining_length(_bs) \
    (_bs->length - _bs->position)

U32 cel_reverse_bits32(U32 bits, U32 nbits);

void cel_bitstream_attach(CelBitStream* bs, const BYTE *buffer, U32 capacity);


#ifdef __cplusplus
}
#endif

#endif 
