/**
 * CEL(C Extension Library)
 * Copyright (C)2008 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/sql/sqlconpool.h"
#include "cel/error.h"
//#define _CEl_DEBUG
#include "cel/log.h"
#include "cel/allocator.h"

int cel_sqlconpool_init(CelSqlConPool *pool, CelSqlConType type,
                        const char *host, unsigned int port, 
                        const char *name, 
                        const char *user, const char *pswd,
                        int pool_min, int pool_max)
{
    CelSqlCon *con;

    memset(pool, 0, sizeof(CelSqlConPool));
    pool->type = type;
    CEL_PTR_STRDUP(pool->host, host);
    CEL_PTR_STRDUP(pool->user, user);
    CEL_PTR_STRDUP(pool->passwd, pswd);
    CEL_PTR_STRDUP(pool->db, name);
    pool->port = port;
    pool->min = pool_min;
    pool->max = pool_max;
    if (cel_ringlist_init(&(pool->frees), 256, (CelFreeFunc)cel_sqlcon_free) == -1)
        return -1;
    pool->n_conns = 0;
    do
    {
        if ((con = cel_sqlcon_new(pool->type, 
            pool->host, pool->port, 
            pool->db, pool->user, pool->passwd)) == NULL)
        {
            cel_ringlist_destroy(&(pool->frees));
            return -1;
        }
        cel_ringlist_push_do_sp(&(pool->frees), con, 1);
        cel_atomic_increment(&(pool->n_conns), 1);
    }while (pool->n_conns < pool->min);
    return 0;
}

void cel_sqlconpool_destroy(CelSqlConPool *pool)
{
    CEL_PTR_FREE(pool->host);
    CEL_PTR_FREE(pool->user);
    CEL_PTR_FREE(pool->passwd);
    CEL_PTR_FREE(pool->db);
    cel_ringlist_destroy(&(pool->frees));
}

CelSqlConPool *cel_sqlconpool_new(CelSqlConType type, 
                                  const char *host, unsigned int port, 
                                  const char *name, 
                                  const char *user, const char *pswd,
                                  int pool_min, int pool_max)
{
    CelSqlConPool *pool;

    if ((pool = (CelSqlConPool *)cel_malloc(sizeof(CelSqlConPool))) != NULL)
    {
        if (cel_sqlconpool_init(
            pool, type, host, port, name, user, pswd, pool_min, pool_max) == 0)
            return pool;
        cel_free(pool);
    }

    return NULL;
}

void cel_sqlconpool_free(CelSqlConPool *pool)
{
    cel_sqlconpool_destroy(pool);
    cel_free(pool);
}

CelSqlCon *cel_sqlconpool_get(CelSqlConPool *pool)
{
    CelSqlCon *con = NULL;

    /*
    printf(" cel_sqlconpool_get count %d min %d total %d\r\n", 
        cel_ringlist_get_count(&(pool->frees)), pool->min, pool->n_conns);
    */
    if (cel_ringlist_pop_do_mp(&(pool->frees), &con, 1) == 1)
    {
        CEL_DEBUG((_T("cel_sqlconpool_get %p"), con));
        return con;
    }
    if (pool->n_conns < pool->max)
    {
        if ((con = cel_sqlcon_new(pool->type, 
            pool->host, pool->port, 
            pool->db, pool->user, pool->passwd)) != NULL)
        {
            cel_atomic_increment(&(pool->n_conns), 1);
            return con;
        }
    }
    CEL_SETERR((CEL_ERR_LIB, _T("cel_sqlconpool_get return NULL, cons %d, max %d"), 
		pool->n_conns, pool->max));
    return NULL;
}

void cel_sqlconpool_return(CelSqlConPool *pool, CelSqlCon *con)
{
	/*
	printf(" cel_sqlconpool_return count %d min %d total %d\r\n", 
	cel_ringlist_get_count(&(pool->frees)), pool->min, pool->n_conns);
	*/
	CEL_DEBUG((_T("cel_sqlconpool_return %p"), con));
	if (cel_ringlist_get_count(&(pool->frees)) < pool->min)
	{
		cel_ringlist_push_do_mp(&(pool->frees), con, 1);
		return ;
	}
	cel_sqlcon_free(con);
	cel_atomic_increment(&(pool->n_conns), -1);   
}
 
long cel_sqlconpool_execute_nonequery(CelSqlConPool *pool, const char *fmt, ...)
{
	CelSqlCon *con;
	long affected_rows;
	if ((con = cel_sqlconpool_get(pool)) == NULL)
		return -1;
	CEL_SQLCON_SQLSTR_FMT();
	affected_rows = _cel_sqlcon_execute_nonequery(con);
	cel_sqlconpool_return(pool, con);
	return affected_rows;
}
 
CelSqlRes *cel_sqlconpool_execute_onequery(CelSqlConPool *pool, const char *fmt, ...)
{
	CelSqlCon *con;
	CelSqlRes *res;
	if ((con = cel_sqlconpool_get(pool)) == NULL)
		return NULL;
	CEL_SQLCON_SQLSTR_FMT();
	res = _cel_sqlcon_execute_onequery(con);
	cel_sqlconpool_return(pool, con);
	return res;
}

CelSqlRes *cel_sqlconpool_execute_query(CelSqlConPool *pool, const char *fmt, ...)
{
	CelSqlCon *con;
	CelSqlRes *res;
	if ((con = cel_sqlconpool_get(pool)) == NULL)
		return NULL;
	CEL_SQLCON_SQLSTR_FMT();
	//puts(con->sqlstr.str);
	res = _cel_sqlcon_execute_query(con);
	cel_sqlconpool_return(pool, con);
	return res;
}

int cel_sqlconpool_execute_onequery_results(CelSqlConPool *pool,
											CelSqlRowEachFunc each_func, void *user_data, 
											const char *fmt, ...)
{
	CelSqlCon *con;
	int ret;

	if ((con = cel_sqlconpool_get(pool)) == NULL)
		return -1;
	CEL_SQLCON_SQLSTR_FMT();
	ret = _cel_sqlcon_execute_onequery_results(con, each_func, user_data);
	cel_sqlconpool_return(pool, con);
	return ret;
}

int cel_sqlconpool_execute_query_results(CelSqlConPool *pool,
										 CelSqlRowEachFunc each_func, void *user_data,
										 const char *fmt, ...)
{
	CelSqlCon *con;
	int ret;

	if ((con = cel_sqlconpool_get(pool)) == NULL)
		return -1;
	CEL_SQLCON_SQLSTR_FMT();
	ret = _cel_sqlcon_execute_query_results(con, each_func, user_data);
	cel_sqlconpool_return(pool, con);
	return ret;
}
