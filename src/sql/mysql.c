/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2016 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/sql/mysql.h"
#include "cel/allocator.h"
#include "cel/log.h"
#include "cel/error.h"
#include "cel/thread.h"
#ifdef _CEL_UNIX
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <fcntl.h>
#endif
#ifdef _CEL_WIN
#include <ws2tcpip.h>
#endif
#include <mysql/mysql.h>

/* Debug defines */
#define Debug(args)   /* cel_log_debug args */
#define Warning(args) CEL_SETERRSTR(args)/* cel_log_warning args */
#define Err(args)   CEL_SETERRSTR(args)/* cel_log_err args */

struct st_mysql_ex
{
    struct st_mysql mysql;
    char *host, *user, *passwd, *db;
    unsigned int port;
};

#define CEL_MYSQL_STRDUP(ptr, value)  if ((ptr) != value) { \
        if ((ptr) != NULL) { cel_free(ptr); } ptr = cel_strdup(value); }
#define CEL_MYSQL_TCSDUP(ptr, value) if ((ptr) != value) { \
        if ((ptr) != NULL) { cel_free(ptr); } ptr = cel_tcsdup(value); }
#define CEL_MYSQL_FREE(ptr) if ((ptr) != NULL) { cel_free(ptr); (ptr) = NULL; }

CelMysqlCon *cel_mysqlcon_new(const char *dbhost, unsigned int dbport, 
                              const char *dbname, 
                              const char *dbuser, const char *dbpswd)
{
    CelMysqlCon *con;

    if ((con = (CelMysqlCon *)cel_malloc(sizeof(CelMysqlCon))) == NULL)
    {
        return NULL;
    }
    memset(con, 0, sizeof(CelMysqlCon));
    CEL_MYSQL_STRDUP(con->host, dbhost);
    CEL_MYSQL_STRDUP(con->user, dbuser);
    CEL_MYSQL_STRDUP(con->passwd, dbpswd);
    CEL_MYSQL_STRDUP(con->db, dbname);
    con->port = dbport;

    return (cel_mysqlcon_open(con) == 0 ? con : NULL);
}

void cel_mysqlcon_free(CelMysqlCon *con)
{
    if (con == NULL) 
        return;
    mysql_close(&con->mysql);

    CEL_MYSQL_FREE(con->host);
    CEL_MYSQL_FREE(con->user);
    CEL_MYSQL_FREE(con->passwd);
    CEL_MYSQL_FREE(con->db);

    cel_free(con);
}

int cel_mysqlcon_open(CelMysqlCon *con)
{
    my_bool reconnect = 1;

    if (mysql_init(&con->mysql) == NULL)
    {
        Err((_T("mysql_init:%s."), mysql_error(&con->mysql)));
        return -1;
    }
    if (mysql_options(&con->mysql, MYSQL_SET_CHARSET_NAME, "utf8") != 0
        || mysql_options(&con->mysql, MYSQL_OPT_RECONNECT, &reconnect) != 0)
    {
        Err((_T("mysql_options:%s."), mysql_error(&con->mysql)));
        return -1;
    }
    if (mysql_real_connect(&con->mysql, 
        con->host, con->user, con->passwd, 
        con->db, con->port, NULL, 0) == NULL)
    {
        Err((_T("mysql_real_connect:%s."), mysql_error(&con->mysql)));
        return -1;
    }

    return 0;
}

void cel_mysqlcon_close(CelMysqlCon *con)
{
    mysql_close(&con->mysql);
}

long cel_mysqlcon_execute_nonequery(CelMysqlCon *con, const char *sqlstr)
{
    CelMysqlRes *res;
    long affected_rows;

    if (con == NULL) return -1;

    if (mysql_real_query(
        &con->mysql, sqlstr, (unsigned long)strlen(sqlstr)) != 0)
    {
        Err((_T("mysql_real_query:%s."), mysql_error(&con->mysql)));
        mysql_close(&con->mysql);
        if (cel_mysqlcon_open(con) == -1)
            return -1;
        if (mysql_real_query(
            &con->mysql, sqlstr, (unsigned long)strlen(sqlstr)) != 0)
        {
            Err((_T("mysql_real_query:%s."), mysql_error(&con->mysql)));
            return -1;
        }
    }
    if ((res = mysql_store_result(&con->mysql)) != NULL)
    {
        mysql_free_result(res);
    }
    affected_rows = (long)mysql_affected_rows(&con->mysql);
    //printf(_T("affected_rows %d\r\n"), affected_rows);

    return affected_rows;
}

CelMysqlRes *cel_mysqlcon_execute_onequery(CelMysqlCon *con, const char *sqlstr)
{
    if (con == NULL) return NULL;

    if (mysql_real_query(
        &con->mysql, sqlstr, (unsigned long)strlen(sqlstr)) != 0)
    {
        Err((_T("mysql_real_query:%s."), mysql_error(&con->mysql)));
        mysql_close(&con->mysql);
        if (cel_mysqlcon_open(con) == -1)
        {
            return NULL;
        }
        if (mysql_real_query(
            &con->mysql, sqlstr, (unsigned long)strlen(sqlstr)) != 0)
        {
            Err((_T("mysql_real_query:%s."),mysql_error(&con->mysql)));
            return NULL;
        }
    }
    return mysql_store_result(&con->mysql);
}

CelMysqlRes *cel_mysqlcon_execute_query(CelMysqlCon *con, const char *sqlstr)
{
    if (con == NULL) return NULL;

    if (mysql_real_query(
        &con->mysql, sqlstr, (unsigned long)strlen(sqlstr)) != 0)
    {
        Err((_T("mysql_real_query:%s."),mysql_error(&con->mysql)));
        mysql_close(&con->mysql);
        if (cel_mysqlcon_open(con) == -1)
        {
            return NULL;
        }
        if (mysql_real_query(
            &con->mysql, sqlstr, (unsigned long)strlen(sqlstr)) != 0)
        {
            Err((_T("mysql_real_query:%s."),mysql_error(&con->mysql)));
            return NULL;
        }
    }
    return mysql_store_result(&con->mysql);
}

long cel_mysqlres_rows(CelMysqlRes *res)
{
    return (long)mysql_num_rows(res);
}

int cel_mysqlres_cols(CelMysqlRes *res)
{
    return mysql_num_fields(res);
}

int cel_mysqlres_get_int(CelMysqlRes *res, int col_index, int *value)
{
    if (res->lengths[col_index] != 0) {
        *value = atoi(res->current_row[col_index]);
        return 0;
    }
    *value = 0;
    return -1;
}

int cel_mysqlres_get_long(CelMysqlRes *res, int col_index, long *value)
{
    if (res->lengths[col_index] != 0) {
        *value = atol(res->current_row[col_index]);
        return 0;
    }
    *value = 0;
    return -1;
}

int cel_mysqlres_get_string(CelMysqlRes *res, int col_index,
                            char *value, size_t size)
{
    if (res->lengths[col_index] != 0) {
        strncpy(value, res->current_row[col_index], size);
        return 0;
    }
    value[0] = '\0';
    return -1;
}

unsigned long *cel_mysqlres_fetch_lengths(CelMysqlRes *res)
{
    return mysql_fetch_lengths(res);
}

CelMysqlRow cel_mysqlres_fetch_row(CelMysqlRes *res)
{
    return mysql_fetch_row(res);
}

CelMysqlField *cel_mysqlres_fetch_field(CelMysqlRes *res)
{
    return mysql_fetch_field(res);
}

const char *cel_mysqlres_field_name(CelMysqlRes *res, 
                                    unsigned int field_offset)
{
    if ((res->fields == NULL && mysql_fetch_field(res) == NULL)
        || res->field_count > field_offset)
    {
        Err((_T("Mysql fetch field error.")));
        return NULL;
    }
    return res->fields[field_offset].name;
}

int cel_mysqlres_field_len(CelMysqlRes *res, unsigned int field_offset)
{
    if ((res->fields == NULL 
        && mysql_fetch_field(res) == NULL)
        || res->field_count > field_offset)
    {
        Err((_T("Mysql fetch field error.")));
        return -1;
    }
    return res->fields[field_offset].name_length;
}

/* offset: 0 - (rows - 1)*/
int cel_mysqlres_rows_seek(CelMysqlRes *res, 
                           unsigned long long offset, CelMysqlRow *row)
{
    MYSQL_ROW_OFFSET row_offset;

    mysql_data_seek(res, offset);
    if ((row_offset = mysql_row_tell(res)) != NULL)
    {
        *row = row_offset->data;
        return 0;
    }
    Err((_T("Mysql data seek failed, offset %lld out of range"), offset));

    return -1;
}

void cel_mysqlres_free(CelMysqlRes *res)
{
    if (res == NULL) 
        return;
    mysql_free_result(res);
    res = NULL;
}

int cel_mysqlcon_execute_onequery_results(CelMysqlCon *con, 
                                          const char *sqlstr, 
                                          CelMysqlRowEachFunc each_func, 
                                          void *user_data)
{
    int ret, cols;
    CelMysqlRes *res;
    CelMysqlRow row;

    if ((res = cel_mysqlcon_execute_onequery(con, sqlstr)) == NULL)
        return -1;
    cols = cel_mysqlres_cols(res);
    if ((row = cel_mysqlres_fetch_row(res)) == NULL)
    {
        if ((ret = each_func((void **)row, cols, user_data)) == 1)
        {
            cel_mysqlres_free(res);
            return 0;
        }
        else if (ret == -1)
        {
            cel_mysqlres_free(res);
            return -1;
        }
    }
    cel_mysqlres_free(res);

    return 0;
}

int cel_mysqlcon_execute_query_results(CelMysqlCon *con, const char *sqlstr, 
                                       CelMysqlRowEachFunc each_func, 
                                       void *user_data)
{
    int ret, cols;
    CelMysqlRes *res;
    CelMysqlRow row;

    if ((res = cel_mysqlcon_execute_query(con, sqlstr)) == NULL)
        return -1;
    cols = cel_mysqlres_cols(res);
    while ((row = cel_mysqlres_fetch_row(res)) == NULL)
    {
        if ((ret = each_func((void **)row, cols, user_data)) == 1)
        {
            cel_mysqlres_free(res);
            return 0;
        }
        else if (ret == -1)
        {
            cel_mysqlres_free(res);
            return -1;
        }
    }
    cel_mysqlres_free(res);

    return 0;
}