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
#ifndef __CEL_NET_ETHERNET_H__
#define __CEL_NET_ETHERNET_H__

#include "cel/types.h"
#include "cel/net/if.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 ******************************************************************************
 * Ether header
 ******************************************************************************
 */
#define ETH_PLEN             4
#ifdef _CEL_UNIX
#include <net/ethernet.h>
typedef struct ether_header CelEtherHdr;
#endif
#ifdef _CEL_WIN

#define ETH_ALEN             6

#define ETHERTYPE_IP    0x0800 
#define ETHERTYPE_ARP   0x0806
#define ETHERTYPE_RARP  0x8035
#define ETHERTYPE_IPV6  0x86DD

typedef struct ether_header
{
    U8 ether_dhost[ETH_ALEN];
    U8 ether_shost[ETH_ALEN];
    U16 ether_type; 
}CelEtherHdr;

#define ETHER_HDR_LEN       14
#endif
/*
 ******************************************************************************
 * Arp header and request
 ******************************************************************************
 */
#define ARPHRD_ETHER         1   /* Ethernet hardware format */
#define ARPHRD_IEEE802       6   /* Token Ring hardware format */
#define ARPHRD_FRELAY       15   /* Frame Relay hardware format */

#define ARPOP_REQUEST        1   /* Request to resolve the ip address */
#define ARPOP_REPLY          2   /* The response to a request */
#define ARPOP_REVREQUEST     3   /* Reverse request */
#define ARPOP_REVREPLY       4   /* Response to a reserve request */
#define ARPOP_INVREQUEST     8   /* Request to identify peer */
#define ARPOP_INVREPLY       9   /* Response identifying peer  */

#ifdef _CEL_UNIX
#include <net/if_arp.h>
typedef struct arphdr CelArpHdr;
#endif
#ifdef _CEL_WIN
typedef struct arphdr
{
    U16 ar_hrd;                  /**< Format of hardware address.  */
    U16 ar_pro;                  /**< Format of protocol address.  */
    U8 ar_hln;                   /**< Length of hardware address.  */
    U8 ar_pln;                   /**< Length of protocol address.  */
    U16 ar_op;                   /**< Arp opcode (command).  */
}CelArpHdr;
#endif

typedef struct _CelArpRequest
{
    CelArpHdr ar_hdr;
    U8 ar_sha[ETH_ALEN];         /**< Sender hardware address.  */
    U8 ar_sip[ETH_PLEN];         /**< Sender IP address.  */
    U8 ar_tha[ETH_ALEN];         /**< Target hardware address.  */
    U8 ar_tip[ETH_PLEN];         /**< Target IP address.  */
}CelArpRequest;

#define CEL_ARPREQUEST_LEN    sizeof(CelArpRequest)  

int cel_ethernet_build_arp_request(struct ether_header *eth_hdr, size_t size,
                                   CelIpAddr *ip_addr, CelHrdAddr *hrd_addr);
int cel_ethernet_send_arp();

#ifdef __cplusplus
}
#endif

#endif
