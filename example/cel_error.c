#include "cel/error.h"

int error_test(int argc, TCHAR *argv[])
{
    CEL_SETERRSTR((_T("1-%s"), cel_geterrstr(cel_geterrno())));
    _tprintf(_T("%s\r\n"), cel_geterrstr(cel_geterrno()));
    CEL_SETERRSTR((_T("2-%s"), cel_geterrstr(cel_geterrno())));
    _tprintf(_T("%s\r\n"), cel_geterrstr(cel_geterrno()));
    CEL_SETERRSTR((_T("3-%s"), cel_geterrstr(cel_geterrno())));
    _tprintf(_T("%s\r\n"), cel_geterrstr(cel_geterrno()));
    CEL_SETERRSTR((_T("4-%s"), cel_geterrstr(cel_geterrno())));
    _tprintf(_T("%s\r\n"), cel_geterrstr(cel_geterrno()));

    return 0;
}