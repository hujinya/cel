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
#ifndef __CEL_KEYWORD_H__
#define __CEL_KEYWORD_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelKeywordA
{
    size_t key_len;
    const CHAR *key_word;
    const void *value;
    const void *value2;
}CelKeywordA;

typedef struct _CelKeywordW
{
    size_t key_len;
    const WCHAR *key_word;
    const void *value;
    const void *value2;
}CelKeywordW;

#ifdef _UNICODE
#define CelKeyword CelKeywordW
#else
#define CelKeyword CelKeywordA
#endif

typedef int (*CelKeywordACmpFunc) (CelKeywordA *key, 
                                   const CHAR *str, size_t size);
typedef int (*CelKeywordWCmpFunc) (CelKeywordW *key, 
                                   const WCHAR *str, size_t size);

/* int cel_keyword_cmp_a(CelKeyword *key, const char *str); */
#define cel_keyword_cmp_a(key, str) strncmp((key)->key_word, str, (key)->len)
int cel_keyword_ncmp_a(CelKeywordA *key, const CHAR *str, size_t size);
int cel_keyword_ncasecmp_a(CelKeywordA *key, const CHAR *str, size_t size);
int cel_keyword_regex_a(CelKeywordA *key, const CHAR *str, size_t size);

int cel_keyword_search_a(CelKeywordA *kw_list, CelKeywordACmpFunc cmp_func, 
                         size_t list_size, const CHAR *key, size_t size);
#define cel_keyword_binary_search_a(kw_list, list_size, key, size) \
    cel_keyword_search_a(kw_list, cel_keyword_ncmp_a, list_size, key, size)
#define cel_keyword_case_search_a(kw_list, list_size, key, size) \
    cel_keyword_search_a(kw_list, cel_keyword_ncasecmp_a, list_size, key, size)
#define cel_keyword_regex_search_a(kw_list, list_size, key, size) \
    cel_keyword_search_a(kw_list, cel_keyword_regex_a, list_size, key, size)

#define cel_keyword_cmp_w(key, str) wcscmp((key)->key_word, str, (key)->len)
int cel_keyword_ncmp_w(CelKeywordW *key, const WCHAR *str, size_t size);
int cel_keyword_regex_w(CelKeywordW *key, const WCHAR *str, size_t size);

int cel_keyword_search_w(CelKeywordW *kw_list, CelKeywordWCmpFunc cmp_func,
                         size_t list_size, const WCHAR *key, size_t size);
#define cel_keyword_binary_search_w(kw_list, list_size, key, size) \
    cel_keyword_search_w(kw_list, cel_keyword_ncmp_w, list_size, key, size)
#define cel_keyword_regex_search_w(kw_list, list_size, key, size) \
    cel_keyword_search_w(kw_list, cel_keyword_regex_w, list_size, key, size)

#ifdef _UNICODE
#define cel_keyword_cmp cel_keyword_cmp_w
#define cel_keyword_ncmp cel_keyword_ncmp_w
#define cel_keyword_regex cel_keyword_regex_w
#define cel_keyword_binary_search cel_keyword_binary_search_w
#define cel_keyword_regex_search cel_keyword_regex_search_w
#else
#define cel_keyword_cmp cel_keyword_cmp_a
#define cel_keyword_ncmp cel_keyword_ncmp_a
#define cel_keyword_regex cel_keyword_regex_a
#define cel_keyword_binary_search cel_keyword_binary_search_a
#define cel_keyword_regex_search cel_keyword_regex_search_w
#endif

#ifdef __cplusplus
}
#endif

#endif
