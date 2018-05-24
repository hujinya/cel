#include "cel/file.h"

int dir_each(const TCHAR *dir_name, const TCHAR *file_name, 
             const CelDirent *dirent, void *user_data)
{
    _tprintf(_T("%s\r\n"), file_name);
    return 0;
}

int file_test(int argc, TCHAR *argv[])
{
    TCHAR *rel_path = _T("./example.c");
    TCHAR *full_path, *path, *name, *ext;

    full_path = cel_fullpath(rel_path);
    path = cel_filedir(full_path);
    name = cel_filename(full_path);
    ext = cel_fileext(full_path);
    _tprintf(_T("Rel path:%s\r\nFull path:%s\r\nPath:%s\r\nName:%s\r\nExtension:%s\r\n"), rel_path, full_path,path,name, ext);
    cel_mkdirs("../hujy@VAST/xxx/x", 0);
    cel_foreachdir(path, dir_each, NULL);

    return 0;
}
