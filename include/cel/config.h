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
#ifndef __CEL_CONFIG_H__
#define __CEL_CONFIG_H__

#ifdef _WIN32
#undef _CEL_UNIX
#define _CEL_WIN
#else
#undef _CEL_WIN
#define _CEL_UNIX
#endif

#define _CEL_ASSERT  /* log.h */
#define _CEL_ERR     /* log.h */

#define GCC_VERSION \
    (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

//#define _CEL_BIGEDIAN
#define _CEL_MULTITHREAD

#define HAVE_LIBZ             0

#define HAVE_OPENSSL_CRYPTO_H 1
#define HAVE_OPENSSL_SSL_H    1



#endif
