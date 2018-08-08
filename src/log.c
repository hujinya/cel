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
#include "cel/log.h"
#include "cel/error.h"
#include "cel/allocator.h"
#include "cel/atomic.h"
#include "cel/convert.h"
#include "cel/file.h"
#include "cel/multithread.h"
#include "cel/net/sockaddr.h"

#define CEL_LOGBUF_NUM   10 * 1024

typedef struct _CelLogSpecific
{
    unsigned long pid;
    CelLogMsg msg;
}CelLogSpecific;

typedef struct _CelLogerHookItem
{
    CelListItem item;
    TCHAR name[CEL_KNLEN];
    CelLogMsgWriteFunc write_func;
    CelLogMsgFlushFunc flush_func;
    void *user_data;
}CelLogerHookItem;

static const TCHAR *s_loglevelstr[] = { 
    _T("Emerg"),
    _T("Alert"), 
    _T("Crit"), 
    _T("Err"), 
    _T("Warning"), 
    _T("Notice"), 
    _T("Info"), 
    _T("Debug") 
};

CelLogger g_logger = { 
    CEL_LOGFACILITY_LOCAL1, /* facility */
    { 
        -1,-1, -1, -1,-1,                  /* 0 - 4 */
        -1,-1,-1,-1,-1,                    /* 5 - 9 */
        -1,-1,-1,-1,-1,                    /* 10 - 14 */
        -1, CEL_LOGLEVEL_DEBUG, CEL_LOGLEVEL_DEBUG,-1, -1, /* 15 - 19 */
        -1,-1,-1,-1        /* 20 - 23 */
    },  /* level */
    { '\0' }, /* hostname */
    { '\0' }, /* processname */
    0,        /* styles */
    NULL,     /* hook_list */
    FALSE,    /* is_flush */
    CEL_LOGBUF_NUM,
    NULL,     /* msg_bufs */
    NULL      /* ring list */
};

void cel_logger_facility_set(CelLogger *logger, CelLogFacility facility)
{
    CelLogLevel level;

    level = logger->level[facility];
    logger->facility = facility;
    logger->level[facility] = level;
}

void _cel_logger_level_set(CelLogger *logger,
                           CelLogFacility facility, CelLogLevel level)
{
    if (logger == NULL)
        logger = &g_logger;
    logger->level[facility] = level;
}

int cel_logger_buffer_num_set(CelLogger *logger, size_t num)
{
    if (logger == NULL)
        logger = &g_logger;
    cel_multithread_mutex_lock(CEL_MT_MUTEX_LOG);
    if (logger->msg_bufs == NULL)
    {
        logger->n_bufs = cel_power2min((int)num);
        cel_multithread_mutex_unlock(CEL_MT_MUTEX_LOG);
        return 0;
    }
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_LOG);
    return -1;
}

int cel_logger_hook_register(CelLogger *logger, const TCHAR *name,
                             CelLogMsgWriteFunc write_func, 
                             CelLogMsgFlushFunc flush_func, void *user_data)
{
    CelLogerHookItem *hook;

    if (logger->hook_list == NULL)
    {
        if ((logger->hook_list = cel_list_new(cel_free)) == NULL)
        {
            _tprintf(_T("Hook list new failed.\r\n"));
            return -1;
        }
    }
    if ((hook = cel_malloc(sizeof(CelLogerHookItem))) == NULL)
    {
        _tprintf(_T("Hook malloc failed.\r\n"));
        return -1;
    }
    if (write_func == NULL)
    {
        write_func = cel_logmsg_fwrite;
        flush_func = cel_logmsg_fflush;
    }
    if (name != NULL)
        strncpy(hook->name, name, CEL_KNLEN);
    else
        strncpy(hook->name, "unnamed", CEL_KNLEN);
    hook->write_func = write_func;
    hook->flush_func = flush_func;
    hook->user_data = user_data;
    cel_list_push_back(logger->hook_list, (CelListItem *)hook);

    return 0;
}

int cel_logger_hook_unregister(CelLogger *logger, const TCHAR *name)
{
    CelLogerHookItem *hook1, *hook2;

    if (logger->hook_list == NULL)
        return -1;
    if (name == NULL)
        return -1;
    hook1 = (CelLogerHookItem *)g_logger.hook_list->head.next;
    while ((hook2 = hook1) != (CelLogerHookItem *)&(logger->hook_list->tail))
    {
        hook1 = (CelLogerHookItem *)hook2->item.next;
        if (strncmp(hook2->name, name, CEL_KNLEN) == 0)
        {
            cel_list_remove(logger->hook_list, (CelListItem *)hook2);
            cel_free(hook2);
        } 
    }
    return -1;
}

static int cel_logger_write(CelLogger *logger, CelLogMsg *log_msg)
{
    CelLogerHookItem *hook1, *hook2;

    hook1 = (CelLogerHookItem *)logger->hook_list->head.next;
    while ((hook2 = hook1) != (CelLogerHookItem *)&(logger->hook_list->tail))
    {
        //puts(hook2->name);
        hook1 = (CelLogerHookItem *)hook2->item.next;
        hook2->write_func(log_msg, hook2->user_data);
    }
    return 0;
}

static int _cel_loggger_flush(CelLogger *logger)
{
    CelLogerHookItem *hook1, *hook2;

    hook1 = (CelLogerHookItem *)logger->hook_list->head.next;
    while ((hook2 = hook1) != (CelLogerHookItem *)&(logger->hook_list->tail))
    {
        //puts(hook2->name);
        hook1 = (CelLogerHookItem *)hook2->item.next;
        if (hook2->flush_func != NULL)
            hook2->flush_func(hook2->user_data);
    }
    return 0;
}

void _cel_logger_ringlist_pop_msg(CelRingList *ring_list, 
                                  unsigned long cons_head, size_t n,
                                  void *user_data)
{
    U32 idx = (cons_head & ring_list->mask);
    CelLogger *logger = (CelLogger *)user_data;
    CelLogMsg *log_msg;

    for (idx = (cons_head & ring_list->mask); n > 0; n--,idx++)
    {
        if (idx >= ring_list->size)
            idx = 0;
        log_msg = &g_logger.msg_bufs[idx];
        //printf("pop idx=%d\r\n", idx);
        if (g_logger.hook_list == NULL)
            cel_logmsg_puts(log_msg, NULL);
        else 
            cel_logger_write(logger, log_msg);
    }
}

int cel_logger_flush(CelLogger *logger)
{
    int n_msg = 0;

    if (logger->msg_bufs == NULL)
    {
        cel_multithread_mutex_lock(CEL_MT_MUTEX_LOG);
        if (logger->msg_bufs == NULL)
        {
            logger->n_bufs = cel_power2min(logger->n_bufs);
            if ((logger->msg_bufs = (CelLogMsg *)_cel_sys_malloc(
                sizeof(CelLogMsg) * logger->n_bufs)) == NULL)
            {
                cel_multithread_mutex_unlock(CEL_MT_MUTEX_LOG);
                return -1;
            }
            /*printf("size %d * %d = %d\r\n", 
                sizeof(CelLogMsg), logger->n_bufs, sizeof(CelLogMsg) * logger->n_bufs);*/
            if ((logger->ring_list = 
                cel_ringlist_new(logger->n_bufs)) == NULL)
            {
                _cel_sys_free(logger->msg_bufs);
                logger->msg_bufs = NULL;
                cel_multithread_mutex_unlock(CEL_MT_MUTEX_LOG);
                return -1;
            }
            logger->is_flush = TRUE;
        }
        cel_multithread_mutex_unlock(CEL_MT_MUTEX_LOG);
    }
    //printf("n_msg %ld\r\n", cel_getticks());
    n_msg = _cel_ringlist_pop_do_mp(logger->ring_list, logger->n_bufs, 
        (CelRingListProdFunc)_cel_logger_ringlist_pop_msg, logger);
    if (n_msg > 0 && logger->hook_list != NULL)
        _cel_loggger_flush(logger);
    //printf("n_msg = %d %ld\r\n", n_msg, cel_getticks());

    return n_msg;
}

static CelLogSpecific *_cel_logger_specific_get()
{
    CelLogSpecific *log_specific;

    if ((log_specific = (CelLogSpecific *)
        cel_multithread_get_keyvalue(CEL_MT_KEY_LOG)) == NULL)
    {
        if ((log_specific = 
            (CelLogSpecific *)_cel_sys_malloc(sizeof(CelLogSpecific))) != NULL)
        {
            log_specific->pid = cel_thread_getid();
            if (cel_multithread_set_keyvalue(
                CEL_MT_KEY_LOG, log_specific) != -1
                && cel_multithread_set_keydestructor(
                CEL_MT_KEY_LOG, _cel_sys_free) != -1)
                return log_specific;
            _cel_sys_free(log_specific);
        }
        return NULL;
    }
    return log_specific;
}

typedef struct _CelLogUserData
{
    CelLogger *logger;
    CelLogFacility facility;
    CelLogLevel level;
    int type;
    union {
        const TCHAR *str;
        struct {
            const BYTE *p;
            size_t len;
        };
        struct {
            const TCHAR *fmt;
            va_list ap;
        };
    };
}CelLogUserData;

#define CEL_LOGMSG_WRITE() do { \
    switch (user_data->type){ \
    case 1: \
    _tcsncpy(log_msg->content, user_data->str, CEL_LOGCONTENT_LEN); \
    break; \
    case 2: \
    cel_hexdump(log_msg->content, CEL_LOGCONTENT_LEN,  \
    user_data->p, user_data->len); \
    break; \
    case 3: \
    _vsntprintf(log_msg->content, CEL_LOGCONTENT_LEN,  \
    user_data->fmt, user_data->ap); \
    break; \
    default: \
    break; \
} \
    log_msg->facility = user_data->facility; \
    log_msg->level = user_data->level; \
    cel_datetime_init_now(&msg_time); \
    log_msg->timestamp = msg_time; \
    log_msg->hostname = logger->hostname; \
    log_msg->processname = logger->processname; \
    log_msg->pid = cel_thread_getid(); \
}while(0)

#define CEL_LOG_FLUSH() do { \
    if (logger->hook_list == NULL) \
    cel_logmsg_puts(log_msg, NULL); else \
    cel_logger_write(logger, log_msg); \
    if (logger->hook_list != NULL) \
    _cel_loggger_flush(logger); \
}while(0)

void _cel_logger_ringlist_push_msg(CelRingList *ring_list, 
                                   unsigned long prod_head, size_t n,
                                   CelLogUserData *user_data)
{
    U32 idx;
    CelLogger *logger = user_data->logger;
    CelLogMsg *log_msg;
    CelDateTime msg_time;

    for (idx = (prod_head & ring_list->mask); n > 0; idx++, n--)
    {
        if (idx >= ring_list->size)
            idx = 0;
        //printf("[%d]push idx=[%d]%d \r\n",
        //    cel_thread_getid(), prod_head, idx);
        log_msg = &(logger->msg_bufs[idx]);
        CEL_LOGMSG_WRITE();
        ring_list->ring[idx] = log_msg;
    }
}

int cel_logger_puts(CelLogger *logger,
                    CelLogFacility facility, CelLogLevel level, 
                    const TCHAR *str)
{
    CelLogUserData _user_data, *user_data;
    CelLogMsg *log_msg;
    CelDateTime msg_time;

    if (logger->level[facility] < level)
        return -1;
    _user_data.logger = logger;
    _user_data.facility = logger->facility;
    _user_data.level = level;
    _user_data.type = 1;
    _user_data.str = str;
    if (logger->is_flush)
    {
        if (_cel_ringlist_push_do_mp(logger->ring_list, 1, 
            (CelRingListProdFunc)_cel_logger_ringlist_push_msg,
            &_user_data) <= 0)
        {
            puts("Drop message.");
            return -1;
        }
    }
    else
    {
        log_msg = &(_cel_logger_specific_get()->msg);
        user_data = &_user_data;
        CEL_LOGMSG_WRITE();
        CEL_LOG_FLUSH();
    }

    return 0;
}

int cel_logger_hexdump(CelLogger *logger, 
                       CelLogFacility facility, CelLogLevel level, 
                       const BYTE *p, size_t len)
{
    CelLogUserData _user_data, *user_data;
    CelLogMsg *log_msg;
    CelDateTime msg_time;

    if (g_logger.level[facility] < level)
        return -1;
    _user_data.logger = logger;
    _user_data.facility = logger->facility;
    _user_data.level = level;
    _user_data.type = 2;
    _user_data.p = p;
    _user_data.len = len;
    if (g_logger.is_flush)
    {
        if (_cel_ringlist_push_do_mp(logger->ring_list, 1, 
            (CelRingListProdFunc)_cel_logger_ringlist_push_msg,
            &_user_data) <= 0)
        {
            puts("Drop message.");
            return -1;
        }
    }
    else
    {
        log_msg = &(_cel_logger_specific_get()->msg);
        user_data = &_user_data;
        CEL_LOGMSG_WRITE();
        CEL_LOG_FLUSH();
    }

    return 0;
}

int cel_logger_vprintf(CelLogger *logger, 
                       CelLogFacility facility, CelLogLevel level, 
                       const TCHAR *fmt, va_list ap)
{
    CelLogUserData _user_data, *user_data;
    CelLogMsg *log_msg;
    CelDateTime msg_time;

    //_tprintf(_T("%d %d\r\n"), logger->level[CEL_DEFAULT_FACILITY],  level);
    if (logger->level[facility] < level)
        return -1;
    _user_data.logger = logger;
    _user_data.facility = logger->facility;
    _user_data.level = level;
    _user_data.type = 3;
    _user_data.fmt = fmt;
    va_copy(_user_data.ap, ap);

    if (logger->is_flush)
    {
        if (_cel_ringlist_push_do_mp(logger->ring_list, 1, 
            (CelRingListProdFunc)_cel_logger_ringlist_push_msg,
            &_user_data) <= 0)
        {
            puts("Drop message.");
            return -1;
        }
    }
    else
    {
        log_msg = &(_cel_logger_specific_get()->msg);
        user_data = &_user_data;
        CEL_LOGMSG_WRITE();
        CEL_LOG_FLUSH();
    }

    return 0;
}

int cel_log_printf(CelLogLevel level, const TCHAR *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    cel_log_vprintf(CEL_LOGLEVEL_DEBUG, fmt, args);
    va_end(args);

    return 0;
}

int cel_log_debug(const TCHAR *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    cel_log_vprintf(CEL_LOGLEVEL_DEBUG, fmt, args);
    va_end(args);

    return 0;
}

int cel_log_info(const TCHAR *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    cel_log_vprintf(CEL_LOGLEVEL_INFO, fmt, args);
    va_end(args);

    return 0;
}

int cel_log_notice(const TCHAR *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    cel_log_vprintf(CEL_LOGLEVEL_NOTICE, fmt, args);
    va_end(args);

    return 0;
}

int cel_log_warning(const TCHAR *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    cel_log_vprintf(CEL_LOGLEVEL_WARNING, fmt, args);
    va_end(args);

    return 0;
}

int cel_log_err(const TCHAR *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    cel_log_vprintf(CEL_LOGLEVEL_ERR, fmt, args);
    va_end(args);

    return 0;
}

int cel_log_crit(const TCHAR *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    cel_log_vprintf(CEL_LOGLEVEL_CRIT, fmt, args);
    va_end(args);

    return 0;
}

int cel_log_alert(const TCHAR *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    cel_log_vprintf(CEL_LOGLEVEL_ALERT, fmt, args);
    va_end(args);

    return 0;
}

int cel_log_emerg(const TCHAR *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    cel_log_vprintf(CEL_LOGLEVEL_EMERG, fmt, args);
    va_end(args);

    return 0;
}

static FILE *s_fp = NULL;

static int cel_log_fremove_callback(const TCHAR *dir, const TCHAR *file_name,
                                    const CelDirent *dirent, void *user_data)
{
    TCHAR file[CEL_PATHLEN];

    //puts(file_name);
    if (_ttol(file_name) < *((long *)user_data))
    {
        _sntprintf(file, CEL_PATHLEN, _T("%s%s"), dir, file_name);
        cel_fremove(file);
    }
    return 0;
}

int cel_logmsg_fwrite(CelLogMsg *msg, void *path)
{
    static int file_day = -1;
    static CelDateTime timestamp_cached;
    static TCHAR strtime[26];
    int msg_day;
    long lfile;
    TCHAR filename[15], file_path[CEL_PATHLEN];

    cel_datetime_get_date(&(msg->timestamp), NULL, NULL, &msg_day, NULL);
    //_tprintf("msg_day %d, file_day %d\r\n", msg_day, file_day);
    if (msg_day != file_day)
    {
        file_day = msg_day;
        if (s_fp != NULL)
        {
            cel_fclose(s_fp);
            s_fp = NULL;
        }
        cel_datetime_add_days(&(msg->timestamp), -30);
        cel_datetime_strfltime(&(msg->timestamp), filename, 13, 
            _T("%Y%m%d.log"));
        lfile = _ttol(filename);
        cel_foreachdir((TCHAR *)path, cel_log_fremove_callback, &lfile);
        cel_datetime_add_days(&(msg->timestamp), 30);
    }
    /* Open log file, uninstall log hook if open failed. */
    if (s_fp == NULL)
    {
        cel_datetime_strfltime(&(msg->timestamp), filename, 15, 
            _T("%Y%m%d.log"));
        _sntprintf(file_path, CEL_PATHLEN, _T("%s%s"), 
            (TCHAR *)path, filename);
        //_putts(path);
        if ((s_fp = cel_fopen(file_path, _T("a"))) == NULL 
            && (cel_mkdirs((TCHAR *)path, 
            S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) == -1 
             || (s_fp = cel_fopen(file_path, _T("a"))) == NULL))
        {
            _putts(cel_geterrstr(cel_sys_geterrno()));
            return 0;
        }
        //cel_chmod(file_path, S_IRWXU|S_IRGRP|S_IROTH);
        _fputts(CEL_CRLF, s_fp);
    }
    /* Write log line*/
    if (timestamp_cached != msg->timestamp)
    {
        timestamp_cached = msg->timestamp;
        cel_datetime_strfltime(&(msg->timestamp), strtime, 26, _T("%b %d %X"));
    }
    if (_ftprintf(s_fp, _T("%s [%ld]: <%s>%s.")CEL_CRLF, 
        /*((msg->facility << 3) | msg->level), */
        strtime, msg->pid, s_loglevelstr[msg->level], msg->content) == EOF)
    {
        cel_fclose(s_fp);
        s_fp = NULL;
    }

    return 0;
}

int cel_logmsg_fflush(void *path)
{
    return cel_fflush(s_fp);
}

int cel_logmsg_puts(CelLogMsg *msg, void *user_data)
{
    static CelDateTime timestamp_cached;
    static TCHAR strtime[26];

    if (timestamp_cached != msg->timestamp)
    {
        timestamp_cached = msg->timestamp;
        cel_datetime_strfltime(&(msg->timestamp), strtime, 26, _T("%b %d %X"));
    }
    _tprintf(_T("%s [%ld]: <%s>%s.")CEL_CRLF, 
        /*((msg->facility << 3) | msg->level), */
        strtime, 
        msg->pid,
        s_loglevelstr[msg->level],
        msg->content);
   /* _tprintf(_T("<%d>%s %s %s[%ld]: %s")CEL_CRLF, 
        ((msg->facility << 3) | msg->level), 
        strtime, 
        msg->hostname, 
        msg->processname, msg->pid,
        msg->content);*/

    return 0;
}
