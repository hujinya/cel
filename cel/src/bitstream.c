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
#include "cel/bitstream.h"
#include "cel/allocator.h"

CelBitStream *cel_bitstream_new()
{
    CelBitStream* bs = NULL;
    bs = (CelBitStream*)cel_calloc(1, sizeof(CelBitStream));

    return bs;
}

void cel_bitstream_free(CelBitStream *bs)
{
    if (!bs)
        return;

    free(bs);
}

U32 cel_reverse_bits32(U32 bits, U32 nbits)
{
    U32 rbits = 0;
    do
    {
        rbits = (rbits | (bits & 1)) << 1;
        bits >>= 1;
        nbits--;
    }
    while (nbits > 0);
    rbits >>= 1;
    return rbits;
}

void cel_bitstream_attach(CelBitStream *bs, const BYTE *buffer, U32 capacity)
{
    bs->position = 0;
    bs->buffer = buffer;
    bs->offset = 0;
    bs->accumulator = 0;
    bs->pointer = (BYTE*)bs->buffer;
    bs->capacity = capacity;
    bs->length = bs->capacity * 8;
}
