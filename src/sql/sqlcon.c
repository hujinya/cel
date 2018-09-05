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
#undef _CEL_DEBUG
//#define _CEL_DEBUG
#include "cel/log.h"
#include "cel/error.h"

static CelSqlConClass mysql_kclass = 
{
    (CelSqlConDestroyFunc)cel_mysqlcon_destroy,
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
    switch (type)
    {
    case CEL_SQLCON_MYSQL:
        if (cel_mysqlcon_init(
            &(con->con.mysql_con), host, port, name, user, pswd) == -1)
            return -1;
        con->sqlstr[0] = '\0';
        con->kclass = &mysql_kclass;
        break;
    default:
        return -1;
    }
    return 0;
}

void cel_sqlcon_destroy(CelSqlCon *con)
{
    con->kclass->con_destroy(con);
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

CelSqlRes *cel_sqlcon_execute_onequery(CelSqlCon *con, const char *sqlstr)
{
    CelSqlRes *res;
    if ((res = cel_malloc(sizeof(CelSqlRes))) != NULL)
    {
        if ((res->_res = con->kclass->con_execute_onequery(con, sqlstr)) != NULL)
        {
            res->kclass = con->kclass;
            return res;
        }
        cel_free(res);
    }
    return NULL;
}

CelSqlRes *cel_sqlcon_execute_query(CelSqlCon *con, const char *sqlstr)
{
    CelSqlRes *res;
    if ((res = cel_malloc(sizeof(CelSqlRes))) != NULL)
    {
        if ((res->_res = con->kclass->con_execute_query(con, sqlstr)) != NULL)
        {
            res->kclass = con->kclass;
            return res;
        }
        cel_free(res);
    }
    return NULL;
}

int cel_sqlcon_execute_onequery_results(CelSqlCon *con, 
                                        const char *sqlstr, 
                                        CelSqlRowEachFunc each_func, 
                                        void *user_data)
{
    int ret, cols;
    CelSqlRes *res;
    CelSqlRow row;

    if ((res = cel_sqlcon_execute_onequery(con, sqlstr)) == NULL)
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

int cel_sqlcon_execute_query_results(CelSqlCon *con, const char *sqlstr, 
                                     CelSqlRowEachFunc each_func, 
                                     void *user_data)
{
    int ret, cols;
    CelSqlRes *res;
    CelSqlRow row;

    if ((res = cel_sqlcon_execute_query(con, sqlstr)) == NULL)
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
