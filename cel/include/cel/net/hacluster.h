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
#ifndef __CEL_NET_HACLUSTER_H__
#define __CEL_NET_HACLUSTER_H__

#include "cel/types.h"
#include "cel/refcounted.h"
#include "cel/timer.h"
#include "cel/net/if.h"
#include "cel/net/socket.h"


#ifdef __cplusplus
extern "C" {
#endif

#define CEL_HA_VERSION                  1  /* High Availability protocol version */
#define CEL_HA_IPPROTO                112  /* IP protocol number */
#define CEL_HA_MULTICAST_ADDR  0xE0000012  /* Multicast addr(224.0.0.18) */
#define CEL_HA_MULTICAST_TTL          255  /* Multicast ttl */
#define CEL_HA_UNICAST_PORT          9092  /* Unicast port */
#define CEL_HA_UNICAST_ADDR_NUM         8  /* Unicast support max address */
#define CEL_HA_PRIORITY_RESIGN          0  /* Priority to resign */
#define CEL_HA_PRIORITY_COUP          255  /* Priority coup */

#define CEL_HA_DEVICE_NUM             254  /* Max devices */
#define CEL_HA_MEMBER_NUM             254  /* Group support max members [1-254] */
#define CEL_HA_VADDR_NUM              255  /* Group support max virtual address */
#define CEL_HA_GROUP_NUM              255  /* Max groups [1-255] */

typedef enum _CelHaState
{
    CEL_HA_STATE_INIT,
    CEL_HA_STATE_ACTIVE,
    CEL_HA_STATE_STANDYBY
}CelHaState;

typedef enum _CelHaEvent
{
    CEL_HA_EVENT_STANDYBY_EXPIRED,
    CEL_HA_EVENT_ACTIVE_EXPIRED,
    CEL_HA_EVENT_FORCE_TO_STANDYBY = 0x070,
    CEL_HA_EVENT_STARTUP = 0x000000FF,
    CEL_HA_EVENT_SHUTDOWN = 0xFFFFFF00
}CelHaEvent;

/*
 * 0                   1                   2                   3
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |Version| Type  |Device Name Len|    Device Security Code       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Count Groups |   Adver Int   |          Checksum             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | Group ID(1)   |     Prio      |  Current Prio |      State    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      ......                                   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | Group ID(n)   |     Prio      |  Current Prio |      State    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      Device Name data                         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
typedef struct _CelHaDeviceMsg
{
    U8 type:4;   
    U8 version:4;
    U8 device_nl;
    U16 device_security;
    U8 n_group;
    U8 adver_int;
    U16 check_sum;
}CelHaDeviceMsg;

typedef struct _CelHaGroupMsg
{
    U8 group_id;
    U8 priority;
    U8 current_priority;
    U8 state;
}CelHaGroupMsg;

typedef struct _CelHaDevice
{
    TCHAR name[CEL_HNLEN];
    BOOL unicast_enabled;      /**< Advertisment unicast enabled */
    int n_unicast_addrs;
    CelIpAddr *unicast_addrs[CEL_HA_UNICAST_ADDR_NUM];
    unsigned short port;
    //struct timeval down_timer;
    CelRefCounted ref_counted;
}CelHaDevice;

typedef struct _CelHaMember
{
    CelHaState state, want_state;
    BYTE priority;
    TCHAR dev_name[CEL_HNLEN];
    CelHaDevice *dev;
    CelIpAddr ip;
    union {
        struct timeval coup_timer;
        struct timeval down_timer;
    };
}CelHaMember;

typedef struct _CelVirtualAddress
{
    TCHAR if_name[CEL_IFNLEN];
    int if_index;
    CelHrdAddr if_hrd;
    CelIpAddr if_ip;

    CelIpAddr ip;
    BYTE prefix;
    int area;
    TCHAR label[CEL_IFNLEN];
}CelVirtualAddress;

typedef struct _CelHaGroup
{
    BYTE id;                   /**< Ha group id,[1 - 255] */
    int adver_int;             /**< Delay between advertisements(milliseconds) */   
    int preempt;               /**< True if a higher priority preempt a lower one */
    TCHAR if_name[CEL_IFNLEN]; /**< The bind interface name */
    int n_vaddrs;              /**< Number of virtual addresses */ 
    CelVirtualAddress *vaddrs[CEL_HA_VADDR_NUM];
                               /**< Virtual address array */
    TCHAR *active_action;
    TCHAR *standyby_action;
    BOOL vhrd_on;
    CelHrdAddr vhrd;           /**< Virtual mac address[00:00:5E:00:01:<group_id>] */
    int n_members;
    CelHaMember *members[CEL_HA_MEMBER_NUM];
                               /**< Group members, priority[1 - 254] */
    CelHaMember *self, *active, *next;
    BOOL is_update;
    CelHaEvent evt;
    struct timeval adver_timer;
}CelHaGroup;

typedef struct _CelHaCluster
{
    BOOL multicast, unicast;
    BYTE ttl;
    TCHAR multicast_if[CEL_IFNLEN];
    CelIpAddr multicast_src;
    CelIpAddr multicast_addr;   /**< Advertisment multicast address */
    unsigned short unicast_port;
    SOCKET multicast_fd, unicast_fd;
    CelHaDevice *self;
    CelHaDevice *ha_devices[CEL_HA_DEVICE_NUM];
    CelHaGroup *ha_grps[CEL_HA_GROUP_NUM]; 
}CelHaCluster;

extern const TCHAR *c_hastatestr[];

int cel_hadevice_init(CelHaDevice *dev);
void cel_hadevice_destroy(CelHaDevice *dev);

CelHaDevice *cel_hadevice_new(void);
void cel_hadevice_free(CelHaDevice *dev);

//int cel_hacluster_init(CelHaCluster *cluster, CelHaDevice *self_dev);
//void cel_hacluster_destroy(CelHaCluster *cluster);
//
//CelHaCluster *cel_hacluster_new(CelHaDevice *self_dev);
//void cel_hacluster_free(CelHaCluster *cluster);

void cel_hacluster_unicast_enable(BOOL enable);
void cel_hacluster_unicast_port_set(unsigned short port);
void cel_hacluster_multicast_enable(BOOL enable);
void cel_hacluster_multicast_src_set(CelIpAddr *src);
//void cel_hacluster_multicast_if_set(const TCHAR *ifname);
//void cel_hacluster_multicast_addr_set(CelIpAddr *addr);
//void cel_hacluster_multicast_ttl_set(BYTE ttl);

//void cel_hacluster_advertisment_interval_set(unsigned int seconds);

//int cel_hacluster_multicast_set(CelHaCluster *hac, 
//                                BOOL enable, const TCHAR *ifname, 
//                                const TCHAR *addr, BYTE ttl);
//int cel_hacluster_device_add(CelHaCluster *hac, 
//                             const TCHAR *name, BOOL self,
//                             BOOL unicast_enable, unsigned short unicast_port,
//                             TCHAR **unicast_addrs, int n_unicast_addrs);
//int cel_hacluster_device_delete(CelHaCluster *hac, const TCHAR *name);
//void cel_hacluster_device_clear(CelHaCluster *hac);
//
//int cel_hacluster_group_add(CelHaCluster *hac, 
//                            BYTE id, int preempt, 
//                            const TCHAR *if_name, TCHAR **vaddrs, int n_vaddrs,
//                            TCHAR **members, int n_members);
//int cel_hacluster_group_delete(CelHaCluster *hac, BYTE id);
//void cel_hacluster_group_clear(CelHaCluster *hac);
//
//int cel_hacluster_group_startup(CelHaCluster *hac, BYTE id);
//int cel_hacluster_group_shutdown(CelHaCluster *hac, BYTE id);
//int cel_hacluster_group_force_to_standyby(CelHaCluster *hac, BYTE id);
//int cel_hacluster_group_check_state(CelHaCluster *hac,BYTE id, 
//                                    CelHaState *state, struct timeval *now);

int cel_hagroup_init(CelHaGroup *ha_grp, int id, int preempt,
                     TCHAR *if_name,
                     TCHAR **vaddrs, int n_vaddrs,
                     TCHAR *active_action, TCHAR *standyby_action,
                     TCHAR **members, int n_members);
void cel_hagroup_destroy(CelHaGroup *ha_grp);
CelHaGroup *cel_hagroup_new(int id, int preempt, TCHAR *if_name, 
                            TCHAR **vaddrs, int n_vaddrs,
                            TCHAR *active_action, TCHAR *standyby_action,
                            TCHAR **members, int n_members);
void cel_hagroup_free(CelHaGroup *ha_grp);

int cel_hagroup_reload(CelHaGroup *ha_grp, int preempt, TCHAR *if_name,
                       TCHAR **vaddrs, int n_vaddrs,
                       TCHAR *active_action, TCHAR *standyby_action,
                       TCHAR **members, int n_members);

#define cel_hagroup_states(ha_grp) c_hastatestr[(ha_grp)->self->state]

#define cel_hagroup_startup(ha_grp) \
    CEL_CLRFLAG((ha_grp)->evt, CEL_HA_EVENT_SHUTDOWN)
#define cel_hagroup_shutdown(ha_grp) \
    CEL_SETFLAG((ha_grp)->evt, CEL_HA_EVENT_SHUTDOWN)
#define cel_hagroup_force_to_standyby(ha_grp) \
    CEL_SETFLAG((ha_grp)->evt, CEL_HA_EVENT_FORCE_TO_STANDYBY)

int cel_hagroup_check_state(CelHaGroup *ha_grp, 
                            CelHaState *state, struct timeval *now);

#ifdef __cplusplus
}
#endif

#endif
