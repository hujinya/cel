#include "cel/conf.h"

typedef struct _VpBalancer
{
    TCHAR host[15];
    unsigned short port;
    unsigned short wmip_port;
}VpBalancer;

typedef struct _Vp
{
    TCHAR host[15];
    unsigned short port;
    VpBalancer balancer[32];
}Vp;

int profile_test(int argc, TCHAR *argv[])
{
    return 0;
}
