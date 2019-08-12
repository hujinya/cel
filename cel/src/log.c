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

typedef struct _CelLogSpecific
{
    unsigned long pid;
    CelLogMsg msg;
    CelLogMsg *msg_buf;
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
        -1, -1,/* lib */ CEL_LOGLEVEL_DEBUG, /* default */ -1, -1, /* 15 - 19 */
        -1,-1,-1,-1        /* 20 - 23 */
    },  /* level */
    { '\0' }, /* hostname */
    { '\0' }, /* processname */
    NULL,     /* sink_list */
    FALSE,    /* is_flush */
    CEL_LOGGER_FLUSH_NUM,
    CEL_LOGGER_BUF_NUM,
    NULL,     /* free list */
    NULL,     /* ring list */
    NULL      /* mem_caches */
};

void cel_logger_facility_set(CelLogger *logger, CelLogFacility facility)
{
    CelLogLevel level;

    level = logger->level[facility];
    logger->facility = facility;
    logger->level[facility] = level;
}

void cel_logger_level_set(CelLogger *logger,
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
    if (logger->mem_caches == NULL
        && num >= logger->n_flushs)
    {
        logger->n_bufs = cel_power2min((int)num);
        cel_multithread_mutex_unlock(CEL_MT_MUTEX_LOG);
        return 0;
    }
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_LOG);
    return -1;
}

int cel_logger_flush_num_set(CelLogger *logger, size_t num)
{
    if (logger == NULL)
        logger = &g_logger;
    cel_multithread_mutex_lock(CEL_MT_MUTEX_LOG);
    if (logger->mem_caches == NULL
        && num <= logger->n_bufs)
    {
        logger->n_flushs = num;
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

    if (logger->sink_list == NULL)
    {
        if ((logger->sink_list = cel_list_new(cel_free)) == NULL)
        {
            _putts(_T("Hook list new failed"));
            return -1;
        }
    }
    if ((hook = cel_malloc(sizeof(CelLogerHookItem))) == NULL)
    {
        _putts(_T("Hook malloc failed."));
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
    cel_list_push_back(logger->sink_list, (CelListItem *)hook);

    return 0;
}

int cel_logger_hook_unregister(CelLogger *logger, const TCHAR *name)
{
    CelLogerHookItem *hook1, *hook2;

    if (logger->sink_list == NULL)
        return -1;
    if (name == NULL)
        return -1;
    hook1 = (CelLogerHookItem *)g_logger.sink_list->head.next;
    while ((hook2 = hook1) != (CelLogerHookItem *)&(logger->sink_list->tail))
    {
        hook1 = (CelLogerHookItem *)hook2->item.next;
        if (name == NULL
            || strncmp(hook2->name, name, CEL_KNLEN) == 0)
        {
            cel_list_remove(logger->sink_list, (CelListItem *)hook2);
            cel_free(hook2);
        } 
    }
    return -1;
}

static int cel_logger_cache_file_write(CelLogger *logger)
{
    return -1;
    /*int i = 0, n;
    char file_path[CEL_PATHLEN];
    FILE *fp;
    CelLogMsg *msg;
    CelDateTime timestamp_cached;
    char strtime[26];

    cel_datetime_init_now(&timestamp_cached);
    cel_datetime_strfltime(&timestamp_cached, strtime, 26, _T("%Y%m%d%H%M%S"));
    snprintf(file_path, CEL_PATHLEN, "%s%s%ld.log", 
        cel_fullpath_a(CEL_LOGGER_CACHE_PATH), strtime, cel_getticks());

    cel_multithread_mutex_lock(CEL_MT_MUTEX_LOG);
    if ((fp = fopen(file_path, "wb+")) == NULL 
        && (cel_mkdirs_a(cel_filedir_a(file_path), CEL_UMASK) == -1
        || (fp = fopen(file_path, "wb+")) == NULL))
    {
        cel_multithread_mutex_unlock(CEL_MT_MUTEX_LOG);
        CEL_SETERR((CEL_ERR_LIB, _T("cel_httprequest_save_body_data failed")));
        return -1;
    }
    n = cel_ringlist_pop_do_sp(
        logger->ring_list, logger->file_caches, logger->n_bufs);
    timestamp_cached = 0;
    while (i < n)
    {
        msg = logger->file_caches[i];
        if (timestamp_cached != msg->timestamp)
        {
            timestamp_cached = msg->timestamp;
            cel_datetime_strfltime(
                &(msg->timestamp), strtime, 26, _T("%b %d %X"));
        }
        if (_ftprintf(fp, _T("<%s>%s [%ld]: %s.")CEL_CRLF, 
            ((msg->facility << 3) | msg->level), 
            strtime, msg->pid,  msg->content) == EOF)
        {
            _putts("write cache file failed.");
            break;
        }
    }
    cel_fclose(fp);
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_LOG);
    cel_ringlist_push_do_sp(logger->free_list, logger->file_caches[0], n);

    return n;*/
}

static int cel_logger_cache_file_read(CelLogger *logger, CelLogMsg **msgs, size_t n)
{
    return -1;
}

static int cel_logger_writing(CelLogger *logger, CelLogMsg **msgs, size_t n)
{
    CelLogerHookItem *hook1, *hook2;

    hook1 = (CelLogerHookItem *)logger->sink_list->head.next;
    while ((hook2 = hook1) != (CelLogerHookItem *)&(logger->sink_list->tail))
    {
        //puts(hook2->name);
        hook1 = (CelLogerHookItem *)hook2->item.next;
        hook2->write_func(msgs, n, hook2->user_data);
    }
    return 0;
}

static int cel_loggger_flushing(CelLogger *logger)
{
    CelLogerHookItem *hook1, *hook2;

    hook1 = (CelLogerHookItem *)logger->sink_list->head.next;
    while ((hook2 = hook1) != (CelLogerHookItem *)&(logger->sink_list->tail))
    {
        //puts(hook2->name);
        hook1 = (CelLogerHookItem *)hook2->item.next;
        if (hook2->flush_func != NULL)
            hook2->flush_func(hook2->user_data);
    }
    return 0;
}

int _cel_logger_freelist_init(CelLogger *logger)
{
    size_t i;
    CelLogMsg *msgs;

    cel_multithread_mutex_lock(CEL_MT_MUTEX_LOG);
    if (logger->mem_caches == NULL)
    {
        logger->n_bufs = cel_power2min(logger->n_bufs);
        if ((logger->mem_caches = (CelLogMsg **)_cel_sys_malloc(
            (sizeof(void *) + sizeof(CelLogMsg)) * logger->n_bufs)) == NULL)
        {
            cel_multithread_mutex_unlock(CEL_MT_MUTEX_LOG);
            return -1;
        }
        msgs = (CelLogMsg *)(logger->mem_caches + logger->n_bufs);
        for (i = 0; i < logger->n_bufs; i++)
        {
            logger->mem_caches[i] = &msgs[i];
        }
        /*printf("size %d * %d = %d\r\n", 
            sizeof(CelLogMsg), logger->n_bufs, 
            sizeof(CelLogMsg) * logger->n_bufs);*/
        if ((logger->ring_list = 
            cel_ringlist_new(logger->n_bufs, NULL)) == NULL)
        {
            _cel_sys_free(logger->mem_caches);
            logger->mem_caches = NULL;
            cel_multithread_mutex_unlock(CEL_MT_MUTEX_LOG);
            return -1;
        }
        if ((logger->free_list = 
            cel_ringlist_new(logger->n_bufs, NULL)) == NULL)
        {
            _cel_sys_free(logger->mem_caches);
            logger->mem_caches = NULL;
            cel_ringlist_free(logger->free_list);
            cel_multithread_mutex_unlock(CEL_MT_MUTEX_LOG);
            return -1;
        }
        i = cel_ringlist_push_do_sp(logger->free_list,
            logger->mem_caches[0], logger->n_bufs);
        //printf("i = %d %d\r\n", i, logger->n_bufs);
        logger->is_flush = TRUE;
    }
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_LOG);
    return 0;
}

int cel_logger_flush(CelLogger *logger)
{
    int n;
    BOOL is_mem;

    if (logger->mem_caches == NULL)
        _cel_logger_freelist_init(logger);
    //printf("n_msg %ld\r\n", cel_getticks());
    if (!cel_multithread_mutex_trylock(CEL_MT_MUTEX_LOG))
        return 0;
    is_mem = FALSE;
    if ((n = cel_logger_cache_file_read(
        logger, logger->mem_caches, logger->n_bufs)) <= 0)
    {
        n = cel_ringlist_pop_do_sp(
            logger->ring_list, logger->mem_caches, logger->n_bufs);
        is_mem = TRUE;
    }
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_LOG);
    if (n > 0)
    {
        if (logger->sink_list == NULL)
            cel_logmsg_puts(logger->mem_caches, n, NULL);
        else 
            cel_logger_writing(logger, logger->mem_caches, n);
        if (is_mem)
            cel_ringlist_push_do_sp(
            logger->free_list, logger->mem_caches[0], n);
        if (logger->sink_list != NULL)
            cel_loggger_flushing(logger);
    }
    //printf("n_msg = %d %ld\r\n", n, cel_getticks());

    return n;
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
            log_specific->msg_buf = NULL;
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

#define CEL_LOGMSG_SPECIFIC_GET() do { \
    if ((log_msg = (log_specific->msg_buf)) == NULL) { \
        if (cel_ringlist_pop_do_mp( \
            logger->free_list, &(log_specific->msg_buf), 1) < 1) { \
            if (cel_logger_cache_file_write(logger) == -1 \
                || cel_ringlist_pop_do_mp( \
                logger->free_list, &(log_specific->msg_buf), 1) < 1) { \
                _putts(_T("Drop message.")); \
                return -1; } } \
        log_msg = log_specific->msg_buf; }\
}while(0)

#define CEL_LOGMSG_WRITE() do { \
    log_msg->facility = facility; \
    log_msg->level = level; \
    cel_datetime_init_now(&msg_time); \
    log_msg->timestamp = msg_time; \
    log_msg->hostname = logger->hostname; \
    log_msg->processname = logger->processname; \
    log_msg->pid = log_specific->pid; \
}while(0)

#define CEL_LOG_FLUSH() do { \
    if (logger->sink_list == NULL) \
        cel_logmsg_puts(&log_msg, 1, NULL); \
    else \
        cel_logger_writing(logger, &log_msg, 1); \
    if (logger->sink_list != NULL) \
        cel_loggger_flushing(logger); \
}while(0)

int cel_logger_puts(CelLogger *logger,
                    CelLogFacility facility, CelLogLevel level, 
                    const TCHAR *str)
{
    CelLogSpecific *log_specific;
    CelLogMsg *log_msg;
    CelDateTime msg_time;

    if (logger->level[facility] < level)
        return -1;
    log_specific = _cel_logger_specific_get();
    if (logger->is_flush)
    {
        CEL_LOGMSG_SPECIFIC_GET();
        _tcsncpy(log_msg->content, str, CEL_LOGMSG_CONTENT_SIZE);
        CEL_LOGMSG_WRITE();
        cel_ringlist_push_do_mp(logger->ring_list, log_msg, 1);
        log_specific->msg_buf = NULL;
    }
    else
    {
        log_msg = &(log_specific->msg);
        _tcsncpy(log_msg->content, str, CEL_LOGMSG_CONTENT_SIZE);
        CEL_LOGMSG_WRITE();
        CEL_LOG_FLUSH();
    }

    return 0;
}

int cel_logger_write(CelLogger *logger, 
                     CelLogFacility facility, CelLogLevel level, 
                     void *buf, size_t size)
{
    CelLogSpecific *log_specific;
    CelLogMsg *log_msg;
    CelDateTime msg_time;

    if (g_logger.level[facility] < level)
        return -1;
    log_specific = _cel_logger_specific_get();
    if (logger->is_flush)
    {
        CEL_LOGMSG_SPECIFIC_GET();
        memcpy(log_msg->content, buf, 
            (size > CEL_LOGMSG_CONTENT_SIZE 
            ? CEL_LOGMSG_CONTENT_SIZE : size));
        CEL_LOGMSG_WRITE();
        cel_ringlist_push_do_mp(logger->ring_list, log_msg, 1);
        log_specific->msg_buf = NULL;
    }
    else
    {
        log_msg = &(log_specific->msg);
        memcpy(log_msg->content, buf, 
            (size > CEL_LOGMSG_CONTENT_SIZE  
            ? CEL_LOGMSG_CONTENT_SIZE : size));
        CEL_LOGMSG_WRITE();
        CEL_LOG_FLUSH();
    }

    return 0;
}

int cel_logger_hexdump(CelLogger *logger, 
                       CelLogFacility facility, CelLogLevel level, 
                       const BYTE *p, size_t len)
{
    CelLogSpecific *log_specific;
    CelLogMsg *log_msg;
    CelDateTime msg_time;

    if (g_logger.level[facility] < level)
        return -1;
    log_specific = _cel_logger_specific_get();
    if (logger->is_flush)
    {
        CEL_LOGMSG_SPECIFIC_GET();
        cel_hexdump(log_msg->content, CEL_LOGMSG_CONTENT_SIZE, p, len);
        CEL_LOGMSG_WRITE();
        cel_ringlist_push_do_mp(logger->ring_list, log_msg, 1);
        log_specific->msg_buf = NULL;
    }
    else
    {
        log_msg = &(log_specific->msg);
        cel_hexdump(log_msg->content, CEL_LOGMSG_CONTENT_SIZE, p, len);
        CEL_LOGMSG_WRITE();
        CEL_LOG_FLUSH();
    }

    return 0;
}

int cel_logger_vprintf(CelLogger *logger, 
                       CelLogFacility facility, CelLogLevel level, 
                       const TCHAR *fmt, va_list ap)
{
    CelLogSpecific *log_specific;
    CelLogMsg *log_msg;
    CelDateTime msg_time;

    //_tprintf(_T("%d %d\r\n"), logger->level[CEL_DEFAULT_FACILITY],  level);
    if (logger->level[facility] < level)
        return -1;
    log_specific = _cel_logger_specific_get();
    if (logger->is_flush)
    {
        CEL_LOGMSG_SPECIFIC_GET();
        _vsntprintf(log_msg->content, CEL_LOGMSG_CONTENT_SIZE, fmt, ap);
        CEL_LOGMSG_WRITE();
        cel_ringlist_push_do_mp(logger->ring_list, log_msg, 1);
        log_specific->msg_buf = NULL;
    }
    else
    {
        log_msg = &(log_specific->msg);
        _vsntprintf(log_msg->content, CEL_LOGMSG_CONTENT_SIZE, fmt, ap);
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

int cel_logmsg_fwrite(CelLogMsg **msgs, size_t n, void *path)
{
    static int file_day = -1;
    static CelDateTime timestamp_cached = 0;
    static TCHAR strtime[26];
    CelLogMsg *msg;
    size_t i = 0;
    int msg_day;
    long lfile;
    TCHAR filename[15], file_path[CEL_PATHLEN];

    while (i < n)
    {
        msg = msgs[i++];
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
                && (cel_mkdirs((TCHAR *)path, CEL_UMASK) == -1
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
    }

    return 0;
}

int cel_logmsg_fflush(void *path)
{
    return cel_fflush(s_fp);
}

int cel_logmsg_puts(CelLogMsg **msgs, size_t n, void *user_data)
{
    static CelDateTime timestamp_cached = 0;
    static TCHAR strtime[26];
    CelLogMsg *msg;
    size_t i = 0;

    while (i < n)
    {
        msg = msgs[i++];
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
    }

    return 0;
}
