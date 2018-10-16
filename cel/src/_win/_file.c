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
#include "cel/file.h"
#ifdef _CEL_WIN

CelDir *cel_opendir(const TCHAR *dir_name, CelDirent *dirent)
{
    DWORD len;
    CelDir *dir;
    TCHAR fileName[CEL_PATHLEN];

    len = _sntprintf(fileName, CEL_PATHLEN, _T("%s\\*"), dir_name);
    fileName[len] = _T('\0');
    //_putts(fileName);
    return (((dir = FindFirstFile(fileName, dirent)) == INVALID_HANDLE_VALUE)
        ? NULL : dir);
}
#endif