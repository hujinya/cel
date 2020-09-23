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
#include "cel/_unix/_rand.h"
#include "cel/error.h"
#include "cel/log.h"

#ifdef _CEL_UNIX
void os_rand_bytes(void *out, size_t out_len)
{
    int urandom_fd;

    memset(out, 0, out_len);
    if ((urandom_fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC)) < 0)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Could not read from /dev/urandom: %s"),
            strerror(errno)));
    }
    read(urandom_fd, out, out_len);
    //puts((char *)out);
    close(urandom_fd);
}
#endif
