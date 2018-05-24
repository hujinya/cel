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
#ifndef __CEL_NET_HTTPMULTIPART_H__
#define __CEL_NET_HTTPMULTIPART_H__

#include "cel/types.h"
#include "cel/stream.h"
#include "cel/vstring.h"
#include "cel/list.h"
#include "cel/net/http.h"

typedef enum _CelHttpMultipartEntityState
{
    CEL_HTTPMULTIPART_ENTITY_BOUNDARY,
    CEL_HTTPMULTIPART_ENTITY_HEADER,
    CEL_HTTPMULTIPART_ENTITY_VALUE
}CelHttpMultipartEntityState;

typedef struct _CelHttpMultipartEntity
{
    CelListItem item;
    BOOL is_file;
    CelVStringA param_name;
    CelVStringA file_name;
    CelHttpBodyCache cache;
}CelHttpMultipartEntity;

typedef struct _CelHttpMultipart
{
    CelVStringA boundary;
    CelHttpMultipartEntityState reading_state;
    CelHttpMultipartEntity *reading_entity;
    CelList entity_list;
}CelHttpMultipart;

#ifdef __cplusplus
extern "C" {
#endif

CelHttpMultipartEntity *cel_httpmultipart_entity_new();
void cel_httpmultipart_entity_free(CelHttpMultipartEntity *entity);

static __inline 
CelHttpMultipartEntity *cel_httpmultipart_entity_get_next(
    CelHttpMultipartEntity *entity)
{
    return (CelHttpMultipartEntity *)entity->item.next;
}
#define cel_httpmultipart_entity_is_file(entity) (entity)->is_file
static __inline 
long long cel_httpmulitpart_entity_save_file(CelHttpMultipartEntity *entity,
                                             long long first, long long last,
                                             const char *file_path)
{
    return cel_httpbodycache_save_file(
        &(entity->cache), first, last, file_path);
}
static __inline 
int cel_httpmulitpart_entity_move_file(CelHttpMultipartEntity *entity,
                                       const char *file_path)
{
    return cel_httpbodycache_move_file(&(entity->cache), file_path);
}

int cel_httpmultipart_init(CelHttpMultipart *multipart);
void cel_httpmultipart_destroy(CelHttpMultipart *multipart);


CelHttpMultipart *cel_httpmultipart_new(void);
void cel_httpmultipart_free(CelHttpMultipart *multipart);

static __inline 
void cel_httpmultipart_set_boundary(CelHttpMultipart *multipart, 
                                    const char *boundary)
{
    cel_vstring_assign_a(&(multipart->boundary), boundary, strlen(boundary));
}
int cel_httpmultipart_reading(CelHttpMultipart *multipart, 
                              CelStream *s, size_t len);

static __inline 
CelHttpMultipartEntity *cel_httpmultipart_get_first_entity(
    CelHttpMultipart *multipart)
{
    return cel_list_is_empty(&(multipart->entity_list)) ? NULL :
        (CelHttpMultipartEntity *)
        cel_list_get_front(&(multipart->entity_list));
}

#ifdef __cplusplus
}
#endif

#endif
