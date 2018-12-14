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
#include "cel/sql/sqlcon.h"
#include "cel/log.h"
#include "cel/error.h"

static CelSqlConClass mysql_kclass = 
{
    (CelSqlConOpenFunc)cel_mysqlcon_open,
    (CelSqlConCloseFunc)cel_mysqlcon_close,
    (CelSqlConExecuteNonequeryFunc)cel_mysqlcon_execute_nonequery,
    (CelSqlConExecuteOnequeryFunc)cel_mysqlcon_execute_onequery,
    (CelSqlConExecuteQueryFunc)cel_mysqlcon_execute_query,
    (CelSqlResRowsFunc)cel_mysqlres_rows,
    (CelSqlResColsFunc)cel_mysqlres_cols,
    (CelSqlResFetchLengthsFunc)cel_mysqlres_fetch_lengths,
    (CelSqlResFetchRowFunc)cel_mysqlres_fetch_row,
    (CelSqllResFetchFieldFunc)cel_mysqlres_fetch_field,
    (CelSqlResFreeFunc)cel_mysqlres_free
};

int cel_sqlcon_init(CelSqlCon *con, CelSqlConType type,
                    const char *host, unsigned int port, 
                    const char *name, 
                    const char *user, const char *pswd)
{
    memset(con, 0, sizeof(CelSqlCon));
    CEL_PTR_STRDUP(con->host, host);
    CEL_PTR_STRDUP(con->user, user);
    CEL_PTR_STRDUP(con->passwd, pswd);
    CEL_PTR_STRDUP(con->db, name);
    con->port = port;
    switch (type)
    {
    case CEL_SQLCON_MYSQL:
        con->sqlstr[0] = '\0';
        con->kclass = &mysql_kclass;
        return cel_sqlcon_open(con);
    default:
        break;
    }
    return -1;
}

void cel_sqlcon_destroy(CelSqlCon *con)
{
    con->kclass->con_close(con);
    CEL_PTR_FREE(con->host);
    CEL_PTR_FREE(con->user);
    CEL_PTR_FREE(con->passwd);
    CEL_PTR_FREE(con->db);
}

CelSqlCon *cel_sqlcon_new(CelSqlConType type, 
                          const char *host, unsigned int port, 
                          const char *name, 
                          const char *user, const char *pswd)
{
    CelSqlCon *con;

    if ((con = (CelSqlCon *)cel_malloc(sizeof(CelSqlCon))) != NULL)
    {
        if (cel_sqlcon_init(con, type, host, port, name, user, pswd) == 0)
            return con;
        cel_free(con);
    }

    return NULL;
}

void cel_sqlcon_free(CelSqlCon *con)
{
    cel_sqlcon_destroy(con);
    cel_free(con);
}

long cel_sqlcon_execute_nonequery(CelSqlCon *con, const char *fmt, ...)
{
    long affected_rows;
    va_list args;

    va_start(args, fmt);
    con->len = _vsntprintf(con->sqlstr, 1024, fmt, args);
    va_end(args);
    if ((affected_rows = con->kclass->con_execute_nonequery(
        con, con->sqlstr, con->len)) == -1)
    {
        cel_sqlcon_close(con);
        if (cel_sqlcon_open(con) == -1)
            return -1;
        return con->kclass->con_execute_nonequery(con, con->sqlstr, con->len);
    }
    return affected_rows;
}

CelSqlRes *_cel_sqlcon_execute_onequery(CelSqlCon *con)
{
    CelSqlRes *res;
    if ((res = cel_malloc(sizeof(CelSqlRes))) != NULL)
    {
        if ((res->_res = con->kclass->con_execute_onequery(
            con, con->sqlstr, con->len)) == NULL)
        {
            cel_sqlcon_close(con);
            if (cel_sqlcon_open(con) == -1)
                return NULL;
        }
        if ((res->_res = con->kclass->con_execute_onequery(
            con, con->sqlstr, con->len)) != NULL)
        {
            res->kclass = con->kclass;
            return res;
        }
        cel_free(res);
    }
    return NULL;
}

CelSqlRes *_cel_sqlcon_execute_query(CelSqlCon *con)
{
    CelSqlRes *res;
    if ((res = cel_malloc(sizeof(CelSqlRes))) != NULL)
    {
        //printf("_cel_sqlcon_execute_query %s %d\r\n", con->sqlstr, con->len);
        if ((res->_res = con->kclass->con_execute_query(
            con, con->sqlstr, con->len)) == NULL)
        {
            cel_sqlcon_close(con);
            if (cel_sqlcon_open(con) == -1)
                return NULL;
        }
        //printf("_cel_sqlcon_execute_query2 %s %d\r\n", con->sqlstr, con->len);
        if ((res->_res = con->kclass->con_execute_query(
            con, con->sqlstr, con->len)) != NULL)
        {
            res->kclass = con->kclass;
            return res;
        }
        cel_free(res);
    }
    return NULL;
}

int cel_sqlcon_execute_onequery_results(CelSqlCon *con, 
                                        CelSqlRowEachFunc each_func,
                                        void *user_data,
                                        const char *fmt, ...)
{
    int ret, cols;
    CelSqlRes *res;
    CelSqlRow row;
    va_list args;

    va_start(args, fmt);
    con->len = _vsntprintf(con->sqlstr, 1024, fmt, args);
    va_end(args);
    if ((res = _cel_sqlcon_execute_onequery(con)) == NULL)
        return -1;
    cols = cel_sqlres_cols(res);
    if ((row = cel_sqlres_fetch_row(res)) == NULL)
    {
        if ((ret = each_func((void **)row, cols, user_data)) == 1)
        {
            cel_sqlres_free(res);
            return 0;
        }
        else if (ret == -1)
        {
            cel_sqlres_free(res);
            return -1;
        }
    }
    cel_sqlres_free(res);

    return 0;
}

int cel_sqlcon_execute_query_results(CelSqlCon *con,
                                     CelSqlRowEachFunc each_func, 
                                     void *user_data,
                                     const char *fmt, ...)
{
    int ret, cols;
    CelSqlRes *res;
    CelSqlRow row;
    va_list args;

    va_start(args, fmt);
    _vsntprintf(con->sqlstr, 1024, fmt, args);
    va_end(args);
    if ((res = _cel_sqlcon_execute_query(con)) == NULL)
        return -1;
    cols = cel_sqlres_cols(res);
    while ((row = cel_sqlres_fetch_row(res)) == NULL)
    {
        if ((ret = each_func((void **)row, cols, user_data)) == 1)
        {
            cel_sqlres_free(res);
            return 0;
        }
        else if (ret == -1)
        {
            cel_sqlres_free(res);
            return -1;
        }
    }
    cel_sqlres_free(res);

    return 0;
}
