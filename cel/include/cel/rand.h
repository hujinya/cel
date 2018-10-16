/**
 * CEL(C Extension Library)
 * Copyright (C)2008-2017 Hu Jinya(hu_jinya@163.com) 
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
#ifndef __CEL_RAND_H__
#define __CEL_RAND_H__

#include "cel/config.h"
#if defined(_CEL_UNIX)
#include "cel/_unix/_rand.h"
#elif defined(_CEL_WIN)
#include "cel/_win/_rand.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define cel_rand_bytes os_rand_bytes
static __inline U64 cel_rand_u64()
{
    U64 number;
    os_rand_bytes(&number, sizeof(number));
    return number;
}
int cel_rand_int(int min, int max);
double cel_rand_double_interval(U64 bits);
static __inline double cel_rand_double()
{
    return cel_rand_double_interval(cel_rand_u64());
}
char *cel_rand_str(char *out, size_t out_len);

#ifdef __cplusplus
}
#endif

#endif
