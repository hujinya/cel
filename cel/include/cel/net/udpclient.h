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
#ifndef __CEL_NET_UDPCLIENT_H__
#define __CEL_NET_UDPCLIENT_H__

#include "cel/types.h"
#include "cel/net/socket.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelUdpClient
{
    CelSocket sock;
    CelSockAddr local_addr;
}CelUdpClient;

int cel_udpclient_init(CelUdpClient *client, int addr_family);
void cel_udpclient_destroy(CelUdpClient *client);

CelUdpClient *cel_udpclient_new(int addr_family);
void cel_udpclient_free(CelUdpClient *client);

/* int cel_udpclient_bind(CelUdpClient *client, CelSockAddr *sock_addr); */
#define cel_udpclient_bind(client, sock_addr) \
    cel_socket_bind(&((client)->sock), sock_addr)
/* 
 * int cel_udpclient_bind_host(CelUdpClient *client, 
 *                             const TCHAR *host, unsigned short port);
 */
#define cel_udpclient_bind_host(client, host, port) \
    cel_socket_bind_host(&((client)->sock), host, port)

/* 
 * int cel_udpclient_sendto(CelUdpClient *client, void *buf, size_t size,
 *                          CelSockAddr *sock_addr);
 */
#define cel_udpclient_sendo(client, buf, size, sock_addr) \
    cel_udpclient_sendto(&((client)->sock), buf, size, sock_addr)
/*
 * int cel_udpclient_recvfrom(CelUdpClient *client, void *buf, size_t size,
 *                            CelSockAddr *sock_addr);
 */
#define cel_udpclient_recvfrom(client, buf, size, sock_addr) \
    cel_socket_recvfrom(&((client)->sock), buf, size, sock_addr)

#ifdef __cplusplus
}
#endif

#endif
