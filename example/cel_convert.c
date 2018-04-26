#include "cel/convert.h"

int convert_test(int argc, TCHAR *argv[])
{
    int j = 0;
    long long i = 1;

    while (j++ < 20)
    {
        _tprintf(_T("%lld -> %s\r\n"), i, cel_lltodda(i));
        i = i * 12;
    }
    _tprintf(_T("9223372036854775807 -> %s\r\n"), 
        cel_lltodda(LL(9223372036854775807)));
    i = -1;
    j = 0;
    while (j++ < 20)
    {
        _tprintf(_T("%lld -> %s\r\n"), i, cel_lltodda(i));
        i = i * 12;
    }
    _tprintf(_T("-9223372036854775807 -> %s\r\n"), 
        cel_lltodda(LL(-9223372036854775807)));

    return 0;
}
