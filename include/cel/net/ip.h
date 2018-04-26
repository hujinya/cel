/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_NET_IP_H__
#define __CEL_NET_IP_H__

#include "cel/config.h"
#if defined(_CEL_UNIX)
#include "cel/_unix/_ip.h"
#elif defined(_CEL_WIN)
#include "cel/_win/_ip.h"
#endif

typedef struct _CelIpHdrEx
{
    CelIpHdr hdr;
    U32 options;
}CelIpHdrEx;

#define IP_HDR_LEN   sizeof(CelIpHdr)
#define IP_HDREX_LEN sizeof(CelIpHdrEx)

#endif
