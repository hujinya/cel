/**
 * CEL(C Extension Library)
 * Copyright (C)2008 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/types.h"
#ifdef _CEL_UNIX
#include "cel/_unix/_netlink.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"
#include <time.h> /* time() */


#include <arpa/inet.h>
int cel_netlink_init(CelNetLink *nl, int protocol, 
                     unsigned long pid, unsigned long groups)
{
    if ((nl->fd = socket(AF_NETLINK, SOCK_RAW, protocol)) != INVALID_SOCKET)
    {
        nl->local_addr.nl_family = AF_NETLINK;
        nl->local_addr.nl_pid = pid;
        nl->local_addr.nl_groups = groups;
        if (bind(nl->fd, 
            (struct sockaddr *)&(nl->local_addr), sizeof(nl->local_addr)) == 0)
        {
            nl->seq = time(NULL);
            return 0;
        }
        close(nl->fd);
    }
    return -1;
}

void cel_netlink_destroy(CelNetLink *nl)
{
    close(nl->fd);
    nl->fd = INVALID_SOCKET;
}

CelNetLink *cel_netlink_new(int protocol, unsigned long pid, unsigned long groups)
{
    CelNetLink *nl;

    if ((nl = (CelNetLink *)cel_malloc(sizeof(CelNetLink))) != NULL)
    {   
        if (cel_netlink_init(nl, protocol, pid, groups) == 0)
            return nl;
        cel_free(nl);
    }
    return NULL;
}

void cel_netlink_free(CelNetLink *nl)
{
    cel_netlink_destroy(nl);
    cel_free(nl);
}

int cel_nlmsg_build(struct nlmsghdr *nlmsg, size_t size,
                    int type, int flags, unsigned long seq, unsigned long pid)
{
    size_t nlmsg_len;

    if ((nlmsg_len = NLMSG_ALIGN(sizeof(struct nlmsghdr))) > size)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Length %d is too short, excepted %d."), size, nlmsg_len));
        return -1;
    }
    nlmsg->nlmsg_len = nlmsg_len; 
    nlmsg->nlmsg_type = type; 
    nlmsg->nlmsg_flags = flags; 
    nlmsg->nlmsg_seq = seq; 
    nlmsg->nlmsg_pid = pid; 

    return 0;
}

int cel_nlmsg_add_ifinfomsg(struct nlmsghdr *nlmsg, size_t size, 
                            BYTE family, unsigned short type, int index, 
                            unsigned int flags, unsigned int change)
{
    size_t nlmsg_len1, nlmsg_len2;
    struct ifinfomsg *ifi;

    nlmsg_len1 = NLMSG_ALIGN(nlmsg->nlmsg_len);
    nlmsg_len2 = NLMSG_ALIGN(nlmsg->nlmsg_len + sizeof(struct ifinfomsg));
    if (nlmsg_len2 > size)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Length %d is too short, excepted %d."), size, nlmsg_len2));
        return -1;
    }
    ifi = (struct ifinfomsg *)((char *)nlmsg + nlmsg_len1);
    ifi->ifi_family = family; 
    ifi->__ifi_pad = 0;
    ifi->ifi_type = type;
    ifi->ifi_index = index; 
    ifi->ifi_flags = flags;
    ifi->ifi_change = change;
    nlmsg->nlmsg_len = nlmsg_len2;

    return 0;
}

int cel_nlmsg_add_ifaddrmsg(struct nlmsghdr *nlmsg, size_t size,
                            BYTE family, BYTE prefixlen, 
                            BYTE flags, BYTE scope,  
                            unsigned int index)
{
    size_t nlmsg_len1, nlmsg_len2;
    struct ifaddrmsg *ifa;

    nlmsg_len1 = NLMSG_ALIGN(nlmsg->nlmsg_len);
    nlmsg_len2 = NLMSG_ALIGN(nlmsg->nlmsg_len + sizeof(struct ifaddrmsg));
    if (nlmsg_len2 > size)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Length %d is too short, excepted %d."), size, nlmsg_len2));
        return -1;
    }
    ifa = (struct ifaddrmsg *)((char *)nlmsg + nlmsg_len1);
    ifa->ifa_family = family; 
    ifa->ifa_prefixlen = prefixlen;
    ifa->ifa_flags = flags; 
    ifa->ifa_scope = scope;
    ifa->ifa_index = index;
    nlmsg->nlmsg_len = nlmsg_len2;

    return 0;
}

int cel_nlmsg_add_rtmsg(struct nlmsghdr *nlmsg, size_t size,
                        BYTE family, 
                        BYTE dest_len, BYTE src_len, 
                        BYTE tos, BYTE table, BYTE protocol,
                        BYTE socp, BYTE type, unsigned int flags)
{
    size_t nlmsg_len1, nlmsg_len2;
    struct rtmsg *rt;

    nlmsg_len1 = NLMSG_ALIGN(nlmsg->nlmsg_len);
    nlmsg_len2 = NLMSG_ALIGN(nlmsg->nlmsg_len + sizeof(struct ifaddrmsg));
    if (nlmsg_len2 > size)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Length %d is too short, excepted %d."), size, nlmsg_len2));
        return -1;
    }
    rt = (struct rtmsg *)((char *)nlmsg + nlmsg_len1);
    rt->rtm_family = family; 
    rt->rtm_dst_len = dest_len;
    rt->rtm_src_len = src_len; 
    rt->rtm_tos = tos;
    rt->rtm_table = table;
    rt->rtm_protocol = protocol;
    rt->rtm_scope = socp;
    rt->rtm_type = type;
    rt->rtm_flags = flags;
    nlmsg->nlmsg_len = nlmsg_len2;

    return 0;
}

int cel_nlmsg_add_rtattr(struct nlmsghdr *nlmsg, size_t size, 
                         int type, const void *data, size_t data_len)
{
    size_t nlmsg_len1, nlmsg_len2, rta_len;
    struct rtattr *rta;

    nlmsg_len1 = NLMSG_ALIGN(nlmsg->nlmsg_len);
    rta_len = RTA_LENGTH(data_len);
    nlmsg_len2 = NLMSG_ALIGN(nlmsg->nlmsg_len) + rta_len;
    if (nlmsg_len2 > size)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Length %d is too short, excepted %d."), size, nlmsg_len2));
        return -1;
    }
    rta = (struct rtattr *)((char *)nlmsg + nlmsg_len1);
    rta->rta_len = rta_len; 
    rta->rta_type = type;
    memcpy(RTA_DATA(rta), data, data_len);
    nlmsg->nlmsg_len = nlmsg_len2;

    return 0;
}

int cel_netlink_send(CelNetLink *nl, struct nlmsghdr *nl_msg, 
                     unsigned long pid, unsigned long groups, int flags)
{
    struct sockaddr_nl nl_addr;
    struct iovec iov = { (void *)nl_msg, nl_msg->nlmsg_len };
    struct msghdr msg = { (void *)&nl_addr, sizeof(nl_addr), &iov, 1, NULL, 0, 0};

    memset(&nl_addr, 0, sizeof(nl_addr));
    nl_addr.nl_family = AF_NETLINK;
    nl_addr.nl_pid = pid;
    nl_addr.nl_groups = groups;

    return sendmsg(nl->fd, &msg, 0);
}

int cel_netlink_recv(CelNetLink *nl, struct nlmsghdr *nl_msg, 
                     unsigned long *pid, unsigned long *groups, int flags)
{
    int send_size;
    struct sockaddr_nl nl_addr;
    struct iovec iov = { (void*)nl_msg, nl_msg->nlmsg_len };
    struct msghdr msg = { (void*)&nl_addr, sizeof(nl_addr), &iov, 1, NULL, 0, 0};

    if ((send_size = recvmsg(nl->fd, &msg, 0)) > 0
        && msg.msg_namelen == sizeof(struct sockaddr_nl))
    {
        if (pid != NULL)
            *pid = nl_addr.nl_pid;
        if (groups != NULL)
            *groups = nl_addr.nl_groups;
    }

    return send_size;
}

#endif
