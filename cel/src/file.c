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
#include "cel/error.h"
#include "cel/log.h"
#include "cel/convert.h"

CHAR *cel_filedir_r_a(const CHAR *path, CHAR *file_dir, size_t size)
{
    static volatile int i = 0;
    static CHAR s_buf[CEL_BUFNUM][CEL_PATHLEN] = { {'\0'}, {'\0'} };
    size_t j = 0, k = 0;

    if (file_dir == NULL)
    {
        file_dir = s_buf[((++i) % (int)CEL_BUFNUM)];
        size = CEL_PATHLEN;
    }
    while (path[j] != '\0')
    {
        if (path[j] == '/' || path[j] == '\\')
            k = j + 1;
        j++;
    }
    if (k > size) 
        k = size;
    memmove(file_dir, path, k * sizeof(CHAR));
    file_dir[k] = '\0';

    return file_dir;
}

WCHAR *cel_filedir_r_w(const WCHAR *path, WCHAR *file_dir, size_t size)
{
    static volatile int i = 0;
    static WCHAR s_buf[CEL_BUFNUM][CEL_PATHLEN] = { {L'\0'}, {L'\0'} };
    size_t j = 0, k = 0;

    if (file_dir == NULL)
    {
        file_dir = s_buf[((++i) % (int)CEL_BUFNUM)];
        size = CEL_PATHLEN;
    }
    while (path[j] != L'\0')
    {
        if (path[j] == L'/' || path[j] == L'\\')
            k = j + 1;
        j++;
    }
    if (k > size) 
        k = size;
    memmove(file_dir, path, k * sizeof(WCHAR));
    file_dir[k] = L'\0';

    return file_dir;
}

CHAR *cel_filename_r_a(const CHAR *path, CHAR *file_name, size_t size)
{
    static volatile int i = 0;
    static CHAR s_buf[CEL_BUFNUM][CEL_FNLEN] = { {'\0'}, {'\0'} };
    size_t j = 0, k = 0;

    if (file_name == NULL)
    {
        file_name = s_buf[((++i)% (int)CEL_BUFNUM)];
        size = CEL_FNLEN;
    }
    j = 0;
    while (path[j] != '\0')
    {
        if (path[j] == '/' || path[j] == '\\')
            k = j;
        j++;
    }
    if (k++ == 0) 
        return NULL;
    if (j - k > size) 
        k = j - size;
    memmove(file_name, &path[k], (j - k) * sizeof(CHAR));
    file_name[j - k] = '\0';

    return file_name;
}

WCHAR *cel_filename_r_w(const WCHAR *path, WCHAR *file_name, size_t size)
{
    static volatile int i = 0;
    static WCHAR s_buf[CEL_BUFNUM][CEL_FNLEN] = { {L'\0'}, {L'\0'} };
    size_t j = 0, k = 0;

    if (file_name == NULL)
    {
        file_name = s_buf[((++i)% (int)CEL_BUFNUM)];
        size = CEL_FNLEN;
    }
    j = 0;
    while (path[j] != L'\0')
    {
        if (path[j] == L'/' || path[j] == L'\\')
            k = j;
        j++;
    }
    if (k++ == 0) 
        return NULL;
    if (j - k > size) 
        k = j - size;
    memmove(file_name, &path[k], (j - k) * sizeof(WCHAR));
    file_name[j - k] = L'\0';

    return file_name;
}

CHAR *cel_fileext_r_a(const CHAR *path, CHAR *ext, size_t size)
{
    static volatile int i = 0;
    static CHAR s_buf[CEL_BUFNUM][CEL_EXTLEN] = { {'\0'}, {'\0'} };
    size_t j = 0, k = 0;

    if (ext == NULL)
    {
        ext = s_buf[((++i)% (int)CEL_BUFNUM)];
        size = CEL_EXTLEN;
    }
    while (path[j] != '\0')
    {
        if (path[j] == '.' && path[j + 1] != '.' 
            && path[j + 1] != '/' && path[j + 1] != '\\')
            k = j;
        j++;
    }
    if (k++ == 0) 
        return NULL;
    if (j - k > size) 
        k = j - size;
    memmove(ext, &path[k], (j - k) * sizeof(CHAR));
    ext[j - k] = '\0';

    return ext;
}

WCHAR *cel_fileext_r_w(const WCHAR *path, WCHAR *ext, size_t size)
{
    static volatile int i = 0;
    static WCHAR s_buf[CEL_BUFNUM][CEL_EXTLEN] = { {L'\0'}, {L'\0'} };
    size_t j = 0, k = 0;

    if (ext == NULL)
    {
        ext = s_buf[((++i)% (int)CEL_BUFNUM)];
        size = CEL_EXTLEN;
    }
    while (path[j] != L'\0')
    {
        if (path[j] == L'.' && path[j + 1] != L'.' 
            && path[j + 1] != L'/' && path[j + 1] != L'\\')
            k = j;
        j++;
    }
    if (k++ == 0) 
        return NULL;
    if (j - k > size) 
        k = j - size;
    memmove(ext, &path[k], (j - k) * sizeof(WCHAR));
    ext[j - k] = L'\0';

    return ext;
}

CHAR *cel_modulefile_r_a(CHAR *buf, size_t size)
{
    static CHAR s_buf[CEL_PATHLEN] = { '\0'};

    if (s_buf[0] == '\0')
    {
        if (
#ifdef _CEL_UNIX
            readlink("/proc/self/exe", s_buf, CEL_PATHLEN) <= 0
#else
            GetModuleFileNameA(NULL, s_buf, CEL_PATHLEN) <= 0
#endif
            )
            return NULL;
    }
    //_putts(s_buf);
    return (buf == NULL ? s_buf : strncpy(buf, s_buf, size));
}

WCHAR *cel_modulefile_r_w(WCHAR *buf, size_t size)
{
    static WCHAR s_buf[CEL_PATHLEN] = { L'\0'};

    if (s_buf[0] == L'\0')
    {
        if (
#ifdef _CEL_UNIX
            cel_utf82unicode(cel_modulefile_a(), -1, s_buf, CEL_PATHLEN) <= 0
#else
            GetModuleFileNameW(NULL, s_buf, CEL_PATHLEN) <= 0
#endif
            )
            return NULL;
    }
    //_putts(s_buf);
    return (buf == NULL ? s_buf : wcsncpy(buf, s_buf, size));
}

CHAR *cel_escapepath_r_a(const CHAR *unescape_path,
                         CHAR *escape_path, size_t size)
{
    int i = 0, j = 0;

    while (unescape_path[i] != '\0' 
        && j + 2 <= (int)size)
    {
        if (unescape_path[i] == '\\')
            escape_path[j++] = '\\';
        escape_path[j++] = unescape_path[i++];
    }
    escape_path[j] = '\0';
    return escape_path;
}

CHAR *cel_fullpath_r_a(const CHAR *rel_path, CHAR *full_path, size_t size)
{
    static volatile int i = 0;
    static CHAR s_buf[CEL_BUFNUM][CEL_PATHLEN] = { {'\0'}, {'\0'} };
    int rel_start = 0, rel_depth = 0, full_len;

    if (full_path == NULL)
    {
        full_path = s_buf[((++i) % (int)CEL_BUFNUM)];
        size = CEL_PATHLEN;
    }
    /* Path is full path */
    if (rel_path[0] != '.')
    {
        snprintf(full_path, size, "%s", rel_path);
        return full_path;
    }
    /* Get parent path depth */
    while (rel_path[rel_start] != '\0')
    {
        //_tprintf(_T("rel_start %d\r\n"), rel_start);
        if (rel_path[rel_start] != '.' 
            && rel_path[rel_start] != '/' 
            && rel_path[rel_start] != '\\')
            break;
        if (rel_path[rel_start] == '.' 
            && rel_path[rel_start + 1] == '.')
        {
            rel_depth++;
            rel_start += 2;
            continue;
        }
        rel_start++;
    }
    /* Get current module file path */
    if (cel_modulefile_r_a(full_path, size) == NULL)
        return NULL;
    full_len = (int)strlen(full_path);
    /*
    _tprintf(_T("rel_depth = %d, path = %s, module path = %s\r\n"), 
        rel_depth, &rel_path[rel_depth], full_path);
        */
    while (rel_depth-- >= 0)
    {
        while (full_len >= 0 
            && full_path[--full_len] != '\\' 
            && full_path[full_len] != '/') ;
    }
    full_len++;
    snprintf(&full_path[full_len], size - full_len, "%s", 
        &rel_path[rel_start]);
    //_tprintf(_T("%s\r\n"), full_path);

    return full_path;
}

WCHAR *cel_fullpath_r_w(const WCHAR *rel_path, WCHAR *full_path, size_t size)
{
    static volatile int i = 0;
    static WCHAR s_buf[CEL_BUFNUM][CEL_PATHLEN] = { {L'\0'}, {L'\0'} };
    int rel_start = 0, rel_depth = 0, full_len;

    if (full_path == NULL)
    {
        full_path = s_buf[((++i) % (int)CEL_BUFNUM)];
        size = CEL_PATHLEN;
    }
    /* Path is full path */
    if (rel_path[0] != L'.')
    {
        snwprintf(full_path, size, L"%s", rel_path);
        return full_path;
    }
    /* Get parent path depth */
    while (rel_path[rel_start] != L'\0')
    {
        //_tprintf(_T("rel_start %d\r\n"), rel_start);
        if (rel_path[rel_start] != L'.' 
            && rel_path[rel_start] != L'/' 
            && rel_path[rel_start] != L'\\')
            break;
        if (rel_path[rel_start] == L'.' 
            && rel_path[rel_start + 1] == L'.')
        {
            rel_depth++;
            rel_start += 2;
            continue;
        }
        rel_start++;
    }
    /* Get current module file path */
    if (cel_modulefile_r_w(full_path, size) == NULL)
        return NULL;
    full_len = (int)wcslen(full_path);
    /*
    _tprintf(_T("rel_depth = %d, path = %s, module path = %s\r\n"), 
        rel_depth, &rel_path[rel_depth], full_path);
        */
    while (rel_depth-- >= 0)
    {
        while (full_len >= 0 
            && full_path[--full_len] != '\\' 
            && full_path[full_len] != '/') ;
    }
    full_len++;
    snwprintf(&full_path[full_len], size - full_len, L"%s", 
        &rel_path[rel_start]);
    //_tprintf(_T("%s\r\n"), full_path);

    return full_path;
}

BOOL cel_fexists_w(const WCHAR *file_name)
{
#ifdef _CEL_UNIX
    CEL_SETERR((CEL_ERR_LIB, _T("cel_fexists_w is null############")));
    return FALSE;
#endif
#ifdef _CEL_WIN
    CelStat my_stat;
    return (_wstat(file_name, &my_stat) == 0);
#endif
}

FILE *cel_fopen(const TCHAR *file_name, const TCHAR *mode)
{
    FILE *fp;

    if ((fp = _tfopen(file_name, mode)) == NULL)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Open file '%s' failed(%s)."), 
            file_name, cel_geterrstr(cel_sys_geterrno())));
        return NULL;
    }

    return fp;
}

int cel_fsync(const TCHAR *dest_file,const TCHAR *src_file)
{
    char buf[CEL_LINELEN];
	int len;
    FILE *fp, *fp_;

    if ((fp = cel_fopen(src_file, _T("r"))) == NULL 
        || (fp_ = cel_fopen(dest_file, _T("w"))) == NULL)
    {
        if (fp != NULL)
            cel_fclose(fp);
        return -1;
    }
	while (!feof(fp))
	{
		if ((len = fread(buf, 1, sizeof(buf), fp)) <= 0)
			break;
		if (fwrite(buf, 1, sizeof(buf), fp_) != len)
		{
			CEL_SETERR((CEL_ERR_LIB, _T("fwrite():%s."), 
				cel_geterrstr(cel_sys_geterrno())));
			break;
		}
	}
    fclose(fp_);
    fclose(fp);

    return 0;
}

int cel_fmove(const TCHAR *old_file, const TCHAR *new_file)
{
	if (rename(old_file, new_file) == 0)
		return 0;
	if (cel_fsync(new_file, old_file) == 0
		|| (cel_mkdirs_a(cel_filedir_a(new_file), CEL_UMASK) != -1
		&& cel_fsync(new_file, old_file) == 0))
	{
		cel_fremove(old_file);
		return 0;
	}
	CEL_SETERR((CEL_ERR_LIB, _T("file move %s"), cel_geterrstr(cel_sys_geterrno())));
	return -1;
}

int cel_fforeach(const TCHAR *file_name, 
                 CelFileLineEachFunc each_func, void *user_data)
{
    int ret = 0;
    FILE *fp;
    TCHAR line_buf[CEL_LINELEN];

    if ((fp = _tfopen(file_name, _T("r"))) == NULL)
        return -1;
    while (_fgetts(line_buf, CEL_LINELEN, fp) != NULL)
    {
        if ((ret = each_func(line_buf, user_data)) != 0)
            break;
    }
    fclose(fp);

    return ret;
}

int cel_mkdirs_intern_a(CHAR *dir, int dir_idx, int mode)
{
    CHAR _c;

    while (dir_idx > 0)
    {
        if (dir[dir_idx] == '\\' ||  dir[dir_idx] == '/')
        {
            _c = dir[dir_idx];
            dir[dir_idx] = '\0';
            //puts(dir);
            if (!cel_fexists_a(dir))
            {
                if (cel_mkdirs_intern_a(dir, dir_idx, mode) != 0)
                    return -1;
            }
            dir[dir_idx] = _c;
            if (cel_mkdir_a(dir, mode) != 0)
            {
                if (cel_sys_geterrno() != CEL_ERROR_EXIST)
                {
                    CEL_SETERR((CEL_ERR_LIB, _T("mkdir %s error"), dir));
                    return -1;
                }
            }
#ifdef _CEL_UNIX
            cel_chmod_a(dir, mode);
#endif
            return 0;
        }
        dir_idx--;
    }
    return -1;
}

int cel_mkdirs_a(const CHAR *dir, int mode)
{
    int dir_idx;
    CHAR _dir[CEL_PATHLEN + 1];

    if ((dir_idx = strlen(dir)) > CEL_PATHLEN)
        return -1;
    memcpy(_dir, dir, dir_idx);
    _dir[dir_idx] = '\0';
    return cel_mkdirs_intern_a(_dir, dir_idx - 1, mode);
}

int cel_mkdirs_intern_w(WCHAR *dir, int dir_idx, int mode)
{
    WCHAR _c;

    while (dir_idx > 0)
    {
        if (dir[dir_idx] == L'\\' ||  dir[dir_idx] == L'/')
        {
            _c = dir[dir_idx];
            dir[dir_idx] = L'\0';
            //puts(dir);
            if (!cel_fexists_w(dir))
            {
                if (cel_mkdirs_intern_w(dir, dir_idx, mode) != 0)
                    return -1;
            }
            dir[dir_idx] = _c;
            if (cel_mkdir_w(dir, mode) != 0)
            {
                if (cel_sys_geterrno() != CEL_ERROR_EXIST)
                {
                    puts("mkdir error ");
                    return -1;
                }
            }
#ifdef _CEL_UNIX
            cel_chmod_w(dir, mode);
#endif
            return 0;
        }
        dir_idx--;
    }
    return -1;
}

int cel_mkdirs_w(const WCHAR *dir, int mode)
{
    int dir_idx;
    WCHAR _dir[CEL_PATHLEN + 1];

    if ((dir_idx = wcslen(dir)) > CEL_PATHLEN)
        return -1;
    memcpy(_dir, dir, dir_idx * sizeof(WCHAR));
    _dir[dir_idx] = L'\0';
    return cel_mkdirs_intern_w(_dir, dir_idx - 1, mode);
}

int cel_rmdirs(const TCHAR *dir_name)
{
    CelDir *dir;
    CelDirent dirent;
    TCHAR childdir_name[CEL_PATHLEN];
    size_t len, count = 0;
    int temp = 0;

    if ((dir = cel_opendir(dir_name, &dirent)) == NULL)
        return -1;
    do
    {
        //_putts(CEL_DIRENT_NAME(&dirent));
        if (_tcscmp(CEL_DIRENT_NAME(&dirent), _T(".")) == 0 
            || _tcscmp(CEL_DIRENT_NAME(&dirent), _T("..")) == 0)
            continue;
        len = _sntprintf(childdir_name, CEL_PATHLEN, 
            _T("%s/%s"), dir_name, CEL_DIRENT_NAME(&dirent));
        childdir_name[len] = _T('\0');
        if (CEL_DIRENT_ISDIR(&dirent))
        {
            if ((temp = cel_rmdirs(childdir_name)) == -1)
            {
                cel_closedir(dir);
                return -1;
            }
            count += temp;
            continue;
        }
        if (cel_fremove(childdir_name) != 0)
        {
            cel_closedir(dir);
            return -1;
        }
        count += 1;
    }while (cel_readdir(dir, &dirent) == 0);
    cel_closedir(dir);
    if (cel_rmdir(dir_name) == -1)
    {
        return -1;
    }

    return 0;
}

int cel_foreachdir(const TCHAR *dir_name,
                   CelDirFileEachFunc each_func, void *user_data)
{
    int ret = 0;
    CelDir *dir;
    CelDirent dirent;

    if ((dir = cel_opendir(dir_name, &dirent)) == NULL)
        return -1;
    do
    {
        if (_tcscmp(CEL_DIRENT_NAME(&dirent), _T(".")) != 0 
            && _tcscmp(CEL_DIRENT_NAME(&dirent), _T("..")) != 0)
        {
            if ((ret = each_func(
                dir_name, CEL_DIRENT_NAME(&dirent), &dirent, user_data)) != 0)
                break;
        }
    }while (cel_readdir(dir, &dirent) == 0);
    cel_closedir(dir);

    return ret;
}
