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
#ifndef __CEL_NET_HTTPSESSION_H__
#define __CEL_NET_HTTPSESSION_H__

#include "cel/datetime.h"
#include "cel/hashtable.h"
#include "cel/net/httpcookie.h"

typedef struct _CelHttpSessionMap
{
    char *id;
    CelDateTime last_access_time;
    CelHashTable key_values;
}CelHttpSession;

typedef struct _CelHttpSessionContext
{
    long timeout;
    CelHashTable sessions;
}CelHttpSessionContext;

#ifdef __cplusplus
extern "C" {
#endif

CelHttpSession *cel_httpsession_new();
void cel_httpsession_free(CelHttpSession *session);

void cel_httpsession_set(CelHttpSession *session, 
                        const char *key, const char *value);
void *cel_httpsession_get(CelHttpSession *session, const char *key);
void cel_httpsession_del(CelHttpSession *session, const char *key);

int cel_httpsessioncontext_init(CelHttpSessionContext *session_ctx);
void cel_httpsessioncontext_destroy(CelHttpSessionContext *session_ctx);

CelHttpSessionContext *cel_httpsessioncontext_new();
void cel_httpsessioncontext_free(CelHttpSessionContext *session_ctx);

void cel_httpsessioncontext_get(CelHttpSessionContext *session_ctx, 
                                const char *seesion_id);

#ifdef __cplusplus
}
#endif

#endif
