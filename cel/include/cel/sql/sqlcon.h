/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2019 Hu Jinya(hu_jinya@163.com) 
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

#include "cel/allocator.h"
#include "cel/ringlist.h"
#include "cel/vstring.h"
#include <stdarg.h>

typedef enum _CelSqlConType
{
    CEL_SQLCON_UNDEFINED,
    CEL_SQLCON_MYSQL,
	CEL_SQLCON_SQLLITE,
    CEL_SQLCON_MSSQL
}CelSqlConType;

typedef struct _CelSqlCon CelSqlCon;
typedef struct _CelSqlRes CelSqlRes;
typedef struct _CelSqlField CelSqlField;
typedef char** CelSqlRow;

/* return 0 = continue;-1 = error;1 = break */
typedef int (* CelSqlRowEachFunc) (void **row, int cols, void *user_data);

typedef int  (* CelSqlConOpenFunc)(void *con,
								   const char *host, unsigned int port, 
								   const char *name, 
								   const char *user, const char *pswd);
typedef void (* CelSqlConCloseFunc)(void *con);
typedef long (* CelSqlConExecuteNonequeryFunc)(
	void *con, const char *sqlstr, unsigned long len);
typedef void* (* CelSqlConExecuteOnequeryFunc)(
	void *con, const char *sqlstr, unsigned long len);
typedef CelSqlRes* (* CelSqlConExecuteQueryFunc)(
	void *con, const char *sqlstr, unsigned long len);
typedef long (* CelSqlResRowsFunc)(void *res);
typedef int (* CelSqlResColsFunc)(void *res);
typedef unsigned long* (* CelSqlResFetchLengthsFunc)(void *res);
typedef char** (* CelSqlResFetchRowFunc)(void *res);
typedef void* (*CelSqlResFetchFieldFunc)(void *res);
typedef void (* CelSqlResFreeFunc)(void *res);

typedef struct _CelSqlConClass
{
    CelSqlConOpenFunc con_open;
    CelSqlConCloseFunc con_close;
    CelSqlConExecuteNonequeryFunc con_execute_nonequery;
    CelSqlConExecuteOnequeryFunc con_execute_onequery;
    CelSqlConExecuteQueryFunc con_execute_query;
    CelSqlResRowsFunc res_rows;
    CelSqlResColsFunc res_cols;
    CelSqlResFetchLengthsFunc res_fetch_lengths;
    CelSqlResFetchRowFunc res_fetch_row;
    CelSqlResFetchFieldFunc res_fetch_field;
    CelSqlResFreeFunc res_free;
}CelSqlConClass;

struct _CelSqlRes
{
    void *st_res;
    CelSqlConClass *kclass;
};

struct _CelSqlField
{
    void *st_field;
    CelSqlConClass *kclass;
};

struct _CelSqlCon
{
    void *st_con;
    char *host, *user, *passwd, *db;
    unsigned int port;
	CelVStringA sqlstr;
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
	return con->kclass->con_open(
		con->st_con, con->host, con->port, con->db, con->user, con->passwd);
}
static __inline void cel_sqlcon_close(CelSqlCon *con)
{
	con->kclass->con_close(con->st_con);
}

static __inline int cel_sqlcon_sqlstr_resize(CelSqlCon *con, size_t size)
{
	return cel_vstring_resize_a(&(con->sqlstr), size);
}

long _cel_sqlcon_execute_nonequery(CelSqlCon *con);
CelSqlRes *_cel_sqlcon_execute_onequery(CelSqlCon *con);
CelSqlRes *_cel_sqlcon_execute_query(CelSqlCon *con);

#define CEL_SQLCON_SQLSTR_FMT() {\
	va_list args;\
	va_start(args, fmt);\
	cel_vstring_vprintf_a(&(con->sqlstr), fmt, args); \
	va_end(args); }

static __inline
long cel_sqlcon_execute_nonequery(CelSqlCon *con, const char *fmt, ...)
{
	CEL_SQLCON_SQLSTR_FMT();
	return _cel_sqlcon_execute_nonequery(con);
}
static __inline 
CelSqlRes *cel_sqlcon_execute_onequery(CelSqlCon *con, const char *fmt, ...)
{
	CEL_SQLCON_SQLSTR_FMT();
	return _cel_sqlcon_execute_onequery(con);
}
static __inline 
CelSqlRes *cel_sqlcon_execute_query(CelSqlCon *con, const char *fmt, ...)
{
	CEL_SQLCON_SQLSTR_FMT(); 
	return _cel_sqlcon_execute_query(con);
}

static __inline long cel_sqlres_rows(CelSqlRes *res)
{
	return res->kclass->res_rows(res->st_res);
}
static __inline int cel_sqlres_cols(CelSqlRes *res)
{
	return res->kclass->res_cols(res->st_res);
}

static __inline unsigned long *cel_sqlres_fetch_lengths(CelSqlRes *res)
{
	return res->kclass->res_fetch_lengths(res->st_res);
}
static __inline CelSqlRow cel_sqlres_fetch_row(CelSqlRes *res)
{
	return res->kclass->res_fetch_row(res->st_res);
}
static __inline CelSqlField *cel_sqlres_fetch_field(CelSqlRes *res)
{
	return (CelSqlField *)res->kclass->res_fetch_field(res->st_res);
}

static __inline void cel_sqlres_free(CelSqlRes *res)
{
	if (res != NULL)
	{
		res->kclass->res_free(res->st_res);
		cel_free(res);
	}
}

int _cel_sqlcon_execute_onequery_results(CelSqlCon *con,
										 CelSqlRowEachFunc each_func, 
										 void *user_data);
int _cel_sqlcon_execute_query_results(CelSqlCon *con,
									  CelSqlRowEachFunc each_func, 
									  void *user_data);
static __inline int cel_sqlcon_execute_onequery_results(CelSqlCon *con,
														CelSqlRowEachFunc each_func, 
														void *user_data, 
														const char *fmt, ...)
{
	CEL_SQLCON_SQLSTR_FMT(); 
	return _cel_sqlcon_execute_onequery_results(con, each_func, user_data);
}
static __inline int cel_sqlcon_execute_query_results(CelSqlCon *con,
													 CelSqlRowEachFunc each_func, 
													 void *user_data,
													 const char *fmt, ...)
{
	CEL_SQLCON_SQLSTR_FMT(); 
	return _cel_sqlcon_execute_query_results(con, each_func, user_data);
}

#ifdef __cplusplus
}
#endif

#endif