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
#ifndef __CEL_CRYPTO_BER_H__
#define __CEL_CRYPTO_BER_H__

#include "cel/types.h"
#include "cel/stream.h"
/* BER type */

/* Class - bits 8 and 7 */
#define CEL_BER_CLASS_MASK          0xC0
#define CEL_BER_CLASS_UNIV          0x00 /* 0 0 */
#define CEL_BER_CLASS_APPL          0x40 /* 0 1 */
#define CEL_BER_CLASS_CTXT          0x80 /* 1 0 */
#define CEL_BER_CLASS_PRIV          0xC0 /* 1 1 */

/* P/C - bit 6 */
#define CEL_BER_PC_MASK             0x20
#define CEL_BER_PRIMITIVE           0x00 /* 0 */
#define CEL_BER_CONSTRUCT           0x20 /* 1 */

/* Tag - bits 5 to 1 */
#define CEL_BER_TAG_MASK            0x1F
#define CEL_BER_TAG_BOOLEAN         0x01
#define CEL_BER_TAG_INTEGER         0x02
#define CEL_BER_TAG_BIT_STRING      0x03
#define CEL_BER_TAG_OCTET_STRING    0x04
#define CEL_BER_TAG_OBJECT_IDENFIER 0x06
#define CEL_BER_TAG_ENUMERATED      0x0A
#define CEL_BER_TAG_SEQUENCE        0x10
#define CEL_BER_TAG_SEQUENCE_OF     0x10

#define CEL_BER_PC(_pc)    (_pc ? CEL_BER_CONSTRUCT : CEL_BER_PRIMITIVE)

#ifdef __cplusplus
extern "C" {
#endif

BOOL cel_ber_read_length(CelStream *s, int *length);
int cel_ber_write_length(CelStream *s, int length);
int cel_ber_sizeof_length(int length);

BOOL cel_ber_read_universal_tag(CelStream *s, BYTE tag, BOOL pc);
int cel_ber_write_universal_tag(CelStream *s, BYTE tag, BOOL pc);

BOOL cel_ber_read_application_tag(CelStream *s, BYTE tag, int *length);
void cel_ber_write_application_tag(CelStream *s, BYTE tag, int length);

BOOL cel_ber_read_enumerated(CelStream *s, BYTE *enumerated, BYTE count);
void cel_ber_write_enumerated(CelStream *s, BYTE enumerated, BYTE count);

BOOL cel_ber_read_contextual_tag(CelStream *s, BYTE tag, int *length, BOOL pc);
int cel_ber_write_contextual_tag(CelStream *s, BYTE tag, int length, BOOL pc);
int cel_ber_sizeof_contextual_tag(int length);

BOOL cel_ber_read_sequence_tag(CelStream *s, int *length);
int cel_ber_write_sequence_tag(CelStream *s, int length);
int cel_ber_sizeof_sequence(int length);
int cel_ber_sizeof_sequence_tag(int length);

BOOL cel_ber_read_bit_string(CelStream *s, int *length, BYTE *padding);
int cel_ber_write_octet_string(CelStream *s, const BYTE *oct_str, int length);
BOOL cel_ber_read_octet_string_tag(CelStream *s, int *length);
int cel_ber_write_octet_string_tag(CelStream *s, int length);
int cel_ber_sizeof_octet_string(int length);

BOOL cel_ber_read_bool(CelStream *s, BOOL *value);
void cel_ber_write_bool(CelStream *s, BOOL value);

BOOL cel_ber_read_integer(CelStream *s, U32 *value);
int cel_ber_write_integer(CelStream *s, U32 value);
BOOL cel_ber_read_integer_length(CelStream *s, int *length);
int cel_ber_sizeof_integer(U32 value);

#ifdef __cplusplus
}
#endif

#endif
