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
#ifndef __CEL_FILE_UNIX_H__
#define __CEL_FILE_UNIX_H__

#include "cel/types.h"
#include <dirent.h>
#include <sys/types.h>
#include <sys/statfs.h>

#ifdef __cplusplus
extern "C" {
#endif

int _fseeki64(FILE *_File, S64 _Offset, int _Origin);

#define _stat stat
typedef struct stat CelStat;
typedef DIR CelDir;
typedef struct dirent CelDirent;

#define CEL_ERROR_EXIST       EEXIST

#define CEL_DIRENT_NAME(dirent) ((dirent)->d_name)
#define CEL_DIRENT_ISLINK(dirent) ((dirent)->d_type == DT_LINK)
#define CEL_DIRENT_ISREG(dirent) ((dirent)->d_type == DT_REG)
#define CEL_DIRENT_ISDIR(dirent) ((dirent)->d_type == DT_DIR)

#define cel_chmod_a(path, mode) chmod(path, mode)
int cel_chmod_w(const WCHAR *path, int mode);
#define cel_chown(path, uid, gid) chown(path, uid, gid)

static __inline int cel_fremove(const TCHAR *file_name)
{
    return ((unlink(file_name) == 0
        || (chown(file_name, 0, 0) != -1 
        && unlink(file_name) == 0)) ? 0 : -1);
}

CelDir* cel_opendir(const TCHAR *dir_name, CelDirent *dirent);
static __inline int cel_readdir(CelDir *dir, CelDirent *dirent)
{
    CelDirent *result;
    return ((readdir_r(dir, dirent, &result) == 0 && result != NULL)? 0 : -1);
}
/* void cel_closedir(CelDir *dir)*/
#define cel_closedir(dir) closedir(dir)
#define cel_mkdir_a(dir_name, mode) mkdir(dir_name, mode)
int cel_mkdir_w(const WCHAR *dir_name, int mode);
/* int cel_rmdir(const TCHAR *dir_name); */
#define cel_rmdir(dir_name) ((rmdir(dir_name) == 0 \
    || (chown(dir_name, 0, 0) != -1 && rmdir(dir_name) == 0)) ? 0 : -1)

#ifdef __cplusplus
}
#endif

#endif
