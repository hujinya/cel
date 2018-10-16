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
#ifndef __CEL_NET_VRRP_H__
#define __CEL_NET_VRRP_H__

#include "cel/types.h"
#include "cel/timer.h"
#include "cel/net/if.h"
#include "cel/net/socket.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CEL_VRRP_DEST_ADDR  0xE0000012  /* Multicast addr(224.0.0.18)- RFC3768.5.2.2 */
#define CEL_VRRP_IP_TTL            255  /* In and out pkt ttl - RFC3768.5.2.3 */
#define CEL_VRRP_IPPROTO           112  /* IP protocol number - RFC3768.5.2.4*/

#define CEL_VRRP_VERSION             2  /* Current version - RFC3768.5.3.1 */
#define CEL_VRRP_PKT_ADVERT          1  /* Packet type - RFC3768.5.3.2 */

#define CEL_VRRP_PRIO_OWNER        255  /* Priority of the ip owner - RFC3768.5.3.4 */
#define CEL_VRRP_PRIO_DFL          100  /* Default priority - RFC3768.5.3.4 */
#define CEL_VRRP_PRIO_STOP           0  /* Priority to stop - RFC3768.5.3.4 */

#define CEL_VRRP_ADVER_DFL           1  /* Advert. interval (in sec)- RFC3768.5.3.7 */
#define CEL_VRRP_PREEMPT_DFL         1  /* RFC3768.6.1.2.Preempt_Mode */

#define CEL_VRRP_AUTH_LEN           32
#define CEL_VRRP_VADDR_NUM           6

typedef enum _CelVrrpEvent
{
    CEL_VRRP_EVENT_SHUTDOWN = 0,
    CEL_VRRP_EVENT_STARTUP = 1
}CelVrrpEvent;

typedef enum _CelVrrpAuthType
{
    CEL_VRRP_AUTH_NONE = 0,             /* No authentification - RFC3768.5.3.6 */
    CEL_VRRP_AUTH_PASS = 1,             /* Password authentification - RFC2338.5.3.6 */
    CEL_VRRP_AUTH_AH = 2                /* AH(IPSec)authentification - RFC2338.5.3.6 */
}CelVrrpAuthType;

typedef enum _CelVrrpState
{
    CEL_VRRP_STATE_INIT = 0,            /* RFC3768.6.4.1 */
    CEL_VRRP_STATE_BACK = 1,            /* RFC3768.6.4.2 */
    CEL_VRRP_STATE_MAST = 2,            /* RFC3768.6.4.3 */
}CelVrrpState;

/*
 * 0                   1                   2                   3
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |Version| Type  | Virtual Rtr ID|   Priority    | Count IP Addrs|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Auth Type   |   Adver Int   |          Checksum             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         IP Address (1)                        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                            .                                  |
 * |                            .                                  |
 * |                            .                                  |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         IP Address (n)                        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                     Authentication Data (1)                   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                     Authentication Data (2)                   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
typedef struct _CelVrrpHdr
{
    U8 type:4;     /**< RFC3768.5.3.2 */
    U8 version:4;  /**< RFC3768.5.3.1 */
    U8 vrid;       /**< Virtual router identifier - RFC3768.5.3.3 */
    U8 priority;   /**< RFC3768.5.3.4 */
    U8 n_addr;     /**< Count ip addrs - RFC3768.5.3.5 */

    U8 auth_type;  /**< Authentication type - RFC3768.5.3.6 */
    U8 adver_int;  /**< Advertisement interval (seconds)- RFC3768.5.3.7 */
    U16 check_sum; /**< RFC3768.5.3.8 */
}CelVrrpHdr;

#define VRRP_HDR_LEN        sizeof(CelVrrpHdr)

typedef struct _CelVrrpRouter
{
    /* Router configuration */
    TCHAR if_name[CEL_IFNLEN];             /**< The device name */
    int if_index;                          /**< The device index */
    CelHrdAddr if_hrd;                     /**< Owner mac address */
    CelIpAddr if_ip;                       /**< The address of the interface */
    BYTE vrid;                    /**< Virtual router id,[1 - 255] */
    BYTE priority;                /**< Priority[1 - 254] */
    int adver_int;                         /**< Delay between advertisements(seconds) */   
    int preempt;                           /**< True if a higher prio preempt a lower one */
    CelVrrpAuthType auth_type;             /**< Authentification type */
    int auth_size;                      /**< Authentification date size(bytes) */
    TCHAR auth_data[CEL_VRRP_AUTH_LEN];    /**< Authentification data */
    int n_vaddr;                           /**< Number of ip addresses */
    struct 
    {
        CelIpAddr ip;
        BYTE prefix;
        int area;
    }vaddr[CEL_VRRP_VADDR_NUM];            /**< Virtual ip address array */
    CelHrdAddr vhrd;                       /**< Router mac address */

    /* Router state and buffer */
    CelVrrpState state, want_state;
    CelVrrpEvent recv_event;
    BYTE current_priority;
    BOOL vhrd_on;
    SOCKET recv_fd, send_fd;
    struct sockaddr send_addr;
    unsigned short ipmsg_id;
    unsigned short vrrp_len;
    int buf_size, receive_size;
    void *buf;
    /* RFC2336.6.2,Time interval for backup to declare master down */
    struct timeval down_timer; 
    struct timeval adver_timer;
}CelVrrpRouter;

extern const char *c_vrrpauthtype[];
extern const char *c_vrrpstate[];

int cel_vrrprouter_init(CelVrrpRouter *router, const TCHAR *if_name, 
                        int vrid, int priority, int adver_int, int preempt, 
                        int auth_type, const TCHAR *auth_data, 
                        const TCHAR **vaddr, int n_vaddr);
void cel_vrrprouter_destroy(CelVrrpRouter *router);
CelVrrpRouter *cel_vrrprouter_new(const TCHAR *if_name, 
                                  int vrid, int priority, 
                                  int adver_int, int preempt, 
                                  int auth_type, const TCHAR *auth_data, 
                                  const TCHAR **vaddr, int n_vaddr);
void cel_vrrprouter_free(CelVrrpRouter *router);

int cel_vrrprouter_set_multicast_local(CelVrrpRouter *router);
int cel_vrrprouter_set_unicast_local(CelVrrpRouter *router);
int cel_vrrprouter_set_unicast_peer(CelVrrpRouter *router);

#define cel_vrrprouter_authtypes(router) c_vrrpauthtype[(router)->auth_type]
#define cel_vrrprouter_states(router) c_vrrpstate[(router)->state]

#define cel_vrrprouter_startup(router) \
    (router)->recv_event = CEL_VRRP_EVENT_STARTUP;
#define cel_vrrprouter_shutdown(router) \
    (router)->recv_event = CEL_VRRP_EVENT_SHUTDOWN;

int cel_vrrprouter_check_state(CelVrrpRouter *router, 
                               CelVrrpState *state, struct timeval *now);

#ifdef __cplusplus
}
#endif

#endif
