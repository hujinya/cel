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
#include "cel/convert.h"
#include "cel/file.h"
#include "cel/multithread.h"
#include "cel/net/sockaddr.h"
#include <stdarg.h>

#define CEL_LOGBUF_NUM   1024

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
    CEL_DEFAULT_FACILITY, /* facility */
    { CEL_LOGLEVEL_DEBUG, CEL_LOGLEVEL_DEBUG },  /* level */
    { '\0' }, /* hostname */
    { '\0' }, /* processname */
    0,        /* styles */
    NULL,     /* hook_list */
    FALSE,    /* is_flush */
    0,        /* read_pos */
    1,        /* write_pos */
    NULL      /* msg_buf */
};

void cel_logger_facility_set(CelLogFacility facility)
{
    CelLogLevel level;

    level = g_logger.level[facility];
    g_logger.facility = facility;
    g_logger.level[facility] = level;
}

void _cel_logger_level_set(CelLogFacility facility, CelLogLevel level)
{
    g_logger.level[facility] = level;
}

void cel_logger_styles_set(int styles)
{
    g_logger.styles = styles;
}

int cel_logger_hook_register(const TCHAR *name,
                             CelLogMsgWriteFunc write_func, 
                             CelLogMsgFlushFunc flush_func, void *user_data)
{
    CelLogerHookItem *hook;

    if (g_logger.hook_list == NULL)
    {
        if ((g_logger.hook_list = cel_list_new(cel_free)) == NULL)
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
    cel_list_push_back(g_logger.hook_list, (CelListItem *)hook);

    return 0;
}

int cel_logger_hook_unregister(const TCHAR *name)
{
    CelLogerHookItem *hook1, *hook2;

    if (g_logger.hook_list == NULL)
        return -1;
    if (name == NULL)
        return -1;
    hook1 = (CelLogerHookItem *)g_logger.hook_list->head.next;
    while ((hook2 = hook1) != (CelLogerHookItem *)&(g_logger.hook_list->tail))
    {
        hook1 = (CelLogerHookItem *)hook2->item.next;
        if (strncmp(hook2->name, name, CEL_KNLEN) == 0)
        {
            cel_list_remove(g_logger.hook_list, (CelListItem *)hook2);
            cel_free(hook2);
        } 
    }
    return -1;
}

static int cel_logger_write(CelLogMsg *log_msg)
{
    CelLogerHookItem *hook1, *hook2;

    hook1 = (CelLogerHookItem *)g_logger.hook_list->head.next;
    while ((hook2 = hook1) != (CelLogerHookItem *)&(g_logger.hook_list->tail))
    {
        //puts(hook2->name);
        hook1 = (CelLogerHookItem *)hook2->item.next;
        hook2->write_func(log_msg, hook2->user_data);
    }
    return 0;
}

static int cel_logger_flush(void)
{
    CelLogerHookItem *hook1, *hook2;

    hook1 = (CelLogerHookItem *)g_logger.hook_list->head.next;
    while ((hook2 = hook1) != (CelLogerHookItem *)&(g_logger.hook_list->tail))
    {
        //puts(hook2->name);
        hook1 = (CelLogerHookItem *)hook2->item.next;
        if (hook2->flush_func != NULL)
            hook2->flush_func(hook2->user_data);
    }
    return 0;
}

int cel_log_flush(void)
{
    unsigned int read_pos;
    int n_msg = 0;
    CelLogMsg *log_msg;

    /* Thread mutex lock */
    if (!cel_multithread_mutex_trylock(CEL_MT_MUTEX_LOG_READ))
        return 0;
    if (g_logger.msg_buf == NULL)
    {
        if ((g_logger.msg_buf = (CelLogMsg *)_cel_sys_malloc(
            sizeof(CelLogMsg) * CEL_LOGBUF_NUM)) == NULL)
        {
            /* Thread mutex unlock */
            cel_multithread_mutex_unlock(CEL_MT_MUTEX_LOG_READ);
            return -1;
        }
        g_logger.is_flush = TRUE;
    }
    while ((read_pos = (g_logger.read_pos + 1)) != g_logger.write_pos)
    {
        log_msg = &(g_logger.msg_buf[read_pos % CEL_LOGBUF_NUM]);
        /* Log message is writing, must wait.*/
        if (log_msg->level == CEL_LOGLEVEL_UNDEFINED)
        {
            _tprintf(_T("wait pos = %d, write pos %d.\t\n"), 
                g_logger.write_pos, g_logger.read_pos);
            break;
        }
        //_tprintf(_T("read pos = %d\r\n"), read_pos % CEL_LOGBUF_NUM);
        if (g_logger.hook_list == NULL)
            cel_logmsg_puts(log_msg, NULL);
        else 
            cel_logger_write(log_msg);
        log_msg->level = CEL_LOGLEVEL_UNDEFINED;
        g_logger.read_pos++;
        n_msg++;
    }
    if (n_msg > 0 && g_logger.hook_list != NULL)
        cel_logger_flush();
    /* Thread mutex unlock */
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_LOG_READ);
    //_tprintf("n_msg = %d\r\n", n_msg);

    return n_msg;
}

static CelLogSpecific *_cel_log_specific_get()
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

static __inline CelLogMsg *_cel_logmsg_get()
{
    CelDateTime msg_time;
    CelLogSpecific *log_specific;
    unsigned int write_pos;
    CelLogMsg *log_msg;

    cel_datetime_init_now(&msg_time);
    log_specific = _cel_log_specific_get();
    //_tprintf(_T("Thread %p specific %p\r\n"), 
    //    cel_thread_self(), log_specific);
    if (g_logger.is_flush)
    {
        /* Thread mutex lock */
        cel_multithread_mutex_lock(CEL_MT_MUTEX_LOG_WRITE);
        write_pos = g_logger.write_pos++;
        while (write_pos == (g_logger.read_pos + CEL_LOGBUF_NUM))
        {
            //_tprintf(_T("wait\r\n"));
            usleep(1000);
            if (write_pos != (g_logger.read_pos + CEL_LOGBUF_NUM))
                break;
            cel_log_flush();
        }
        cel_multithread_mutex_unlock(CEL_MT_MUTEX_LOG_WRITE);
        //printf("start write pos = %d\r\n", write_pos);
        log_msg = &(g_logger.msg_buf[write_pos % CEL_LOGBUF_NUM]);
    }
    else
    {
        log_msg = &(log_specific->msg);
    }
    log_msg->timestamp = msg_time;
    log_msg->hostname = g_logger.hostname;
    log_msg->processname = g_logger.processname;
    log_msg->pid = log_specific->pid;

    return log_msg;
}

static __inline void _cel_logmsg_put(CelLogMsg *log_msg)
{
    if (!(g_logger.is_flush))
    {
        /* Thread mutex lock */
        cel_multithread_mutex_lock(CEL_MT_MUTEX_LOG_WRITE);
        if (g_logger.hook_list == NULL)
            cel_logmsg_puts(log_msg, NULL);
        else
            cel_logger_write(log_msg);
        if (g_logger.hook_list != NULL)
            cel_logger_flush();
        cel_multithread_mutex_unlock(CEL_MT_MUTEX_LOG_WRITE);
    }
}

int _cel_log_puts(CelLogFacility facility, CelLogLevel level, const TCHAR *str)
{
    CelLogMsg *log_msg;

    if (g_logger.level[facility] < level)
        return -1;
    log_msg = _cel_logmsg_get();
    _tcsncpy(log_msg->content, str, CEL_LOGCONTENT_LEN);
    log_msg->facility = facility;
    log_msg->level = level;
    _cel_logmsg_put(log_msg);

    return 0;
}

int _cel_log_vprintf(CelLogFacility facility, CelLogLevel level, 
                     const TCHAR *fmt, va_list ap)
{
    CelLogMsg *log_msg;

    //_tprintf(_T("%d %d\r\n"), g_logger.level[CEL_DEFAULT_FACILITY],  level);
    if (g_logger.level[facility] < level)
        return -1;
    log_msg = _cel_logmsg_get();
    _vsntprintf(log_msg->content, CEL_LOGCONTENT_LEN, fmt, ap);
    //puts(log_msg->content);
    log_msg->facility = facility;
    log_msg->level = level;
    _cel_logmsg_put(log_msg);

    return 0;
}

int _cel_log_printf(CelLogFacility facility, CelLogLevel level, 
                    const TCHAR *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    _cel_log_vprintf(facility, CEL_LOGLEVEL_DEBUG, fmt, args);
    va_end(args);

    return 0;
}

int _cel_log_hexdump(CelLogFacility facility, CelLogLevel level, 
                     const BYTE *p, size_t len)
{
    CelLogMsg *log_msg;

    if (g_logger.level[facility] < level)
        return -1;
    log_msg = _cel_logmsg_get();
    cel_hexdump(log_msg->content, CEL_LOGCONTENT_LEN, p, len);
    log_msg->level = level;
    log_msg->facility = facility;
    _cel_logmsg_put(log_msg);

    return 0;
}

int _cel_log_debug(CelLogFacility facility, const TCHAR *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    _cel_log_vprintf(facility, CEL_LOGLEVEL_DEBUG, fmt, args);
    va_end(args);

    return 0;
}

int _cel_log_info(CelLogFacility facility, const TCHAR *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    _cel_log_vprintf(facility, CEL_LOGLEVEL_INFO, fmt, args);
    va_end(args);

    return 0;
}

int _cel_log_notice(CelLogFacility facility, const TCHAR *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    _cel_log_vprintf(facility, CEL_LOGLEVEL_NOTICE, fmt, args);
    va_end(args);

    return 0;
}

int _cel_log_warning(CelLogFacility facility, const TCHAR *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    _cel_log_vprintf(facility, CEL_LOGLEVEL_WARNING, fmt, args);
    va_end(args);

    return 0;
}

int _cel_log_err(CelLogFacility facility, const TCHAR *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    _cel_log_vprintf(facility, CEL_LOGLEVEL_ERR, fmt, args);
    va_end(args);

    return 0;
}

int _cel_log_crit(CelLogFacility facility, const TCHAR *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    _cel_log_vprintf(facility, CEL_LOGLEVEL_CRIT, fmt, args);
    va_end(args);

    return 0;
}

int _cel_log_alert(CelLogFacility facility, const TCHAR *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    _cel_log_vprintf(facility, CEL_LOGLEVEL_ALERT, fmt, args);
    va_end(args);

    return 0;
}

int _cel_log_emerg(CelLogFacility facility, const TCHAR *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    _cel_log_vprintf(facility, CEL_LOGLEVEL_EMERG, fmt, args);
    va_end(args);

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
    int msg_day;
    long lfile;
    TCHAR strtime[26], filename[15], file_path[CEL_PATHLEN];

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
    cel_datetime_strfltime(&(msg->timestamp), strtime, 26, _T("%b %d %X"));
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
    TCHAR strtime[26];

    cel_datetime_strfltime(&(msg->timestamp), strtime, 26, _T("%b %d %X"));
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
