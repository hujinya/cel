/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_NET_WEBSESSION_H__
#define __CEL_NET_WEBSESSION_H__

#include "cel/datetime.h"
#include "cel/hashtable.h"
#include "cel/net/webcookie.h"

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
