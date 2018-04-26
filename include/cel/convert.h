/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_CONVERT_H__
#define __CEL_CONVERT_H__

#include "cel/config.h"
#if defined(_CEL_UNIX)
#include "cel/_unix/_convert.h"
#elif defined(_CEL_WIN)
#include "cel/_win/_convert.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define cel_mb2unicode(mbcs, mbcs_count, wcs, wcs_count) \
    os_mb2unicode(mbcs, mbcs_count, wcs, wcs_count) 
#define cel_mb2utf8(mbcs, mbcs_count, utf8, utf_8count) \
    os_mb2utf8(mbcs, mbcs_count, utf8, utf_8count)
#define cel_utf82mb(utf8, utf8_count, mbcs, mbcs_count) \
    os_utf82mb(utf8, utf8_count, mbcs, mbcs_count)
#define cel_utf82unicode(utf8, utf8_count, wcs, wcs_count) \
    os_utf82unicode(utf8, utf8_count, wcs, wcs_count)
#define cel_unicode2mb(wcs, wcs_count, mbcs, mbcs_count) \
    os_unicode2mb(wcs, wcs_count, mbcs, mbcs_count)
#define cel_unicode2utf8(wcs, wcs_count, utf8, utf8_count) \
    os_unicode2utf8(wcs, wcs_count, utf8, utf8_count)

static __inline TCHAR *cel_skiptoken(const TCHAR *p)
{
    while (CEL_ISSPACE(*p))  p++;
    while (*p && !CEL_ISSPACE(*p)) p++;
    return (TCHAR *)p;
}

static __inline TCHAR *cel_skipws(const TCHAR *p)
{
    while (*p && CEL_ISSPACE(*p)) p++;
    return (TCHAR *)p;
}

int cel_power2bits(int n, int min_bits, int max_bits);
/**
 * \brief  Integer converted to dotted decimal chars,
 *         example 9,223,372,036,854,775,807.
 */
TCHAR *cel_lltodda_r(long long ll, TCHAR *dda, size_t size);
#define cel_lltodda(ll) cel_lltodda_r(ll, NULL, 0)
int cel_hextobin(const TBYTE *input, size_t ilen, 
                 TBYTE *output, size_t *olen);
int cel_bintohex(const TBYTE *input, size_t ilen, 
                 TBYTE *output, size_t *olen, int is_caps);
int cel_hexdump(void *dest, size_t dest_size, 
                const BYTE *src, size_t src_size);
/*  
 * \brief  Replace password characters.
 */
TCHAR *cel_strreppswd(TCHAR *str, const TCHAR *rep, TCHAR pswd_chr);

char *cel_strncpy(char *_dst, size_t *dest_size,
                  const char *_src, size_t src_size);
CHAR *cel_strgetkeyvalue_a(const CHAR *str, 
                           CHAR key_delimiter, CHAR value_delimiter,
                           const CHAR *key, CHAR *value, size_t *size);

int cel_strindexof(const TCHAR *str, const TCHAR *sub_str);
/* lastindexof */
int cel_strlindexof(const TCHAR *str, const TCHAR *sub_str);

/* lefttrim, righttrim */
CHAR *_cel_strltrim_a(CHAR *str, size_t *len);
WCHAR *_cel_strltrim_w(WCHAR *str, size_t *len);
CHAR *_cel_strrtrim_a(CHAR *str, size_t *len);
WCHAR *_cel_strrtrim_w(WCHAR *str, size_t *len);
static __inline CHAR *cel_strltrim_a(CHAR *str)
{
    size_t len = 0;
    return _cel_strltrim_a(str, &len);
}
static __inline WCHAR *cel_strltrim_w(WCHAR *str)
{
    size_t len = 0;
    return _cel_strltrim_w(str, &len);
}
static __inline CHAR *cel_strrtrim_a(CHAR *str)
{
    size_t len = 0;
    return _cel_strrtrim_a(str, &len);
}
static __inline WCHAR *cel_strrtrim_w(WCHAR *str)
{
    size_t len = 0;
    return _cel_strrtrim_w(str, &len);
}
/* TCHAR *cel_strtrim(TCHAR *str)*/
#define cel_strtrim_a(str) cel_strtrim_a(cel_strltrim_a(str))
#define cel_strtrim_w(str) cel_strtrim_a(cel_strltrim_w(str))

#ifdef _UNICODE
#define _cel_strltrim _cel_strltrim_w
#define _cel_strrtrim _cel_strrtrim_w
#define cel_strltrim cel_strltrim_w
#define cel_strrtrim cel_strrtrim_w
#define cel_strtrim cel_strtrim_w
#else
#define _cel_strltrim _cel_strltrim_a
#define _cel_strrtrim _cel_strrtrim_a
#define cel_strltrim cel_strltrim_a
#define cel_strrtrim cel_strrtrim_a
#define cel_strtrim cel_strtrim_a
#endif

#ifdef __cplusplus
}
#endif

#endif
