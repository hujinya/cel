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

#ifdef __cplusplus
extern "C" {
#endif

/* return 0 = continue;-1 = error;1 = break */
typedef int (* CelMysqlRowEachFunc) (void **row, int cols, void *user_data);

typedef struct st_mysql_ex CelMysqlCon;
typedef struct st_mysql_res CelMysqlRes;
typedef struct st_mysql_field CelMysqlField;
typedef char** CelMysqlRow;

CelMysqlCon *cel_mysqlcon_new(const char *dbhost, unsigned int dbport, 
                              const char *dbname, 
                              const char *dbuser, const char *dbpswd);
void cel_mysqlcon_free(CelMysqlCon *con);

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

long cel_mysqlres_rows(CelMysqlRes *res);
int cel_mysqlres_cols(CelMysqlRes *res);

unsigned long *cel_mysqlres_fetch_lengths(CelMysqlRes *res);
CelMysqlRow cel_mysqlres_fetch_row(CelMysqlRes *res);
CelMysqlField *cel_mysqlres_fetch_field(CelMysqlRes *res);

const char *cel_mysqlres_field_name(CelMysqlRes *res, 
                                    unsigned int field_offset);
int cel_mysqlres_field_len(CelMysqlRes *res, unsigned int field_offset);

/* offset: 0 - (rows - 1)*/
int cel_mysqlres_rows_seek(CelMysqlRes *res, 
                           unsigned long long offset, CelMysqlRow *row);
void cel_mysqlres_free(CelMysqlRes *res);

int cel_mysqlcon_execute_onequery_results(CelMysqlCon *con, const char *sqlstr,
                                          CelMysqlRowEachFunc each_func, 
                                          void *user_data);
int cel_mysqlcon_execute_query_results(CelMysqlCon *con, const char *sqlstr, 
                                       CelMysqlRowEachFunc each_func, 
                                       void *user_data);

#ifdef __cplusplus
}
#endif

#endif
