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
#include "cel/convert.h"
#ifdef _CEL_UNIX
#include "cel/log.h"
int os_mb2unicode(const CHAR *mbcs, int mbcs_count, WCHAR *wcs, int wcs_count)
{
    CEL_SETERR((CEL_ERR_LIB,  _T("convert.c - cel_mb2unicode is null*********************")));
    return 0;
}

int os_unicode2mb(const WCHAR *wcs, int wcs_count, CHAR *mbcs, int mbcs_count)
{
    CEL_SETERR((CEL_ERR_LIB,  _T("convert.c - cel_unicode2mb is null*********************")));
    return 0;
}

int os_utf82unicode(const CHAR *utf8, int utf8_count, WCHAR *wcs, int wcs_count)
{
    CEL_SETERR((CEL_ERR_LIB,  _T("convert.c - cel_utf82unicode is null*********************")));
    return 0;
}

int snwprintf(WCHAR *str, size_t size, const WCHAR *format, ...)
{
    CEL_SETERR((CEL_ERR_LIB,  _T("convert.c - snwprintf is null*********************")));
    return 0;
}

char *itoa(int num, char *str, int radix)
{
    char index[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    unsigned unum;
    int i = 0, j, k;
    char temp;

    if (radix == 10 && num < 0)
    {
        unum = (unsigned) - num;
        str[i++] = '-';
    }
    else 
        unum = (unsigned)num;
    do  
    {
        str[i++] = index[unum % (unsigned)radix];
        unum  /= radix;
    }while (unum);
    str[i] = '\0';
    if (str[0] == '-') 
        k = 1;
    else 
        k = 0;
    for (j = k; j <= (i - k - 1) / 2.0; j++)
    {
        temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }
    return str;
}

/* 
将一个无符号长整形数左循环移位的函数 
*/  

//unsigned _rotl(unsigned val,int shift)  
//{  
//    register unsigned hibit;        /* non-zero means hi bit set */  
//    register unsigned num = val;    /* number to rotate */  
//    shift &= 0x1f;                  /* modulo 32 -- this will also make negative shifts work */  
//    while (shift--) {  
//        hibit = num & 0x80000000;   /* get high bit */  
//        num <<= 1;                  /* shift left one bit */  
//        if (hibit)  
//            num |= 1;               /* set lo bit if hi bit was set */  
//    }  
//    return num;  
//} 

/* 
_rotr()将一个无符号长整形数右循环移位的函数 
*/  

//unsigned _rotr(unsigned val,int shift)  
//{  
//    register unsigned lobit;        /* non-zero means lo bit set */  
//    register unsigned num = val;    /* number to rotate */  
//
//    shift &= 0x1f;              /* modulo 32 -- this will also make  negative shifts work */  
//    while (shift--) {  
//        lobit = num & 1;        /* get high bit */  
//        num >>= 1;              /* shift right one bit */  
//        if (lobit)  
//            num |= 0x80000000;  /* set hi bit if lo bit was set */  
//    }  
//    return num;  
//}

#endif