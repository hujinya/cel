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
#ifndef __CEL_PROCESS_H__
#define __CEL_PROCESS_H__

#include "cel/config.h"
#include "cel/types.h"

#if defined(_CEL_UNIX)
#include "cel/_unix/_process.h"
#elif defined(_CEL_WIN)
#include "cel/_win/_process.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelProcessStat
{
    volatile TCHAR state;
    CelProcessId pid;
    CelProcessId ppid;
    TCHAR name[CEL_FNLEN];
}CelProcessStat;

typedef struct _CelProcessStatEx
{
    volatile TCHAR state;
    CelProcessId pid;
    CelProcessId ppid;
    TCHAR name[CEL_FNLEN];
}CelProcessStatEx;

typedef int (* CelProcessEachFunc) (CelProcessStat *proc_stat,
                                    void *user_data);

typedef struct _CelProcessAttr
{
    TCHAR command_line[CEL_CMDLLEN];
    TCHAR working_directory[CEL_PATHLEN];
}CelProcessAttr;

CelProcess cel_process_create(TCHAR *cmd);
void cel_process_terminate();
void cel_process_wait();

int cel_process_getbypid(CelProcessStat *proc_stat, CelProcessId pid);
int cel_process_foreach(CelProcessEachFunc each_func, void *user_data);

int cel_processmutex_init();
void cel_processmutex_destroy();
void cel_processmutex_new();
void cel_processmutex_free();

void cel_processmutex_lock();
int cel_processmutex_trylock();
void cel_processmutex_unlock(); 

int cel_processsemaphore_init();
void cel_processsemaphore_destroy();
void cel_processsemaphore_new();
void cel_processsemaphore_free();

void cel_processsemaphore_lock();
int cel_processsemaphore_trylock();
void cel_processsemaphore_unlock();

#ifdef __cplusplus
}
#endif

#endif
