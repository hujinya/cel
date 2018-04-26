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