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

int cel_mysqlcon_open(CelMysqlCon *con,
                      const char *host, unsigned int port, 
                      const char *name, 
                      const char *user, const char *pswd)
{
    my_bool reconnect = 1;

    if (mysql_init(con) == NULL)
    {
        CEL_SETERR((CEL_ERR_MYSQL, _T("mysql_init:%s."), mysql_error(con)));
        return -1;
    }
    if (mysql_options(con, MYSQL_SET_CHARSET_NAME, "utf8") != 0
        || mysql_options(con, MYSQL_OPT_RECONNECT, &reconnect) != 0)
    {
        CEL_SETERR((CEL_ERR_MYSQL, _T("mysql_options:%s."), mysql_error(con)));
        return -1;
    }
    if (mysql_real_connect(con, host, user, pswd, name, port, NULL, 0) == NULL)
    {
        CEL_SETERR((CEL_ERR_MYSQL, _T("mysql_real_connect:%s."), mysql_error(con)));
        return -1;
    }

    return 0;
    }

void cel_mysqlcon_close(CelMysqlCon *con)
{
    mysql_close(con);
}

long cel_mysqlcon_execute_nonequery(CelMysqlCon *con, 
                                    const char *sqlstr, unsigned long len)
{
    CelMysqlRes *res;
    long affected_rows;

    if (con == NULL) 
        return -1;

    if (mysql_real_query(con, sqlstr, len) != 0)
    {
        CEL_SETERR((CEL_ERR_MYSQL, 
            _T("mysql_real_query:%s."), mysql_error(con)));
        return -1;
    }
    if ((res = mysql_store_result(con)) != NULL)
    {
        mysql_free_result(res);
    }
    affected_rows = (long)mysql_affected_rows(con);
    //printf(_T("%s affected_rows %ld\r\n"), sqlstr, affected_rows);

    return affected_rows;
}

CelMysqlRes *cel_mysqlcon_execute_onequery(CelMysqlCon *con,
                                           const char *sqlstr, 
                                           unsigned long len)
{
    if (con == NULL) 
        return NULL;

    if (mysql_real_query(con, sqlstr, len) != 0)
    {
        CEL_SETERR((CEL_ERR_MYSQL,
            _T("mysql_real_query:%s."), mysql_error(con)));
        mysql_close(con);
        return NULL;
    }
    return mysql_store_result(con);
}

CelMysqlRes *cel_mysqlcon_execute_query(CelMysqlCon *con,
                                        const char *sqlstr, unsigned long len)
{
    if (con == NULL) 
        return NULL;

    if (mysql_real_query(con, sqlstr, len) != 0)
    {
        CEL_SETERR((CEL_ERR_MYSQL,
            _T("mysql_real_query:%s."),mysql_error(con)));
        return NULL;
    }
    return mysql_store_result(con);
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
    if ((res->fields == NULL 
        && mysql_fetch_field(res) == NULL)
        || res->field_count > field_offset)
    {
        CEL_SETERR((CEL_ERR_MYSQL, _T("Mysql fetch field error.")));
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
        CEL_SETERR((CEL_ERR_MYSQL, _T("Mysql fetch field error.")));
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
    CEL_SETERR((CEL_ERR_MYSQL, 
        _T("Mysql data seek failed, offset %lld out of range"), offset));

    return -1;
}


