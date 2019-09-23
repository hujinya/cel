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
#include "cel/rand.h"
#include "cel/convert.h"

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.[proto-quic-master\src\base\rand_util.c]

U64 cel_rand_generator(U64 range) 
{
    // We must discard random results above this number, as they would
    // make the random generator non-uniform (consider e.g. if
    // MAX_UINT64 was 7 and |range| was 5, then a result of 1 would be twice
    // as likely as a result of 3 or 4).
    U64 max_acceptable_value = ((~((U64)(-1) >> 1)) / range) * range - 1;
    U64 value;

    do 
    {
        value = cel_rand_u64();
    }while (value > max_acceptable_value);

    return value % range;
}

int cel_rand_int(int min, int max) 
{
    U64 range = (U64)(max) - min + 1;
    // |range| is at most UINT_MAX + 1, so the result of cel_rand_generator(range)
    // is at most UINT_MAX.  Hence it's safe to cast it from U64 to int64_t.
    int result = (int)(min + cel_rand_generator(range));

    return result;
}

double cel_rand_double_interval(U64 bits) 
{
    // We try to get maximum precision by masking out as many bits as will fit
    // in the target type's mantissa, and raising it to an appropriate power to
    // produce output in the range [0, 1).  For IEEE 754 doubles, the mantissa
    // is expected to accommodate 53 bits.

    //static_assert(std::numeric_limits<double>::radix == 2,
    //    "otherwise use scalbn");
    //static const int kBits = std::numeric_limits<double>::digits;
    //U64 random_bits = bits & ((UINT64_C(1) << kBits) - 1);
    //double result = ldexp(static_cast<double>(random_bits), -1 * kBits);

    //return result;
    return 0;
}

char *cel_rand_str(char *out, size_t out_len)
{
    size_t olen1 = 0, olen2;
    U64 rand_bytes;

    while (olen1 < out_len)
    {
        os_rand_bytes(&rand_bytes, sizeof(rand_bytes));
        olen2 = out_len - olen1;
        cel_bintohex((BYTE *)&rand_bytes, sizeof(rand_bytes),
            (BYTE *)out + olen1, &olen2, 0);
        olen1 += olen2;
        //printf("cel_bintohex %d %d\r\n", olen1, olen2);
    }
    out[out_len - 1] = '\0';

    return out;
}
