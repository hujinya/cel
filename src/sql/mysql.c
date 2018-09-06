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

int cel_mysqlcon_init(CelMysqlCon *con,
                      const char *dbhost, unsigned int dbport, 
                      const char *dbname, 
                      const char *dbuser, const char *dbpswd)
{
    memset(con, 0, sizeof(CelMysqlCon));
    CEL_PTR_STRDUP(con->host, dbhost);
    CEL_PTR_STRDUP(con->user, dbuser);
    CEL_PTR_STRDUP(con->passwd, dbpswd);
    CEL_PTR_STRDUP(con->db, dbname);
    con->port = dbport;

    return cel_mysqlcon_open(con);
}

void cel_mysqlcon_destroy(CelMysqlCon *con)
{
    mysql_close(&con->mysql);
    CEL_PTR_FREE(con->host);
    CEL_PTR_FREE(con->user);
    CEL_PTR_FREE(con->passwd);
    CEL_PTR_FREE(con->db);
}

int cel_mysqlcon_open(CelMysqlCon *con)
{
    my_bool reconnect = 1;

    if (mysql_init(&con->mysql) == NULL)
    {
        CEL_ERR((_T("mysql_init:%s."), mysql_error(&con->mysql)));
        return -1;
    }
    if (mysql_options(&con->mysql, MYSQL_SET_CHARSET_NAME, "utf8") != 0
        || mysql_options(&con->mysql, MYSQL_OPT_RECONNECT, &reconnect) != 0)
    {
        CEL_ERR((_T("mysql_options:%s."), mysql_error(&con->mysql)));
        return -1;
    }
    if (mysql_real_connect(&con->mysql, 
        con->host, con->user, con->passwd, 
        con->db, con->port, NULL, 0) == NULL)
    {
        CEL_ERR((_T("mysql_real_connect:%s."), mysql_error(&con->mysql)));
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
        CEL_ERR((_T("mysql_real_query:%s."), mysql_error(&con->mysql)));
        mysql_close(&con->mysql);
        if (cel_mysqlcon_open(con) == -1)
            return -1;
        if (mysql_real_query(
            &con->mysql, sqlstr, (unsigned long)strlen(sqlstr)) != 0)
        {
            CEL_ERR((_T("mysql_real_query:%s."), mysql_error(&con->mysql)));
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
        CEL_ERR((_T("mysql_real_query:%s."), mysql_error(&con->mysql)));
        mysql_close(&con->mysql);
        if (cel_mysqlcon_open(con) == -1)
        {
            return NULL;
        }
        if (mysql_real_query(
            &con->mysql, sqlstr, (unsigned long)strlen(sqlstr)) != 0)
        {
            CEL_ERR((_T("mysql_real_query:%s."),mysql_error(&con->mysql)));
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
        CEL_ERR((_T("mysql_real_query:%s."),mysql_error(&con->mysql)));
        mysql_close(&con->mysql);
        if (cel_mysqlcon_open(con) == -1)
        {
            return NULL;
        }
        if (mysql_real_query(
            &con->mysql, sqlstr, (unsigned long)strlen(sqlstr)) != 0)
        {
            CEL_ERR((_T("mysql_real_query:%s."),mysql_error(&con->mysql)));
            return NULL;
        }
    }
    return mysql_store_result(&con->mysql);
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

const char *cel_mysqlres_field_name(CelMysqlRes *res, 
                                    unsigned int field_offset)
{
    if ((res->fields == NULL && mysql_fetch_field(res) == NULL)
        || res->field_count > field_offset)
    {
        CEL_ERR((_T("Mysql fetch field error.")));
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
        CEL_ERR((_T("Mysql fetch field error.")));
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
    CEL_ERR((_T("Mysql data seek failed, offset %lld out of range"), offset));

    return -1;
}


