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
#ifndef __CEL_TYPES_H__
#define __CEL_TYPES_H__

#include "cel/config.h"

#if defined(_CEL_UNIX)
#include "cel/_unix/_types.h"
#elif defined(_CEL_WIN)
#include "cel/_win/_types.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined __x86_64__
#ifndef _UINTPTR_T_DEFINED
typedef unsigned long uintptr_t;
#define _UINTPTR_T_DEFINED
#endif
#else
#ifndef _UINTPTR_T_DEFINED
typedef unsigned int uintptr_t;
#define _UINTPTR_T_DEFINED
#endif
#endif

#define ADDRESS_BITS   (8 * sizeof(void*))

#define CEL_INDENT  "    "
#define CEL_INDENTLEN  (sizeof(CEL_INDENT) - 1)

#ifndef FALSE
#define FALSE 0
#define TRUE (!FALSE)
#endif

#ifndef _WINDEF_
typedef int               BOOL;
#endif

#define U(x)               x##U
#define L(x)                  x
#define UL(x)              x##UL
#if defined(_MSC_VER) || defined(__WATCOMC__)
#define LL(x)             x##I64
#define ULL(x)           x##UI64
#define LLFMT            "%I64d"   /* As in printf("%I64d", ...) */
#define ULLFMT           "%I64u"   /* As in printf("%I64u", ...) */
#else
#define LL(x)              x##LL
#define ULL(x)            x##ULL
#define LLFMT             "%lld"   /* As in printf("%lld", ...) */
#define ULLFMT            "%llu"   /* As in printf("%llu", ...) */
#endif

#define CEL_GNLEN           128    /* Group name length */
#define CEL_UNLEN           128    /* User name length */
#define CEL_PWLEN           128    /* Password length */

#define CEL_SNLEN            80    /* Service name length */
#define CEL_HNLEN            64    /* Host name length */
#define CEL_DNLEN            64    /* Domain name length */
#define CEL_NPLEN             6    /* Net port(protcol)length */
#define CEL_IFNLEN           80    /* Interface name length */
#define CEL_DBNLEN           64    /* Database name length */
#define CEL_KNLEN            64    /* Key name length */
#define CEL_ADDRLEN         140    /* Address string length 
                                      5(@) + 128 + 1(:) + 5 + 1(\0) */

#define CEL_PATHLEN         260    /* Full path length */
#define CEL_DIRLEN          256    /* Path length */
#define CEL_FNLEN            64    /* File name length */
#define CEL_EXTLEN           64    /* File extension length */

#define CEL_PRONUM          128    /* The maximum number of processors */
#define CEL_HDNUM            16    /* The maximum number of processors harddisk */
#define CEL_NETDEVNUM        16    /* The maximum number of processors net device */
#define CEL_THDNUM           64    /* Max threads for each process */
#define CEL_BUFNUM            4    /* The number of static cache for each function */

#define CEL_ERRSLEN         512    /* Error string length */
#define CEL_ARGSLEN         512    /* Args length */
#define CEL_CMDLLEN         512    /* Command line length */
#define CEL_LINELEN        1024    /* Line buffer length */

#define CEL_DIFF(i1, i2)    ((i1) > (i2) ? ((i1) - (i2)) : ((i1) + (0 - (i2))))
#define CEL_OFFSET(structure, member)  (size_t)&(((structure*)0)->member)
/* Flag operation */
#define CEL_ZEROFLAG(flags) (flags) = 0
#define CEL_SETFLAG(flags, flag)  (flags) |= (flag)
#define CEL_CLRFLAG(flags, flag)  (flags) &= (~(flag))
#define CEL_CHECKFLAG(flags, flag)  (((flags) & (flag)) == flag)


#define CEL_ISSPACE_A(ch) ((ch) == ' ' || (ch) == '\t'   \
    || (ch) == '\n' || (ch) == '\r')
#define CEL_ISSPACE_W(ch) ((ch) == L' ' || (ch) == L'\t'   \
    || (ch) == L'\n' || (ch) == L'\r')
#define CEL_ISUPPER_A(ch) ((ch) >= 'A' && (ch) <= 'F')
#define CEL_ISUPPER_W(ch) ((ch) >= L'A' && (ch) <= L'F')
#define CEL_ISDIGIT_A(ch) ((ch) >= '0' && (ch) <= '9')
#define CEL_ISDIGIT_W(ch) ((ch) >= L'0' && (ch) <= L'9')
#define CEL_ISXDIGIT_A(ch) (((ch) >= '0' && (ch) <= '9') \
    || ((ch) >= 'a' && (ch) <= 'f') \
    || ((ch) >= 'A' && (ch) <= 'F'))
#define CEL_ISXDIGIT_W(ch) (((ch) >= L'0' && (ch) <= L'9') \
    || ((ch) >= L'a' && (ch) <= L'f') \
    || ((ch) >= L'A' && (ch) <= L'F'))
#define CEL_TOLOWER_A(ch) ((ch) + 32)
#define CEL_TOLOWER_W(ch) ((ch) + 32)

#ifdef _UNICODE
#define CEL_ISSPACE CEL_ISSPACE_W
#define CEL_ISUPPER CEL_ISUPPER_W
#define CEL_ISDIGIT CEL_ISDIGIT_W
#define CEL_ISXDIGIT CEL_ISXDIGIT_W
#define CEL_TOLOWER CEL_TOLOWER_W
#else
#define CEL_ISSPACE CEL_ISSPACE_A
#define CEL_ISUPPER CEL_ISUPPER_A
#define CEL_ISDIGIT CEL_ISDIGIT_A
#define CEL_ISXDIGIT CEL_ISXDIGIT_A
#define CEL_TOLOWER CEL_TOLOWER_A
#endif
#define CEL_ISVALID(n, min, max) (((n) >= (min) && (n) <= (max)))
#define CEL_SETVALID(n, min, max) \
    ((n < min) ? n = min : ((n > max) ? n = max : 0))
#define CEL_SETMAX(n, max) ((n) < (max) ? (n) = (max) : 0)

/* Function type defines */
typedef int   (* CelMainFunc) (int argc, TCHAR **argv);
typedef int   (* CelConstructFunc) (void *);
typedef void  (* CelDestroyFunc) (void *);
typedef void *(* CelMallocFunc) (size_t size);
typedef void *(* CelReallocFunc) (void *mem_address, size_t new_size);
typedef void  (* CelFreeFunc) (void *mem_address);
typedef void  (* CelVoidFunc) (void);
typedef void  (* CelFunc) (void *data, void *user_data);
/* return 0 = continue;-1 = error;1 = break */
typedef int   (* CelEachFunc) (void *value, void *user_data); 
typedef int   (* CelKeyValuePairEachFunc) (
    void *key, void *value, void *user_data);
typedef BOOL  (* CelEqualFunc) (const void *src, const void *dest);
typedef  int  (* CelCompareFunc) (const void *src, const void *dest);

#ifdef __cplusplus
}
#endif

#endif
