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
#ifndef __CEL_FILE_H__
#define __CEL_FILE_H__

#include "cel/config.h"
#include "cel/types.h"
#include <sys/stat.h>
#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelFileStatus
{
    time_t atime;
    time_t mtime;
    unsigned long dnum, fnum;
    unsigned long blk_size;
    unsigned long blk_num;
    unsigned long long size;
}CelFileStatus;

typedef struct _CelFileSystemStatus
{
    TCHAR name[CEL_FNLEN];
    TCHAR dir[CEL_DIRLEN];
    /* Range 0 - 2^64 bytes */
    unsigned long long total;
    unsigned long long available;
    unsigned long long free;
}CelFileSystemStatus;

#if defined(_CEL_UNIX)
#include "cel/_unix/_file.h"
#elif defined(_CEL_WIN)
#include "cel/_win/_file.h"
#endif

typedef int (* CelDirFileEachFunc) (const TCHAR *dir_name, 
                                    const TCHAR *file_name, 
                                    const CelDirent *dirent, void *user_data);
typedef int (* CelFileLineEachFunc) (TCHAR *line, void *user_data);

/* Get file dir.*/
CHAR *cel_filedir_r_a(const CHAR *path, CHAR *file_path, size_t size);
WCHAR *cel_filedir_r_w(const WCHAR *path, WCHAR *file_path, size_t size);
#define cel_filedir_a(path) cel_filedir_r_a(path, NULL, 0)
#define cel_filedir_w(path) cel_filedir_r_w(path, NULL, 0)

/* Get file name.*/
CHAR *cel_filename_r_a(const CHAR *path, CHAR *file_name, size_t size);
WCHAR *cel_filename_r_w(const WCHAR *path, WCHAR *file_name, size_t size);
#define cel_filename_a(path) cel_filename_r_a(path, NULL, 0)
#define cel_filename_w(path) cel_filename_r_w(path, NULL, 0)

/* Get file extension.*/
CHAR *cel_fileext_r_a(const CHAR *path, CHAR *ext, size_t size);
WCHAR *cel_fileext_r_w(const WCHAR *path, WCHAR *ext, size_t size);
#define cel_fileext_a(ext) cel_fileext_r_a(ext, NULL, 0)
#define cel_fileext_w(ext) cel_fileext_r_w(ext, NULL, 0)

/* Get module file.*/
CHAR *cel_modulefile_r_a(CHAR *buf, size_t size);
WCHAR *cel_modulefile_r_w(WCHAR *buf, size_t size);
#define cel_modulefile_a() cel_modulefile_r_a(NULL, 0)
#define cel_modulefile_w() cel_modulefile_r_w(NULL, 0)

CHAR *cel_escapepath_r_a(const char *unescape_path,
                         char *escape_path, size_t size);
/* 
 * Creates an absolute or full path name for the specified relative path name.
 */
CHAR *cel_fullpath_r_a(const CHAR *rel_path, CHAR *full_path, size_t size);
WCHAR *cel_fullpath_r_w(const WCHAR *rel_path, WCHAR *full_path, size_t size);
#define cel_fullpath_a(rel_path) cel_fullpath_r_a(rel_path, NULL, 0)
#define cel_fullpath_w(rel_path) cel_fullpath_r_w(rel_path, NULL, 0)

static __inline BOOL cel_fexists_a(const CHAR *file_name)
{
    CelStat my_stat; 
    return (_stat(file_name, &my_stat) == 0);
}
BOOL cel_fexists_w(const WCHAR *file_name);
FILE *cel_fopen(const TCHAR *file_name, const TCHAR *mode);
#define cel_fclose(fp) fclose(fp)
#define cel_fflush(fp) fflush(fp)
int cel_fsync(const TCHAR *old_file, const TCHAR *new_file);
int cel_fmove(const TCHAR *dest_file, const TCHAR *src_file);
int cel_fforeach(const TCHAR *file_name, 
                 CelFileLineEachFunc each_func, void *user_data);

int cel_mkdirs_a(const CHAR *dir_name, int mode);
int cel_mkdirs_w(const WCHAR *dir_name, int mode);
int cel_rmdirs(const TCHAR *dir_name);
int cel_foreachdir(const TCHAR *dir_name, 
                   CelDirFileEachFunc each_func, void *user_data);

#ifndef _UNICODE
#define cel_filedir_r cel_filedir_r_a
#define cel_filedir cel_filedir_a
#define cel_filename_r cel_filename_r_a
#define cel_filename cel_filename_a
#define cel_fileext_r cel_fileext_r_a
#define cel_fileext cel_fileext_a
#define cel_modulefile_r cel_modulefile_r_a
#define cel_modulefile cel_modulefile_a
#define cel_fullpath_r cel_fullpath_r_a
#define cel_fullpath cel_fullpath_a
#define cel_fexists cel_fexists_a
#define cel_mkdirs cel_mkdirs_a
#else
#define cel_filedir_r cel_filedir_r_w
#define cel_filedir cel_filedir_w
#define cel_filename_r cel_filename_r_w
#define cel_filename cel_filename_w
#define cel_fileext_r cel_fileext_r_w
#define cel_fileext cel_fileext_w
#define cel_modulefile_r cel_modulefile_r_w
#define cel_modulefile cel_modulefile_w
#define cel_fullpath_r cel_fullpath_r_w
#define cel_fullpath cel_fullpath_w
#define cel_fexists cel_fexists_w
#define cel_mkdirs cel_mkdirs_w
#endif

#ifdef __cplusplus
}
#endif

#endif
