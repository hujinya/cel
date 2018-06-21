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
#include "cel/version.h"
#include "cel/allocator.h"

/* Debug defines */
#define Debug(args)   /* cel_log_debug args */
#define Warning(args) CEL_SETERRSTR(args)/* cel_log_warning args */
#define Err(args)   CEL_SETERRSTR(args)/* cel_log_err args */

CelVersion cel_ver = { 
    CEL_MAJOR, CEL_MINOR, CEL_REVISION, CEL_BUILD, (TCHAR *)CEL_EXTRA };

#ifdef _CEL_WIN
int cel_version_init(CelVersion *ver, const TCHAR *file)
{
    DWORD dwSize = 0;
    UINT i, nTranslate = 0, nBytes = 0;
    TCHAR chSubBlock[256], chFilename[MAX_PATH];
    TCHAR *lpszBuffer, *lpszProductVersion;
    struct LANGANDCODEPAGE
    { 
        WORD wLanguage;
        WORD wCodePage;
    }*lpTranslate;

    if (file == NULL)
    {
        if (GetModuleFileName(NULL, chFilename, MAX_PATH) == 0)
        {
            //_putts(_T("GetModuleFileName"));
            return -1;
        }
        file = chFilename;
    }
    if ((dwSize = GetFileVersionInfoSize(file, NULL)) == 0
        || (lpszBuffer = (TCHAR *)
        cel_malloc((dwSize + 1) * sizeof(TCHAR))) == NULL)
    {
        //_putts(_T("cel_malloc"));
        return -1;
    }
    if (!GetFileVersionInfo(file, 0, dwSize, lpszBuffer))
    {
        //_putts(_T("GetFileVersionInfo"));
        cel_free(lpszBuffer);
        return -1;
    }
    if (!VerQueryValue(lpszBuffer, 
        TEXT("\\VarFileInfo\\Translation"), 
        (LPVOID*)&lpTranslate, &nTranslate))
    {
        //_putts(_T("VerQueryValue"));
        cel_free(lpszBuffer);
        return FALSE;
    }
    for (i = 0; i < (int) (nTranslate/sizeof(struct LANGANDCODEPAGE)); i++)
    { 
        wsprintf(chSubBlock, 
            TEXT("\\StringFileInfo\\%04x%04x\\ProductVersion"), 
            lpTranslate[i].wLanguage, 
            lpTranslate[i].wCodePage);
        //Retrieve file description for language and code page "i ".   
        VerQueryValue(lpszBuffer, chSubBlock,  
            (void **)&(lpszProductVersion), &nBytes);
        _sntscanf(lpszProductVersion, nBytes, _T("%d,%d,%d,%d"), 
            &(ver->major), &(ver->minor), &(ver->revision), &(ver->build));
        //_putts(lpszProductVersion);
    }
    cel_free(lpszBuffer);

    return 0;
}
#endif

/* 
 * Unix - Major_Version.Minor_Version[.Revision[build Build]]
 * Wnidows - Major_Version.Minor_Version[.Revision[.Build]]
 */

TCHAR *_cel_version_release(CelVersion *ver, const TCHAR *uts)
{
    static TCHAR release[256];
    size_t size;

    size = _sntprintf(release, 256, _T("%d.%d.%d"), 
        ver->major, ver->minor, ver->revision);
    if (ver->build != 0)
        size += _sntprintf(release + size, 256 - size, _T(".%d"), ver->build);
    if (ver->extra != NULL)
        size += _sntprintf(release + size, 256 - size, _T(" %s"), ver->extra);
    if (uts != NULL)
        size += _sntprintf(release + size, 256 - size, _T(" (%s)"), uts);
    else
        size += _sntprintf(release + size, 256 - size, 
        _T(" (") CEL_UTS _T(")"));

    return release;
}
