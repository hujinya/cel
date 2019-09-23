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
#include "cel/crypto/ber.h"
#include "cel/error.h"
#include "cel/log.h"


BOOL cel_ber_read_length(CelStream *s, int *length)
{
    BYTE byte;

    if (cel_stream_get_remaining_length(s) < 1)
        return FALSE;
    cel_stream_read_u8(s, byte);

    if (byte & 0x80)
    {
        byte &= ~(0x80);

        if (cel_stream_get_remaining_length(s) < byte)
            return FALSE;

        if (byte == 1)
            cel_stream_read_u8(s, *length);
        else if (byte == 2)
            cel_stream_read_u16_be(s, *length);
        else
            return FALSE;
    }
    else
    {
        *length = byte;
    }
    return TRUE;
}

int cel_ber_write_length(CelStream *s, int length)
{
    if (length > 0xFF)
    {
        cel_stream_write_u8(s, 0x80 ^ 2);
        cel_stream_write_u16_be(s, length);
        return 3;
    }
    if (length > 0x7F)
    {
        cel_stream_write_u8(s, 0x80 ^ 1);
        cel_stream_write_u8(s, length);
        return 2;
    }
    cel_stream_write_u8(s, length);
    return 1;
}

int cel_ber_sizeof_length(int length)
{
    if (length > 0xFF)
        return 3;
    if (length > 0x7F)
        return 2;
    return 1;
}

BOOL cel_ber_read_universal_tag(CelStream *s, BYTE tag, BOOL pc)
{
    BYTE byte;

    if (cel_stream_get_remaining_length(s) < 1)
        return FALSE;
    cel_stream_read_u8(s, byte);

    if (byte != (CEL_BER_CLASS_UNIV | CEL_BER_PC(pc) | (CEL_BER_TAG_MASK & tag)))
        return FALSE;

    return TRUE;
}

int cel_ber_write_universal_tag(CelStream *s, BYTE tag, BOOL pc)
{
    cel_stream_write_u8(s, 
        (CEL_BER_CLASS_UNIV | CEL_BER_PC(pc)) | (CEL_BER_TAG_MASK & tag));
    return 1;
}

BOOL cel_ber_read_application_tag(CelStream *s, BYTE tag, int *length)
{
    BYTE byte;

    if (tag > 30)
    {
        if (cel_stream_get_remaining_length(s) < 1)
            return FALSE;
        cel_stream_read_u8(s, byte);

        if (byte != ((CEL_BER_CLASS_APPL | CEL_BER_CONSTRUCT) | CEL_BER_TAG_MASK))
            return FALSE;

        if (cel_stream_get_remaining_length(s) < 1)
            return FALSE;
        cel_stream_read_u8(s, byte);

        if (byte != tag)
            return FALSE;

        return cel_ber_read_length(s, length);
    }
    else
    {
        if (cel_stream_get_remaining_length(s) < 1)
            return FALSE;
        cel_stream_read_u8(s, byte);

        if (byte != ((CEL_BER_CLASS_APPL | CEL_BER_CONSTRUCT) 
            | (CEL_BER_TAG_MASK & tag)))
            return FALSE;

        return cel_ber_read_length(s, length);
    }

    return TRUE;
}

void cel_ber_write_application_tag(CelStream *s, BYTE tag, int length)
{
    if (tag > 30)
    {
        cel_stream_write_u8(s, (CEL_BER_CLASS_APPL | CEL_BER_CONSTRUCT) 
            | CEL_BER_TAG_MASK);
        cel_stream_write_u8(s, tag);
        cel_ber_write_length(s, length);
    }
    else
    {
        cel_stream_write_u8(s, (CEL_BER_CLASS_APPL | CEL_BER_CONSTRUCT) 
            | (CEL_BER_TAG_MASK & tag));
        cel_ber_write_length(s, length);
    }
}

BOOL cel_ber_read_contextual_tag(CelStream *s, BYTE tag, int *length, BOOL pc)
{
    BYTE byte;

    if (cel_stream_get_remaining_length(s) < 1)
        return FALSE;
    cel_stream_read_u8(s, byte);

    if (byte != ((CEL_BER_CLASS_CTXT | CEL_BER_PC(pc)) | (CEL_BER_TAG_MASK & tag)))
    {
        cel_stream_seek(s, -1);
        return FALSE;
    }

    return cel_ber_read_length(s, length);
}

int cel_ber_write_contextual_tag(CelStream *s, BYTE tag, int length, BOOL pc)
{
    cel_stream_write_u8(s, (CEL_BER_CLASS_CTXT | CEL_BER_PC(pc)) 
        | (CEL_BER_TAG_MASK & tag));
    return 1 + cel_ber_write_length(s, length);
}

int cel_ber_sizeof_contextual_tag(int length)
{
    return 1 + cel_ber_sizeof_length(length);
}

BOOL cel_ber_read_sequence_tag(CelStream *s, int *length)
{
    BYTE byte;

    if (cel_stream_get_remaining_length(s) < 1)
        return FALSE;
    cel_stream_read_u8(s, byte);

    if (byte != ((CEL_BER_CLASS_UNIV | CEL_BER_CONSTRUCT) 
        | (CEL_BER_TAG_SEQUENCE_OF)))
        return FALSE;

    return cel_ber_read_length(s, length);
}

int cel_ber_write_sequence_tag(CelStream *s, int length)
{
    cel_stream_write_u8(s, (CEL_BER_CLASS_UNIV | CEL_BER_CONSTRUCT) 
        | (CEL_BER_TAG_MASK & CEL_BER_TAG_SEQUENCE));
    return 1 + cel_ber_write_length(s, length);
}

int cel_ber_sizeof_sequence(int length)
{
    return 1 + cel_ber_sizeof_length(length) + length;
}

int cel_ber_sizeof_sequence_tag(int length)
{
    return 1 + cel_ber_sizeof_length(length);
}

BOOL cel_ber_read_enumerated(CelStream *s, BYTE *enumerated, BYTE count)
{
    int length;

    if (!cel_ber_read_universal_tag(s, CEL_BER_TAG_ENUMERATED, FALSE) 
        || !cel_ber_read_length(s, &length))
        return FALSE;

    if (length != 1 || cel_stream_get_remaining_length(s) < 1)
        return FALSE;

    cel_stream_read_u8(s, *enumerated);

    /* check that enumerated value falls within expected range */
    if (*enumerated + 1 > count)
        return FALSE;

    return TRUE;
}

void cel_ber_write_enumerated(CelStream *s, BYTE enumerated, BYTE count)
{
    cel_ber_write_universal_tag(s, CEL_BER_TAG_ENUMERATED, FALSE);
    cel_ber_write_length(s, 1);
    cel_stream_write_u8(s, enumerated);
}

BOOL cel_ber_read_bit_string(CelStream *s, int *length, BYTE *padding)
{
    if (!cel_ber_read_universal_tag(s, CEL_BER_TAG_BIT_STRING, FALSE) 
        || !cel_ber_read_length(s, length))
        return FALSE;

    if (cel_stream_get_remaining_length(s) < 1)
        return FALSE;
    cel_stream_read_u8(s, *padding);
    return TRUE;
}

int cel_ber_write_octet_string(CelStream *s, const BYTE *oct_str, int length)
{
    int size = 0;
    size += cel_ber_write_universal_tag(s, CEL_BER_TAG_OCTET_STRING, FALSE);
    size += cel_ber_write_length(s, length);
    cel_stream_write(s, oct_str, length);
    size += length;
    return size;
}

BOOL cel_ber_read_octet_string_tag(CelStream *s, int *length)
{
    return cel_ber_read_universal_tag(s, CEL_BER_TAG_OCTET_STRING, FALSE) 
        && cel_ber_read_length(s, length);
}

int cel_ber_write_octet_string_tag(CelStream *s, int length)
{
    cel_ber_write_universal_tag(s, CEL_BER_TAG_OCTET_STRING, FALSE);
    cel_ber_write_length(s, length);
    return 1 + cel_ber_sizeof_length(length);
}

int cel_ber_sizeof_octet_string(int length)
{
    return 1 + cel_ber_sizeof_length(length) + length;
}

BOOL cel_ber_read_bool(CelStream *s, BOOL *value)
{
    int length;
    BYTE v;

    if (!cel_ber_read_universal_tag(s, CEL_BER_TAG_BOOLEAN, FALSE) 
        || !cel_ber_read_length(s, &length))
        return FALSE;

    if (length != 1 || cel_stream_get_remaining_length(s) < 1)
        return FALSE;

    cel_stream_read_u8(s, v);
    *value = (v ? TRUE : FALSE);
    return TRUE;
}

void cel_ber_write_bool(CelStream *s, BOOL value)
{
    cel_ber_write_universal_tag(s, CEL_BER_TAG_BOOLEAN, FALSE);
    cel_ber_write_length(s, 1);
    cel_stream_write_u8(s, (value == TRUE) ? 0xFF : 0);
}

int cel_ber_sizeof_integer(U32 value)
{
    if (value < 0x80)
        return 3;
    else if (value < 0x8000)
        return 4;
    else if (value < 0x800000)
        return 5;
    else if (value < 0x80000000)
        return 6;
    else
        /* treat as signed integer i.e. NT/HRESULT error codes */
        return 6;
    return 0;
}

BOOL cel_ber_read_integer(CelStream *s, U32* value)
{
    int length;

    if (!cel_ber_read_universal_tag(s, CEL_BER_TAG_INTEGER, FALSE) 
        || !cel_ber_read_length(s, &length) 
        || ((int) cel_stream_get_remaining_length(s)) < length)
        return FALSE;

    if (value == NULL)
    {
        // even if we don't care the integer value, check the announced size
        return cel_stream_safe_seek(s, length);
    }

    if (length == 1)
    {
        cel_stream_read_u8(s, *value);
    }
    else if (length == 2)
    {
        cel_stream_read_u16_be(s, *value);
    }
    else if (length == 3)
    {
        BYTE byte;
        cel_stream_read_u8(s, byte);
        cel_stream_read_u16_be(s, *value);
        *value += (byte << 16);
    }
    else if (length == 4)
    {
        cel_stream_read_u32_be(s, *value);
    }
    else if (length == 8)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Should implement reading an 8 bytes integer")));
        return FALSE;
    }
    else
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Should implement reading an integer with length=%d"), length));
        return FALSE;
    }

    return TRUE;
}

int cel_ber_write_integer(CelStream *s, U32 value)
{
    if (value <  0x80)
    {
        cel_ber_write_universal_tag(s, CEL_BER_TAG_INTEGER, FALSE);
        cel_ber_write_length(s, 1);
        cel_stream_write_u8(s, value);
        return 3;
    }
    else if (value <  0x8000)
    {
        cel_ber_write_universal_tag(s, CEL_BER_TAG_INTEGER, FALSE);
        cel_ber_write_length(s, 2);
        cel_stream_write_u16_be(s, value);
        return 4;
    }
    else if (value <  0x800000)
    {
        cel_ber_write_universal_tag(s, CEL_BER_TAG_INTEGER, FALSE);
        cel_ber_write_length(s, 3);
        cel_stream_write_u8(s, (value >> 16));
        cel_stream_write_u16_be(s, (value & 0xFFFF));
        return 5;
    }
    else if (value <  0x80000000)
    {
        cel_ber_write_universal_tag(s, CEL_BER_TAG_INTEGER, FALSE);
        cel_ber_write_length(s, 4);
        cel_stream_write_u32_be(s, value);
        return 6;
    }
    else
    {
        /* treat as signed integer i.e. NT/HRESULT error codes */
        cel_ber_write_universal_tag(s, CEL_BER_TAG_INTEGER, FALSE);
        cel_ber_write_length(s, 4);
        cel_stream_write_u32_be(s, value);
        return 6;
    }

    return 0;
}

BOOL cel_ber_read_integer_length(CelStream *s, int *length)
{
    return cel_ber_read_universal_tag(s, CEL_BER_TAG_INTEGER, FALSE) 
        && cel_ber_read_length(s, length);
}
