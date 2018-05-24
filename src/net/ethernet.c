/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2016 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/net/ethernet.h"

int cel_ethernet_build_arp_request(struct ether_header *eth_hdr, size_t size,
                                   CelIpAddr *ip_addr, CelHrdAddr *hrd_addr)
{
    CelArpRequest *arp_req;

    if ((ETHER_HDR_LEN + CEL_ARPREQUEST_LEN) > size)
        return -1;
    memset(eth_hdr->ether_dhost, 0xFF, ETH_ALEN);
    memcpy(eth_hdr->ether_shost, hrd_addr, ETH_ALEN);
    eth_hdr->ether_type    = htons(ETHERTYPE_ARP);

    /* Build the arp payload */
    arp_req = (CelArpRequest *)(eth_hdr + 1);
    memset(arp_req, 0, sizeof(struct arphdr));
    arp_req->ar_hdr.ar_hrd = htons(ARPHRD_ETHER);
    arp_req->ar_hdr.ar_pro = htons(ETHERTYPE_IP);
    arp_req->ar_hdr.ar_hln = ETH_ALEN;
    arp_req->ar_hdr.ar_pln = ETH_PLEN;
    arp_req->ar_hdr.ar_op = htons(ARPOP_REQUEST);
    memcpy(arp_req->ar_sha, hrd_addr, ETH_ALEN);
    memcpy(arp_req->ar_sip, ip_addr, ETH_PLEN);
    memset(arp_req->ar_tha, 0xFF, ETH_ALEN);
    memcpy(arp_req->ar_tip, ip_addr, ETH_PLEN);

    return (ETHER_HDR_LEN + CEL_ARPREQUEST_LEN);
}
