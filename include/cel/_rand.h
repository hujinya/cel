/**
 * CEL(C Extension Library)
 * Copyright (C)2008-2017 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_RAND_H__
#define __CEL_RAND_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

static __inline U64 cel_rand_u64(void)
{
    return ((U64)rand() << 32) + rand();
}

int cel_rand_int(int min, int max);
double cel_rand_double();
char *cel_rand_string(size_t length);

#ifdef __cplusplus
}
#endif

#endif
