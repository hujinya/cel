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
#ifndef __CEL_SQL_DBCONPOOL_H__
#define __CEL_SQL_DBCONPOOL_H__

#include "cel/multithread.h"
#include "cel/sql/_dbcon.h"
#include "cel/list.h"

typedef struct _CelDbConPool
{
    int init_cons;
    int max_cons, min_cons;
    CelList free_list;
    CelList active_list;
    CelMutex *mutex;
}CelDbConPool;

#ifdef __cplusplus
extern "C" {
#endif

int cel_dbconpool_init(CelDbConPool *dbcp);
void cel_dbconpool_destroy(CelDbConPool *dbcp);

CelDbConPool *cel_dbconpool_new();
void cel_dbconpool_free(CelDbConPool *dbcp);

CelDbCon *cel_dbconpool_get_connection(CelDbConPool *dbcp);
void cel_dbconpool_return_connection(CelDbConPool *dbcp, CelDbCon *dbc);

#ifdef __cplusplus
}
#endif

#endif