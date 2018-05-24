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
#ifndef __CEL_NET_WEBSESSION_H__
#define __CEL_NET_WEBSESSION_H__

#include "cel/datetime.h"
#include "cel/hashtable.h"
#include "cel/net/httpwebcookie.h"

typedef struct _CelWebSessionMap
{
    char *id;
    CelDateTime last_access_time;
    CelHashTable key_values;
}CelWebSeesion;

typedef struct _CelWebSessionContext
{
    long timeout;
    CelHashTable sessions; 
}CelWebSessionContext;

#ifdef __cplusplus
extern "C" {
#endif

CelWebSeesion *cel_websession_new();
void cel_websession_free(CelWebSeesion *session);

void cel_websession_set(CelWebSeesion *session, 
                        const char *key, const char *value);
void *cel_websession_get(CelWebSeesion *session, const char *key);
void cel_websession_del(CelWebSeesion *session, const char *key);

int cel_websessioncontext_init(CelWebSessionContext *session_ctx);
void cel_websessioncontext_destroy(CelWebSessionContext *session_ctx);

CelWebSessionContext *cel_websessioncontext_new();
void cel_websessioncontext_free(CelWebSessionContext *session_ctx);

void cel_websessioncontext_get(CelWebSessionContext *session_ctx, 
                               const char *seesion_id);

#ifdef __cplusplus
}
#endif

#endif
