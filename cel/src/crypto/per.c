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
#include "cel/crypto/per.h"

BOOL cel_per_read_length(CelStream *s, U16 *length)
{
    BYTE byte;

    if (cel_stream_get_remaining_length(s) < 1)
        return FALSE;

    cel_stream_read_u8(s, byte);

    if (byte & 0x80)
    {
        if (cel_stream_get_remaining_length(s) < 1)
            return FALSE;

        byte &= ~(0x80);
        *length = (byte << 8);
        cel_stream_read_u8(s, byte);
        *length += byte;
    }
    else
    {
        *length = byte;
    }

    return TRUE;
}

void cel_per_write_length(CelStream *s, int length)
{
    if (length > 0x7F)
        cel_stream_write_u16_be(s, (length | 0x8000));
    else
        cel_stream_write_u8(s, length);
}

BOOL cel_per_read_choice(CelStream *s, BYTE *choice)
{
    if (cel_stream_get_remaining_length(s) < 1)
        return FALSE;

    cel_stream_read_u8(s, *choice);
    return TRUE;
}

void cel_per_write_choice(CelStream *s, BYTE choice)
{
    cel_stream_write_u8(s, choice);
}

BOOL cel_per_read_selection(CelStream *s, BYTE *selection)
{
    if (cel_stream_get_remaining_length(s) < 1)
        return FALSE;

    cel_stream_read_u8(s, *selection);
    return TRUE;
}

void cel_per_write_selection(CelStream *s, BYTE selection)
{
    cel_stream_write_u8(s, selection);
}

BOOL cel_per_read_number_of_sets(CelStream *s, BYTE *number)
{
    if (cel_stream_get_remaining_length(s) < 1)
        return FALSE;

    cel_stream_read_u8(s, *number);
    return TRUE;
}

void cel_per_write_numcel_ber_of_sets(CelStream *s, BYTE number)
{
    cel_stream_write_u8(s, number);
}

BOOL cel_per_read_padding(CelStream *s, int length)
{
    if (((int) cel_stream_get_remaining_length(s)) < length)
        return FALSE;

    cel_stream_seek(s, length);
    return TRUE;
}

void cel_per_write_padding(CelStream *s, int length)
{
    int i;

    for (i = 0; i < length; i++)
        cel_stream_write_u8(s, 0);
}

BOOL cel_per_read_integer(CelStream *s, U32* integer)
{
    U16 length;

    if (!cel_per_read_length(s, &length))
        return FALSE;

    if (cel_stream_get_remaining_length(s) < length)
        return FALSE;

    if (length == 0)
        *integer = 0;
    else if (length == 1)
        cel_stream_read_u8(s, *integer);
    else if (length == 2)
        cel_stream_read_u16_be(s, *integer);
    else
        return FALSE;

    return TRUE;
}

void cel_per_write_integer(CelStream *s, U32 integer)
{
    if (integer <= 0xFF)
    {
        cel_per_write_length(s, 1);
        cel_stream_write_u8(s, integer);
    }
    else if (integer <= 0xFFFF)
    {
        cel_per_write_length(s, 2);
        cel_stream_write_u16_be(s, integer);
    }
    else if (integer <= 0xFFFFFFFF)
    {
        cel_per_write_length(s, 4);
        cel_stream_write_u32_be(s, integer);
    }
}

BOOL cel_per_read_integer16(CelStream *s, U16 *integer, U16 min)
{
    if (cel_stream_get_remaining_length(s) < 2)
        return FALSE;

    cel_stream_read_u16_be(s, *integer);

    if (*integer + min > 0xFFFF)
        return FALSE;

    *integer += min;

    return TRUE;
}

void cel_per_write_integer16(CelStream *s, U16 integer, U16 min)
{
    cel_stream_write_u16_be(s, integer - min);
}

BOOL cel_per_read_enumerated(CelStream *s, BYTE *enumerated, BYTE count)
{
    if (cel_stream_get_remaining_length(s) < 1)
        return FALSE;

    cel_stream_read_u8(s, *enumerated);

    /* check that enumerated value falls within expected range */
    if (*enumerated + 1 > count)
        return FALSE;

    return TRUE;
}

void cel_per_write_enumerated(CelStream *s, BYTE enumerated, BYTE count)
{
    cel_stream_write_u8(s, enumerated);
}

BOOL cel_per_read_object_identifier(CelStream *s, BYTE oid[6])
{
    BYTE t12;
    U16 length;
    BYTE a_oid[6];

    if (!cel_per_read_length(s, &length))
        return FALSE;

    if (length != 5)
        return FALSE;

    if (cel_stream_get_remaining_length(s) < length)
        return FALSE;

    cel_stream_read_u8(s, t12); /* first two tuples */
    a_oid[0] = (t12 >> 4);
    a_oid[1] = (t12 & 0x0F);

    cel_stream_read_u8(s, a_oid[2]); /* tuple 3 */
    cel_stream_read_u8(s, a_oid[3]); /* tuple 4 */
    cel_stream_read_u8(s, a_oid[4]); /* tuple 5 */
    cel_stream_read_u8(s, a_oid[5]); /* tuple 6 */

    if ((a_oid[0] == oid[0]) && (a_oid[1] == oid[1]) &&
        (a_oid[2] == oid[2]) && (a_oid[3] == oid[3]) &&
        (a_oid[4] == oid[4]) && (a_oid[5] == oid[5]))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void cel_per_write_object_identifier(CelStream *s, BYTE oid[6])
{
    BYTE t12 = (oid[0] << 4) & (oid[1] & 0x0F);
    cel_stream_write_u8(s, 5); /* length */
    cel_stream_write_u8(s, t12); /* first two tuples */
    cel_stream_write_u8(s, oid[2]); /* tuple 3 */
    cel_stream_write_u8(s, oid[3]); /* tuple 4 */
    cel_stream_write_u8(s, oid[4]); /* tuple 5 */
    cel_stream_write_u8(s, oid[5]); /* tuple 6 */
}

void cel_per_write_string(CelStream *s, BYTE *str, int length)
{
    int i;

    for (i = 0; i < length; i++)
        cel_stream_write_u8(s, str[i]);
}

BOOL cel_per_read_octet_string(CelStream *s, BYTE *oct_str, int length, int min)
{
    int i;
    U16 mlength;
    BYTE *a_oct_str;

    if (!cel_per_read_length(s, &mlength))
        return FALSE;

    if (mlength + min != length)
        return FALSE;

    if (((int) cel_stream_get_remaining_length(s)) < length)
        return FALSE;

    a_oct_str = cel_stream_get_pointer(s);
    cel_stream_seek(s, length);

    for (i = 0; i < length; i++)
    {
        if (a_oct_str[i] != oct_str[i])
            return FALSE;
    }

    return TRUE;
}

void cel_per_write_octet_string(CelStream *s, BYTE *oct_str, int length, int min)
{
    int i;
    int mlength;

    mlength = (length - min >= 0) ? length - min : min;

    cel_per_write_length(s, mlength);

    for (i = 0; i < length; i++)
        cel_stream_write_u8(s, oct_str[i]);
}

BOOL cel_per_read_numeric_string(CelStream *s, int min)
{
    int length;
    U16 mlength;

    if (!cel_per_read_length(s, &mlength))
        return FALSE;

    length = (mlength + min + 1) / 2;

    if (((int) cel_stream_get_remaining_length(s)) < length)
        return FALSE;

    cel_stream_seek(s, length);
    return TRUE;
}

void cel_per_write_numeric_string(CelStream *s, BYTE *num_str, int length, int min)
{
    int i;
    int mlength;
    BYTE num, c1, c2;

    mlength = (length - min >= 0) ? length - min : min;

    cel_per_write_length(s, mlength);

    for (i = 0; i < length; i += 2)
    {
        c1 = num_str[i];
        c2 = ((i + 1) < length) ? num_str[i + 1] : 0x30;

        c1 = (c1 - 0x30) % 10;
        c2 = (c2 - 0x30) % 10;
        num = (c1 << 4) | c2;

        cel_stream_write_u8(s, num); /* string */
    }
}
