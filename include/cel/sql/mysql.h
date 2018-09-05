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
#ifndef __CEL_SQL_MYSQL_H__
#define __CEL_SQL_MYSQL_H__

#include "cel/types.h"
#include <mysql/mysql.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct st_mysql_ex 
{
    struct st_mysql mysql;
    char *host, *user, *passwd, *db;
    unsigned int port;
}CelMysqlCon;

typedef struct st_mysql_res CelMysqlRes;
typedef struct st_mysql_field CelMysqlField;
typedef char** CelMysqlRow;

int cel_mysqlcon_init(CelMysqlCon *con, 
                      const char *dbhost, unsigned int dbport, 
                      const char *dbname, 
                      const char *dbuser, const char *dbpswd);
void cel_mysqlcon_destroy(CelMysqlCon *con);

int cel_mysqlcon_open(CelMysqlCon *con);
void cel_mysqlcon_close(CelMysqlCon *con);

long cel_mysqlcon_execute_nonequery(CelMysqlCon *con, const char *sqlstr);
CelMysqlRes *cel_mysqlcon_execute_onequery(CelMysqlCon *con, 
                                           const char *sqlstr);
CelMysqlRes *cel_mysqlcon_execute_query(CelMysqlCon *con, const char *sqlstr);

#define cel_mysqlres_next(res) (mysql_fetch_row(res) == NULL ? 0 : 1)

int cel_mysqlres_get_int(CelMysqlRes *res, int col_index, int *value);
int cel_mysqlres_get_long(CelMysqlRes *res, int col_index, long *value);
int cel_mysqlres_get_string(CelMysqlRes *res, int col_index,
                            char *value, size_t size);

static __inline long cel_mysqlres_rows(CelMysqlRes *res)
{
    return (long)mysql_num_rows(res);
}
static __inline int cel_mysqlres_cols(CelMysqlRes *res)
{
    return mysql_num_fields(res);
}

static __inline unsigned long *cel_mysqlres_fetch_lengths(CelMysqlRes *res)
{
    return mysql_fetch_lengths(res);
}

static __inline CelMysqlRow cel_mysqlres_fetch_row(CelMysqlRes *res)
{
    return mysql_fetch_row(res);
}

static __inline CelMysqlField *cel_mysqlres_fetch_field(CelMysqlRes *res)
{
    return mysql_fetch_field(res);
}

const char *cel_mysqlres_field_name(CelMysqlRes *res, 
                                    unsigned int field_offset);
int cel_mysqlres_field_len(CelMysqlRes *res, unsigned int field_offset);

/* offset: 0 - (rows - 1)*/
int cel_mysqlres_rows_seek(CelMysqlRes *res, 
                           unsigned long long offset, CelMysqlRow *row);
static __inline void cel_mysqlres_free(CelMysqlRes *res)
{
    mysql_free_result(res);
}

#ifdef __cplusplus
}
#endif

#endif
