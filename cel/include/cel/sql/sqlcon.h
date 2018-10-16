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
#ifndef __CEL_SQLCON_H__
#define __CEL_SQLCON_H__

#include "cel/sql/mssql.h"
#include "cel/sql/mysql.h"
#include "cel/allocator.h"
#include "cel/ringlist.h"
#include "cel/vstring.h"

typedef enum _CelSqlConType
{
    CEL_SQLCON_UNDEFINED,
    CEL_SQLCON_MYSQL,
    CEL_SQLCON_MSSQL
}CelSqlConType;

typedef struct _CelSqlCon CelSqlCon;
typedef struct _CelSqlRes CelSqlRes;
typedef struct _CelSqlField CelSqlField;
typedef char** CelSqlRow;

/* return 0 = continue;-1 = error;1 = break */
typedef int (* CelSqlRowEachFunc) (void **row, int cols, void *user_data);

typedef void (* CelSqlConDestroyFunc) (CelSqlCon *con);
typedef int  (* CelSqlConOpenFunc)(CelSqlCon *con);
typedef void (* CelSqlConCloseFunc)(CelSqlCon *con);
typedef long (* CelSqlConExecuteNonequeryFunc)(CelSqlCon *con, const char *sqlstr);
typedef CelSqlRes* (* CelSqlConExecuteOnequeryFunc)(CelSqlCon *con, const char *sqlstr);
typedef CelSqlRes* (* CelSqlConExecuteQueryFunc)(CelSqlCon *con, const char *sqlstr);
typedef long (* CelSqlResRowsFunc)(CelSqlRes *res);
typedef int (* CelSqlResColsFunc)(CelSqlRes *res);
typedef unsigned long* (* CelSqlResFetchLengthsFunc)(CelSqlRes *res);
typedef CelSqlRow (* CelSqlResFetchRowFunc)(CelSqlRes *res);
typedef CelSqlField* (*CelSqllResFetchFieldFunc)(CelSqlRes *res);
typedef void (* CelSqlResFreeFunc)(CelSqlRes *res);

typedef struct _CelSqlConClass
{
    CelSqlConDestroyFunc con_destroy;
    CelSqlConOpenFunc con_open;
    CelSqlConCloseFunc con_close;
    CelSqlConExecuteNonequeryFunc con_execute_nonequery;
    CelSqlConExecuteOnequeryFunc con_execute_onequery;
    CelSqlConExecuteQueryFunc con_execute_query;
    CelSqlResRowsFunc res_rows;
    CelSqlResColsFunc res_cols;
    CelSqlResFetchLengthsFunc res_fetch_lengths;
    CelSqlResFetchRowFunc res_fetch_row;
    CelSqllResFetchFieldFunc res_fetch_field;
    CelSqlResFreeFunc res_free;
}CelSqlConClass;

struct _CelSqlRes
{
    union {
        CelSqlRes *_res;
        CelMysqlRes *mysql_res;
    };
    CelSqlConClass *kclass;
};

struct _CelSqlField
{
    union {
        CelMysqlField mysql_field;
    };
    CelSqlConClass *kclass;
};

struct _CelSqlCon
{
    union {
        CelMysqlCon mysql_con;
    }con;
    char sqlstr[1024];
    CelVStringA sql_str;
    CelSqlConClass *kclass;
};

#ifdef __cplusplus
extern "C" {
#endif

int cel_sqlcon_init(CelSqlCon *con, CelSqlConType type,
                    const char *host, unsigned int port, 
                    const char *name, 
                    const char *user, const char *pswd);
void cel_sqlcon_destroy(CelSqlCon *con);

CelSqlCon *cel_sqlcon_new(CelSqlConType type, 
                          const char *host, unsigned int port, 
                          const char *name, 
                          const char *user, const char *pswd);
void cel_sqlcon_free(CelSqlCon *con);

static __inline int cel_sqlcon_open(CelSqlCon *con)
{
    return con->kclass->con_open(con);
}
static __inline void cel_sqlcon_close(CelSqlCon *con)
{
    con->kclass->con_close(con);
}

static __inline 
long cel_sqlcon_execute_nonequery(CelSqlCon *con, const char *sqlstr)
{
    return con->kclass->con_execute_nonequery(con, sqlstr);
}
CelSqlRes *cel_sqlcon_execute_onequery(CelSqlCon *con, const char *sqlstr);
CelSqlRes *cel_sqlcon_execute_query(CelSqlCon *con, const char *sqlstr);

static __inline long cel_sqlres_rows(CelSqlRes *res)
{
    return res->kclass->res_rows(res->_res);
}
static __inline int cel_sqlres_cols(CelSqlRes *res)
{
    return res->kclass->res_cols(res->_res);
}

static __inline unsigned long *cel_sqlres_fetch_lengths(CelSqlRes *res)
{
    return res->kclass->res_fetch_lengths(res->_res);
}
static __inline CelSqlRow cel_sqlres_fetch_row(CelSqlRes *res)
{
    return res->kclass->res_fetch_row(res->_res);
}
static __inline CelSqlField *cel_sqlres_fetch_field(CelSqlRes *res)
{
    return res->kclass->res_fetch_field(res->_res);
}

static __inline void cel_sqlres_free(CelSqlRes *res)
{
    if (res != NULL)
    {
        res->kclass->res_free(res->_res);
        cel_free(res);
    }
}

int cel_sqlcon_execute_onequery_results(CelSqlCon *con, const char *sqlstr,
                                        CelSqlRowEachFunc each_func, 
                                        void *user_data);
int cel_sqlcon_execute_query_results(CelSqlCon *con, const char *sqlstr, 
                                     CelSqlRowEachFunc each_func, 
                                     void *user_data);

#ifdef __cplusplus
}
#endif

#endif