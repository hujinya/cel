/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2016 Hu Jinya(hu_jinya@163.com) 
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
#ifdef _CEL_UNIX

int _fseeki64(FILE *_File, S64 _Offset, int _Origin)
{
    //puts("_fseeki64 is null ");
    return fseek(_File, _Offset, _Origin);
}

CelDir *cel_opendir(const TCHAR *dir_name, CelDirent *dirent)
{
    CelDir *dir;
    CelDirent *result;

    if ((dir = opendir(dir_name)) != NULL)
    {
        if (readdir_r(dir, dirent, &result) == 0)
            return dir;
        closedir(dir);
    }
    return NULL;
}
#endif