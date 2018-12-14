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
#define _XOPEN_SOURCE 500 /* or any value > 500 */
#include "cel/sys/service.h"
#ifdef _CEL_UNIX
#include "cel/error.h"
#include "cel/log.h"
#include "cel/file.h"
#include "cel/process.h"
#include "cel/sys/envinfo.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

int os_service_pidfile_create(const TCHAR *name)
{
    static CelProcessStat ps1, ps2;
    FILE *fp;
    pid_t ppid, pid;

    if((fp = _tfopen(name, _T("w"))) == NULL)
    {
        _ftprintf(stderr, _T("Can't open %s (errno %d %s)\n"), 
            name, errno, strerror(errno));
        return -1;
    }
    pid = getpid();
    ppid = getppid();
    cel_process_getbypid(&ps2, pid);
    cel_process_getbypid(&ps1, ppid);
    if (_tcscmp(ps1.name, ps2.name) != 0)
        _ftprintf(fp, _T("%d\n"), pid);
    else
        _ftprintf(fp, _T("%d\n%d\n"), pid, ppid);
    fclose(fp);
    return(0);
}

int os_service_pidfile_exist(const TCHAR *name)
{
    FILE *fp;
    pid_t ppid = 0, pid = 0;

    if ((fp = _tfopen(name, _T("r"))) == NULL)
        return 0;
    fscanf(fp, "%d\n%d", &pid, &ppid);
    fclose(fp);
    //_tprintf(_T("ppid %d, getppid() = %d, pid %d \r\n"),
    //    ppid, getppid(), pid);
    if ((ppid <= 0 
        || (ppid == getppid() || kill(ppid, 0) == -1)) 
        && (pid <= 0 || kill(pid, 0) == -1))
    {
        fprintf(stderr, "Remove a stale pid file %s\n", name);
        unlink(name);
        return 0;
    }
    return -1;
}

BOOL os_service_is_running(const TCHAR *name)
{
    TCHAR file_name[CEL_PATHLEN];

     _sntprintf(file_name, CEL_PATHLEN,
        _T("/var/run/%s.pid"), cel_filename(cel_modulefile()));
    if(os_service_pidfile_exist(file_name) == -1)
        return TRUE;
    os_service_pidfile_create(file_name);
    return FALSE;
}

BOOL os_service_stop(const TCHAR *name)
{
    FILE *fp;
    pid_t ppid = 0, pid = 0;
    TCHAR file_name[CEL_PATHLEN];

    _sntprintf(file_name, CEL_PATHLEN,
        _T("/var/run/%s.pid"), cel_filename(cel_modulefile()));

    if ((fp = _tfopen(file_name, _T("r"))) == NULL)
        return 0;
    fscanf(fp, "%d\n%d", &pid, &ppid);
    fclose(fp);
    //_tprintf(_T("pid %d ppid %d\r\n"), pid, ppid);
    if (ppid > 0 
        && kill(ppid, SIGQUIT) == 0)
        return TRUE;
    if (pid > 0
        && kill(pid, SIGQUIT) == -1)
        return TRUE;
    return FALSE;
}

BOOL os_service_reload(const TCHAR *name)
{
    FILE *fp;
    pid_t ppid = 0, pid = 0;
    TCHAR file_name[CEL_PATHLEN];

    _sntprintf(file_name, CEL_PATHLEN,
        _T("/var/run/%s.pid"), cel_filename(cel_modulefile()));

    if ((fp = _tfopen(file_name, _T("r"))) == NULL)
        return 0;
    fscanf(fp, "%d\n%d", &pid, &ppid);
    fclose(fp);
    //_tprintf(_T("pid %d ppid %d\r\n"), pid, ppid);
    if (ppid > 0 
        && kill(ppid, SIGHUP) == 0)
        return TRUE;
    if (pid > 0
        && kill(pid, SIGHUP) == -1)
        return TRUE;
    return FALSE;
}

static void _os_service_signal_handler(int signo);

static OsServiceEntry sc_entry;
static int singal_process = 0;
static int singal_quit = 0;
static int singal_terminate = 0;
static int singal_reconfigure = 0;
static int singal_child = 0;
static int singal_live = 0;
static CelSignals sc_signals[] = 
{
    { SIGHUP, "sighup", _os_service_signal_handler },   //1
    { SIGQUIT, "sigquit", _os_service_signal_handler }, //3
    { SIGPIPE, "sigpipe", SIG_IGN },  //13
    { SIGTERM, "sigterm", _os_service_signal_handler }, //15
    { SIGCHLD, "sigchld", _os_service_signal_handler }, //17 
    { 0, NULL, NULL }
};

void _os_service_signal_handler(int signo)
{
    pid_t pid;
    int status;

    /*_tprintf(_T("singal_process = %d, signo = %d\r\n"), 
        singal_process, signo);*/
    switch (singal_process)
    {
    case 0:
        switch (signo)
        {
        case SIGQUIT:
            singal_quit = 1;
            break;
        case SIGTERM:
            singal_terminate = 1;
            break;
        case SIGHUP:
            singal_reconfigure = 1;
            break;
        case SIGCHLD:
            singal_child = 1;
            while((pid = waitpid(0, &status, WNOHANG)) > 0)
            {
                //_tprintf(_T("Child %d exit, status = %d.\r\n"), pid, status);
                if (WIFEXITED(status) && WEXITSTATUS(status) == 1)
                {
                    //_tprintf(_T("Child %d quit.\r\n"), pid);
                    singal_quit = 1;
                    singal_live = 0;
                }
            }
            break;
        default:
            break;
        }
        break;
    case 1:
        switch (signo)
        {
        case SIGQUIT:
        case SIGTERM:
            sc_entry.on_stop();
        default:
            break;
        }
        break;
    default:
        break;
    }
}

int _os_service_entry_child(OsServiceEntry *sc_entry, int argc, char **argv)
{
    pid_t pid;
    sigset_t set;

    if ((pid = fork()) < 0) 
        return -1; /**< Create child fork error */
    if (pid == 0)
    {
        /* Service running... */
        singal_process = 1;
        sigemptyset(&set);
        sigprocmask(SIG_SETMASK, &set, NULL);

        sc_entry->on_start(argc, argv);
        return 0;
    }
    else
    {
        return pid;
    }
}

int _os_service_entry_dispatch(OsServiceEntry *sc_entry, 
                               CelServiceStartMode start_mode,
                               int argc, char **argv)
{
    pid_t pid;
    int null_fd;
    sigset_t set;

    if (start_mode == CEL_SERVICE_START_DAEMON)
    {
        if ((pid = fork()) < 0)
            return 1;   /**< Create child fork error */
        if (pid > 0)
            exit(0);    /**< Parent process exit */

        setsid();       /**< Set session leader */

        //chdir("/");
        umask(0);
        if ((null_fd = open("/dev/null", 0)) == -1
            || dup2(null_fd, STDERR_FILENO) == -1
            || dup2(null_fd, STDOUT_FILENO) == -1 
            || dup2(null_fd, STDIN_FILENO) == -1)
            return 1;
        close(null_fd);
    }
    cel_signals_init(sc_signals);
    if (start_mode ==  CEL_SERVICE_START_DEBUG)
    {
        sc_entry->on_start(argc, argv);
        return 0;
    }

    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGCHLD);
    sigaddset(&set, SIGPIPE);

    sigprocmask(SIG_BLOCK, &set, NULL);
    sigemptyset(&set);

    if ((pid = _os_service_entry_child(sc_entry, argc, argv)) == 0)
        return 0;
    singal_live = 1;
    while (TRUE)
    {
        sigsuspend(&set);

        if (singal_child == 1)
        {
            //puts("singal_child");
            singal_child = 0;
            if (singal_live == 0 
                && (singal_quit == 1 || singal_terminate == 1))
                break;
            if ((pid = _os_service_entry_child(sc_entry, argc, argv)) == 0)
                return 0;
        }
        if (singal_quit == 1)
        {
            //puts("singal_quit");
            singal_live = 0;
            kill(pid, SIGQUIT);
        }
        if (singal_terminate == 1)
        {
            //puts("singal_terminate");
            singal_live = 0;
            kill(pid, SIGTERM);
        }
        if (singal_reconfigure == 1)
        {
            //puts("singal_reconfigure");
            singal_reconfigure = 0;
            singal_live = 1;
            kill(pid, SIGQUIT);
        }
    }
    return 0;
}

OsServiceEntry *os_service_entry_create(const TCHAR *name,
                                        CelMainFunc on_start, 
                                        CelVoidFunc on_stop)
{
    sc_entry.on_start = on_start;
    sc_entry.on_stop = on_stop;

    return &sc_entry;
}
#endif