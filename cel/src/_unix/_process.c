/**
 * CEL(C Extension Library)
 * Copyright (C)2008 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/process.h"
#ifdef _CEL_UNIX
#include "cel/file.h"
#include "cel/convert.h" /* cel_skiptoken(),cel_skipws() */
#include "cel/log.h"
#include "cel/error.h"

int cel_process_getbypid(CelProcessStat *proc_stat, CelProcessId pid)
{
    int i;
    FILE *fp;
    TCHAR *p;
    TCHAR buf[CEL_PATHLEN];
    
    _sntprintf(buf, CEL_PATHLEN - 1, _T("/proc/%d/stat"), pid);
    //puts(buf);
    if ((fp = cel_fopen(buf, _T("rt"))) == NULL)
        return -1;
    if (fgets(buf, CEL_PATHLEN - 1, fp) == NULL)
    {
        cel_fclose(fp);
        return -1;
    }              
    proc_stat->pid = _tstoi(buf); /**< Pid */
    p = cel_skiptoken(buf);       /**< Name */
    for (i = 0; p[i] != _T('\0'); i++)
    {
        if (p[i] == _T('('))
        {
            p += (i + 1);
            //puts(p);
            for (i = 0; p[i] != _T('\0'); i++)
            {
                if (p[i] == _T(')'))
                {
                    memcpy(proc_stat->name, p, i);
                    proc_stat->name[i] = _T('\0');
                    break;
                }
            }
            break;
        }
    }
    p = cel_skiptoken(p);   /**< State */
    p = cel_skipws(p);
    proc_stat->state = *p;
    p = cel_skiptoken(p);    /**< Pid */
    proc_stat->ppid = _tstoi(p);
    //p = cel_skiptoken(p); /**< Ppgrp */
    //p = cel_skiptoken(p); /**< Session */
    //p = cel_skiptoken(p); /**< Tty */
    //p = cel_skiptoken(p); /**< Tty pgrp */
    cel_fclose(fp);

    return 0;
}

typedef struct _CelProcessEachData
{
    CelProcessEachFunc each_func;
    void *user_data;
}CelProcessEachData;

static int cel_process_each(const TCHAR *proc_dir, const TCHAR *pid_dir,
                            const void *dirent, 
                            CelProcessEachData *data)
{
    CelProcessId pid;
    CelProcessStat proc;
    
    if (!CEL_ISDIGIT(pid_dir[0])
        || (pid = _tstoi(pid_dir)) <= 0)
        return 0;
    if (cel_process_getbypid(&proc, pid) == -1)
    {
        return (cel_sys_geterrno() == ENOENT ? 0 : -1);
    }
    return data->each_func(&proc, data->user_data);
}

int cel_process_foreach(CelProcessEachFunc each_func, void *user_data)
{
    CelProcessEachData data;

    data.each_func = each_func;
    data.user_data = user_data;
    return cel_foreachdir(_T("/proc/"), (CelDirFileEachFunc)cel_process_each, &data);
}

#endif
