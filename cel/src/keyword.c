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
#include "cel/keyword.h"

int cel_keyword_ncmp_a(CelKeywordA *key, const CHAR *str, size_t size)
{
    const CHAR *kw;
    size_t len;
    int ret = 0;   

    len = (size < key->key_len ? size : key->key_len);
    //printf("len = %d\r\n",len);
    for (kw = key->key_word; len > 0; ++kw, ++str, len--)
        if ((ret = *kw - *str) != 0)
            break;
    return ((ret != 0 || size == key->key_len) 
        ? ret : (key->key_len > size ? 1 : -1));
}

int cel_keyword_ncasecmp_a(CelKeywordA *key, const CHAR *str, size_t size)
{
    const CHAR *kw;
    size_t len;
    int ret = 0;   

    len = (size < key->key_len ? size : key->key_len);
    //printf("len = %d\r\n",len);
    for (kw = key->key_word; len > 0; ++kw, ++str, len--)
    {
        if ((ret = *kw - *str) != 0)
        {
            if (*str >= 'A' && *str <= 'Z'
                && (ret = *kw - (*str | 0x20)) == 0)
                continue;
            break;
        }
    }
    return ((ret != 0 || size == key->key_len) 
        ? ret : (key->key_len > size ? 1 : -1));
}

int cel_keyword_regex_a(CelKeywordA *key, const CHAR *str, size_t size)
{
    const CHAR *kw;
    size_t len;
    int ret = 0;

    len = (size < key->key_len ? size : key->key_len);
    for (kw = key->key_word; len > 0; ++kw, ++str, len--)
        if ((ret = *kw - *str) != 0)
            break;
    /*printf("ret %d, key %s, key_len %d, str %s, size %d\r\n", 
        ret, key->key_word, key->key_len, str, size);*/
    if (ret != 0 || size == key->key_len)
        return ret;
    else if (key->key_len > size)
        return 1;
    else if (*(str - 1) == '/' || *str == '&' || *str == '?')
        return 0;
    else
        return -1;
}

int cel_keyword_search_a(CelKeywordA *kw_list, CelKeywordACmpFunc cmp_func,
                         size_t list_size, const CHAR *key, size_t size)
{
    int l, h, m, r;

    l = 0;
    h = (int)list_size - 1;
    while (l <= h)
    {
        while (l <= h)
        {
            m = (l + h) / 2;
            if ((r = cmp_func(&kw_list[m], key, size)) == 0)
            {
                return m;
            }
            else
            {
                if (r > 0) 
                    h = m - 1;
                else 
                    l = m + 1;
            }
            //printf("%s %d\r\n", kw_list[m].key_word, r);
        }
    }
    return -1;
}

int cel_keyword_ncmp_w(CelKeywordW *key, const WCHAR *str, size_t size)  
{
    const WCHAR *kw;
    size_t len;
    int ret = 0;   

    len = (size < key->key_len ? size : key->key_len);
    for (kw = key->key_word; len > 0; ++kw, ++str, len--)
        if ((ret = *kw - *str) != 0)
            break;
    return ((ret != 0 || size == key->key_len) 
        ? ret : (key->key_len > size ? 1 : -1));
}

int cel_keyword_regex_w(CelKeywordW *key, const WCHAR *str, size_t size)
{
    const WCHAR *kw;
    size_t len;
    int ret = 0;   

    len = (size < key->key_len ? size : key->key_len);
    for (kw = key->key_word; len > 0; ++kw, ++str, len--)
        if ((ret = *kw - *str) != 0)
            break;
    if (ret != 0 || size == key->key_len)
        return ret;
    else if (key->key_len > size)
        return 1;
    else if (*(str - 1) == L'/'|| *str == L'&' || *str == L'?')
        return 0;
    else
        return -1;
}

int cel_keyword_search_w(CelKeywordW *kw_list, CelKeywordWCmpFunc cmp_func,
                         size_t list_size, const WCHAR *key, size_t size)
{
    int l, h, m, r;

    l = 0;
    h = (int)list_size - 1;
    while (l <= h)
    {
        while (l <= h)
        {
            m = (l + h) / 2;
            if ((r = cmp_func(&kw_list[m], key, size)) == 0)
            {
                return m;
            }
            else
            {
                if (r > 0) 
                    h = m - 1;
                else 
                    l = m + 1;
            }
            //printf("%s %d\r\n", kw_list[m].key_word, r);
        }
    }
    return -1;
}
