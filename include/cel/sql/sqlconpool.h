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
#ifndef __CEL_SQLCONPOOL_H__
#define __CEL_SQLCONPOOL_H__

#include "cel/sql/sqlcon.h"
#include "cel/ringlist.h"

typedef struct _CelSqlConPool
{
    CelSqlConType type;
    char *host, *user, *passwd, *db;
    unsigned int port;
    CelAtomic n_conns;
    int max, min;
    CelRingList frees;
}CelSqlConPool;

#ifdef __cplusplus
extern "C" {
#endif

int cel_sqlconpool_init(CelSqlConPool *pool, CelSqlConType type,
                        const char *host, unsigned int port, 
                        const char *name, 
                        const char *user, const char *pswd,
                        int pool_min, int pool_max);
void cel_sqlconpool_destroy(CelSqlConPool *pool);

CelSqlConPool *cel_sqlconpool_new(CelSqlConType type, 
                                  const char *host, unsigned int port, 
                                  const char *name, 
                                  const char *user, const char *pswd,
                                  int pool_min, int pool_max);
void cel_sqlconpool_free(CelSqlConPool *pool);

CelSqlCon *cel_sqlconpool_get(CelSqlConPool *pool);
void cel_sqlconpool_return(CelSqlConPool *pool, CelSqlCon *con);


#ifdef __cplusplus
}
#endif

#endif
