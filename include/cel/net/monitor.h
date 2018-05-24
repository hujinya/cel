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
#ifndef __CEL_NET_MONITOR_H__
#define __CEL_NET_MONITOR_H__

#include "cel/types.h"
#include "cel/refcounted.h"
#include "cel/keyword.h"
#include "cel/eventloop.h"
#include "cel/net/socket.h"
#include "cel/net/udpclient.h"
#include "cel/net/tcpclient.h"
#include "cel/net/httpclient.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelMonitor CelMonitor;

typedef enum _CelMonitorType
{
    CEL_MONITOR_UNDEFINED = -1,
    CEL_MONITOR_HTTP,
    CEL_MONITOR_HTTPS,
    CEL_MONITOR_PING,
    CEL_MONITOR_TCP,
    CEL_MONITOR_UDP,
    CEL_MONITOR_WMIP,
    CEL_MONITOR_COUNT
}CelMonitorType;

typedef int (* CelMonitorCheckerInit) (CelMonitor *monitor);
typedef void (* CelMonitorCheckerDestroy) (CelMonitor *monitor);
typedef void (* CelMonitorCheckerTimeout) (CelMonitor *monitor);
typedef void (* CelMonitorCheckerCheck) (void *monitor_checker);

typedef struct _CelMonitorCheckerClass
{
    CelMonitorCheckerInit checker_init;
    CelMonitorCheckerDestroy checker_destroy;
    CelMonitorCheckerTimeout checker_timeout;
    CelMonitorCheckerCheck checker_check;
}CelMonitorCheckerClass;

typedef struct _CelMonitorContext
{
    TCHAR name[CEL_KNLEN];
    CelMonitorType type;
    TCHAR addr[CEL_ADDRLEN];
    BOOL is_keepalive;
    int timeout, interval;
    int up_threshold, down_threshold;
    CelRefCounted ref_counted;
    char *send, *expect;
    CelMonitorCheckerClass *checker_class;
}CelMonitorContext;

struct _CelMonitor
{
    TCHAR addr[CEL_ADDRLEN];
    BOOL active;
    int up_cnt, down_cnt;
    CelMonitorContext *ctx;
    CelEventLoop *evt_loop;
    void *checker;
    CelRefCounted ref_counted;
    CelTimerId timer_id;
};

extern CelKeyword mntr_types[];

#define cel_monitorcontext_type(type_str) \
    (CelMonitorType)cel_keyword_binary_search(\
    mntr_types, CEL_MONITOR_COUNT, type_str, _tcslen(type_str))

int cel_monitorcontext_init(CelMonitorContext *mntr_ctx, 
                            const TCHAR *name, CelMonitorType type);
void cel_monitorcontext_destroy(CelMonitorContext *mntr_ctx);

CelMonitorContext *cel_monitorcontext_new(const TCHAR *name, 
                                          CelMonitorType type);
void cel_monitorcontext_free(CelMonitorContext *mntr_ctx);

#define cel_monitorcontext_ref(mntr_ctx) \
    (CelMonitorContext *)cel_refcounted_ref_ptr(&(mntr_ctx->ref_counted), mntr_ctx)
#define cel_monitorcontext_deref(mntr_ctx) \
    cel_refcounted_deref(&(mntr_ctx->ref_counted), mntr_ctx)

/* void cel_monitorcontext_set_timeout(CelMonitor *mntr_ctx, int timeout); */
#define cel_monitorcontext_set_timeout(mntr_ctx, _timeout) \
    (mntr_ctx)->timeout = _timeout
/* void cel_monitorcontext_set_interval(CelMonitor *monitor, int interval); */
#define cel_monitorcontext_set_interval(mntr_ctx, _interval) \
    (mntr_ctx)->interval = _interval
/* 
 * void cel_monitorcontext_set_up_threshold(CelMonitor *monitor, 
 *                                          int up_threshold);
 */
#define cel_monitorcontext_set_up_threshold(mntr_ctx, threshold) \
    (mntr_ctx)->up_threshold = threshold
/* 
 * void cel_monitor_set_down_threshold(CelMonitor *monitor, 
 *                                     int down_threshold); 
 */
#define cel_monitorcontext_set_down_threshold(mntr_ctx, threshold) \
    (mntr_ctx)->down_threshold = threshold

int cel_monitor_init(CelMonitor *monitor, 
                     CelMonitorContext *mntr_ctx, const TCHAR *address);
void cel_monitor_destroy(CelMonitor *monitor);

CelMonitor *cel_monitor_new(CelMonitorContext *mntr_ctx, const TCHAR *address);
void cel_monitor_free(CelMonitor *monitor);

/* CelMonitor *cel_monitor_ref(CelMonitor *monitor);*/
#define cel_monitor_ref(monitor) \
    cel_refcounted_ref_ptr(&(monitor->ref_counted), monitor)
/* void cel_monitor_deref(CelMonitor *monitor); */
#define cel_monitor_deref(monitor) \
    cel_refcounted_deref(&(monitor->ref_counted), monitor)

void cel_monitor_check_async(CelMonitor *monitor, CelEventLoop *evt_loop);
#define cel_monitor_is_active(monitor) (monitor)->active

#ifdef __cplusplus
}
#endif

#endif
