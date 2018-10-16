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
#include "cel/crypto/mppc.h"
#include "cel/log.h"
#include "cel/error.h"

const U32 mppc_match_table[256] =
{
    0x00000000, 0x009CCF93, 0x01399F26, 0x01D66EB9, 
    0x02733E4C, 0x03100DDF, 0x03ACDD72, 0x0449AD05,
    0x04E67C98, 0x05834C2B, 0x06201BBE, 0x06BCEB51, 
    0x0759BAE4, 0x07F68A77, 0x08935A0A, 0x0930299D,
    0x09CCF930, 0x0A69C8C3, 0x0B069856, 0x0BA367E9,
    0x0C40377C, 0x0CDD070F, 0x0D79D6A2, 0x0E16A635,
    0x0EB375C8, 0x0F50455B, 0x0FED14EE, 0x1089E481, 
    0x1126B414, 0x11C383A7, 0x1260533A, 0x12FD22CD,
    0x1399F260, 0x1436C1F3, 0x14D39186, 0x15706119,
    0x160D30AC, 0x16AA003F, 0x1746CFD2, 0x17E39F65,
    0x18806EF8, 0x191D3E8B, 0x19BA0E1E, 0x1A56DDB1,
    0x1AF3AD44, 0x1B907CD7, 0x1C2D4C6A, 0x1CCA1BFD,
    0x1D66EB90, 0x1E03BB23, 0x1EA08AB6, 0x1F3D5A49, 
    0x1FDA29DC, 0x2076F96F, 0x2113C902, 0x21B09895,
    0x224D6828, 0x22EA37BB, 0x2387074E, 0x2423D6E1,
    0x24C0A674, 0x255D7607, 0x25FA459A, 0x2697152D,
    0x2733E4C0, 0x27D0B453, 0x286D83E6, 0x290A5379,
    0x29A7230C, 0x2A43F29F, 0x2AE0C232, 0x2B7D91C5,
    0x2C1A6158, 0x2CB730EB, 0x2D54007E, 0x2DF0D011,
    0x2E8D9FA4, 0x2F2A6F37, 0x2FC73ECA, 0x30640E5D,
    0x3100DDF0, 0x319DAD83, 0x323A7D16, 0x32D74CA9,
    0x33741C3C, 0x3410EBCF, 0x34ADBB62, 0x354A8AF5,
    0x35E75A88, 0x36842A1B, 0x3720F9AE, 0x37BDC941, 
    0x385A98D4, 0x38F76867, 0x399437FA, 0x3A31078D,
    0x3ACDD720, 0x3B6AA6B3, 0x3C077646, 0x3CA445D9, 
    0x3D41156C, 0x3DDDE4FF, 0x3E7AB492, 0x3F178425,
    0x3FB453B8, 0x4051234B, 0x40EDF2DE, 0x418AC271,
    0x42279204, 0x42C46197, 0x4361312A, 0x43FE00BD,
    0x449AD050, 0x45379FE3, 0x45D46F76, 0x46713F09,
    0x470E0E9C, 0x47AADE2F, 0x4847ADC2, 0x48E47D55,
    0x49814CE8, 0x4A1E1C7B, 0x4ABAEC0E, 0x4B57BBA1, 
    0x4BF48B34, 0x4C915AC7, 0x4D2E2A5A, 0x4DCAF9ED,
    0x4E67C980, 0x4F049913, 0x4FA168A6, 0x503E3839,
    0x50DB07CC, 0x5177D75F, 0x5214A6F2, 0x52B17685,
    0x534E4618, 0x53EB15AB, 0x5487E53E, 0x5524B4D1,
    0x55C18464, 0x565E53F7, 0x56FB238A, 0x5797F31D,
    0x5834C2B0, 0x58D19243, 0x596E61D6, 0x5A0B3169,
    0x5AA800FC, 0x5B44D08F, 0x5BE1A022, 0x5C7E6FB5,
    0x5D1B3F48, 0x5DB80EDB, 0x5E54DE6E, 0x5EF1AE01,
    0x5F8E7D94, 0x602B4D27, 0x60C81CBA, 0x6164EC4D,
    0x6201BBE0, 0x629E8B73, 0x633B5B06, 0x63D82A99, 
    0x6474FA2C, 0x6511C9BF, 0x65AE9952, 0x664B68E5,
    0x66E83878, 0x6785080B, 0x6821D79E, 0x68BEA731,
    0x695B76C4, 0x69F84657, 0x6A9515EA, 0x6B31E57D,
    0x6BCEB510, 0x6C6B84A3, 0x6D085436, 0x6DA523C9,
    0x6E41F35C, 0x6EDEC2EF, 0x6F7B9282, 0x70186215,
    0x70B531A8, 0x7152013B, 0x71EED0CE, 0x728BA061,
    0x73286FF4, 0x73C53F87, 0x74620F1A, 0x74FEDEAD,
    0x759BAE40, 0x76387DD3, 0x76D54D66, 0x77721CF9, 
    0x780EEC8C, 0x78ABBC1F, 0x79488BB2, 0x79E55B45,
    0x7A822AD8, 0x7B1EFA6B, 0x7BBBC9FE, 0x7C589991,
    0x7CF56924, 0x7D9238B7, 0x7E2F084A, 0x7ECBD7DD,
    0x7F68A770, 0x80057703, 0x80A24696, 0x813F1629,
    0x81DBE5BC, 0x8278B54F, 0x831584E2, 0x83B25475,
    0x844F2408, 0x84EBF39B, 0x8588C32E, 0x862592C1,
    0x86C26254, 0x875F31E7, 0x87FC017A, 0x8898D10D,
    0x8935A0A0, 0x89D27033, 0x8A6F3FC6, 0x8B0C0F59,
    0x8BA8DEEC, 0x8C45AE7F, 0x8CE27E12, 0x8D7F4DA5,
    0x8E1C1D38, 0x8EB8ECCB, 0x8F55BC5E, 0x8FF28BF1,
    0x908F5B84, 0x912C2B17, 0x91C8FAAA, 0x9265CA3D,
    0x930299D0, 0x939F6963, 0x943C38F6, 0x94D90889,
    0x9575D81C, 0x9612A7AF, 0x96AF7742, 0x974C46D5,
    0x97E91668, 0x9885E5FB, 0x9922B58E, 0x99BF8521, 
    0x9A5C54B4, 0x9AF92447, 0x9B95F3DA, 0x9C32C36D
};

#define MPPC_MATCH_INDEX(_sym1, _sym2, _sym3) \
    ((((mppc_match_table[_sym3] << 16) + (mppc_match_table[_sym2] << 8) \
    + mppc_match_table[_sym1]) & 0x07FFF000) >> 12)

int mppc_decompress(CelMppcContext *mppc, BYTE *src, U32 src_size, 
                    BYTE **pdest, U32 *dest_size, U32 flags)
{
    BYTE Literal;
    BYTE *SrcPtr;
    U32 CopyOffset;
    U32 LengthOfMatch;
    U32 accumulator;
    BYTE *history_ptr;
    //U32 history_offset;
    BYTE *history_buffer;
    BYTE *HistoryBufferEnd;
    U32 history_size;
    U32 level;
    CelBitStream* bs = mppc->bs;

    history_buffer = mppc->history_buffer;
    history_size = mppc->history_size;
    HistoryBufferEnd = &history_buffer[history_size - 1];
    level = mppc->level;

    cel_bitstream_attach(bs, src, src_size);
    cel_bitstream_fetch(bs);

    if (flags & PACKET_AT_FRONT)
    {
        //puts("PACKET_AT_FRONT");
        mppc->history_offset = 0;
        mppc->history_ptr = history_buffer;
    }

    if (flags & PACKET_FLUSHED)
    {
        //puts("PACKET_FLUSHED");
        mppc->history_offset = 0;
        mppc->history_ptr = history_buffer;
        memset(history_buffer, 0, mppc->history_size);
    }

    history_ptr = mppc->history_ptr;
    //history_offset = mppc->history_offset;

    if (!(flags & PACKET_COMPRESSED))
    {
        *dest_size = src_size;
        *pdest = src;
        return 1;
    }

    while ((bs->length - bs->position) >= 8)
    {
        accumulator = bs->accumulator;

        /**
         * Literal Encoding
         */

        if (history_ptr > HistoryBufferEnd)
        {
            CEL_ERR((_T("history buffer index out of range")));
            return -1004;
        }

        if ((accumulator & 0x80000000) == 0x00000000)
        {
            /**
             * Literal, less than 0x80
             * bit 0 followed by the lower 7 bits of the literal
             */
            Literal = ((accumulator & 0x7F000000) >> 24);

            *(history_ptr) = Literal;
            history_ptr++;

            cel_bitstream_shift(bs, 8);

            continue;
        }
        else if ((accumulator & 0xC0000000) == 0x80000000)
        {
            /**
             * Literal, greater than 0x7F
             * bits 10 followed by the lower 7 bits of the literal
             */

            Literal = ((accumulator & 0x3F800000) >> 23) + 0x80;

            *(history_ptr) = Literal;
            history_ptr++;

            cel_bitstream_shift(bs, 9);

            continue;
        }

        /**
         * CopyOffset Encoding
         */

        CopyOffset = 0;

        if (level) /* RDP5 */
        {
            if ((accumulator & 0xF8000000) == 0xF8000000)
            {
                /**
                 * CopyOffset, range [0, 63]
                 * bits 11111 + lower 6 bits of CopyOffset
                 */

                CopyOffset = ((accumulator >> 21) & 0x3F);
                cel_bitstream_shift(bs, 11);
            }
            else if ((accumulator & 0xF8000000) == 0xF0000000)
            {
                /**
                 * CopyOffset, range [64, 319]
                 * bits 11110 + lower 8 bits of (CopyOffset - 64)
                 */

                CopyOffset = ((accumulator >> 19) & 0xFF) + 64;
                cel_bitstream_shift(bs, 13);
            }
            else if ((accumulator & 0xF0000000) == 0xE0000000)
            {
                /**
                 * CopyOffset, range [320, 2367]
                 * bits 1110 + lower 11 bits of (CopyOffset - 320)
                 */

                CopyOffset = ((accumulator >> 17) & 0x7FF) + 320;
                cel_bitstream_shift(bs, 15);
            }
            else if ((accumulator & 0xE0000000) == 0xC0000000)
            {
                /**
                * CopyOffset, range [2368, ]
                * bits 110 + lower 16 bits of (CopyOffset - 2368)
                */

                CopyOffset = ((accumulator >> 13) & 0xFFFF) + 2368;
                cel_bitstream_shift(bs, 19);
            }
            else
            {
                /* Invalid CopyOffset Encoding */
                return -1001;
            }
        }
        else /* RDP4 */
        {
            if ((accumulator & 0xF0000000) == 0xF0000000)
            {
                /**
                * CopyOffset, range [0, 63]
                * bits 1111 + lower 6 bits of CopyOffset
                */

                CopyOffset = ((accumulator >> 22) & 0x3F);
                cel_bitstream_shift(bs, 10);
            }
            else if ((accumulator & 0xF0000000) == 0xE0000000)
            {
                /**
                * CopyOffset, range [64, 319]
                * bits 1110 + lower 8 bits of (CopyOffset - 64)
                */

                CopyOffset = ((accumulator >> 20) & 0xFF) + 64;
                cel_bitstream_shift(bs, 12);
            }
            else if ((accumulator & 0xE0000000) == 0xC0000000)
            {
                /**
                * CopyOffset, range [320, 8191]
                * bits 110 + lower 13 bits of (CopyOffset - 320)
                */

                CopyOffset = ((accumulator >> 16) & 0x1FFF) + 320;
                cel_bitstream_shift(bs, 16);
            }
            else
            {
                /* Invalid CopyOffset Encoding */
                return -1002;
            }
        }

        /**
        * LengthOfMatch Encoding
        */

        LengthOfMatch = 0;
        accumulator = bs->accumulator;

        if ((accumulator & 0x80000000) == 0x00000000)
        {
            /**
            * LengthOfMatch [3]
            * bit 0 + 0 lower bits of LengthOfMatch
            */

            LengthOfMatch = 3;
            cel_bitstream_shift(bs, 1);
        }
        else if ((accumulator & 0xC0000000) == 0x80000000)
        {
            /**
            * LengthOfMatch [4, 7]
            * bits 10 + 2 lower bits of LengthOfMatch
            */

            LengthOfMatch = ((accumulator >> 28) & 0x0003) + 0x0004;
            cel_bitstream_shift(bs, 4);
        }
        else if ((accumulator & 0xE0000000) == 0xC0000000)
        {
            /**
            * LengthOfMatch [8, 15]
            * bits 110 + 3 lower bits of LengthOfMatch
            */

            LengthOfMatch = ((accumulator >> 26) & 0x0007) + 0x0008;
            cel_bitstream_shift(bs, 6);
        }
        else if ((accumulator & 0xF0000000) == 0xE0000000)
        {
            /**
            * LengthOfMatch [16, 31]
            * bits 1110 + 4 lower bits of LengthOfMatch
            */

            LengthOfMatch = ((accumulator >> 24) & 0x000F) + 0x0010;
            cel_bitstream_shift(bs, 8);
        }
        else if ((accumulator & 0xF8000000) == 0xF0000000)
        {
            /**
            * LengthOfMatch [32, 63]
            * bits 11110 + 5 lower bits of LengthOfMatch
            */

            LengthOfMatch = ((accumulator >> 22) & 0x001F) + 0x0020;
            cel_bitstream_shift(bs, 10);
        }
        else if ((accumulator & 0xFC000000) == 0xF8000000)
        {
            /**
            * LengthOfMatch [64, 127]
            * bits 111110 + 6 lower bits of LengthOfMatch
            */

            LengthOfMatch = ((accumulator >> 20) & 0x003F) + 0x0040;
            cel_bitstream_shift(bs, 12);
        }
        else if ((accumulator & 0xFE000000) == 0xFC000000)
        {
            /**
            * LengthOfMatch [128, 255]
            * bits 1111110 + 7 lower bits of LengthOfMatch
            */

            LengthOfMatch = ((accumulator >> 18) & 0x007F) + 0x0080;
            cel_bitstream_shift(bs, 14);
        }
        else if ((accumulator & 0xFF000000) == 0xFE000000)
        {
            /**
            * LengthOfMatch [256, 511]
            * bits 11111110 + 8 lower bits of LengthOfMatch
            */

            LengthOfMatch = ((accumulator >> 16) & 0x00FF) + 0x0100;
            cel_bitstream_shift(bs, 16);
        }
        else if ((accumulator & 0xFF800000) == 0xFF000000)
        {
            /**
            * LengthOfMatch [512, 1023]
            * bits 111111110 + 9 lower bits of LengthOfMatch
            */

            LengthOfMatch = ((accumulator >> 14) & 0x01FF) + 0x0200;
            cel_bitstream_shift(bs, 18);
        }
        else if ((accumulator & 0xFFC00000) == 0xFF800000)
        {
            /**
            * LengthOfMatch [1024, 2047]
            * bits 1111111110 + 10 lower bits of LengthOfMatch
            */

            LengthOfMatch = ((accumulator >> 12) & 0x03FF) + 0x0400;
            cel_bitstream_shift(bs, 20);
        }
        else if ((accumulator & 0xFFE00000) == 0xFFC00000)
        {
            /**
            * LengthOfMatch [2048, 4095]
            * bits 11111111110 + 11 lower bits of LengthOfMatch
            */

            LengthOfMatch = ((accumulator >> 10) & 0x07FF) + 0x0800;
            cel_bitstream_shift(bs, 22);
        }
        else if ((accumulator & 0xFFF00000) == 0xFFE00000)
        {
            /**
            * LengthOfMatch [4096, 8191]
            * bits 111111111110 + 12 lower bits of LengthOfMatch
            */

            LengthOfMatch = ((accumulator >> 8) & 0x0FFF) + 0x1000;
            cel_bitstream_shift(bs, 24);
        }
        else if (((accumulator & 0xFFF80000) == 0xFFF00000) && level) /* RDP5 */
        {
            /**
            * LengthOfMatch [8192, 16383]
            * bits 1111111111110 + 13 lower bits of LengthOfMatch
            */

            LengthOfMatch = ((accumulator >> 6) & 0x1FFF) + 0x2000;
            cel_bitstream_shift(bs, 26);
        }
        else if (((accumulator & 0xFFFC0000) == 0xFFF80000) && level) /* RDP5 */
        {
            /**
            * LengthOfMatch [16384, 32767]
            * bits 11111111111110 + 14 lower bits of LengthOfMatch
            */

            LengthOfMatch = ((accumulator >> 4) & 0x3FFF) + 0x4000;
            cel_bitstream_shift(bs, 28);
        }
        else if (((accumulator & 0xFFFE0000) == 0xFFFC0000) && level) /* RDP5 */
        {
            /**
            * LengthOfMatch [32768, 65535]
            * bits 111111111111110 + 15 lower bits of LengthOfMatch
            */

            LengthOfMatch = ((accumulator >> 2) & 0x7FFF) + 0x8000;
            cel_bitstream_shift(bs, 30);
        }
        else
        {
            /* Invalid LengthOfMatch Encoding */
            return -1003;
        }
        //CEL_DEBUG((_T("<copy offset %d,length of match %d>"), (int)CopyOffset, (int)LengthOfMatch));


        if ((history_ptr + LengthOfMatch - 1) > HistoryBufferEnd)
        {
            CEL_ERR((_T("history buffer overflow")));
            return -1005;
        }

        SrcPtr = &history_buffer[(history_ptr - history_buffer - CopyOffset) & (level ? 0xFFFF : 0x1FFF)];

        do {
            *history_ptr++ = *SrcPtr++;
        } while (--LengthOfMatch);
    }

    *dest_size = (U32)(history_ptr - mppc->history_ptr);
    *pdest = mppc->history_ptr;
    mppc->history_ptr = history_ptr;

    return 1;
}

int mppc_compress(CelMppcContext *mppc, BYTE *src, U32 src_size, BYTE **pdest, U32 *dest_size, U32* pflags)
{
    BYTE* pSrcPtr;
    BYTE* pSrcEnd;
    //BYTE* pDstEnd;
    BYTE* MatchPtr;
    U32 DstSize;
    BYTE* dest;
    U32 MatchIndex;
    U32 accumulator;
    BOOL PacketFlushed;
    BOOL PacketAtFront;
    U32 CopyOffset;
    U32 LengthOfMatch;
    BYTE* history_buffer;
    BYTE* history_ptr;
    U32 history_offset;
    U32 history_size;
    BYTE Sym1, Sym2, Sym3;
    U32 level;
    CelBitStream* bs = mppc->bs;

    history_buffer = mppc->history_buffer;
    history_size = mppc->history_size;
    level = mppc->level;

    history_ptr = mppc->history_ptr;
    history_offset = mppc->history_offset;

    *pflags = 0;
    PacketFlushed = FALSE;

    if (((history_offset + src_size) < (history_size - 3)) && history_offset)
    {
        PacketAtFront = FALSE;
    }
    else
    {
        if (history_offset == (history_size + 1))
            PacketFlushed = TRUE;

        history_offset = 0;
        PacketAtFront = TRUE;
    }

    history_ptr = &(history_buffer[history_offset]);
    dest = *pdest;

    if (!dest)
        return -1;

    if (*dest_size > src_size)
        DstSize = src_size;
    else
        DstSize = *dest_size;

    cel_bitstream_attach(bs, dest, DstSize);

    pSrcPtr = src;
    pSrcEnd = &(src[src_size - 1]);
    //pDstEnd = &(dest[DstSize - 1]);

    while (pSrcPtr < (pSrcEnd - 2))
    {
        Sym1 = pSrcPtr[0];
        Sym2 = pSrcPtr[1];
        Sym3 = pSrcPtr[2];

        *history_ptr++ = *pSrcPtr++;

        MatchIndex = MPPC_MATCH_INDEX(Sym1, Sym2, Sym3);
        MatchPtr = &(history_buffer[mppc->match_buffer[MatchIndex]]);

        if (MatchPtr != (history_ptr - 1))
            mppc->match_buffer[MatchIndex] = (U16) (history_ptr - history_buffer);

        if (mppc->history_ptr < history_ptr)
            mppc->history_ptr = history_ptr;

        if ((Sym1 != *(MatchPtr - 1)) || (Sym2 != MatchPtr[0]) || (Sym3 != MatchPtr[1]) ||
            (&MatchPtr[1] > mppc->history_ptr) || (MatchPtr == history_buffer) ||
            (MatchPtr == (history_ptr - 1)) || (MatchPtr == history_ptr))
        {
            if (((bs->position / 8) + 2) > (DstSize - 1))
            {
                mppc_context_reset(mppc, TRUE);
                *pflags |= PACKET_FLUSHED;
                *pflags |= level;
                *pdest = src;
                *dest_size = src_size;
                return 1;
            }

            accumulator = Sym1;

#ifdef DEBUG_MPPC
            WLog_DBG(TAG, "%c", accumulator);
#endif

            if (accumulator < 0x80)
            {
                /* 8 bits of literal are encoded as-is */
                cel_bitstream_write_bits(bs, accumulator, 8);
            }
            else
            {
                /* bits 10 followed by lower 7 bits of literal */
                accumulator = 0x100 | (accumulator & 0x7F);
                cel_bitstream_write_bits(bs, accumulator, 9);
            }
        }
        else
        {
            CopyOffset = (history_size - 1) & (history_ptr - MatchPtr);

            *history_ptr++ = Sym2;
            *history_ptr++ = Sym3;
            pSrcPtr += 2;

            LengthOfMatch = 3;
            MatchPtr += 2;

            while ((*pSrcPtr == *MatchPtr) && (pSrcPtr < pSrcEnd) && (MatchPtr <= mppc->history_ptr))
            {
                MatchPtr++;
                *history_ptr++ = *pSrcPtr++;
                LengthOfMatch++;
            }

#ifdef DEBUG_MPPC
            WLog_DBG(TAG, "<%d,%d>", (int) CopyOffset, (int) LengthOfMatch);
#endif

            /* Encode CopyOffset */

            if (((bs->position / 8) + 7) > (DstSize - 1))
            {
                mppc_context_reset(mppc, TRUE);
                *pflags |= PACKET_FLUSHED;
                *pflags |= level;
                *pdest = src;
                *dest_size = src_size;
                return 1;
            }

            if (level) /* RDP5 */
            {
                if (CopyOffset < 64)
                {
                    /* bits 11111 + lower 6 bits of CopyOffset */
                    accumulator = 0x07C0 | (CopyOffset & 0x003F);
                    cel_bitstream_write_bits(bs, accumulator, 11);
                }
                else if ((CopyOffset >= 64) && (CopyOffset < 320))
                {
                    /* bits 11110 + lower 8 bits of (CopyOffset - 64) */
                    accumulator = 0x1E00 | ((CopyOffset - 64) & 0x00FF);
                    cel_bitstream_write_bits(bs, accumulator, 13);
                }
                else if ((CopyOffset >= 320) && (CopyOffset < 2368))
                {
                    /* bits 1110 + lower 11 bits of (CopyOffset - 320) */
                    accumulator = 0x7000 | ((CopyOffset - 320) & 0x07FF);
                    cel_bitstream_write_bits(bs, accumulator, 15);
                }
                else
                {
                    /* bits 110 + lower 16 bits of (CopyOffset - 2368) */
                    accumulator = 0x060000 | ((CopyOffset - 2368) & 0xFFFF);
                    cel_bitstream_write_bits(bs, accumulator, 19);
                }
            }
            else /* RDP4 */
            {
                if (CopyOffset < 64)
                {
                    /* bits 1111 + lower 6 bits of CopyOffset */
                    accumulator = 0x03C0 | (CopyOffset & 0x003F);
                    cel_bitstream_write_bits(bs, accumulator, 10);
                }
                else if ((CopyOffset >= 64) && (CopyOffset < 320))
                {
                    /* bits 1110 + lower 8 bits of (CopyOffset - 64) */
                    accumulator = 0x0E00 | ((CopyOffset - 64) & 0x00FF);
                    cel_bitstream_write_bits(bs, accumulator, 12);
                }
                else if ((CopyOffset >= 320) && (CopyOffset < 8192))
                {
                    /* bits 110 + lower 13 bits of (CopyOffset - 320) */
                    accumulator = 0xC000 | ((CopyOffset - 320) & 0x1FFF);
                    cel_bitstream_write_bits(bs, accumulator, 16);
                }
            }

            /* Encode LengthOfMatch */

            if (LengthOfMatch == 3)
            {
                /* 0 + 0 lower bits of LengthOfMatch */
                cel_bitstream_write_bits(bs, 0, 1);
            }
            else if ((LengthOfMatch >= 4) && (LengthOfMatch < 8))
            {
                /* 10 + 2 lower bits of LengthOfMatch */
                accumulator = 0x0008 | (LengthOfMatch & 0x0003);
                cel_bitstream_write_bits(bs, accumulator, 4);
            }
            else if ((LengthOfMatch >= 8) && (LengthOfMatch < 16))
            {
                /* 110 + 3 lower bits of LengthOfMatch */
                accumulator = 0x0030 | (LengthOfMatch & 0x0007);
                cel_bitstream_write_bits(bs, accumulator, 6);
            }
            else if ((LengthOfMatch >= 16) && (LengthOfMatch < 32))
            {
                /* 1110 + 4 lower bits of LengthOfMatch */
                accumulator = 0x00E0 | (LengthOfMatch & 0x000F);
                cel_bitstream_write_bits(bs, accumulator, 8);
            }
            else if ((LengthOfMatch >= 32) && (LengthOfMatch < 64))
            {
                /* 11110 + 5 lower bits of LengthOfMatch */
                accumulator = 0x03C0 | (LengthOfMatch & 0x001F);
                cel_bitstream_write_bits(bs, accumulator, 10);
            }
            else if ((LengthOfMatch >= 64) && (LengthOfMatch < 128))
            {
                /* 111110 + 6 lower bits of LengthOfMatch */
                accumulator = 0x0F80 | (LengthOfMatch & 0x003F);
                cel_bitstream_write_bits(bs, accumulator, 12);
            }
            else if ((LengthOfMatch >= 128) && (LengthOfMatch < 256))
            {
                /* 1111110 + 7 lower bits of LengthOfMatch */
                accumulator = 0x3F00 | (LengthOfMatch & 0x007F);
                cel_bitstream_write_bits(bs, accumulator, 14);
            }
            else if ((LengthOfMatch >= 256) && (LengthOfMatch < 512))
            {
                /* 11111110 + 8 lower bits of LengthOfMatch */
                accumulator = 0xFE00 | (LengthOfMatch & 0x00FF);
                cel_bitstream_write_bits(bs, accumulator, 16);
            }
            else if ((LengthOfMatch >= 512) && (LengthOfMatch < 1024))
            {
                /* 111111110 + 9 lower bits of LengthOfMatch */
                accumulator = 0x3FC00 | (LengthOfMatch & 0x01FF);
                cel_bitstream_write_bits(bs, accumulator, 18);
            }
            else if ((LengthOfMatch >= 1024) && (LengthOfMatch < 2048))
            {
                /* 1111111110 + 10 lower bits of LengthOfMatch */
                accumulator = 0xFF800 | (LengthOfMatch & 0x03FF);
                cel_bitstream_write_bits(bs, accumulator, 20);
            }
            else if ((LengthOfMatch >= 2048) && (LengthOfMatch < 4096))
            {
                /* 11111111110 + 11 lower bits of LengthOfMatch */
                accumulator = 0x3FF000 | (LengthOfMatch & 0x07FF);
                cel_bitstream_write_bits(bs, accumulator, 22);
            }
            else if ((LengthOfMatch >= 4096) && (LengthOfMatch < 8192))
            {
                /* 111111111110 + 12 lower bits of LengthOfMatch */
                accumulator = 0xFFE000 | (LengthOfMatch & 0x0FFF);
                cel_bitstream_write_bits(bs, accumulator, 24);
            }
            else if (((LengthOfMatch >= 8192) && (LengthOfMatch < 16384)) && level) /* RDP5 */
            {
                /* 1111111111110 + 13 lower bits of LengthOfMatch */
                accumulator = 0x3FFC000 | (LengthOfMatch & 0x1FFF);
                cel_bitstream_write_bits(bs, accumulator, 26);
            }
            else if (((LengthOfMatch >= 16384) && (LengthOfMatch < 32768)) && level) /* RDP5 */
            {
                /* 11111111111110 + 14 lower bits of LengthOfMatch */
                accumulator = 0xFFF8000 | (LengthOfMatch & 0x3FFF);
                cel_bitstream_write_bits(bs, accumulator, 28);
            }
            else if (((LengthOfMatch >= 32768) && (LengthOfMatch < 65536)) && level) /* RDP5 */
            {
                /* 111111111111110 + 15 lower bits of LengthOfMatch */
                accumulator = 0x3FFF0000 | (LengthOfMatch & 0x7FFF);
                cel_bitstream_write_bits(bs, accumulator, 30);
            }
        }
    }

    /* Encode trailing symbols as literals */

    while (pSrcPtr <= pSrcEnd)
    {
        if (((bs->position / 8) + 2) > (DstSize - 1))
        {
            mppc_context_reset(mppc, TRUE);
            *pflags |= PACKET_FLUSHED;
            *pflags |= level;
            *pdest = src;
            *dest_size = src_size;
            return 1;
        }

        accumulator = *pSrcPtr;

#ifdef DEBUG_MPPC
        WLog_DBG(TAG, "%c", accumulator);
#endif

        if (accumulator < 0x80)
        {
            /* 8 bits of literal are encoded as-is */
            cel_bitstream_write_bits(bs, accumulator, 8);
        }
        else
        {
            /* bits 10 followed by lower 7 bits of literal */
            accumulator = 0x100 | (accumulator & 0x7F);
            cel_bitstream_write_bits(bs, accumulator, 9);
        }

        *history_ptr++ = *pSrcPtr++;
    }

    cel_bitstream_flush(bs);

    *pflags |= PACKET_COMPRESSED;
    *pflags |= level;

    if (PacketAtFront)
        *pflags |= PACKET_AT_FRONT;

    if (PacketFlushed)
        *pflags |= PACKET_FLUSHED;

    *dest_size = ((bs->position + 7) / 8);

    mppc->history_ptr = history_ptr;
    mppc->history_offset = history_ptr - history_buffer;

    return 1;
}

void mppc_set_cmpr_level(CelMppcContext *mppc, U32 level)
{
    if (level < 1)
    {
        mppc->level = 0;
        mppc->history_size = 8192;
    }
    else
    {
        mppc->level = 1;
        mppc->history_size = 65536;
    }
}

void mppc_context_reset(CelMppcContext *mppc, BOOL flush)
{
    memset(&(mppc->history_buffer), 0, sizeof(mppc->history_buffer));
    memset(&(mppc->match_buffer), 0, sizeof(mppc->match_buffer));

    if (flush)
        mppc->history_offset = mppc->history_size + 1;
    else
        mppc->history_offset = 0;

    mppc->history_ptr = &(mppc->history_buffer[mppc->history_offset]);
}

CelMppcContext *mppc_context_new(U32 level, BOOL compressor)
{
    CelMppcContext* mppc;

    mppc = (CelMppcContext *)calloc(1, sizeof(CelMppcContext));

    if (mppc)
    {
        mppc->compressor = compressor;

        if (level < 1)
        {
            mppc->level = 0;
            mppc->history_size = 8192;
        }
        else
        {
            mppc->level = 1;
            mppc->history_size = 65536;
        }

        mppc->bs = cel_bitstream_new();
        if (!mppc->bs)
        {
            free(mppc);
            return NULL;
        }
        mppc_context_reset(mppc, FALSE);
    }

    return mppc;
}

void mppc_context_free(CelMppcContext* mppc)
{
    if (mppc != NULL)
    {
        cel_bitstream_free(mppc->bs);
        free(mppc);
    }
}
