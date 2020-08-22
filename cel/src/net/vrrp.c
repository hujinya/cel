/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2019 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/net/vrrp.h"
#include "cel/error.h"
#include "cel/allocator.h"
#include "cel/log.h"
#include "cel/net/ethernet.h"
#include "cel/net/ip.h"
#include "cel/crypto/checksum.h" /* cel_checksum() */

#define CEL_VRRP_TIMER_HZ      1000
#define CEL_VRRP_SKEW_TIME(router)     \
    ((256 - (router)->current_priority) * CEL_VRRP_TIMER_HZ / 256)
#define CEL_VRRP_DOWN_INTERVAL(router) \
    ((3 * (router)->adver_int) * CEL_VRRP_TIMER_HZ + CEL_VRRP_SKEW_TIME(router))
#define CEL_VRRP_ADVER_INTERVAL(router) ((router)->adver_int * CEL_VRRP_TIMER_HZ)

const char *c_vrrpauthtype[] = { "NONE", "PASS", "AH" };
const char *c_vrrpstate[] = { "INIT", "BACKUP", "MASTER" };

static int cel_vrrprouter_open_receive_socket(CelVrrpRouter *router)
{
    struct ip_mreq req;

    /* Open the socket */
    if ((router->recv_fd = socket(AF_INET, SOCK_RAW, CEL_VRRP_IPPROTO)) <= 0)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Open receive socket failed(%s)."), cel_geterrstr()));
        return -1;
    }
    /* Join the multicast group */
    memset(&req, 0, sizeof(req));
    req.imr_multiaddr.s_addr = htonl(CEL_VRRP_DEST_ADDR);
    req.imr_interface.s_addr = router->if_ip.s_addr;
    if (setsockopt(router->recv_fd, 
        IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&req, sizeof (struct ip_mreq)) == -1)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Join the multicast group %s failed(%s)."), 
            cel_ipaddr_ntop(&(router->if_ip)), cel_geterrstr()));
        closesocket(router->recv_fd);
        router->recv_fd = -1;
        return -1;
    }
    return 0;
}

static int cel_vrrprouter_open_send_socket(CelVrrpRouter *router)
{
    /* 0x300 is magic */
    if ((router->send_fd = socket(PF_PACKET, SOCK_PACKET, 0x300)) <= 0) 
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Open send socket failed(%s)."), cel_geterrstr()));
        return -1;
    }
    router->send_addr.sa_family = 0;
    strncpy(router->send_addr.sa_data, (char *)router->if_name, 14);

    return 0;
}

static int cel_vrrp_sendto(SOCKET fd, void *buf, size_t size, 
                           const struct sockaddr *dest_addr)
{
    int w_size;

    //printf("size = %d\r\n", size);
    w_size = sendto(fd, buf, (int)size, 0, dest_addr, sizeof(struct sockaddr));
    //printf("w_size = %d(%s)\r\n", w_size, cel_geterrstr());

    return w_size;

}

static int cel_vrrp_receive(SOCKET fd, void *buf, size_t size)
{
    fd_set nfds;
    CelTime timeout = { 0, 0 };
    int r_size = 0;

    FD_ZERO(&nfds);
    FD_SET(fd, &nfds);
    if (select((int)(fd + 1), &nfds, NULL, NULL, &timeout) > 0)
    {
        r_size = recv(fd, buf, (int)size, 0);
        //printf("r_size = %d(%s)\r\n", r_size, cel_geterrstr());
    }

    return r_size;
}

static int cel_vrrp_send_gratuitous_arp(CelVrrpRouter *router, 
                                        CelIpAddr *ip_addr, CelHrdAddr *hrd_addr)
{
    struct ether_header *eth_hdr;
    CelArpRequest *arp_req;
    size_t msg_len;

    eth_hdr = (struct ether_header *)router->buf;
    memset(eth_hdr->ether_dhost, 0xFF, ETH_ALEN);
    memcpy(eth_hdr->ether_shost, hrd_addr, sizeof(CelHrdAddr));
    eth_hdr->ether_type    = htons(ETHERTYPE_ARP);

    /* Build the arp payload */
    arp_req = (CelArpRequest *)(eth_hdr + 1);
    memset(arp_req, 0, sizeof(struct arphdr));
    arp_req->ar_hdr.ar_hrd = htons(ARPHRD_ETHER);
    arp_req->ar_hdr.ar_pro = htons(ETHERTYPE_IP);
    arp_req->ar_hdr.ar_hln = 6;
    arp_req->ar_hdr.ar_pln = 4;
    arp_req->ar_hdr.ar_op = htons(ARPOP_REQUEST);
    memcpy(arp_req->ar_sha, hrd_addr, sizeof(CelHrdAddr));
    memcpy(arp_req->ar_sip, ip_addr, sizeof(CelIpAddr));
    memset(arp_req->ar_tha, 0xFF, ETH_ALEN);
    memcpy(arp_req->ar_tip, ip_addr, sizeof(CelIpAddr));

    msg_len = ETHER_HDR_LEN + CEL_ARPREQUEST_LEN;
    return (cel_vrrp_sendto(router->send_fd, 
        router->buf, msg_len, &(router->send_addr)) == (int)msg_len ? 0 : -1);
}

static int cel_vrrp_send_advertisement(CelVrrpRouter *router, BYTE priority)
{
    struct ether_header *eth_hdr;
    struct iphdr *ip_hdr;
    int i;
    CelVrrpHdr *vrrp_hdr;
    CelIpAddr *ip_list;
    char *password;
    size_t msg_len;

    /* Ether header */
    eth_hdr = (struct ether_header *)router->buf;
    /* Destination address -- RFC1112.6.4 */
    eth_hdr->ether_dhost[0] = 0x01;
    eth_hdr->ether_dhost[1] = 0x00;
    eth_hdr->ether_dhost[2] = 0x5E;
    eth_hdr->ether_dhost[3] = ((CEL_VRRP_DEST_ADDR >> 16) & 0x7F);
    eth_hdr->ether_dhost[4] = ((CEL_VRRP_DEST_ADDR >> 8) & 0xFF);
    eth_hdr->ether_dhost[5] = (CEL_VRRP_DEST_ADDR & 0xFF);
    /* Source address -- RFC3768.7.3 */
    memcpy(eth_hdr->ether_shost, 
        (router->vhrd_on ? &(router->vhrd) : &(router->if_hrd)), sizeof(CelHrdAddr));
    /* Type */
    eth_hdr->ether_type = htons(ETHERTYPE_IP);

    /* IP header */
    ip_hdr = (struct iphdr *)(eth_hdr + 1);
    ip_hdr->ihl    = 5;     /**< 5 * 32 / 8 = 20 bytes */
    ip_hdr->version = 4; /**< IPv4 */
    ip_hdr->tos    = 0;
    ip_hdr->tot_len    = htons(ip_hdr->ihl * 4 + router->vrrp_len);
    ip_hdr->id = router->ipmsg_id++;
    //printf("id %d\r\n", ip_hdr->id);
    ip_hdr->frag_off = 0;
    ip_hdr->ttl    = CEL_VRRP_IP_TTL;
    ip_hdr->protocol = CEL_VRRP_IPPROTO;
    ip_hdr->check = 0;
    ip_hdr->saddr = *((unsigned long *)&(router->if_ip));
    ip_hdr->daddr = htonl((unsigned long)CEL_VRRP_DEST_ADDR);
    /* n_word = ip_hdr->ihl * 32 / 8 */
    ip_hdr->check = cel_checksum((u_short *)ip_hdr, ip_hdr->ihl * 4);

    /* Vrrp header */
    vrrp_hdr = (CelVrrpHdr *)(ip_hdr + 1);
    vrrp_hdr->version    = CEL_VRRP_VERSION;
    vrrp_hdr->type = CEL_VRRP_PKT_ADVERT;
    vrrp_hdr->vrid = router->vrid;
    vrrp_hdr->priority = priority;
    vrrp_hdr->n_addr = router->n_vaddr;
    vrrp_hdr->auth_type    = router->auth_type;
    vrrp_hdr->adver_int    = router->adver_int;
    /* Copy the ip addresses */
    ip_list = (CelIpAddr *)(vrrp_hdr + 1);
    for (i = 0; i < router->n_vaddr; i++ )
    {
        memcpy(&ip_list[i], &router->vaddr[i].ip, sizeof(CelIpAddr));
    }
    vrrp_hdr->check_sum = 0;
    /* Copy the passwd if the authentication is VRRP_AH_PASS */
    if (vrrp_hdr->auth_type != CEL_VRRP_AUTH_NONE && router->auth_size > 0)
    {
        password = (char *)vrrp_hdr + sizeof(*vrrp_hdr)
            + router->n_vaddr * sizeof(CelIpAddr);
        memcpy(password, router->auth_data, router->auth_size);
    }
    vrrp_hdr->check_sum = 
        cel_checksum((u_short *)vrrp_hdr, router->vrrp_len);
    /*
    printf("check_sum = %x\r\n", 
        cel_checksum((u_short *)vrrp_hdr, router->vrrp_len));
     */
    msg_len = ETHER_HDR_LEN + IP_HDR_LEN + router->vrrp_len;

    return (cel_vrrp_sendto(router->send_fd, 
        router->buf, msg_len, &(router->send_addr)) != (int)msg_len ? -1 : 0);
}

static int cel_vrrp_recieve_advertisement(CelVrrpRouter *router, 
                                          CelIpAddr *peer_ip,
                                          BYTE *peer_priority)
{
    int iphdr_len, vrrp_len;
    CelIpHdr *ip_hdr;
    CelVrrpHdr *vrrp_hdr;
    CelIpAddr ip;

    if ((router->receive_size = 
        cel_vrrp_receive(router->recv_fd, router->buf, router->buf_size)) <= 0)
        return router->receive_size;
    ip_hdr = (struct iphdr *)router->buf;
    iphdr_len = (ip_hdr->ihl << 2);
    /* MUST verify that the ip ttl is 255 */
    if (ip_hdr->ttl != CEL_VRRP_IP_TTL)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Invalid ttl %d, excepted %d, from %s."), 
            ip_hdr->ttl, CEL_VRRP_IP_TTL, cel_ipaddr_ntop(&ip)));
        return -1;
    }
    if (peer_ip != NULL)
        peer_ip->s_addr = ip_hdr->saddr; 
    ip.s_addr = ip_hdr->saddr;
    vrrp_hdr = (CelVrrpHdr *)((char *)ip_hdr + iphdr_len);
    /* MUST verify the vrrp version */
    if (vrrp_hdr->version != CEL_VRRP_VERSION)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Invalid version %d, excepted %d, from %s."), 
            vrrp_hdr->version, CEL_VRRP_VERSION, cel_ipaddr_ntop(&ip)));
        return -1;
    }
    /* 
     * MUST verify that the received packet length is greater
     * than or equal to the VRRP header 
     */
    if ((vrrp_len = (ntohs(ip_hdr->tot_len) - iphdr_len)) <= (int)sizeof(CelVrrpHdr))
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Received ip payload length %d too short,")
            _T(" excepted at least %d, from %s."),
            vrrp_len, cel_ipaddr_ntop(&ip)));
        return -1;
    }
    /*
    _tprintf(_T("Recevied advertissement,ip %s, id %d, vrid %d, ")
        _T("peer priority %d, local priority %d.\r\n"), 
        cel_ipaddr_ntop(&ip), ip_hdr->id, vrrp_hdr->vrid, 
        vrrp_hdr->priority, router->current_priority);
        */
    /* MUST verify the vrrp checksum */
    if (cel_checksum((U16 *)vrrp_hdr, vrrp_len))
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Invalid vrrp checksum, from %s."), 
            cel_ipaddr_ntop(&ip)));
        return -1;
    }
    /* MUST verify that the vrid is valid on the receiving interface */
    if (router->vrid != vrrp_hdr->vrid) 
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Received vrid mismatch, received %d, expected %d, from %s."),
            vrrp_hdr->vrid, router->vrid, cel_ipaddr_ntop(&ip)));
        return 0;
    }
    if (peer_priority != NULL)
        *peer_priority = vrrp_hdr->priority;
    /*
     * MUST verify that the adver interval in the packet is the same as
     * the locally configured for this virtual router
     */
    if (router->adver_int != vrrp_hdr->adver_int)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Received advertissement interval mismatch,")
            _T(" received:%d, expected:%d, from %s."),
            vrrp_hdr->adver_int, router->adver_int, cel_ipaddr_ntop(&ip)));
        return -1;
    }

    return router->receive_size;
}

static int cel_vrrp_add_vaddr(CelVrrpRouter *router)
{
    int i;
    /* Set the vrrp mac address -- rfc2338.7.3 */
    if (router->vhrd_on
        && cel_if_sethrdaddr(router->if_name, &(router->vhrd)) == -1)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Set hrdaddr %s to %s failed(%s)."), 
            cel_hrdaddr_notp(&(router->vhrd)), router->if_name, cel_geterrstr()));
        return -1;
    }
    /* Add the ip addresses */
    for (i = 0; i < router->n_vaddr; i++)
    {
        /* 17 = File exists */
        if (cel_if_newipaddr(router->if_index, &(router->vaddr[i].ip), NULL) == -1
            && errno != 17)
        {
            CEL_SETERR((CEL_ERR_LIB, _T("Set ipaddr %s to %s failed,%s."), 
                cel_ipaddr_ntop(&(router->vaddr[i].ip)), router->if_name, 
                cel_geterrstr()));
            return -1;
        }
        CEL_DEBUG((_T("Add virtual ip address %s to %s."), 
            cel_ipaddr_ntop(&(router->vaddr[i].ip)), router->if_name));
    }
    /* Send gratuitous arp for each virtual ip */
    for (i = 0; i < router->n_vaddr; i++)
    {
        if (cel_vrrp_send_gratuitous_arp(router, &(router->vaddr[i].ip), 
            (router->vhrd_on ? &(router->vhrd) : &(router->if_hrd))) == -1)
        {
            CEL_SETERR((CEL_ERR_LIB, _T("Send add ipaddr %s arp messager failed(%s)."), 
                cel_ipaddr_ntop(&(router->vaddr[i].ip)), cel_geterrstr()));
            return -1;
        }
        CEL_DEBUG((_T("Send gratuitous arp.")));
    }

    return 0;
}

static int cel_vrrp_remove_vaddr(CelVrrpRouter *router)
{
    int i;
    /* Restore the original MAC addresses */
    if (router->vhrd_on
        && cel_if_sethrdaddr(router->if_name, &(router->if_hrd)) == -1)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Set hrdaddr %s to %s failed(%s)."), 
            cel_hrdaddr_notp(&(router->if_hrd)), router->if_name, 
            cel_geterrstr()));
        return -1;
    }
    /* Remove the ip addresses */
    for (i = 0; i < router->n_vaddr; i++)
    {
        /* 99 = Cannot assign requested address */
        if (cel_if_delipaddr(router->if_index, &(router->vaddr[i].ip)) == -1
            && errno != 99)
        {
            //printf("errno = %d\r\n", errno);
            CEL_DEBUG((_T("Remove ipaddr %s from %s failed(%s)."), 
                cel_ipaddr_ntop(&(router->vaddr[i].ip)), router->if_name, 
                cel_geterrstr()));
            return -1;
        }
        CEL_DEBUG((_T("Delete virtual ip address %s from %s."), 
            cel_ipaddr_ntop(&(router->vaddr[i].ip)), router->if_name));
    }
    /* 
     * Send gratuitous arp for all the non-vrrp ip addresses to update
     * the cache of remote hosts using these addresses 
     */
    if (router->vhrd_on)
    {
        if (cel_vrrp_send_gratuitous_arp(
            router, &(router->if_ip), &(router->if_hrd)) == -1)
        {
            CEL_SETERR((CEL_ERR_LIB, _T("Send remove ipaddr %s arp messager failed(%s)."), 
                cel_ipaddr_ntop(&(router->if_ip)), cel_geterrstr()));
            return -1;
        }
    }

    return 0;
}

static int cel_vrrprouter_transition_state(CelVrrpRouter *router, CelTime *now)
{
    switch (router->want_state)
    {
    case CEL_VRRP_STATE_MAST:
        if (cel_vrrp_add_vaddr(router) == -1
            || cel_vrrp_send_advertisement(router, router->current_priority) == -1)
            return -1;
        /* Init Advertisement ticks */
        cel_time_set_milliseconds(&(router->adver_timer), now, CEL_VRRP_ADVER_INTERVAL(router));
        CEL_DEBUG((_T("Enter master state.")));
        router->state = CEL_VRRP_STATE_MAST;
        break;
    case CEL_VRRP_STATE_BACK:
        cel_time_set_milliseconds(&(router->down_timer), now, CEL_VRRP_DOWN_INTERVAL(router));
        cel_time_clear(&(router->adver_timer));
         /* If we goto back, warn the other routers to speed up the recovery */
        if (cel_vrrp_remove_vaddr(router) == -1)
            return -1;
        cel_vrrp_send_advertisement(router, CEL_VRRP_PRIO_STOP);
        CEL_DEBUG((_T("Enter backup state.")));
        router->state = CEL_VRRP_STATE_BACK;
        break;
    case CEL_VRRP_STATE_INIT:
        if (router->state == CEL_VRRP_STATE_MAST)
        {
            /* If we goto init, warn the other routers to speed up the recovery */
            if (cel_vrrp_remove_vaddr(router) == -1)
                return -1;
            cel_vrrp_send_advertisement(router, CEL_VRRP_PRIO_STOP);
        }
        CEL_DEBUG((_T("Enter init state.")));
        router->state = CEL_VRRP_STATE_INIT;
        break;
    default:
        CEL_SETERR((CEL_ERR_LIB, _T("Vrrp router want state %d undefined."), router->want_state));
        return -1;
    }
    return 0;
}

static int cel_vrrprouter_state_init(CelVrrpRouter *router, CelTime *now)
{
    int ret;

    if (router->recv_event == CEL_VRRP_EVENT_STARTUP)
    {
        if (router->current_priority == CEL_VRRP_PRIO_OWNER)
        {
            router->want_state = CEL_VRRP_STATE_MAST;
            cel_vrrprouter_transition_state(router, now);
        }
        else
        {
            router->want_state = CEL_VRRP_STATE_BACK;
            cel_vrrprouter_transition_state(router, now);
        }
    }
    else
    {
        do 
        {
            if ((ret = cel_vrrp_recieve_advertisement(router, NULL, NULL)) == -1)
                return -1;
        }while (router->receive_size > 0);
    }

    return 0;
}

static int cel_vrrprouter_state_master(CelVrrpRouter *router, CelTime *now)
{
    BYTE peer_priority;
    int ret;
    CelIpAddr peer_ip;

    if (router->recv_event == CEL_VRRP_EVENT_SHUTDOWN)
    {
        cel_time_clear(&(router->adver_timer));
        cel_vrrp_send_advertisement(router, CEL_VRRP_PRIO_STOP);
        router->want_state = CEL_VRRP_STATE_INIT;
        CEL_DEBUG((_T("Transition to the Init state.")));
        return cel_vrrprouter_transition_state(router, now);
    }
    /* Check expired, send advertisment */
    if (cel_time_is_expired(&(router->adver_timer), now))
    {
        CEL_DEBUG((_T("Advertisment expired, send an advertisement.")));
        if (cel_vrrp_send_advertisement(router, router->current_priority) == -1)
            return -1;
        cel_time_set_milliseconds(
            &(router->adver_timer), now, CEL_VRRP_ADVER_INTERVAL(router));
        CEL_DEBUG((_T("Reset the Adver_Timer to Advertisement_Interval.")));
    }
    /* Receive advertiement */
    do 
    {
        if ((ret = 
            cel_vrrp_recieve_advertisement(router, &peer_ip, &peer_priority)) == -1)
            return -1;
        if (ret > 0)
        {
            if (peer_priority == 0)
            {
                CEL_DEBUG((_T("Receive master stop message.")));
                if (cel_vrrp_send_advertisement(router, router->current_priority) == -1)
                    return -1;
                cel_time_set_milliseconds(
                    &(router->adver_timer), now, CEL_VRRP_ADVER_INTERVAL(router));
            }
            else 
            {
                if (peer_priority  > router->current_priority
                    || (peer_priority == router->current_priority 
                    &&  ntohl(peer_ip.s_addr) > ntohl(router->if_ip.s_addr)))
                {
                    CEL_DEBUG((_T("Receive master advertisement message, goto backup state.")));
                    cel_time_clear(&(router->adver_timer));
                    cel_time_set_milliseconds(
                        &(router->adver_timer), now, CEL_VRRP_DOWN_INTERVAL(router));
                    router->want_state = CEL_VRRP_STATE_BACK;
                }
                else
                {
                    CEL_DEBUG((_T("Discard the ADVERTISEMENT.")));
                }
            }
        }
    }while (router->receive_size > 0);
    /* State has changed */
    if (router->want_state == CEL_VRRP_STATE_BACK)
        return cel_vrrprouter_transition_state(router, now);

    return 0;
}

static int cel_vrrprouter_state_backup(CelVrrpRouter *router, CelTime *now)
{
    BYTE peer_priority;
    int ret;

    if (router->recv_event == CEL_VRRP_EVENT_SHUTDOWN)
    {
        cel_time_clear(&(router->down_timer));
        router->want_state = CEL_VRRP_STATE_INIT;
        return cel_vrrprouter_transition_state(router, now);
    }
    if (cel_time_is_expired(&(router->down_timer), now))
    {
        CEL_DEBUG((_T("Receive Master_Down_Timer expired, goto master.")));
        router->want_state = CEL_VRRP_STATE_MAST;
    }
    /* Receive advertiement */
    do
    {
        if ((ret = 
            cel_vrrp_recieve_advertisement(router, NULL, &peer_priority)) == -1)
            return -1;
        if (ret > 0)
        {
            if (peer_priority == 0)
            {
                CEL_DEBUG((_T("Set the Master_Down_Timer to Skew_Time.")));
                cel_time_set_milliseconds(
                    &(router->down_timer), now, CEL_VRRP_SKEW_TIME(router));
            }
            else 
            {
                if(!(router->preempt) || peer_priority >= router->current_priority)
                {
                    CEL_DEBUG((_T("Reset the Master_Down_Timer to Master_Down_Interval.")));
                    cel_time_set_milliseconds(
                        &(router->down_timer), now, CEL_VRRP_DOWN_INTERVAL(router));
                }
                else
                {
                    CEL_DEBUG((_T("Discard the ADVERTISEMENT.")));
                }
            }
        }
    }while (router->receive_size > 0);
    /*printf("master down timer %lld, timer %lld\r\n",
        router->down_timer, cel_vrrp_timer_clock());*/
    if (router->want_state == CEL_VRRP_STATE_MAST)
        return cel_vrrprouter_transition_state(router, now);
    
    return 0;
}

static int cel_vrrp_get_vaddr(const TCHAR *vaddr, 
                              CelIpAddr *ip, BYTE *prefix, int *area)
{
    int v = 0;
    TCHAR v_addr[CEL_HNLEN];

    _tcsncpy(v_addr, vaddr, CEL_HNLEN);
    *prefix = 32;
    *area = -1;
    while (v_addr[v] != _T('\0'))
    {
        if (v_addr[v] == _T(' '))
        {
            v_addr[v] = _T('\0');
        }
        else if (v_addr[v] == _T('/'))
        {
            *prefix = _tstoi(&v_addr[v + 1]);
            v_addr[v] = _T('\0');
        }
        else if (v_addr[v] == _T('a') && v_addr[v + 1] == _T(' '))
        {
            *area = _tstoi(&v_addr[v + 2]);
        }
        v++;
    }
    return ((cel_ipaddr_pton(v_addr, ip)  == 1) ? 0 : -1);
}

int cel_vrrprouter_init(CelVrrpRouter *router, const TCHAR *if_name, 
                        int vrid, int priority, int adver_int, int preempt, 
                        int auth_type, const TCHAR *auth_data, 
                        const TCHAR **vaddr, int n_vaddr)
{
    int i,j;

    if (if_name == NULL 
        || cel_if_getindex(if_name, &(router->if_index)) == -1
        || cel_if_gethrdaddr(if_name, &(router->if_hrd)) == -1
        || cel_if_getipaddr(if_name, &(router->if_ip)) == -1)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Bad interface %s(%s)."), if_name, cel_geterrstr()));
        return -1;
    }
    CEL_DEBUG((_T("Get ifname %s address, hrd_addr:%s, ip_addr %s."), if_name,
        cel_hrdaddr_notp(&(router->if_hrd)), cel_ipaddr_ntop(&(router->if_ip))));
    _tcsncpy(router->if_name, if_name, CEL_IFNLEN);
    if (!CEL_ISVALID(vrid, 1, 255))
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Bad virtual router id %d."), vrid));
        return -1;
    }
    router->vrid = (BYTE)vrid;
    if (!CEL_ISVALID(priority, 1, 254))
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Bad priority %d."), priority));
        return -1;
    }
    router->priority = (BYTE)priority;
    if (adver_int <= 0)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Bad advertisement interval %d."), adver_int));
        return -1;
    }
    router->adver_int = adver_int;
    router->preempt = preempt;
    router->auth_size = 0;
    if ((router->auth_type = (CelVrrpAuthType)auth_type) != CEL_VRRP_AUTH_NONE
        && (router->auth_size = (int)(_tcslen(auth_data) * sizeof(TCHAR))) > 0)
        memcpy(router->auth_data, auth_data, router->auth_size);
    router->n_vaddr = 0;
    for (i = 0, j = 0; i < n_vaddr && j < CEL_VRRP_VADDR_NUM; i++)
    {
        //_tprintf(_T("%d/%d %s\r\n"), i, n_vaddr, vaddr[i]);
        if (vaddr[i] != NULL
            && cel_vrrp_get_vaddr(vaddr[i], &(router->vaddr[j].ip),
                &(router->vaddr[j].prefix), &(router->vaddr[j].area)) == 0)
        {
            /*
            _tprintf(_T("%s/%d area %d\r\n"), 
                cel_ipaddr_ntop(&(router->vaddr[j].ip)),
                router->vaddr[j].prefix, router->vaddr[j].area);
            */
            j++;
            router->n_vaddr++;
            continue;
        }
        CEL_SETERR((CEL_ERR_LIB, _T("Bad virtual address %s."), vaddr[i]));
        return -1;
    }
    if (router->n_vaddr == 0)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("No virtual address.")));
        return -1;
    }
    /* Virtual Router MAC Address RFC 3768.7.3 */
    router->vhrd[0] = 0x00;
    router->vhrd[1] = 0x00;
    router->vhrd[2] = 0x5E;
    router->vhrd[3] = 0x00;
    router->vhrd[4] = 0x01;
    router->vhrd[5] = router->vrid;

    router->state = CEL_VRRP_STATE_INIT;
    router->want_state = CEL_VRRP_STATE_INIT;
    router->recv_event = CEL_VRRP_EVENT_SHUTDOWN;
    router->current_priority = router->priority;
    router->vhrd_on = 0;
    if (cel_vrrprouter_open_receive_socket(router) == -1
        || cel_vrrprouter_open_send_socket(router) == -1)
        return -1;
    router->ipmsg_id = 0;
    router->vrrp_len = (unsigned short)(VRRP_HDR_LEN 
        + router->n_vaddr * sizeof(CelIpAddr) + router->auth_size);
    //printf("%ld %ld \r\n", router->n_vaddr, router->auth_size);
    router->buf_size =  ETHER_HDR_LEN + IP_HDR_LEN + router->vrrp_len;
    if ((router->buf = cel_malloc(router->buf_size)) == NULL)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Malloc buffer size %d failed(%s)."), 
            router->buf_size, cel_geterrstr()));
        return -1;
    }

    return 0;
}

void cel_vrrprouter_destroy(CelVrrpRouter *router)
{
    if (router->state == CEL_VRRP_STATE_MAST)
    {
        router->want_state = CEL_VRRP_STATE_BACK;
        cel_vrrprouter_transition_state(router, NULL);
    }
    closesocket(router->recv_fd);
    closesocket(router->send_fd);
    if (router->buf != NULL)
        cel_free(router->buf);
    router->buf_size = 0;
}

CelVrrpRouter *cel_vrrprouter_new(const TCHAR *if_name, 
                                  int vrid, int priority, int adver_int, int preempt, 
                                  int auth_type, const TCHAR *auth_data, 
                                  const TCHAR **vaddr, int n_vaddr)
{
    CelVrrpRouter *router;

    if ((router = (CelVrrpRouter *)cel_malloc(sizeof(CelVrrpRouter))) != NULL)
    {
        if (cel_vrrprouter_init(router, if_name, vrid, priority, adver_int, preempt, 
            auth_type, auth_data, vaddr, n_vaddr) != -1)
            return router;
        cel_free(router);
    }

    return NULL;
}

void cel_vrrprouter_free(CelVrrpRouter *router)
{
    cel_vrrprouter_destroy(router);
    cel_free(router);
}

int cel_vrrprouter_check_state(CelVrrpRouter *router, 
                               CelVrrpState *state, CelTime *now)
{
    int ret = -1;

    CEL_TIME_NOW(now);
    switch (router->state)
    {
    case CEL_VRRP_STATE_MAST:
        ret = cel_vrrprouter_state_master(router, now);
        break;
    case CEL_VRRP_STATE_BACK:
        ret = cel_vrrprouter_state_backup(router, now);
        break;
    case CEL_VRRP_STATE_INIT:
        ret = cel_vrrprouter_state_init(router, now);
        break;
    default:
        CEL_SETERR((CEL_ERR_LIB, _T("Vrrp router state %d undefined."), router->state));
        ret = -1;
    }
    if (state != NULL)
        *state = router->state;

    return ret;
}
