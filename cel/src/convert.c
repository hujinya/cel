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
#include "cel/convert.h"
#include "cel/error.h"
#include "cel/allocator.h"

#define LLSTRLEN      27 /* -9,223,372,036,854,775,807 */

TCHAR *cel_lltodda_r(long long ll, TCHAR *dda, size_t size)
{
    static volatile int i = 0;
    static TCHAR s_buf[CEL_BUFNUM][LLSTRLEN];
    TCHAR digval, *p;
    size_t n, d = 0, negative = 0;

    if ((p = dda) == NULL)
    {
        p = s_buf[((++i) % (int)CEL_BUFNUM)];
        size = LLSTRLEN;
    }
    n = size;
    p[--n] = _T('\0');
    if (ll < 0) 
    {
        ll = -ll;
        negative = 1;
    }
    do 
    {
        digval = (TCHAR)(ll % 10);
        ll /= 10;
        if (++d > 3) 
        {
            p[--n] = _T(',');
            d = 1;
        }
        p[--n] = digval + _T('0');
    }while (ll > 0 && n > 0);
    if (negative == 1 && n > 0) p[--n] = _T('-');

    //printf("size = %d\r\n",  size - n);
    return (dda == NULL ?  &(p[n]) : (TCHAR *)memmove(dda, &(p[n]), size - n));
}

int cel_power2bits(int n, int min_bits, int max_bits)
{
    int max = (1 << max_bits)- 1;
    int i = 0, tmp, a, b;

    if (n > max)
        return max_bits;

    /* count the bits input n. */
    tmp = n;
    do
    {
        tmp >>= 1;
        ++i;
    }while (tmp != 0);

    if (i <= min_bits)
        return min_bits;

    /* Which is nearest? */
    a = (1 << i)- n;
    b = (1 << (i - 1))- n;
    if (b < 0)
        b = -b;
    if (b < a)
        return i - 1;
    return i;
}

#define HEXTOBIN(hex, bin) \
    if (hex >= _T('0') && hex <= _T('9')){ bin = hex - _T('0'); } \
    else if (hex >= _T('A') && hex <= _T('F')){ bin = hex - _T('A')+ 10; }\
    else if (hex >= _T('a') && hex <= _T('f')){ bin = hex - _T('a')+ 10; }\
    else { bin = _T('0'); }

int cel_hextobin(const BYTE *input, size_t ilen, BYTE *output, size_t *olen)
{
    size_t n1, n2;
    BYTE ch;

    if (input == NULL || output == NULL) 
        return -1;

    for (n1 = 0, n2 = 0; n1 < ilen && n2 <  (*olen - 1);)
    {
        output[n2] = 0;
        HEXTOBIN(input[n1], ch);
        if (input[n1++] == _T('\0'))
        {
            output[n2++] = ch;
            break;
        }
        output[n2] |= (ch << 4);

        HEXTOBIN(input[n1], ch);
        output[n2] |= ch;
        n1++;
        n2++;
    }
    output[n2] = _T('\0');
    *olen = n2;

    return 0;
}

int cel_bintohex(const BYTE *input, size_t ilen, 
                 BYTE *output, size_t *olen, int is_caps)
{
    size_t n1, n2;
    BYTE ch, hex_ch;

    if (input == NULL || output == NULL) return -1;

    hex_ch = (is_caps == 0 ? _T('a') : _T('A'));
    for (n1 = 0, n2 = 0; n1 < ilen; ++n1)
    {
        if (n2 >= *olen)
            break;
        ch = ((input[n1] >> 4) & 0x0F);
        output[n2++] = (ch <= 0x09 ? (ch + _T('0')): (ch - 10 + hex_ch));

        if (n2 >= *olen)
            break;
        ch = input[n1] & 0x0F;
        output[n2++] = (ch <= 0x09 ? (ch + _T('0')): (ch - 10 + hex_ch));
    }
    output[n2] = _T('\0');
    *olen = n2;

    return 0;
}

int cel_hexdump(void *dest, size_t dest_size, const BYTE *src, size_t src_size)
{
    BYTE *line;
    size_t i, this_line;
    size_t dest_offset, src_offset;

    line = (BYTE *)src;
    dest_offset = src_offset = 0;
    while (src_offset < src_size)
    {
        dest_offset += _stprintf((char *)dest + dest_offset, _T("%04x "),
            (int)src_offset);
        this_line = src_size - src_offset;
        if (this_line > 16)
            this_line = 16;
        for (i = 0; i < this_line; i++)
            dest_offset += 
            _stprintf((char *)dest + dest_offset, _T("%02x "), line[i]);
        for (; i < 16; i++)
            dest_offset += _stprintf((char *)dest + dest_offset, _T("   "));
        for (i = 0; i < this_line; i++)
            dest_offset += _stprintf((char *)dest + dest_offset, _T("%c"),
            (line[i] >= 0x20 && line[i] < 0x7f) ? line[i] : '.');
        dest_offset += _stprintf((char *)dest + dest_offset, CEL_CRLF);
        src_offset += this_line;
        line += this_line;
    }
    return dest_offset;
}

TCHAR *cel_strreppswd(TCHAR *str, const TCHAR *rep, TCHAR pswd_chr)
{
    TCHAR *sptr = str, *sptr1 = str;
    const TCHAR *rptr = rep;

    while (*sptr != _T('\0'))
    {
        do
        {
            if (*rptr == _T('\0'))
            {
                while (*sptr != _T('\0') && *sptr != ' ')
                {
                    *sptr = pswd_chr; 
                    sptr++;
                }
                sptr1 = sptr;
                break;
            }
            if (*sptr != *rptr)
            {
                sptr1++;
                break;
            }
            sptr++; rptr++;
        }while (*sptr != _T('\0'));
        rptr = rep;
        sptr = sptr1;
    }
    return str;
}

char *cel_strncpy(char *_dst, size_t *dest_size,
                  const char *_src, size_t src_size)
{
    *dest_size = ((*dest_size > src_size) ? src_size : (*dest_size - 1));
    if (*dest_size == 0)
        return NULL;
    memcpy(_dst, _src, *dest_size);
    _dst[*dest_size] = '\0';
    return _dst;
}

CHAR *cel_strgetkeyvalue_a(const CHAR *str, 
                           CHAR key_delimiter, CHAR value_delimiter,
                           const CHAR *key, CHAR *value, size_t *size)
{
    BOOL value_reading = FALSE;
    size_t key_start = 0, key_end = 0;
    size_t value_start = 0, value_end = 0;
    size_t i = 0, key_len;

    value[0] = '\0';
    if (str != NULL)
    {
        //puts(str);
        key_len = strlen(key);
        while (TRUE)
        {
            if (str[i] == value_delimiter && !value_reading)
            {
                value_reading = TRUE;
                key_end = i;
                value_start = i + 1;
            }
            if (str[i] == key_delimiter || str[i] == '\0')
            {
                value_reading = FALSE;
                value_end = i;
                while (str[key_start] == ' ' && key_start < key_end)
                    key_start++;
                //puts(&str[key_start]);
                if (key_end - key_start == key_len
                    && memcmp(&str[key_start], 
                    key, key_end - key_start) == 0)
                {
                    //printf("value dest size = %d, src size = %d\r\n",
                    //    *size, value_end - value_start);
                    //puts(&str[value_start]);
                    //printf("%p\r\n", value);
                    return cel_strncpy(value, size, &str[value_start], value_end - value_start);
                }
                if (str[i] == '\0')
                    break;
                key_start = value_start = i + 1;
            }
            i++;
        }
    }
    *size = 0;
    return NULL;
}

int cel_strindexof(const TCHAR *str, const TCHAR *sub_str)
{
    const TCHAR *p;  
      
    if ((p = _tcsstr(str, sub_str)) == NULL)
        return -1;
    return (int)(p - str);
}

int cel_strlindexof(const TCHAR *str, const TCHAR *sub_str)
{
    const TCHAR *p;
    int index = -1;

    if ((p = _tcsstr(str, sub_str)) == NULL)
        return index;
    do
    {
        index  = (int)(p - str);
    }while ((p = _tcsstr(str + index, sub_str)) != NULL);

    return index;
}

CHAR *_cel_strltrim_a(CHAR *str, size_t *len)
{
    int i = 0;
    CHAR ch;

    while ((ch = str[i]) != '\0')
    {
        if ((ch) == ' ' || (ch) == '\t'
            || (ch) == '\n' || (ch) == '\r')
        {
            i++;
            continue;
        }
        break;
    }
    (*len) -= i;
    memmove(str, &str[i], (*len)+ 1);

    return str;
}

WCHAR *_cel_strltrim_w(WCHAR *str, size_t *len)
{
    int i = 0;
    WCHAR ch;

    while ((ch = str[i]) != L'\0')
    {
        if ((ch) == L' ' || (ch) == L'\t'
            || (ch) == L'\n' || (ch) == L'\r')
        {
            i++;
            continue;
        }
        break;
    }
    (*len) -= i;
    memmove(str, &str[i], (*len)+ 1);

    return str;
}

CHAR *_cel_strrtrim_a(CHAR *str, size_t *len)
{
    size_t i = 0, j = 0;
    CHAR ch;

    if (*len <= 0)
    {
        while (str[i++] != '\0');
        *len = i;
    }
    else
    {
        i = *len;
    }
    while ((--i) > 0)
    {
        if ((ch = str[i]) == ' ' || (ch) == '\t'
            || (ch) == '\n' || (ch) == '\r')
        {
            j++;
            continue;
        }
        break;
    }
    str[++i] = '\0';
    (*len)-= j;
    
    return str;
}

WCHAR *_cel_strrtrim_w(WCHAR *str, size_t *len)
{
    size_t i = 0, j = 0;
    WCHAR ch;

    if (*len <= 0)
    {
        while (str[i++] != L'\0');
        *len = i;
    }
    else
    {
        i = *len;
    }
    while ((--i) > 0)
    {
        if ((ch = str[i]) == L' ' || (ch) == L'\t'
            || (ch) == L'\n' || (ch) == L'\r')
        {
            j++;
            continue;
        }
        break;
    }
    str[++i] = L'\0';
    (*len)-= j;
    
    return str;
}
