#include "cel/sys/user.h"

int user_test(int argc, TCHAR *argv[])
{
#ifdef _CEL_WIN
    BOOL b_ret;
    DWORD dwAuto = 0, dwSize = 128;
    char user[128];

    b_ret = cel_getautologon(&dwAuto, user, &dwSize);

    printf("%ld %s", dwAuto, user);
    if (dwAuto == 0)
    {
        dwAuto = 1;
        //cel_setautologon(dwAuto, "test", "123456");
    }
#endif
    return 0;
}
