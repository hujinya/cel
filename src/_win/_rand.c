/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2016 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/_win/_rand.h"

#ifdef _CEL_WIN
void os_rand_bytes(void *out, size_t out_len)
{
    char *out_ptr = out;
    ULONG out_bytes_this_pass;

    while (out_len > 0) 
    {
        out_bytes_this_pass = ~(((U32)(-1)) >> 1);
        if (out_bytes_this_pass > out_len)
            out_bytes_this_pass = out_len;
        RtlGenRandom(out_ptr, out_bytes_this_pass);
        out_len -= out_bytes_this_pass;
        out_ptr += out_bytes_this_pass;
    }
}
#endif
