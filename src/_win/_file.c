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