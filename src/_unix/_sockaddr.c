#include "cel/net/sockaddr.h"
#ifdef _CEL_UNIX
#include "cel/allocator.h"
#include "cel/net/if.h"

const int c_addrlen[] = 
{ 
    0, sizeof(struct sockaddr_un), sizeof(struct sockaddr_in), 0, 0, /* 0 - 4*/
    0, 0, 0, 0, 0, /* 5 - 9 */
    0, 0, 0, 0, 0, /* 10 - 14 */
    0, 0, 0, 0, 0, /* 15 - 19 */
    0, 0, 0, sizeof(struct sockaddr_in6), 0 /* 20 - 24 */
};

int cel_sockaddr_init_unix(CelSockAddr *addr, const TCHAR *path)
{
    addr->addr_un.sun_family = AF_UNIX; /**< Unix domain socket */
    strncpy(addr->addr_un.sun_path, path, sizeof(addr->addr_un.sun_path));

    return 0;
}

int cel_sockaddr_init_nl(CelSockAddr *addr, unsigned long pid, unsigned long groups)
{
    addr->addr_nl.nl_family = AF_NETLINK;
    addr->addr_nl.nl_pid = pid;
    addr->addr_nl.nl_groups = groups;

    return 0;
}

CelSockAddr *cel_sockaddr_new_unix(const TCHAR *path)
{
    CelSockAddr *addr;

    if ((addr = (CelSockAddr *)cel_malloc(sizeof(CelSockAddr))) != NULL)
    {   
        if (cel_sockaddr_init_unix(addr, path) == 0)
            return addr;
        cel_free(addr);
    }

    return addr;
}

CelSockAddr *cel_sockaddr_new_nl(unsigned long pid, unsigned long groups)
{
    CelSockAddr *addr;

    if ((addr = (CelSockAddr *)cel_malloc(sizeof(CelSockAddr))) != NULL)
    {   
        if (cel_sockaddr_init_nl(addr, pid, groups) == 0)
            return addr;
        cel_free(addr);
    }

    return addr;
}

#endif
