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
#ifndef __CEL_NET_NETLINK_UNIX_H__
#define __CEL_NET_NETLINK_UNIX_H__

#include "cel/types.h"
#include "cel/net/socket.h"
#include <arpa/inet.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelNetLink
{
    SOCKET fd;
    struct sockaddr_nl local_addr;
    U32 seq;
}CelNetLink;

int cel_nlmsg_build(struct nlmsghdr *nlmsg, size_t size,
                    int type, int flags, unsigned long seq, unsigned long pid);
int cel_nlmsg_add_ifinfomsg(struct nlmsghdr *nlmsg, size_t size, 
                            BYTE family, unsigned short type, int index, 
                            unsigned int flags, unsigned int change);
int cel_nlmsg_add_ifaddrmsg(struct nlmsghdr *nlmsg, size_t size,
                            BYTE family, BYTE prefixlen, 
                            BYTE flags, BYTE scope, 
                            unsigned int index);
int cel_nlmsg_add_rtmsg(struct nlmsghdr *nlmsg, size_t size,
                        BYTE family, 
                        BYTE dest_len, BYTE src_len, 
                        BYTE tos, BYTE table, BYTE protocol,
                        BYTE socp, BYTE type, unsigned int flags);
int cel_nlmsg_add_rtattr(struct nlmsghdr *nlmsg, size_t size, 
                         int type, const void *data, size_t data_len);

int cel_netlink_init(CelNetLink *nl, int protocol, 
                     unsigned long pid, unsigned long groups);
void cel_netlink_destroy(CelNetLink *nl);
CelNetLink *cel_netlink_new(int protocol, 
                            unsigned long pid, unsigned long groups);
void cel_netlink_free(CelNetLink *nl);

int cel_netlink_send(CelNetLink *nl, struct nlmsghdr *nl_msg, 
                     unsigned long pid, unsigned long groups, int flags);
int cel_netlink_recv(CelNetLink *nl, struct nlmsghdr *nl_msg, 
                     unsigned long *pid, unsigned long *groups, int flags);

#ifdef __cplusplus
}
#endif

#endif
