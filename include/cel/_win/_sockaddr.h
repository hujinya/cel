/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_NET_SOCKADDR_WIN_H__
#define __CEL_NET_SOCKADDR_WIN_H__

#include "cel/types.h"
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#ifdef __cplusplus
extern "C" {
#endif

typedef union _CelSockAddr
{
    ADDRESS_FAMILY sa_family;
    struct sockaddr addr;
    struct sockaddr_in addr_in;
    struct sockaddr_in6 addr_in6;
}CelSockAddr;

/* Socket address len table */
extern const int c_addrlen[];

int sethostname(const char *name, size_t len);

#ifdef __cplusplus
}
#endif


#endif