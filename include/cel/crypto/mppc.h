/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 *
 * MPPC Bulk Data Compression
 * Copyright 2014 Marc-Andre Moreau <marcandre.moreau@gmail.com>  
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
#ifndef __CEL_CRYPTO_MPPC_H__
#define __CEL_CRYPTO_MPPC_H__

#include "cel/types.h"
#include "cel/bitstream.h"


#ifdef __cplusplus
extern "C" {
#endif

#define PACKET_COMPRESSED       0x20
#define PACKET_AT_FRONT         0x40
#define PACKET_FLUSHED          0x80

typedef struct _CelMppcContext
{
    CelBitStream *bs;
    BOOL compressor;
    BYTE *history_ptr;
    U32 history_offset;
    U32 history_size;
    BYTE history_buffer[65536];
    U16 match_buffer[32768];
    U32 level;
}CelMppcContext;


CelMppcContext *mppc_context_new(U32 level, BOOL compressor);
void mppc_context_free(CelMppcContext *mppc);

void mppc_set_cmpr_level(CelMppcContext *mppc, U32 level);
void mppc_context_reset(CelMppcContext *mppc, BOOL flush);

int mppc_compress(CelMppcContext *mppc, BYTE *src, U32 src_size, 
                  BYTE **pdest, U32 *dest_size, U32* pflags);
int mppc_decompress(CelMppcContext *mppc, BYTE *src, U32 src_size, 
                    BYTE **pdest, U32 *dest_size, U32 flags);

#ifdef __cplusplus
}
#endif

#endif
