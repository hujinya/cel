/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com) 
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation; either version 2 
 * of the License, or (at your option) any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 */
#include "cel/net/hacluster.h"
#include "cel/error.h"
#include "cel/allocator.h"
#include "cel/log.h"
#include "cel/convert.h"
#include "cel/net/ethernet.h"
#include "cel/net/ip.h"
#include "cel/crypto/checksum.h" /* cel_checksum() */
#ifdef _CEL_UNIX
#include <ifaddrs.h> /* getifaddrs(struct ifaddrs *ifa) */
#endif

#define Debug(args)    cel_log_debug args
#define Warning(args)  /*CEL_SETERRSTR(args)*/ cel_log_warning args
#define Err(args)    /*CEL_SETERRSTR(args)*/ cel_log_err args

#define CEL_HA_ADVER_INTERVAL(ha_grp) ((ha_grp)->adver_int)
#define CEL_HA_DOWN_INTERVAL(ha_grp) (6 * (ha_grp)->adver_int)

const TCHAR *c_hastatestr[] = { _T("init"), _T("active"), _T("standby")};
static CelHaCluster *s_hac = NULL;

static void cel_hacluster_vaddrs_clear(void)
{
#ifdef _CEL_UNIX
    struct ifaddrs *ifaddr, *ifa;
    int ifa_index = 0;

    if (getifaddrs(&ifaddr) == 0) 
    {
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
        {  
            if (ifa->ifa_addr == NULL
                || ifa->ifa_addr->sa_family != AF_INET
                || _tcsrchr(ifa->ifa_name, _T(':')) == NULL
                || cel_if_getindex(ifa->ifa_name, &ifa_index) == -1)
                continue;
            cel_if_delipaddr(ifa_index, 
                &(((struct sockaddr_in *)ifa->ifa_addr)->sin_addr));
            Debug((_T("Ha cluster delete virtual address %s dev %s"),
                 cel_ipaddr_ntop(&(((struct sockaddr_in *)ifa->ifa_addr)->sin_addr)), 
                 ifa->ifa_name));
        }
        freeifaddrs(ifaddr);
    }
#endif
}

static int cel_hacluster_init(void)
{
    int i;

    if ((s_hac = (CelHaCluster *)cel_malloc(sizeof(CelHaCluster))) == NULL)
        return -1;
    s_hac->unicast = TRUE;
    s_hac->unicast_port = CEL_HA_UNICAST_PORT;
    s_hac->multicast = FALSE;
    s_hac->ttl = CEL_HA_MULTICAST_TTL;
    s_hac->multicast_addr.s_addr = htonl(CEL_HA_MULTICAST_ADDR);
    memset(s_hac->multicast_if, 0, CEL_IFNLEN);
    memset(&(s_hac->multicast_src), 0, sizeof(s_hac->multicast_src));
    for (i = 0; i < CEL_HA_DEVICE_NUM; i++)
        s_hac->ha_devices[i] = NULL;
    s_hac->self = NULL;
    for (i = 0; i < CEL_HA_GROUP_NUM; i++)
        s_hac->ha_grps[i] = NULL;
    cel_hacluster_vaddrs_clear();
    s_hac->multicast_fd = -1;
    s_hac->unicast_fd = -1;

    return 0;
}

void cel_hacluster_destroy(void)
{
    if (s_hac->multicast_fd > 0)
    {
        closesocket(s_hac->multicast_fd);
        s_hac->multicast_fd = -1;
    }
    if (s_hac->unicast_fd > 0)
    {
        closesocket(s_hac->multicast_fd);
        s_hac->multicast_fd = -1;
    }
}

void cel_hacluster_unicast_enable(BOOL enable)
{
    if (s_hac == NULL)
        cel_hacluster_init();
    if (!(s_hac->unicast = enable) && s_hac->unicast_fd > 0)
    {
        closesocket(s_hac->unicast_fd);
        s_hac->unicast_fd = -1;
    }
}

void cel_hacluster_multicast_enable(BOOL enable)
{
    if (s_hac == NULL)
        cel_hacluster_init();
    if (!(s_hac->multicast = enable) && s_hac->multicast_fd > 0)
    {
        closesocket(s_hac->multicast_fd);
        s_hac->multicast_fd = -1;
    }
}

void cel_hacluster_multicast_src_set(CelIpAddr *src)
{
    if (s_hac == NULL)
        cel_hacluster_init();
    if (memcmp(&(s_hac->multicast_src), src, sizeof(s_hac->multicast_src)) != 0)
    {
        memcpy(&(s_hac->multicast_src), src, sizeof(s_hac->multicast_src));
        if (s_hac->multicast_fd > 0)
        {
            closesocket(s_hac->multicast_fd);
            s_hac->multicast_fd = -1;
        }
        if (s_hac->unicast_fd > 0)
        {
            closesocket(s_hac->unicast_fd);
            s_hac->unicast_fd = -1;
        }
    }
}

void cel_hacluster_unicast_port_set(unsigned short port)
{
    if (s_hac == NULL)
        cel_hacluster_init();
    if (s_hac->unicast_port != port)
    {
        if (s_hac->multicast_fd > 0)
        {
            closesocket(s_hac->multicast_fd);
            s_hac->multicast_fd = -1;
        }
        if (s_hac->unicast_fd > 0)
        {
            closesocket(s_hac->unicast_fd);
            s_hac->unicast_fd = -1;
        }
    }
}

//static SOCKET cel_mulitcast_open(const TCHAR *ifname,
//                                 CelIpAddr *multiaddr, BYTE ttl)
//{
//    char loop = 0;
//    SOCKET fd;
//    struct in_addr ifaddr;
//    struct ip_mreq mreq;
//
//    if ((fd = socket(AF_INET, SOCK_RAW, CEL_HA_IPPROTO)) <= 0)
//    {
//        Err((_T("Create multicast socket failed(%s)."), cel_geterrstr(cel_sys_geterrno())));
//        return -1;
//    }
//    /* Set multicast ip options */
//    if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP,
//        (char *)&loop, sizeof(loop)) == -1
//        || setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(ttl)) == -1
//        || cel_if_getipaddr(ifname, &ifaddr) == -1
//        || setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF,
//        (char *)&ifaddr, sizeof(ifaddr)) == -1)
//    {
//        Err((_T("Set muliticast ip options failed(%s)."), cel_geterrstr(cel_sys_geterrno())));
//        closesocket(fd);
//        return -1;
//    }
//    /* Join the multicast group */
//    memset(&mreq, 0, sizeof(mreq));
//    mreq.imr_multiaddr = *multiaddr;
//    mreq.imr_interface = ifaddr;
//    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
//        (char *)&mreq, sizeof (mreq)) == -1)
//    {
//        Err((_T("Add membership failed, multiaddr %s, interface %s(%s)."), 
//            cel_ipaddr_ntop(multiaddr), cel_ipaddr_ntop(&ifaddr), cel_geterrstr(cel_sys_geterrno())));
//        closesocket(fd);
//        return -1;
//    }
//    /*_tprintf(_T("Ok, multicast fd %d, localaddr %s, multiaddr %s.\r\n"), 
//        fd, cel_ipaddr_ntop(&ifaddr), cel_ipaddr_ntop(multiaddr));*/
//
//    return fd;
//}

//
//static SOCKET cel_unicast_open(unsigned short port)
//{
//    SOCKET fd;
//    int reuseaddr = 1;
//    struct sockaddr_in addr;
//
//    if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) <= 0)
//    {
//        Err((_T("Create unicast socket failed(%s)."), cel_geterrstr(cel_sys_geterrno())));
//        return -1;
//    }
//    addr.sin_family = AF_INET;
//    addr.sin_addr.s_addr = INADDR_ANY;
//    addr.sin_port = htons(port);
//    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, 
//        (char *)&reuseaddr, sizeof(reuseaddr)) == -1
//        || bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
//    {
//        Err((_T("Bind address %s failed(%s)."), 
//            cel_sockaddr_ntop((CelSockAddr *)&addr), cel_geterrstr(cel_sys_geterrno())));
//        closesocket(fd);
//        return -1;
//    }
//    /*_tprintf(_T("Ok, unicast fd %d, bind %s.\r\n"), 
//        fd, cel_sockaddr_ntop((CelSockAddr *)&addr));*/
//
//    return fd;
//}
//

static int cel_hacluster_multicast_open(void)
{
    char loop = 0, ttl = CEL_HA_MULTICAST_TTL;
    struct in_addr multi_if;
    struct ip_mreq req;

    if ((s_hac->multicast_fd = socket(AF_INET, SOCK_RAW, CEL_HA_IPPROTO)) <= 0)
    {
        Err((_T("Ha cluster multicast socket failed(%s)."), cel_geterrstr(cel_sys_geterrno())));
        return -1;
    }
    /* Set multicast send options */
    multi_if = s_hac->multicast_src;
    if (setsockopt(s_hac->multicast_fd, 
        IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loop, sizeof(loop)) == -1
        || setsockopt(s_hac->multicast_fd, 
        IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(ttl)) == -1
        || setsockopt(s_hac->multicast_fd, 
        IPPROTO_IP, IP_MULTICAST_IF, (char *)&multi_if, sizeof(multi_if)) == -1)
    {
        Err((_T("Ha cluster set muliticast options failed(%s)."),
            cel_geterrstr(cel_sys_geterrno())));
        closesocket(s_hac->multicast_fd);
        s_hac->multicast_fd = -1;
        return -1;
    }
    /* Join the multicast group */
    memset(&req, 0, sizeof(req));
    req.imr_multiaddr.s_addr = htonl(CEL_HA_MULTICAST_ADDR);
    req.imr_interface = s_hac->multicast_src;
    if (setsockopt(s_hac->multicast_fd, 
        IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&req, sizeof (struct ip_mreq)) == -1)
    {
        Err((_T("Ha cluster join muliticast group %s failed(%s)."), 
            cel_ipaddr_ntop(&(s_hac->multicast_src)), cel_geterrstr(cel_sys_geterrno())));
        closesocket(s_hac->multicast_fd);
        s_hac->multicast_fd = -1;
        return -1;
    }
    //_tprintf("OK %s\r\n", cel_ipaddr_ntop(&(s_hac->multicast_src)));
    
    return 0;
}

static int cel_hacluster_multicast_send(void *buf, size_t size)
{
    struct sockaddr_in to;

    if (s_hac->multicast_fd <= 0 
        && cel_hacluster_multicast_open() == -1)
        return -1;
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = htonl(CEL_HA_MULTICAST_ADDR);
    to.sin_port = 0;
    if (sendto(s_hac->multicast_fd,
        buf, (int)size, 0, (struct sockaddr *)&to, sizeof(struct sockaddr_in)) != (int)size)
    {
        Err((_T("Multicast send failed(%s)."), cel_geterrstr(cel_sys_geterrno())));
        closesocket(s_hac->multicast_fd);
        s_hac->multicast_fd = -1;
        return -1;
    }
    //_tprintf(_T("Send to %s\r\n"), cel_sockaddr_ntop((CelSockAddr *)&to));

    return 0;
}

static int cel_hacluster_multicast_recv(void *buf, size_t size)
{
    fd_set nfds;
    struct timeval timeout = { 0, 0 };
    int r_size = 0;

    if (s_hac->multicast_fd <= 0 
        && cel_hacluster_multicast_open() == -1)
        return -1;
    FD_ZERO(&nfds);
    FD_SET(s_hac->multicast_fd, &nfds);
    if (select((int)(s_hac->multicast_fd + 1), &nfds, NULL, NULL, &timeout) > 0)
    {
        if ((r_size = recv(s_hac->multicast_fd, buf, (int)size, 0)) <= 0)
        {
            Err((_T("Multicast receive failed(%s)."), cel_geterrstr(cel_sys_geterrno())));
            closesocket(s_hac->multicast_fd);
            s_hac->multicast_fd = -1;
            return -1;
        }
    }

    return r_size;
}

static int cel_hacluster_unicast_open(void)
{
    int reuseaddr = 1;
    struct sockaddr_in addr;

    if ((s_hac->unicast_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) <= 0)
    {
        Err((_T("Create unicast socket failed(%s)."), cel_geterrstr(cel_sys_geterrno())));
        return -1;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr = s_hac->multicast_src;
    addr.sin_port = htons(s_hac->unicast_port);
    if (setsockopt(s_hac->unicast_fd, 
        SOL_SOCKET, SO_REUSEADDR, (char *)&reuseaddr, sizeof(reuseaddr)) == -1
        || bind(s_hac->unicast_fd, 
        (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
    {
        Err((_T("Unicast set option failed(%s)."), cel_geterrstr(cel_sys_geterrno())));
        return -1;
    }
    //_putts("ok");
    return 0;
}

static int cel_hacluster_unicast_sendto(void *buf, size_t size, CelIpAddr *ip_addr)
{
    int w_size = 0;
    struct sockaddr_in to;

    if (s_hac->unicast_fd <= 0 
        && cel_hacluster_unicast_open() == -1)
        return -1;
    to.sin_family = AF_INET;
    to.sin_addr = *ip_addr;
    to.sin_port = htons(s_hac->unicast_port);
    if ((w_size = sendto(s_hac->unicast_fd, 
        buf, (int)size, 0, (struct sockaddr *)&to, sizeof(struct sockaddr_in))) != (int)size)
    {
        Err((_T("Unicast send to %s failed(%s)."), 
            cel_sockaddr_ntop((CelSockAddr *)&to), cel_geterrstr(cel_sys_geterrno())));
        closesocket(s_hac->unicast_fd);
        s_hac->unicast_fd = -1;
        return -1;
    }
    //_tprintf(_T("Send to %s\r\n"), cel_sockaddr_ntop((CelSockAddr *)&to));

    return w_size;
}

static int cel_hacluster_unicast_recvfrom(void *buf, size_t size, CelIpAddr *ip_addr)
{
    int r_size = 0;
    fd_set nfds;
    struct timeval timeout = { 0, 0 };
    socklen_t from_len;
    struct sockaddr_in from;

    if (s_hac->unicast_fd <= 0 
        && cel_hacluster_unicast_open() == -1)
        return -1;
    from_len = sizeof(struct sockaddr_in);
    FD_ZERO(&nfds);
    FD_SET(s_hac->unicast_fd, &nfds);
    if (select((int)(s_hac->unicast_fd + 1), &nfds, NULL, NULL, &timeout) > 0)
    {
        if ((r_size = recvfrom(s_hac->unicast_fd, 
            buf, (int)size, 0, (struct sockaddr *)&from, &from_len)) <= 0)
        {
            Err((_T("Unicast receive failed(%s)."), cel_geterrstr(cel_sys_geterrno())));
            closesocket(s_hac->unicast_fd);
            s_hac->unicast_fd = -1;
            return -1;
        }
        *ip_addr = from.sin_addr;
    }

    return r_size;
}

static int cel_hacluster_send_gratuitous_arp(const TCHAR *if_name, 
                                             CelIpAddr *ip_addr, CelHrdAddr *hrd_addr)
{
    SOCKET fd;
    int msg_len;
    struct sockaddr dest;
    char buf[ETHER_HDR_LEN + CEL_ARPREQUEST_LEN];

    /* 0x300 is magic */
    if ((fd = socket(PF_PACKET, SOCK_PACKET, 0x300)) <= 0)
    {
        Err((_T("Create gratuitous arp socket failed(%s)."), cel_geterrstr(cel_sys_geterrno())));
        return -1;
    }
    msg_len = cel_ethernet_build_arp_request((struct ether_header *)buf, 
        ETHER_HDR_LEN + CEL_ARPREQUEST_LEN, ip_addr, hrd_addr);
    dest.sa_family = AF_UNSPEC;
#ifdef _UNICODE
    cel_unicode2utf8(if_name, -1, dest.sa_data, 14);
#else
    _tcsncpy(dest.sa_data, if_name, 14);
#endif
    if (sendto(fd, buf, msg_len, 0, &dest, sizeof(dest)) != msg_len)
    {
        Err((_T("Send gratuitous arp failed(%s)."), cel_geterrstr(cel_sys_geterrno())));
        closesocket(fd);
        return -1;
    }
    closesocket(fd);

    return 0;
}

static int cel_hagroup_send_advertisment(CelHaGroup *ha_grp, BYTE priority)
{
    int n_group, i;
    size_t dev_nl, len;
    CelHaDeviceMsg *dev_msg;
    CelHaGroupMsg *grp_msg;
    char buf[1024];

    dev_msg = (CelHaDeviceMsg *)buf;
    grp_msg = (CelHaGroupMsg *)(dev_msg + 1);
    /* Group message */
    n_group = 0;
    grp_msg->group_id = ha_grp->id;
    grp_msg->priority = ha_grp->self->priority;
    grp_msg->current_priority = priority;
    grp_msg->state = ha_grp->self->state;
    grp_msg += 1;
    n_group++;
    /* Device message */
    if ((dev_nl = _tcslen(ha_grp->self->dev_name)) > 0)
    {
        dev_nl *= sizeof(TCHAR);
        memcpy(grp_msg, ha_grp->self->dev_name, dev_nl);
    }
    dev_msg->version = CEL_HA_VERSION;
    dev_msg->type = 1;
    dev_msg->device_nl = (U8)dev_nl;
    dev_msg->device_security = 0x0000;
    dev_msg->n_group = n_group;
    dev_msg->adver_int = ha_grp->adver_int;
    dev_msg->check_sum = 0x0000;
    len = sizeof(CelHaDeviceMsg) + sizeof(CelHaGroupMsg) * n_group + dev_nl;
    dev_msg->check_sum = cel_checksum((U16 *)dev_msg, len);

    if (s_hac->unicast)
    {
        for (i = 0; i < ha_grp->n_members; i++)
        {
            if (ha_grp->members[i] != ha_grp->self)
                cel_hacluster_unicast_sendto(buf, len, &(ha_grp->members[i]->ip));
        }
    }
    if (s_hac->multicast)
    {
        cel_hacluster_multicast_send(buf, len);
    }

    return 0;
}

static int cel_hacluster_recv_advertisment(void *buf, size_t len, 
                                           CelIpAddr *local, 
                                           CelIpAddr *from, struct timeval *now)
{
    int i;
    CelHaDeviceMsg *dev_msg;
    CelHaGroupMsg *grp_msg;
    TCHAR *device_name;
    CelHaMember *meb = NULL, *self = NULL;
    CelHaGroup *ha_grp = NULL;

    dev_msg = (CelHaDeviceMsg *)buf;
    if (len < sizeof(CelHaDeviceMsg)
        || cel_checksum((u_short *)dev_msg, len) != 0x0000)
    {
        Warning((_T("Invalid checksum, len %d, from %s."), 
            len, cel_ipaddr_ntop(from)));
        return -1;
    }
    //_tprintf(_T("Recevied advertisement, ip %s .\r\n"), cel_ipaddr_ntop(from));
    grp_msg = (CelHaGroupMsg *)(dev_msg + 1);
    device_name = (TCHAR *)(grp_msg + dev_msg->n_group);
    for (i = 0; i < dev_msg->n_group; i++, grp_msg++)
    {
        /* Group [1-255] */
        if (grp_msg->group_id <= 0
            || (ha_grp = s_hac->ha_grps[grp_msg->group_id - 1]) == NULL
            /* Priority [1-254] */
            || grp_msg->priority > CEL_HA_MEMBER_NUM || grp_msg->priority <= 0 
            || (meb = ha_grp->members[CEL_HA_MEMBER_NUM - grp_msg->priority]) == NULL
            /*|| memcmp(meb->dev_name, device_name, dev_msg->device_nl) != 0*/
            || (self = ha_grp->self) == meb)
        {
            Warning((_T("Invalid group message, dev %s, grp %p[id %d], meb %p[priority %d], from %s."), 
                device_name, ha_grp, grp_msg->group_id, meb, grp_msg->priority, cel_ipaddr_ntop(from)));
            continue;
        }
        /*
        _tprintf(_T("dev %s group %d priority %d current_priority %d state %d\r\n"), 
            meb->dev_name, grp_msg->group_id,
            grp_msg->priority, grp_msg->current_priority, grp_msg->state);
         */   
        meb->state = (CelHaState)grp_msg->state;
        cel_timeval_set(&(meb->down_timer), now, CEL_HA_DOWN_INTERVAL(ha_grp)); 
        switch (ha_grp->self->state)
        {
        case CEL_HA_STATE_INIT: 
            if (grp_msg->current_priority == CEL_HA_PRIORITY_COUP)
            {
                /* standyby coup message */
                if (grp_msg->state == CEL_HA_STATE_STANDYBY)
                {
                    meb->state = CEL_HA_STATE_ACTIVE;
                    ha_grp->active = meb;
                }
                /* init coup message  */
                if (grp_msg->state == CEL_HA_STATE_INIT 
                    && self->priority < grp_msg->priority)
                {
                    meb->state = CEL_HA_STATE_STANDYBY;
                    ha_grp->next = meb;
                }
            }
            else if (grp_msg->current_priority == CEL_HA_PRIORITY_RESIGN)
            {
                /* active resign message*/
                if (grp_msg->state == CEL_HA_STATE_ACTIVE)
                    ha_grp->active = NULL;
                /* standyby resign message  */
                if (grp_msg->state == CEL_HA_STATE_STANDYBY)
                    ha_grp->next = NULL;
            }
            else
            {
                /* active advertisment message */
                if (grp_msg->state == CEL_HA_STATE_ACTIVE)
                {
                    ha_grp->active = meb;
                }
                /* standyby advertisment message */ 
                else if (grp_msg->state == CEL_HA_STATE_STANDYBY)
                {
                    if (ha_grp->preempt 
                        && self->priority > grp_msg->priority)
                    {
                        self->want_state = CEL_HA_STATE_STANDYBY;
                        cel_timeval_set(
                            &(self->coup_timer), now, CEL_HA_DOWN_INTERVAL(ha_grp));
                        cel_timeval_clear(&(ha_grp->adver_timer));
                    }
                    else
                    {
                        ha_grp->next = meb;
                    }
                }
            }
            break;
        case CEL_HA_STATE_STANDYBY:
            if (grp_msg->current_priority == CEL_HA_PRIORITY_COUP)
            {
                /* init coup message */ 
                if (grp_msg->state == CEL_HA_STATE_INIT)
                {
                    if (ha_grp->preempt && self->priority < grp_msg->priority)
                        self->want_state = CEL_HA_STATE_INIT;
                }
                /* standyby coup message */
            }
            if (grp_msg->current_priority == CEL_HA_PRIORITY_RESIGN)
            {
                /* active resign message*/
                if (grp_msg->state == CEL_HA_STATE_ACTIVE)
                    ha_grp->active = NULL;
                /* standyby resign message  */
            }
            else 
            {
                /* active advertisment message */
                if (grp_msg->state == CEL_HA_STATE_ACTIVE)
                    ha_grp->active = meb;
                /* standyby advertisment message */ 
                else if (grp_msg->state == CEL_HA_STATE_STANDYBY)
                {
                    if (self->priority < grp_msg->priority
                        || (self->priority == grp_msg->priority 
                        &&  _tcsncmp(device_name, self->dev_name, CEL_HNLEN) > 0))
                        self->want_state = CEL_HA_STATE_INIT;
                }
            }
            break;
        case CEL_HA_STATE_ACTIVE:
            if (grp_msg->current_priority == CEL_HA_PRIORITY_RESIGN)
            {
                /* active resign message*/
                if (grp_msg->state == CEL_HA_STATE_ACTIVE)
                {
                    cel_timeval_set(&(self->coup_timer), now, 0);
                    ha_grp->is_update = TRUE;
                }
                /* standyby resign message  */
                if (grp_msg->state == CEL_HA_STATE_STANDYBY)
                    ha_grp->next = NULL;
            }
            else 
            {
                /* active advertisment message */
                if (grp_msg->state == CEL_HA_STATE_ACTIVE)
                {
                    if (self->priority < grp_msg->priority
                        || (self->priority == grp_msg->priority 
                        &&  _tcsncmp(device_name, self->dev_name, CEL_HNLEN) > 0))
                    {
                        self->want_state = CEL_HA_STATE_INIT;
                    }
                    else
                    {
                        cel_timeval_set(&(self->coup_timer), now, 0);
                        ha_grp->is_update = TRUE;
                    }
                }
                /* standyby advertisment/standyby coup message */ 
                else if (grp_msg->state == CEL_HA_STATE_STANDYBY)
                {
                    if (ha_grp->preempt && self->priority < grp_msg->priority)
                        self->want_state = CEL_HA_STATE_INIT;
                    else
                        ha_grp->next = meb;
                }
            }
            break;
        default:
            break;
        }
    }
    return 0;
}

static int cel_hagroup_recv_advertisment(CelHaGroup *ha_grp, struct timeval *now)
{
    int r_size;
    int iphdr_len, msg_len;
    CelIpHdr *ip_hdr;
    CelHaDeviceMsg *dev_msg;
    CelIpAddr from;
    char buf[1024];

    while (s_hac->multicast 
        && (r_size = cel_hacluster_multicast_recv(buf, 1024)) > 0)
    {
        ip_hdr = (struct iphdr *)buf;
        iphdr_len = (ip_hdr->ihl << 2);
        from.s_addr = ip_hdr->saddr;
        /* MUST verify that the ip ttl is 255 */
        if (ip_hdr->ttl != CEL_HA_MULTICAST_TTL)
        {
            Warning((_T("Invalid ttl %d, excepted %d, from %s."), 
                ip_hdr->ttl, CEL_HA_MULTICAST_TTL, cel_ipaddr_ntop(&from)));
            continue;
        }
        dev_msg = (CelHaDeviceMsg *)((char *)ip_hdr + iphdr_len);
        msg_len = (ntohs(ip_hdr->tot_len) - iphdr_len);
        cel_hacluster_recv_advertisment(
            dev_msg, msg_len, &(s_hac->multicast_src), &from, now);
    }
    while (s_hac->unicast 
        && (r_size = cel_hacluster_unicast_recvfrom(buf, 1024, &from)) > 0)
    {
        //printf("recv size = %d\r\n", r_size);
        cel_hacluster_recv_advertisment(
            (CelHaDeviceMsg *)buf, r_size, &(s_hac->multicast_src), &from, now);
    }
    if (ha_grp->next != NULL
        && ha_grp->next != ha_grp->self
        && (ha_grp->next->state != CEL_HA_STATE_STANDYBY
        || (cel_timeval_is_expired(&(ha_grp->next->down_timer), now))))
    {
        ha_grp->next = NULL;
    }
    if (ha_grp->active != NULL 
        && ha_grp->active != ha_grp->self
        && (ha_grp->active->state != CEL_HA_STATE_ACTIVE
        || cel_timeval_is_expired(&(ha_grp->active->down_timer), now)))
    {
        ha_grp->active = NULL;
    }

    return 0;
}

static int cel_hagroup_add_vaddr(CelHaGroup *ha_grp, CelVirtualAddress *vaddr)
{
    /* Set the vrrp mac address -- rfc2338.7.3 */
    if (ha_grp->vhrd_on
        && cel_if_sethrdaddr(vaddr->if_name, &(ha_grp->vhrd)) == -1)
    {
        Err((_T("Set hrdaddr %s to %s failed(%s)."), 
            cel_hrdaddr_notp(&(ha_grp->vhrd)), vaddr->if_name, 
            cel_geterrstr(cel_sys_geterrno())));
        return -1;
    }
    /* 17 = File exists */
    if (cel_if_newipaddr(vaddr->if_index, &(vaddr->ip), vaddr->label) == -1
        && errno != 17)
    {
        Err((_T("Set ipaddr %s to %s failed,%s."), 
            cel_ipaddr_ntop(&(vaddr->ip)), vaddr->label, 
            cel_geterrstr(cel_sys_geterrno())));
        return -1;
    }
    cel_hacluster_send_gratuitous_arp(vaddr->if_name, &(vaddr->ip), 
        (ha_grp->vhrd_on ? &(ha_grp->vhrd) : &(vaddr->if_hrd)));
#ifdef _CEL_UNIX
    TCHAR cmd[CEL_CMDLLEN];
    if (vaddr->area != -1)
    {
        /* Don't use 'service srhac' */
        _sntprintf(cmd, CEL_CMDLLEN, 
            _T("/etc/init.d/srhac ospf 'network %s/%d area %d'"), 
            cel_ipaddr_ntop(&(vaddr->ip)), vaddr->prefix, vaddr->area);
        system(cmd);
        Debug((_T("System '%s'"), cmd));
    }
#endif
    Debug((_T("Add virtual ip address %s to %s."), 
        cel_ipaddr_ntop(&(vaddr->ip)), ha_grp->if_name));
    return 0;
}

static int cel_hagroup_del_vaddr(CelHaGroup *ha_grp, CelVirtualAddress *vaddr)
{
    /* Restore the original MAC addresses */
    if (ha_grp->vhrd_on)
    {
        if (cel_if_sethrdaddr(vaddr->if_name, &(vaddr->if_hrd)) == -1)
        {
            Err((_T("Set hrdaddr %s to %s failed(%s)."), 
                cel_hrdaddr_notp(&(vaddr->if_hrd)), vaddr->if_name, 
                cel_geterrstr(cel_sys_geterrno())));
            return -1;
        }
        cel_hacluster_send_gratuitous_arp(
            vaddr->if_name, &(vaddr->if_ip), &(vaddr->if_hrd));
    }
    /* 99 = Cannot assign requested address */
    if (cel_if_delipaddr(vaddr->if_index, &(vaddr->ip)) == -1
        && errno != 99)
    {
        //printf("errno = %d\r\n", errno);
        Debug((_T("Remove ipaddr %s from %s failed(%s)."), 
            cel_ipaddr_ntop(&(vaddr->ip)), vaddr->if_name, 
            cel_geterrstr(cel_sys_geterrno())));
        return -1;
    }
#ifdef _CEL_UNIX
    TCHAR cmd[CEL_CMDLLEN];
    if (vaddr->area != -1)
    {
        _sntprintf(cmd, CEL_CMDLLEN, 
            _T("/etc/init.d/srhac ospf 'no network %s/%d area %d'"), 
            cel_ipaddr_ntop(&(vaddr->ip)), vaddr->prefix, vaddr->area);
        system(cmd);
        Debug((_T("System '%s'"), cmd));
    }
#endif
    Debug((_T("Delete virtual ip address %s from %s."), 
        cel_ipaddr_ntop(&(vaddr->ip)), vaddr->if_name));
    return 0;
}

static int cel_hagroup_add_vaddrs(CelHaGroup *ha_grp)
{
    int i;
   
    /* Add the ip addresses */
    for (i = 0; i < ha_grp->n_vaddrs; i++)
       cel_hagroup_add_vaddr(ha_grp, ha_grp->vaddrs[i]); 
    return 0;
}

static int cel_hagroup_remove_vaddrs(CelHaGroup *ha_grp)
{
    int i;
    
    /* Remove the ip addresses */
    for (i = 0; i < ha_grp->n_vaddrs; i++)
        cel_hagroup_del_vaddr(ha_grp, ha_grp->vaddrs[i]);
    return 0;
}

static int cel_hagroup_vaddrs_track(CelHaGroup *ha_grp, BOOL is_update)
{
#ifdef _CEL_UNIX
    static struct ifaddrs *ifaddr, *ifa;
    int i, exist;
    struct sockaddr_in *sa_in;
    CelVirtualAddress *vaddr;

    if (getifaddrs(&ifaddr) == 0) 
    {
        for (i = 0; i < ha_grp->n_vaddrs; i++)
        {
            exist = 0;
            vaddr = ha_grp->vaddrs[i];
            for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
            {  
                if ((sa_in = (struct sockaddr_in *)ifa->ifa_addr) != NULL
                    && sa_in->sin_addr.s_addr == vaddr->ip.s_addr)
                {
                    exist = 1;
                    /*
                    _tprintf(_T("%s %s\r\n"), ifa->ifa_name, cel_ipaddr_ntop(&(vaddr->ip)));
                    */
                    break;
                }
            }
            if (exist == 0)
            {
                cel_hagroup_add_vaddr(ha_grp, vaddr);
            }
            else if (is_update)
            {
                cel_hacluster_send_gratuitous_arp(vaddr->if_name, &(vaddr->ip), 
                    (ha_grp->vhrd_on ? &(ha_grp->vhrd) : &(vaddr->if_hrd)));
                if (vaddr->area != -1)
                {
                    /* Don't use 'service srhac' */
                    TCHAR cmd[CEL_CMDLLEN];
                    _sntprintf(cmd, CEL_CMDLLEN, 
                        _T("/etc/init.d/srhac ospf 'network %s/%d area %d'"), 
                        cel_ipaddr_ntop(&(vaddr->ip)), vaddr->prefix, vaddr->area);
                    system(cmd);
                    Debug((_T("System '%s'"), cmd));
                }
            }
        }
        freeifaddrs(ifaddr);
    }
#endif
    return 0;
}

static int cel_hagroup_transition_state(CelHaGroup *ha_grp, struct timeval *now)
{
    switch (ha_grp->self->want_state)
    {
    case CEL_HA_STATE_ACTIVE:
        if (cel_hagroup_add_vaddrs(ha_grp) == -1
            || cel_hagroup_send_advertisment(ha_grp, CEL_HA_PRIORITY_COUP) == -1)
            return -1;
        if (ha_grp->active_action != NULL)
        {
            system(ha_grp->active_action);
            Debug((_T("System '%s'"), ha_grp->active_action));
        }
        /* Init Advertisement ticks */
        ha_grp->self->state = CEL_HA_STATE_ACTIVE;
        ha_grp->active = ha_grp->self;
        ha_grp->next = NULL;
        ha_grp->is_update = FALSE;
        //cel_hagroup_send_advertisment(ha_grp, ha_grp->self->priority);
        cel_timeval_set(&(ha_grp->adver_timer), now, CEL_HA_ADVER_INTERVAL(ha_grp));
        Debug((_T("Enter active state.")));
        break;
    case CEL_HA_STATE_STANDYBY:
        if (ha_grp->self->state == CEL_HA_STATE_ACTIVE)
        {
            /* If we goto back, warn the other ha_grps to speed up the recovery */
            if (cel_hagroup_remove_vaddrs(ha_grp) == -1)
                return -1;
            cel_hagroup_send_advertisment(ha_grp, CEL_HA_PRIORITY_RESIGN);
            if (ha_grp->standyby_action != NULL)
            {
                system(ha_grp->standyby_action);
                Debug((_T("System '%s'"), ha_grp->standyby_action));
            }
        }
        /* Init Advertisement ticks */
        ha_grp->self->state = CEL_HA_STATE_STANDYBY;
        //ha_grp->active = NULL;
        ha_grp->next = ha_grp->self;
        cel_timeval_set(&(ha_grp->adver_timer), now, CEL_HA_ADVER_INTERVAL(ha_grp));
        Debug((_T("Enter standyby state.")));
        break;
    case CEL_HA_STATE_INIT:
        if (ha_grp->self->state == CEL_HA_STATE_ACTIVE)
        {
            /* If we goto init, warn the other ha_grps to speed up the recovery */
            if (cel_hagroup_remove_vaddrs(ha_grp) == -1)
                return -1;
            cel_hagroup_send_advertisment(ha_grp, CEL_HA_PRIORITY_RESIGN);
            if (ha_grp->standyby_action != NULL)
            {
                system(ha_grp->standyby_action);
                Debug((_T("System '%s'"), ha_grp->standyby_action));
            }
        }
        ha_grp->self->state = CEL_HA_STATE_INIT;
        ha_grp->active = NULL;
        ha_grp->next = NULL;
        cel_timeval_clear(&(ha_grp->adver_timer));
        Debug((_T("Enter inite state.")));
        break;
    default:
        ha_grp->self->want_state = ha_grp->self->state;
        Err((_T("Want state %d undefined."), ha_grp->self->want_state));
        return -1;
    }

    return 0;
}

static int cel_hagroup_state_init(CelHaGroup *ha_grp, struct timeval *now)
{
    //puts("cel_hagroup_state_init");
    if (!CEL_CHECKFLAG(ha_grp->evt, CEL_HA_EVENT_SHUTDOWN))
    {
        if (CEL_CHECKFLAG(ha_grp->evt, CEL_HA_EVENT_FORCE_TO_STANDYBY))
            /*CEL_CLRFLAG(ha_grp->evt, CEL_HA_EVENT_FORCE_TO_STANDYBY*/
            ha_grp->evt = (CelHaEvent)(ha_grp->evt & (~CEL_HA_EVENT_FORCE_TO_STANDYBY));
        /* Receive advertiement */
        cel_hagroup_recv_advertisment(ha_grp, now);
        if (ha_grp->active == NULL || ha_grp->next == NULL)
        {
            if (ha_grp->self->want_state != CEL_HA_STATE_STANDYBY)
            {
                ha_grp->self->want_state = CEL_HA_STATE_STANDYBY;
                cel_timeval_clear(&(ha_grp->adver_timer));
                cel_timeval_set(
                    &(ha_grp->self->coup_timer), now, CEL_HA_DOWN_INTERVAL(ha_grp));
            }
            //_tprintf(_T("want state %d")CEL_CRLF, ha_grp->self->want_state);
            if (cel_timeval_is_expired(&(ha_grp->self->coup_timer), now))
            {
                if (ha_grp->next == NULL)
                    return cel_hagroup_transition_state(ha_grp, now);
            }
            if (cel_timeval_is_expired(&(ha_grp->adver_timer), now))
            {
                cel_timeval_set(
                    &(ha_grp->adver_timer), now, CEL_HA_ADVER_INTERVAL(ha_grp));
                cel_hagroup_send_advertisment(ha_grp, CEL_HA_PRIORITY_COUP);
            }
        }
        else
        {
            ha_grp->self->want_state = CEL_HA_STATE_INIT;
            cel_timeval_clear(&(ha_grp->adver_timer));
            cel_timeval_clear(&(ha_grp->self->coup_timer));
        }
    }

    return 0;
}

static int cel_hagroup_state_active(CelHaGroup *ha_grp, struct timeval *now)
{
    if (CEL_CHECKFLAG(ha_grp->evt, CEL_HA_EVENT_FORCE_TO_STANDYBY)
        || CEL_CHECKFLAG(ha_grp->evt, CEL_HA_EVENT_SHUTDOWN))
    {
        ha_grp->self->want_state = CEL_HA_STATE_INIT;
        return cel_hagroup_transition_state(ha_grp, now);
    }
    /* Receive advertiement */
    cel_hagroup_recv_advertisment(ha_grp, now);
    if (ha_grp->self->want_state != ha_grp->self->state)
        return cel_hagroup_transition_state(ha_grp, now);
     /* Check expired, send advertisment */
    if (cel_timeval_is_expired(&(ha_grp->adver_timer), now))
    {
        cel_timeval_set(&(ha_grp->adver_timer), now, CEL_HA_ADVER_INTERVAL(ha_grp));
        cel_hagroup_send_advertisment(ha_grp, ha_grp->self->priority);
        if (ha_grp->is_update)
        {
            if (ha_grp->standyby_action != NULL)
            {
                system(ha_grp->standyby_action);
                Debug((_T("System '%s'"), ha_grp->standyby_action));
            }
        }
        cel_hagroup_vaddrs_track(ha_grp, ha_grp->is_update);
        if (ha_grp->is_update)
        {
            if (ha_grp->active_action != NULL)
            {
                system(ha_grp->active_action);
                Debug((_T("System '%s'"), ha_grp->active_action));
            }
            ha_grp->is_update = FALSE;
        }
    }

    return 0;
}

static int cel_hagroup_state_standyby(CelHaGroup *ha_grp, struct timeval *now)
{
    if (CEL_CHECKFLAG(ha_grp->evt, CEL_HA_EVENT_FORCE_TO_STANDYBY)
        || CEL_CHECKFLAG(ha_grp->evt, CEL_HA_EVENT_SHUTDOWN))
    {
        ha_grp->self->want_state = CEL_HA_STATE_INIT;
        return cel_hagroup_transition_state(ha_grp, now);
    }
    /* Receive advertiement */
    cel_hagroup_recv_advertisment(ha_grp, now);
    if (ha_grp->active == NULL)
        ha_grp->self->want_state = CEL_HA_STATE_ACTIVE;
    if (ha_grp->self->want_state != ha_grp->self->state)
        return cel_hagroup_transition_state(ha_grp, now);
    /* Check expired, send advertisment */
    if (cel_timeval_is_expired(&(ha_grp->adver_timer), now))
    {
        cel_timeval_set(&(ha_grp->adver_timer),
            now, CEL_HA_ADVER_INTERVAL(ha_grp));
        if (cel_hagroup_send_advertisment(ha_grp, ha_grp->self->priority) == -1)
            return -1;
    }

    return 0;
}

int cel_hagroup_check_state(CelHaGroup *ha_grp, CelHaState *state, struct timeval *now)
{
    int ret = -1;

    CEL_TIMEVAL_NOW(now);
    //_tprintf(_T("event 0x%x\r\n"), ha_grp->evt);
    switch (ha_grp->self->state)
    {
    case CEL_HA_STATE_INIT:
        ret = cel_hagroup_state_init(ha_grp, now);
        break;
    case CEL_HA_STATE_ACTIVE:
        ret = cel_hagroup_state_active(ha_grp, now);
        break;
    case CEL_HA_STATE_STANDYBY:
        ret = cel_hagroup_state_standyby(ha_grp, now);
        break;
    default:
        Err((_T("Ha group %d want state %d undefined."), 
            ha_grp->id, ha_grp->self->state));
        ret = -1;
    }
    if (state != NULL)
        *state = ha_grp->self->state;

    return ret;
}

static int cel_hagroup_resolve_vaddr(const TCHAR *vaddr, CelVirtualAddress *_vaddr)
{
    int v = 0;
    TCHAR v_addr[CEL_HNLEN];

    _tcsncpy(v_addr, vaddr, CEL_HNLEN);
    _vaddr->prefix = 32;
    _vaddr->area = -1;
    while (v_addr[v] != _T('\0'))
    {
        if (v_addr[v] == _T(' '))
        {
            v_addr[v] = _T('\0');
        }
        else if (v_addr[v] == _T('/'))
        {
            _vaddr->prefix = _tstoi(&v_addr[v + 1]);
            v_addr[v] = _T('\0');
        }
        else if (_tcsncmp(&v_addr[v], _T("area "), 5) == 0)
        {
            v += 5;
            //_putts(&v_addr[v]);
            _vaddr->area = _tstoi(&v_addr[v]);
        }
        else if (_tcsncmp(&v_addr[v], _T("dev "), 4) == 0)
        {
            v += 4;
            if (v_addr[v] != _T('\0'))
                _tcsncpy(_vaddr->if_name, &v_addr[v], CEL_IFNLEN);
            //_putts(&v_addr[v]);
        }
        v++;
    }
    if (_vaddr->if_name[0] == _T('\0')
        || cel_if_getindex(_vaddr->if_name, &_vaddr->if_index) == -1
        || cel_if_gethrdaddr(_vaddr->if_name, &_vaddr->if_hrd) == -1
        || cel_if_getipaddr(_vaddr->if_name, &_vaddr->if_ip) == -1)
    {
        Err((_T("Bad interface %s(%s)."), 
            _vaddr->if_name, cel_geterrstr(cel_sys_geterrno())));
        return -1;
    }
    return ((cel_ipaddr_pton(v_addr, &(_vaddr->ip))  == 1) ? 0 : -1);
}

static CelHaDevice *cel_hacluster_get_device(TCHAR *name)
{
    int i;
    CelHaDevice *dev;

    for (i = 0; i < CEL_HA_DEVICE_NUM; i++)
    {
        if ((dev = s_hac->ha_devices[i]) != NULL
            && _tcsncpy(name, dev->name, CEL_HNLEN) == 0)
            return dev;
    }
    return NULL;
}

int cel_hagroup_init(CelHaGroup *ha_grp, int id, int preempt, TCHAR *if_name,
                     TCHAR **vaddr, int n_vaddr,
                     TCHAR *active_action, TCHAR *standyby_action,
                     TCHAR **members, int n_members)
{
    int i;
    CelHaDevice *dev;
    CelIpAddr ip_addr;
    CelVirtualAddress _vaddr;

    if (s_hac == NULL)
        cel_hacluster_init();
    if (!CEL_ISVALID(id, 1, 255))
    {
        Err((_T("Bad virtual ha_grp id %d."), id));
        return -1;
    }
    ha_grp->if_name[0] = _T('\0');
    if (if_name != NULL)
        _tcsncpy(ha_grp->if_name, if_name, CEL_IFNLEN);
    ha_grp->id = (BYTE)id;
    ha_grp->adver_int = 500;
    ha_grp->preempt = preempt;
    ha_grp->n_vaddrs = 0;
    for (i = 0; i < n_vaddr && i < CEL_HA_VADDR_NUM && vaddr[i] != NULL; i++)
    {
        //_tprintf(_T("%d/%d %s\r\n"), i, n_vaddr, vaddr[i]);
        _tcsncpy(_vaddr.if_name, ha_grp->if_name, CEL_IFNLEN);
        if (cel_hagroup_resolve_vaddr(vaddr[i], &_vaddr) != 0
            || (ha_grp->vaddrs[i] = (CelVirtualAddress *)
            cel_malloc(sizeof(CelVirtualAddress))) == NULL)
        {
            Err((_T("Bad virtual address %s."), vaddr[i]));
            return -1;
        }
        memcpy(ha_grp->vaddrs[i], &_vaddr, sizeof(CelVirtualAddress));
        _sntprintf(ha_grp->vaddrs[i]->label, CEL_IFNLEN,
            _T("%s:%d"), ha_grp->vaddrs[i]->if_name, ha_grp->id);
        /*
        _tprintf(_T("%s/%d area %d dev %s\r\n"), 
            cel_ipaddr_ntop(&(ha_grp->vaddrs[i]->ip)),
            ha_grp->vaddrs[i]->prefix, ha_grp->vaddrs[i]->area, 
            ha_grp->vaddrs[i]->if_name);
        */
        ha_grp->n_vaddrs++;
    }

    ha_grp->active_action = 
        (active_action != NULL ? cel_tcsdup(active_action) : NULL);
    ha_grp->standyby_action  = 
        (standyby_action != NULL ? cel_tcsdup(standyby_action) : NULL);

    /* Virtual Router MAC Address RFC 3768.7.3 */
    ha_grp->vhrd[0] = 0x00;
    ha_grp->vhrd[1] = 0x00;
    ha_grp->vhrd[2] = 0x5E;
    ha_grp->vhrd[3] = 0x00;
    ha_grp->vhrd[4] = 0x01;
    ha_grp->vhrd[5] = ha_grp->id;

    ha_grp->n_members = 0;
    for (i = 0; i < n_members && i < CEL_HA_MEMBER_NUM && members[i] != NULL; i++)
    {
        //_tprintf(_T("%d/%d %s\r\n"), i, n_members, members[i]);
        if (((dev = cel_hacluster_get_device(members[i])) == NULL
            && cel_ipaddr_pton(members[i], &ip_addr) == -1)
            || (ha_grp->members[i] = (CelHaMember *)cel_malloc(sizeof(CelHaMember))) == NULL)
        {
            Err((_T("Bad member %s."), members[i]));
            return -1;
        }
        ha_grp->members[i]->state = CEL_HA_STATE_INIT;
        ha_grp->members[i]->priority = CEL_HA_MEMBER_NUM - i;
        memcpy(ha_grp->members[i]->dev_name, 
            members[i], _tcslen(members[i]) * sizeof(TCHAR));
        if (dev != NULL)
        {
            ha_grp->members[i]->dev = dev;
            if (s_hac->self == dev)
                ha_grp->self = ha_grp->members[i];
        }
        else
        {
            memcpy(&(ha_grp->members[i]->ip), &ip_addr, sizeof(ip_addr));
            if (ip_addr.s_addr == s_hac->multicast_src.s_addr)
                ha_grp->self = ha_grp->members[i];
        }
        cel_timeval_clear(&(ha_grp->members[i]->down_timer));
        ha_grp->n_members++;
        /*
        _tprintf(_T("j = %d peer %s\r\n"), 
        j, cel_ipaddr_ntop(&(ha_grp->members[j]->ip)));
        */  
    }
    if (ha_grp->self == NULL)
    {
        Err((_T("No self members.")));
        return -1;
    }
    ha_grp->active = NULL;
    ha_grp->next = NULL;
    ha_grp->self->state = CEL_HA_STATE_INIT;
    ha_grp->self->want_state = CEL_HA_STATE_INIT;
    ha_grp->is_update = FALSE;
    //ha_grp->evt = CEL_HA_EVENT_SHUTDOWN;
    //ha_grp->current_priority = ha_grp->priority;
    ha_grp->vhrd_on = 0;

    s_hac->ha_grps[ha_grp->id - 1] = ha_grp;
    /*if (s_hac->multicast_src.s_addr == 0)
        s_hac->multicast_src = ip_addr;*/

    return 0;
}

int cel_hagroup_reload(CelHaGroup *ha_grp, int preempt, TCHAR *if_name,
                       TCHAR **vaddr, int n_vaddr,
                       TCHAR *active_action, TCHAR *standyby_action,
                       TCHAR **members, int n_members)
{
    int i ,j;
    CelIpAddr ip_addr;
    int n_vaddrs = 0;
    CelVirtualAddress *vaddrs[CEL_HA_VADDR_NUM];
    int vaddr_exists[CEL_HA_VADDR_NUM];
    CelVirtualAddress _vaddr;

    ha_grp->if_name[0] = _T('\0');
    if (if_name != NULL)
        _tcsncpy(ha_grp->if_name, if_name, CEL_IFNLEN);
    ha_grp->adver_int = 500;
    ha_grp->preempt = preempt;
    for (i = 0; i < n_vaddr && i < CEL_HA_VADDR_NUM && vaddr[i] != NULL; i++)
    {
        //_tprintf(_T("%d/%d %s\r\n"), i, n_vaddr, vaddr[i]);
        _tcsncpy(_vaddr.if_name, ha_grp->if_name, CEL_IFNLEN);
        if (cel_hagroup_resolve_vaddr(vaddr[i], &_vaddr) != 0
            || (vaddrs[n_vaddrs] = (CelVirtualAddress *)cel_malloc(sizeof(CelVirtualAddress))) == NULL)
        {
            Err((_T("Bad virtual address %s."), vaddr[i]));
            return -1;
        }
        memcpy(vaddrs[i], &_vaddr, sizeof(CelVirtualAddress));
        _sntprintf(vaddrs[i]->label, CEL_IFNLEN,
            _T("%s:%d"), vaddrs[i]->if_name, ha_grp->id);
        vaddr_exists[i] = 0;
        n_vaddrs++;
    }
    if (ha_grp->self->state == CEL_HA_STATE_ACTIVE)
    {
        for (i = 0; i < ha_grp->n_vaddrs; i++)
        {
            for (j = 0; j < n_vaddrs; j++)
            {
                if (_tcscmp(ha_grp->vaddrs[i]->if_name, vaddrs[j]->if_name) == 0
                    && ha_grp->vaddrs[i]->ip.s_addr == vaddrs[j]->ip.s_addr
                    && ha_grp->vaddrs[i]->prefix == vaddrs[j]->prefix
                    && ha_grp->vaddrs[i]->area == vaddrs[j]->area)
                {
                    vaddr_exists[j] = 1;
                    break;
                }
            }
            if (j == n_vaddrs)
            {
                cel_hagroup_del_vaddr(ha_grp, ha_grp->vaddrs[i]);
                //_tprintf("del vaddr %s\r\n", cel_ipaddr_ntop(&(ha_grp->vaddrs[i]->ip)));
            }
            cel_free(ha_grp->vaddrs[i]);
            ha_grp->vaddrs[i] = NULL;
        }
        for (i = 0; i < n_vaddrs; i++)
        {
            if (vaddr_exists[i] == 1)
                continue;
            cel_hagroup_add_vaddr(ha_grp, vaddrs[i]);
            //_tprintf("add vaddr %s\r\n", cel_ipaddr_ntop(&(vaddrs[i]->ip)));
        }
    }
    memcpy(ha_grp->vaddrs, vaddrs, n_vaddrs * sizeof(CelVirtualAddress *));
    ha_grp->n_vaddrs = n_vaddrs;

    ha_grp->active_action = 
        (active_action != NULL ? cel_tcsdup(active_action) : NULL);
    ha_grp->standyby_action  = 
        (standyby_action != NULL ? cel_tcsdup(standyby_action) : NULL);

    for (i = 0; i < n_members && members[i] != NULL; i++)
    {
        //_putts(members[i]);
        cel_ipaddr_pton(members[i], &ip_addr);
        if (i >= ha_grp->n_members 
            || ip_addr.s_addr != ha_grp->members[i]->ip.s_addr)
        {
            //_tprintf("members XXX\r\n");
            goto members_reload;
        }
    }
    if (i == ha_grp->n_members)
        return 0;
members_reload:
    //_tprintf("members reload\r\n");
    for (i = 0; i < ha_grp->n_members; i++)
        cel_free(ha_grp->members[i]);
    ha_grp->n_members = 0;
    for (i = 0; i < n_members && i < CEL_HA_MEMBER_NUM && members[i] != NULL; i++)
    {
        //_tprintf(_T("%d/%d %s\r\n"), i, n_members, members[i]);
        if (cel_ipaddr_pton(members[i], &ip_addr) == -1
            || (ha_grp->members[i] = 
            (CelHaMember *)cel_malloc(sizeof(CelHaMember))) == NULL)
        {
            Err((_T("Bad member %s."), members[i]));
            return -1;
        }
        ha_grp->members[i]->state = CEL_HA_STATE_INIT;
        ha_grp->members[i]->priority = CEL_HA_MEMBER_NUM - i;
        memcpy(ha_grp->members[i]->dev_name, 
            members[i], _tcslen(members[i]) * sizeof(TCHAR));
        ha_grp->members[i]->dev = NULL;
        memcpy(&(ha_grp->members[i]->ip), &ip_addr, sizeof(ip_addr));
        cel_timeval_clear(&(ha_grp->members[i]->down_timer));
        /*
        _tprintf(_T("j = %d peer %s\r\n"), 
        j, cel_ipaddr_ntop(&(ha_grp->members[i]->ip)));
        */  
        if (ip_addr.s_addr == s_hac->multicast_src.s_addr)
            ha_grp->self = ha_grp->members[i];
        ha_grp->n_members++;
    }
    ha_grp->active = NULL;
    ha_grp->next = NULL;
    //ha_grp->current_priority = ha_grp->priority;

    return 0;
}

void cel_hagroup_destroy(CelHaGroup *ha_grp)
{
    int i;

    if (ha_grp->self->state == CEL_HA_STATE_ACTIVE)
    {
        ha_grp->self->want_state = CEL_HA_STATE_STANDYBY;
        cel_hagroup_transition_state(ha_grp, NULL);
    }
    for (i = 0; i < ha_grp->n_vaddrs; i++)
    {
        cel_free(ha_grp->vaddrs[i]);
        ha_grp->vaddrs[i] = NULL;
    }
    for (i = 0; i < ha_grp->n_members; i++)
    {
        cel_free(ha_grp->members[i]);
        ha_grp->members[i] = NULL;
    }
   ha_grp->active = ha_grp->next = ha_grp->self = NULL;
   if (s_hac != NULL)
       s_hac->ha_grps[ha_grp->id - 1] = NULL;
}

CelHaGroup *cel_hagroup_new(int id, int preempt, TCHAR *if_name,
                            TCHAR **vaddr, int n_vaddr,
                            TCHAR *active_action, TCHAR *standyby_action,
                            TCHAR **members, int n_members)
{
    CelHaGroup *ha_grp;

    if ((ha_grp = (CelHaGroup *)cel_malloc(sizeof(CelHaGroup))) != NULL)
    {
        if (cel_hagroup_init(ha_grp, id, preempt, if_name,
            vaddr, n_vaddr, active_action, standyby_action, members, n_members) != -1)
            return ha_grp;
        cel_free(ha_grp);
    }

    return NULL;
}

void cel_hagroup_free(CelHaGroup *ha_grp)
{
    cel_hagroup_destroy(ha_grp);
    cel_free(ha_grp);
}
