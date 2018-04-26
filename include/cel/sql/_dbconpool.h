/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
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