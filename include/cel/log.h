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
#ifndef __CEL_LOG_H__
#define __CEL_LOG_H__

/* http://www.ietf.org/rfc/rfc3164.txt */

#include "cel/types.h"
#include "cel/atomic.h"
#include "cel/datetime.h"
#include "cel/list.h"
#include "cel/ringlist.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _CelLogSeverity
{
    CEL_LOGLEVEL_UNDEFINED = -1,
    CEL_LOGLEVEL_EMERG = 0,
    CEL_LOGLEVEL_ALERT = 1,
    CEL_LOGLEVEL_CRIT = 2,
    CEL_LOGLEVEL_ERR = 3,
    CEL_LOGLEVEL_WARNING = 4,
    CEL_LOGLEVEL_NOTICE = 5,
    CEL_LOGLEVEL_INFO = 6,
    CEL_LOGLEVEL_DEBUG = 7,

    CEL_LOGSEVERITY_UNDEFINED = -1,
    CEL_LOGSEVERITY_EMERG = 0,
    CEL_LOGSEVERITY_ALERT = 1,
    CEL_LOGSEVERITY_CRIT = 2,
    CEL_LOGSEVERITY_ERR = 3,
    CEL_LOGSEVERITY_WARNING = 4,
    CEL_LOGSEVERITY_NOTICE = 5,
    CEL_LOGSEVERITY_INFO = 6,
    CEL_LOGSEVERITY_DEBUG = 7
}CelLogSeverity;

typedef CelLogSeverity CelLogLevel;

typedef enum _CelLogFacility
{
    CEL_LOGFACILITY_USER = 1,
    CEL_LOGFACILITY_LOCAL0 = 16, 
    CEL_LOGFACILITY_LOCAL1 = 17,
    CEL_LOGFACILITY_LOCAL2 = 18, 
    CEL_LOGFACILITY_LOCAL3 = 19, 
    CEL_LOGFACILITY_LOCAL4 = 20, 
    CEL_LOGFACILITY_LOCAL5 = 21,
    CEL_LOGFACILITY_LOCAL6 = 22,
    CEL_LOGFACILITY_LOCAL7 = 23,

    CEL_LOGFACILITY_COUNT
}CelLogFacility;

#define CEL_DEFAULT_FACILITY     CEL_LOGFACILITY_USER
#define CEL_LOGCONTENT_LEN       512  /* Bytes */

typedef struct _CelLogMsg
{
    CelLogSeverity level;         /**< RFC3164.4.1.1 */
    CelLogFacility facility;
    CelDateTime timestamp;        /**< RFC3164.4.1.2 */
    TCHAR *hostname;
    TCHAR *processname;           /**< RFC3164.4.1.3 */
    unsigned long pid;
    TCHAR content[CEL_LOGCONTENT_LEN];
}CelLogMsg;

typedef int (* CelLogMsgWriteFunc) (CelLogMsg *msg, void *user_data);
typedef int (* CelLogMsgFlushFunc) (void *user_data);

typedef struct _CelLogger
{
    CelLogFacility facility;
    CelLogLevel level[CEL_LOGFACILITY_COUNT];
    TCHAR hostname[CEL_HNLEN];
    TCHAR processname[CEL_FNLEN];
    int styles;
    CelList *hook_list;
    BOOL is_flush;
    size_t n_bufs;
    CelLogMsg *msg_bufs;
    CelRingList *ring_list;
}CelLogger;

extern CelLogger g_logger;

void cel_logger_facility_set(CelLogger *logger, CelLogFacility facility);

void _cel_logger_level_set(CelLogger *logger, 
                           CelLogFacility facility, CelLogLevel level);
#define cel_logger_level_set(logger, level) \
    _cel_logger_level_set(logger, (logger)->facility, level)
#define cel_logger_severity_set cel_logger_level_set

#define cel_logger_hostname_set(logger, _hostname) \
    strncpy((logger)->hostname, _hostname, CEL_HNLEN)
#define cel_logger_processname_set(logger, processname) \
    strncpy((logger)->processname, _processname, CEL_FNLEN)
int cel_logger_buffer_num_set(CelLogger *logger, size_t num);

int cel_logger_hook_register(CelLogger *logger, const TCHAR *name,
                             CelLogMsgWriteFunc write_func, 
                             CelLogMsgFlushFunc flush_func, void *user_data);
int cel_logger_hook_unregister(CelLogger *logger, const TCHAR *name);

int cel_logger_flush(CelLogger *logger);

int _cel_logger_puts(CelLogger *logger, CelLogLevel level, const TCHAR *str);
int _cel_logger_vprintf(CelLogger *logger, CelLogLevel level, 
                        const TCHAR *fmt, va_list ap);
int _cel_logger_printf(CelLogger *logger, CelLogLevel level, 
                       const TCHAR *fmt, ...);
int _cel_logger_hexdump(CelLogger *logger, CelLogLevel level,
                        const BYTE *p, size_t len);

#define cel_log_puts(level, str) \
    _cel_logger_puts(&g_logger, level, str)
#define cel_log_vprintf(level, fmt, ap) \
    _cel_logger_vprintf(&g_logger, level, fmt, ap)
#define cel_log_hexdump(level, p, len) \
    _cel_logger_hexdump(&g_logger, level, p, len)
int cel_logger_printf(CelLogLevel level, const TCHAR *fmt, ...);

int cel_log_debug(const TCHAR *fmt, ...);
int cel_log_info(const TCHAR *fmt, ...);
int cel_log_notice(const TCHAR *fmt, ...);
int cel_log_warning(const TCHAR *fmt, ...);
int cel_log_err(const TCHAR *fmt, ...);
int cel_log_crit(const TCHAR *fmt, ...);
int cel_log_alert(const TCHAR *fmt, ...);
int cel_log_emerg(const TCHAR *fmt, ...);

#define CEL_LOG_DEBUG(args) \
    if (g_logger.level[g_logger.facility] >= CEL_LOGLEVEL_DEBUG) \
       cel_log_debug args
#define CEL_LOG_INFO(args) \
    if (g_logger.level[g_logger.facility] >= CEL_LOGLEVEL_INFO) \
       cel_log_info args
#define CEL_LOG_NOTICE(args) \
    if (g_logger.level[g_logger.facility] >= CEL_LOGLEVEL_NOTICE) \
       cel_log_notice args
#define CEL_LOG_WARNING(args) \
    if (g_logger.level[g_logger.facility] >= CEL_LOGLEVEL_WARNING) \
        cel_log_warning args
#define CEL_LOG_ERR(args) \
    if (g_logger.level[g_logger.facility] >= CEL_LOGLEVEL_ERR) \
       cel_log_err args
#define CEL_LOG_CRIT(args) \
    if (g_logger.level[g_logger.facility] >= CEL_LOGLEVEL_CRIT) \
        cel_log_crit args
#define CEL_LOG_ALERT(args) \
    if (g_logger.level[g_logger.facility] >= CEL_LOGLEVEL_ALERT) \
       cel_log_alert args
#define CEL_LOG_EMERG(args) \
    if (g_logger.level[g_logger.facility] >= CEL_LOGLEVEL_EMERG) \
       cel_log_emerg args

/* Write log file */
int cel_logmsg_fwrite(CelLogMsg *msg, void *user_data);
int cel_logmsg_fflush(void *user_data);
/* Print screen */
int cel_logmsg_puts(CelLogMsg *msg, void *user_data);
/* Insert db */
int cel_logmsg_dbinsert(CelLogMsg *msg, void *user_data);

// Like assert(), but executed even in _CEL_DEBUG mode
#undef CEL_CHECK_CONDITION
#define CEL_CHECK_CONDITION(cond)                              \
do {                                                           \
  if (!(cond)) {                                               \
    cel_log_debug(_T("%s(%d)-%s()-%s"),                        \
        _T(__FILE__), __LINE__, _T(__FUNCTION__), #cond);      \
    abort();                                                   \
  }                                                            \
}while (0)

#undef CEL_ASSERT
#ifdef _CEL_DEBUG
#define CEL_ASSERT(cond) CEL_CHECK_CONDITION(cond)
#else
#define CEL_ASSERT(cond) ((void) 0)
#endif

#ifdef __cplusplus
}
#endif

#endif
