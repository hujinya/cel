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
#include "cel/net/monitor.h"
#include "cel/log.h"
#include "cel/error.h"
#include "cel/allocator.h"

typedef enum _CelCheckerState
{
    CEL_CHECKER_DISCONNECTED,
    CEL_CHECKER_CONNECTING,
    CEL_CHECKER_CONNECTED,
    CEL_CHECKER_SENDING,
    CEL_CHECKER_RECVING,
    CEL_CHECKER_OK
}CelCheckerState;

typedef struct _CelMonitorTcpChecker
{
    CelSocketAsyncArgs args;
    CelAsyncBuf async_buf;
    char buf[1024];
    size_t offset, size;
    CelCheckerState state;
    CelSocket sock;
    CelSockAddr addr;
    CelMonitor *monitor;
}CelMonitorTcpChecker;

static int cel_monitor_tcp_init(CelMonitor *mntr);
static void cel_monitor_tcp_destroy(CelMonitor *mntr);
static void cel_monitor_tcp_timeout(CelMonitor *mntr);
static void cel_monitor_tcp_check(void *checker);

//static int cel_monitor_udp_init(CelMonitor *mntr);
//static void cel_monitor_udp_destroy(CelMonitor *mntr);
//static void cel_monitor_udp_timeout(CelMonitor *mntr);
//static void cel_monitor_udp_check(CelMonitor *mntr_tcp);

CelKeyword mntr_types[] = 
{
    { sizeof(_T("http")) - 1, _T("http")},
    { sizeof(_T("https")) - 1, _T("https")},
    { sizeof(_T("ping")) - 1, _T("ping")},
    { sizeof(_T("tcp")) - 1, _T("tcp")},  
    { sizeof(_T("udp")) - 1, _T("udp")},
    { sizeof(_T("wmip")) - 1, _T("wmip")}
};

static CelMonitorCheckerClass mntr_class[] = 
{
    { NULL, NULL, NULL, NULL },

    { NULL, NULL, NULL, NULL},

    { NULL, NULL, NULL, NULL },

    { cel_monitor_tcp_init, cel_monitor_tcp_destroy, 
    cel_monitor_tcp_timeout, cel_monitor_tcp_check },

    { NULL, NULL, NULL, NULL },

    { NULL, NULL, NULL, NULL }
};

void _cel_monitorcontext_destroy_derefed(CelMonitorContext *mntr_ctx)
{
    if (mntr_ctx->send != NULL)
    {
        cel_free(mntr_ctx->send);
        mntr_ctx->send = NULL;
    }
    if (mntr_ctx->expect != NULL)
    {
        cel_free(mntr_ctx->expect);
        mntr_ctx->expect = NULL;
    }
}

int cel_monitorcontext_init(CelMonitorContext *mntr_ctx, 
                            const TCHAR *name, CelMonitorType type)
{
    size_t len;

    if (type == CEL_MONITOR_UNDEFINED)
    {
        CEL_SETERR((CEL_ERR_LIB,  _T("Monitor type undefined.")));
        return -1;
    }
    len = _tcslen(name);
    memcpy(mntr_ctx->name, name, len * sizeof(TCHAR));
    mntr_ctx->name[len] = _T('\0');
    mntr_ctx->type = type;
    mntr_ctx->is_keepalive = TRUE;
    mntr_ctx->timeout = 6;
    mntr_ctx->interval = 3;
    mntr_ctx->down_threshold = 2;
    mntr_ctx->up_threshold = 1;
    mntr_ctx->send = NULL;
    mntr_ctx->expect = NULL;
    mntr_ctx->checker_class = &mntr_class[type];
    cel_refcounted_init(&(mntr_ctx->ref_counted),
        (CelFreeFunc)_cel_monitorcontext_destroy_derefed);
    return 0;
}

void cel_monitorcontext_destroy(CelMonitorContext *mntr_ctx)
{
    cel_refcounted_destroy(&(mntr_ctx->ref_counted), mntr_ctx);
}

void _cel_monitorcontext_free_derefed(CelMonitorContext *mntr_ctx)
{
    _cel_monitorcontext_destroy_derefed(mntr_ctx);
    cel_free(mntr_ctx);
}

CelMonitorContext *cel_monitorcontext_new(const TCHAR *name, CelMonitorType type)
{
    CelMonitorContext *mntr_ctx;

    if ((mntr_ctx = 
        (CelMonitorContext *)cel_malloc(sizeof(CelMonitorContext))) != NULL)
    {
        if (cel_monitorcontext_init(mntr_ctx, name, type) == 0)
        {
            cel_refcounted_init(&(mntr_ctx->ref_counted),
                (CelFreeFunc)_cel_monitorcontext_free_derefed);
            return mntr_ctx;
        }
        cel_free(mntr_ctx);
    }

    return NULL;
}

void cel_monitorcontext_free(CelMonitorContext *mntr_ctx)
{
    cel_refcounted_destroy(&(mntr_ctx->ref_counted), mntr_ctx);
}

void cel_monitor_tcp_check(void *checker)
{
    CelMonitorTcpChecker *mntr_tcp = (CelMonitorTcpChecker *)checker;

    switch (mntr_tcp->state)
    {
    case CEL_CHECKER_CONNECTING:
        break;
    case CEL_CHECKER_CONNECTED:
        break;
    case CEL_CHECKER_SENDING:
        break;
    case CEL_CHECKER_RECVING:
        break;
    case CEL_CHECKER_OK:
    default:
        break;
    }
}

int cel_monitor_tcp_init(CelMonitor *mntr)
{
    //CelMonitorTcpChecker *mntr_tcp;
    //CelSocketAsyncArgs *args;

    //if ((mntr_tcp = (CelMonitorTcpChecker *)
    //    cel_malloc(sizeof(CelMonitorTcpChecker))) != NULL)
    //{
    //    mntr_tcp->state = CEL_CHECKER_CONNECTING;
    //    if (cel_socket_init(&mntr_tcp->sock, AF_INET, SOCK_STREAM, IPPROTO_TCP) == 0)
    //    {
    //        if (cel_socket_set_reuseaddr(&mntr_tcp->sock, 1) == 0
    //            && cel_socket_set_nonblock(&mntr_tcp->sock, 1) == 0
    //            && cel_socket_set_keepalive(&mntr_tcp->sock, 1, 360, 6, 3) == 0
    //            && cel_eventloop_add_channel(
    //            mntr->evt_loop, &(mntr_tcp->sock.channel), NULL) == 0)
    //        {
    //            args = &(mntr_tcp->args);
    //            memset(&(args->connect_args.ol), 0, sizeof(sizeof(CelOverLapped)));
    //            args->connect_args.async_callback = 
    //                (void (*) (void *))cel_monitor_tcp_check;
    //            args->connect_args.socket = &(mntr_tcp->sock);
    //            //memset(args->connect_args.host, 0, CEL_HNLEN);
    //            //memset(args->connect_args.service, 0, CEL_NPLEN);
    //            _tcscpy(args->connect_args.host, _T("192.168.0.177"));
    //            _tcscpy(args->connect_args.service, _T("9005"));
    //            args->connect_args.buffer = NULL;
    //            args->connect_args.buffer_size = 0;
    //            if (cel_socket_async_connect_host(&(args->connect_args)) == 0)
    //            {
    //                CEL_DEBUG((_T("connect post ok\r\n")));
    //                mntr->checker = mntr_tcp;
    //                return 0;
    //            }
    //            CEL_SETERR((CEL_ERR_LIB,  _T("Load balancer %s post connect failed(%s)."), 
    //                cel_sockaddr_ntop(&(mntr_tcp->addr)), cel_geterrstr(cel_sys_geterrno())));
    //        }
    //        cel_socket_destroy(&mntr_tcp->sock);
    //    }
    //    cel_free(mntr_tcp);
    //}
    //mntr_tcp->state = CEL_CHECKER_DISCONNECTED;

    return -1;
}

void cel_monitor_tcp_destroy(CelMonitor *mntr)
{
    CelMonitorTcpChecker *mntr_tcp;

    if ((mntr_tcp = (CelMonitorTcpChecker *)mntr->checker) != NULL)
    {
        mntr->checker = NULL;
        cel_socket_destroy(&mntr_tcp->sock);
        cel_free(mntr_tcp);
    }
}

void cel_monitor_tcp_timeout(CelMonitor *mntr)
{
    BOOL active;
    CelMonitorTcpChecker *mntr_tcp;

    if ((mntr_tcp = (CelMonitorTcpChecker *)mntr->checker) == NULL
        || cel_monitor_tcp_init(mntr) == -1)
    {
        return ;
    }
    if (mntr_tcp->state == CEL_CHECKER_OK)
    {
        if ((++mntr->up_cnt) >= mntr->ctx->up_threshold)
        {
            mntr->down_cnt = 0;
            active = TRUE;
        }
    }
    else
    {
        if ((++mntr->down_cnt) >= mntr->ctx->down_threshold)
        {
            mntr->up_cnt = 0;
            active = FALSE;
            if (mntr_tcp->state != CEL_CHECKER_DISCONNECTED)
                cel_socket_shutdown(&(mntr_tcp->sock), SHUT_RDWR);
        }
    }
    if (active != mntr->active)
    {
        mntr->active = active;
        CEL_DEBUG((_T("Monitor %s_%s %s."), 
            mntr->addr, mntr->ctx->name, mntr->active ? _T("up"): _T("down")));
    }
    if (mntr_tcp->state == CEL_CHECKER_DISCONNECTED)
    {
        //if (mntr->ref_counted == -1)
        //{
        //    cel_eventloop_cancel_timer(&evt_loop, mntr->timer_id);
        //    cel_monitor_tcp_destroy(mntr_tcp);
        //    cel_monitor_deref(mntr);
        //    return;
        //}
        cel_monitor_tcp_check(mntr);
    }
    else if (mntr_tcp->state == CEL_CHECKER_OK)
    {
        //if (monitor->ref_counted == -1)
        //{
        //    cel_eventloop_cancel_timer(&evt_loop, mntr->timer_id);
        //    cel_monitor_tcp_destroy(mntr_tcp);
        //    cel_monitor_deref(mntr);
        //    return;
        //}
        mntr_tcp->state = CEL_CHECKER_CONNECTED;
        cel_monitor_tcp_check(mntr);
    }
}

void _cel_monitor_destroy_derefed(CelMonitor *monitor)
{
    cel_monitorcontext_deref(monitor->ctx);
    monitor->ctx = NULL;
}

int cel_monitor_init(CelMonitor *monitor, 
                     CelMonitorContext *mntr_ctx, const TCHAR *address)
{
    monitor->active = FALSE;
    monitor->down_cnt = monitor->up_cnt = 0;
    monitor->timer_id = 0;
    if ((monitor->ctx = cel_monitorcontext_ref(mntr_ctx)) == NULL)
        return -1;
    monitor->checker = NULL;
    cel_refcounted_init(
        &(monitor->ref_counted), (CelFreeFunc)_cel_monitor_destroy_derefed);
    return 0;
}

void cel_monitor_destroy(CelMonitor *monitor)
{
    cel_refcounted_destroy(&(monitor->ref_counted), monitor);
}

void _cel_monitor_free_derefed(CelMonitor *monitor)
{
    _cel_monitor_destroy_derefed(monitor);
    cel_free(monitor);
}

CelMonitor *cel_monitor_new(CelMonitorContext *mntr_ctx, const TCHAR *address)
{
    CelMonitor *monitor;

    if ((monitor = (CelMonitor *)cel_malloc(sizeof(CelMonitor))) != NULL)
    {
        if (cel_monitor_init(monitor, mntr_ctx, address) == 0)
        {
            cel_refcounted_init(&(monitor->ref_counted), 
                (CelFreeFunc)_cel_monitor_free_derefed);
            return monitor;
        }
        cel_free(monitor);
    }

    return NULL;
}

void cel_monitor_free(CelMonitor *monitor)
{
    cel_refcounted_destroy(&(monitor->ref_counted), monitor);
}

void cel_monitor_check_async(CelMonitor *monitor, CelEventLoop *evt_loop)
{
    CelMonitorContext *mntr_ctx = monitor->ctx;
    CelMonitorCheckerClass *checker_class = mntr_ctx->checker_class;

    if (monitor->timer_id == 0)
    {
        cel_monitor_ref(monitor);
        monitor->timer_id = cel_eventloop_schedule_timer(
            evt_loop, mntr_ctx->interval, 1, 
            (CelTimerCallbackFunc)checker_class->checker_check, monitor);
    }
}
