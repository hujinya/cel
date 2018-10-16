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
#ifndef __CEL_CRYPTO_PER_H__
#define __CEL_CRYPTO_PER_H__

#include "cel/types.h"
#include "cel/stream.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL cel_per_read_length(CelStream *s, U16 *length);
void cel_per_write_length(CelStream *s, int length);

BOOL cel_per_read_choice(CelStream *s, BYTE *choice);
void cel_per_write_choice(CelStream *s, BYTE choice);

BOOL cel_per_read_selection(CelStream *s, BYTE *selection);
void cel_per_write_selection(CelStream *s, BYTE selection);

BOOL cel_per_read_number_of_sets(CelStream *s, BYTE *number);
void cel_per_write_numcel_ber_of_sets(CelStream *s, BYTE number);

BOOL cel_per_read_padding(CelStream *s, int length);
void cel_per_write_padding(CelStream *s, int length);
BOOL cel_per_read_integer(CelStream *s, U32* integer);

BOOL cel_per_read_integer16(CelStream *s, U16 *integer, U16 min);
void cel_per_write_integer(CelStream *s, U32 integer);
void cel_per_write_integer16(CelStream *s, U16 integer, U16 min);

BOOL cel_per_read_enumerated(CelStream *s, BYTE *enumerated, BYTE count);
void cel_per_write_enumerated(CelStream *s, BYTE enumerated, BYTE count);

void cel_per_write_object_identifier(CelStream *s, BYTE oid[6]);
BOOL cel_per_read_object_identifier(CelStream *s, BYTE oid[6]);

BOOL cel_per_read_octet_string(CelStream *s, 
                               BYTE *oct_str, int length, int min);
void cel_per_write_octet_string(CelStream *s,
                                BYTE *oct_str, int length, int min);

BOOL cel_per_read_numeric_string(CelStream *s, int min);
void cel_per_write_numeric_string(CelStream *s, 
                                  BYTE *num_str, int length, int min);

#ifdef __cplusplus
}
#endif

#endif
