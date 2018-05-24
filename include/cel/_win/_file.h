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
#ifndef __CEL_FILE_WIN_H__
#define __CEL_FILE_WIN_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define S_IRWXU               00700 
#define S_IRUSR               00400
#define S_IWUSR               00200
#define S_IXUSR               00100
#define S_IRWXG               00070
#define S_IRGRP               00040
#define S_IWGRP               00020
#define S_IXGRP               00010
#define S_IRWXO               00007
#define S_IROTH               00004
#define S_IWOTH               00002
#define S_IXOTH               00001

#define CEL_ERROR_EXIST       ERROR_ALREADY_EXISTS

typedef struct _stat CelStat;
typedef DWORD CelDir;
typedef WIN32_FIND_DATA CelDirent;

#define CEL_DIRENT_NAME(dirent) ((dirent)->cFileName)
#define CEL_DIRENT_ISDIR(dirent) \
    ((((dirent)->dwFileAttributes) \
    & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)

#define cel_chmod(path, mode)
#define cel_chown(path, uid, gid) 

/* int cel_fremove(const TCHAR *file_name); */
#define cel_fremove(file_name) (DeleteFile(file_name)? 0: -1)

CelDir *cel_opendir(const TCHAR *dir_name, CelDirent *dirent);
/* int cel_readdir(CelDir *dir, CelDirent *dirent); */
#define cel_readdir(dir, dirent) (FindNextFile(dir, dirent)? 0 : -1)
/* void cel_closedir(CelDir *dir)*/
#define cel_closedir(dir) FindClose(dir)
/* int cel_mkdir(const TCHAR *dir_name, int mode); */
#define cel_mkdir_a(dir_name, mode) (CreateDirectoryA(dir_name, NULL)? 0 : -1)
#define cel_mkdir_w(dir_name, mode) (CreateDirectoryW(dir_name, NULL)? 0 : -1)
/* int cel_rmdir(const TCHAR *dir_name); */
#define cel_rmdir(dir_name) (RemoveDirectory(dir_name)? 0 : -1)

#ifdef __cplusplus
}
#endif

#endif
