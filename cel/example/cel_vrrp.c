#include "cel/net/vrrp.h"
#include "cel/error.h"

int vrrp_test(int argc, TCHAR *argv[])
{
    //TCHAR ip6str[] = {_T("ff80:0000:0001:aaaa:0000:0000:00c2:0000")};

    //CelIpAddr ipaddr;
    //CelIp6Addr ip6addr;

    //cel_ip6addr_pton(ip6str, &ip6addr);
    //_tprintf(_T("%x %x\r\n"), ip6addr.s6_addr[0], ip6addr.s6_addr[1]);
    //_tprintf(_T("Ip6:%s\r\n"), cel_ip6addr_ntop(&ip6addr));

    //cel_ipnetmask_dton(19, &ipaddr);
    //_tprintf(_T("Netmask:%s\r\n"), cel_ipaddr_ntop(&ipaddr));
    //_tprintf(_T("Prefixl en:%d\r\n"), cel_ipnetmask_ntod(&ipaddr));

    //cel_ip6netmask_dton(47, &ip6addr);
    //_tprintf(_T("Netmask:%s\r\n"), cel_ip6addr_ntop(&ip6addr));
    //_tprintf(_T("Prefixlen:%d\r\n"), cel_ip6netmask_ntod(&ip6addr));

#ifdef _CEL_UNIX
    //const TCHAR *vaddr[6] = {"192.168.2.44"};
    //const TCHAR *peers[6] = {"192.168.0.132", "192.168.2.45", "192.168.0.177"};
    //CelVrrpRouter *vrrp_rt;
    //CelIpAddr adver_src;
    //CelVrrpState st1, st2 = -1;

    //cel_if_getipaddr("eth0", &adver_src);
    //cel_vrrp_set_advertisment_src(&adver_src);
    //if ((vrrp_rt = 
    //    cel_vrrprouter_new("eth0", 145, 1, 0, CEL_VRRP_AUTH_NONE, NULL, vaddr,1, peers, 3)) == NULL)
    //{
    //    _tprintf(_T("%s\r\n"), cel_geterrstr());
    //    return 1;
    //}
    //cel_vrrprouter_startup(vrrp_rt);
    //while(TRUE)
    //{
    //    sleep(1);
    //    cel_vrrprouter_check_state(vrrp_rt, &st1, NULL);
    //    if (st1 != st2)
    //    {
    //        st2 = st1;
    //    }
    //    _tprintf(_T("self %s(%d), active %s, next %s.\r\n"),
    //        cel_vrrprouter_states(vrrp_rt), vrrp_rt->current_priority,
    //        vrrp_rt->active != NULL ? cel_ipaddr_ntop(&(vrrp_rt->active->ip)) : _T("Initializing"),
    //        vrrp_rt->next != NULL ? cel_ipaddr_ntop(&(vrrp_rt->next->ip)) : _T("Initializing"));
    //}
#endif

    return 0;
}

