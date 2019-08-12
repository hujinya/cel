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
#include "cel/net/if.h"
#ifdef _CEL_UNIX
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>        //for struct ifreq


int cel_if_getindex(const TCHAR *if_name, int *if_index)
{
    int fd;
    struct ifreq ifr;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) > 0)
    {
        memset(&ifr, 0, sizeof(ifr));     
        strncpy(ifr.ifr_name, if_name, sizeof(ifr.ifr_name) - 1);   
        if (ioctl(fd, SIOCGIFINDEX, (char *)&ifr) == 0)
        {
            *if_index = ifr.ifr_ifindex;
            close(fd);
            return 0;
        }
        close(fd);
    }

    return -1;
}

int cel_if_gethrdaddr(const TCHAR *if_name, CelHrdAddr *hrdaddr)
{
    int fd;
    struct ifreq ifr;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) > 0)
    {
        memset(&ifr, 0, sizeof(ifr));     
        strncpy(ifr.ifr_name, if_name, sizeof(ifr.ifr_name) - 1);   
        if (ioctl(fd, SIOCGIFHWADDR, (char *)&ifr) == 0)
        {
            memcpy(hrdaddr, ifr.ifr_ifru.ifru_hwaddr.sa_data, sizeof(CelHrdAddr));
            close(fd);
            return 0;
        }
        close(fd);
    }

    return -1;
}

int cel_if_getipaddr(const TCHAR *if_name, CelIpAddr *ipaddr)
{
    int fd;
    struct ifreq ifr;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) > 0)
    {
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, if_name, sizeof(ifr.ifr_name) - 1); 
        if (ioctl(fd, SIOCGIFADDR, (char *)&ifr) == 0
            && ifr.ifr_ifru.ifru_addr.sa_family == AF_INET)
        {
            *ipaddr = ((struct sockaddr_in *)(&ifr.ifr_ifru.ifru_addr))->sin_addr;
            close(fd);
            return 0;
        }
        close(fd);
    }

    return -1;
}

int cel_if_getip6addr(const TCHAR *if_name, CelIp6Addr *ip6addr)
{
    return -1;
}

#define cel_if_down(fd, if_name, ifr) \
    memset((ifr), 0, sizeof(struct ifreq)); \
    strncpy((ifr)->ifr_name, if_name, sizeof((ifr)->ifr_name) - 1); \
    ioctl(fd, SIOCGIFFLAGS, (char *)(ifr)); \
    (ifr)->ifr_flags &= ~IFF_UP; \
    ioctl(fd, SIOCSIFFLAGS, (char *)(ifr))
#define cel_if_up(fd, if_name, ifr) \
    memset((ifr), 0, sizeof(struct ifreq)); \
    strncpy((ifr)->ifr_name, if_name, sizeof((ifr)->ifr_name) - 1); \
    ioctl(fd, SIOCGIFFLAGS, (char *)(ifr)); \
    (ifr)->ifr_flags &= IFF_UP; \
    ioctl(fd, SIOCSIFFLAGS, (char *)(ifr))

int cel_if_sethrdaddr(const TCHAR *if_name, CelHrdAddr *hrdaddr)
{
    int fd;
    struct ifreq ifr;

    /* Linux refuse to change the hwaddress if the interface is up */
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) > 0)
    {
        cel_if_down(fd, if_name, &ifr);

        cel_if_up(fd, if_name, &ifr);
    }
    return -1;
}

int cel_if_setipaddr(const TCHAR *if_name, CelIpAddr *ipaddr)
{
    int fd;
    struct ifreq ifr;

    /* Linux refuse to change the ipdress if the interface is up */
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) > 0)
    {
        cel_if_down(fd, if_name, &ifr);

        cel_if_up(fd, if_name, &ifr);
    }

    return -1;
}

int cel_if_setip6addr(const TCHAR *if_name, CelIp6Addr *ip6addr)
{
    return -1;
}

int cel_ipaddr_op(int if_index, CelIpAddr *addr, const TCHAR *label, int type)
{
    int size;
    char buf[256];
    CelNetLink *nl;
    struct nlmsghdr *nl_msg = (struct nlmsghdr *)buf;
    struct nlmsgerr *nl_msgerr;
    
    if ((nl = cel_netlink_new(NETLINK_ROUTE, 0 , 0)) == NULL)
        return -1;
    cel_nlmsg_build(nl_msg, sizeof(buf), type, NLM_F_REQUEST | NLM_F_ACK, 0, 0);
    cel_nlmsg_add_ifaddrmsg(nl_msg, sizeof(buf), AF_INET, 32, 0, 0, if_index);
    cel_nlmsg_add_rtattr(nl_msg, sizeof(buf), IFA_LOCAL, addr, sizeof(CelIpAddr));
    if (label != NULL)
        cel_nlmsg_add_rtattr(nl_msg, sizeof(buf), IFA_LABEL, label, _tcslen(label) + 1);
    //printf("%d\r\n", nl_msg->nlmsg_len);
    if ((size = cel_netlink_send(nl, nl_msg, 0, 0, 0)) < (int)nl_msg->nlmsg_len)
    {
        cel_netlink_free(nl);
        return -1;
    }
    //printf("send size %d\r\n", size);
    nl_msg = (struct nlmsghdr *)buf;
    nl_msg->nlmsg_len = sizeof(buf);
    if ((size = cel_netlink_recv(nl, nl_msg, NULL, NULL, 0)) <= 0)
    {
        cel_netlink_free(nl);
        return -1;
    }
    //printf("receive size %d\r\n", size);
    while (NLMSG_OK(nl_msg, (size_t)size))
    {
        //printf("msg type %d\r\n", nl_msg->nlmsg_type);
        if (nl_msg->nlmsg_type == NLMSG_DONE)
        {
            cel_netlink_free(nl);
            return 0;
        }
        else if (nl_msg->nlmsg_type == NLMSG_ERROR)
        {
            nl_msgerr = (struct nlmsgerr *)NLMSG_DATA(nl_msg);
            if (nl_msgerr->error != 0)
            {
				cel_seterr(-nl_msgerr->error, NULL);
                cel_netlink_free(nl);
                return -1;
            }
        }
        nl_msg = NLMSG_NEXT(nl_msg, size);
    }
    cel_netlink_free(nl);

    return 0;
}

#endif
