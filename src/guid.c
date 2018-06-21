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
#include "cel/guid.h"
#include "cel/rand.h"

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.[proto-quic-master\src\base\guid.cc]

BOOL cel_guid_isvalid_internal(const char *guid, BOOL strict) 
{
    const size_t kGUIDLength = 36U;
    size_t i, len;
    char current;

    if ((len = strlen(guid)) != kGUIDLength)
        return FALSE;

    for (i = 0; i < len; ++i) 
    {
        current = guid[i];
        if (i == 8 || i == 13 || i == 18 || i == 23) 
        {
            if (current != '-')
                return FALSE;
        } 
        else 
        {
            if ((strict && !CEL_ISXDIGIT(current)))
                return FALSE;
        }
    }

    return TRUE;
}

char *cel_guid_rand2str(const U64 bytes[2], char *out) 
{
    sprintf(out, "%08x-%04x-%04x-%04x-%012llx",
        (unsigned int)(bytes[0] >> 32),
        (unsigned int)((bytes[0] >> 16) & 0x0000ffff),
        (unsigned int)(bytes[0] & 0x0000ffff),
        (unsigned int)(bytes[1] >> 48),
        bytes[1] & 0x0000ffffffffffffULL);
    return out;
}

char *cel_guid_generate(char *out) 
{
  U64 sixteen_bytes[2];
  
  // Set the GUID to version 4 as described in RFC 4122, section 4.4.
  // The format of GUID version 4 must be xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx,
  // where y is one of [8, 9, A, B].
  sixteen_bytes[0] = cel_rand_u64();
  sixteen_bytes[1] = cel_rand_u64();

  // Clear the version bits and set the version to 4:
  sixteen_bytes[0] &= 0xffffffffffff0fffULL;
  sixteen_bytes[0] |= 0x0000000000004000ULL;

  // Set the two most significant bits (bits 6 and 7) of the
  // clock_seq_hi_and_reserved to zero and one, respectively:
  sixteen_bytes[1] &= 0x3fffffffffffffffULL;
  sixteen_bytes[1] |= 0x8000000000000000ULL;

  return cel_guid_rand2str(sixteen_bytes, out);
}

BOOL cel_guid_isvalid(const char *guid) 
{
  return cel_guid_isvalid_internal(guid, FALSE /* strict */);
}

BOOL cel_guid_isvalid_str(const char *guid) 
{
  return cel_guid_isvalid_internal(guid, TRUE /* strict */);
}

