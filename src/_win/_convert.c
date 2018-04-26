#include "cel/convert.h"
#include "cel/allocator.h"
#ifdef _CEL_WIN

long long atoll(const char *p)
{  
    int minus = 0;  
    long long value = 0;  
    if (*p == '-')  
    {  
        minus ++;  
        p ++;  
    }  
    while (*p >= '0' && *p <= '9')  
    {  
        value *= 10;  
        value += *p - '0';  
        p ++;  
    }  
    return minus ? 0 - value : value;  
}  
/* 
 * Get next token from string *stringp, where tokens are possibly-empty 
 * strings separated by characters from delim. 
 * 
 * Writes NULs into the string at *stringp to end tokens. 
 * delim need not remain constant from call to call. 
 * On return, *stringp points past the last NUL written (if there might 
 * be further tokens), or is NULL (if there are definitely no moretokens). 
 * 
 * If *stringp is NULL, strsep returns NULL. 
 */
TCHAR *strsep(TCHAR **stringp, const TCHAR *delim)
{    
    TCHAR *s;
    const TCHAR *spanp;
    int c, sc;
    TCHAR *tok;

    if ((s = *stringp)== NULL)
        return (NULL);
    for (tok = s;;)
    {        
        c = *s++;
        spanp = delim;
        do 
        {
            if ((sc =*spanp++) == c)
            {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                *stringp = s;
                return (tok);
            }
        } while (sc != 0);
    }   
    /* NOTREACHED */
}

int os_mb2utf8(const CHAR *mbcs, int mbcs_count, CHAR *utf8, int utf_8count)
{
    int len;
    wchar_t *wcs;

    if ((wcs = cel_malloc(utf_8count * sizeof(wcs))) != NULL)
    {
        if ((len = MultiByteToWideChar(
            CP_ACP, 0, mbcs, mbcs_count, wcs, utf_8count)) > 0)
        {
            wcs[len] = _T('\0');
            if ((len = WideCharToMultiByte(
                CP_UTF8, 0, wcs, len, utf8, utf_8count, NULL, FALSE) > 0))
            {
                cel_free(wcs);
                return len;
            }
        }
        cel_free(wcs);
    }
    return -1;
}

int os_utf82mb(const CHAR *utf8, int utf8_count, CHAR *mbcs, int mbcs_count)
{
    int len;
    wchar_t *wcs;

    if ((wcs = cel_malloc(mbcs_count * sizeof(wcs))) != NULL)
    {
        if ((len = MultiByteToWideChar(
            CP_UTF8, 0, utf8, utf8_count, wcs, mbcs_count)) > 0)
        {
            wcs[len] = _T('\0');
            if ((len = WideCharToMultiByte(
                CP_OEMCP, 0, wcs, len, mbcs, mbcs_count, NULL, FALSE)) > 0)
            {
                cel_free(wcs);
                return len;
            }
        }
        cel_free(wcs);
    }
    return -1;
}

#endif
