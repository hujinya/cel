#include "cel/net/if.h"
#include "cel/net/sockaddr.h"
#include "cel/error.h"

void get_mac()
{
    CelHrdAddr hwad_addr, *p;

    memset(&hwad_addr, 2, sizeof(hwad_addr));

    printf("size = %ld, %s\r\n", sizeof(hwad_addr), cel_hrdaddr_notp(&hwad_addr));
    printf("%02x:%02x:%02x:%02x:%02x:%02x\r\n", 
        hwad_addr[0], hwad_addr[1], hwad_addr[2], hwad_addr[3], hwad_addr[4], hwad_addr[5]);
    p = &hwad_addr;
    printf("%02x:%02x:%02x:%02x:%02x:%02x\r\n", 
        (*p)[0], (*p)[1], (*p)[2], (*p)[3], (*p)[4], (*p)[5]);
}

void show_addr()
{
    CelSockAddr addr;

    if (cel_sockaddr_init_str(&addr, _T("")) == -1)
        _putts(cel_geterrstr());
    _tprintf(_T("%s\r\n"), cel_sockaddr_ntop(&addr));
}

int socket_test(int argc, TCHAR *argv[])
{
    get_mac();
    show_addr();
    return 0;
}
