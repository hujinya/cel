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
#ifndef __CEL_NET_JWT_H__
#define __CEL_NET_JWT_H__

#include "cel/types.h"
#include "cel/json.h"
#include "cel/datetime.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _CelJwtAlg
{
    CEL_JWT_ALG_NONE = -1,
    CEL_JWT_ALG_ES256,     /* ECDSA using P-256 and SHA-256 */
    CEL_JWT_ALG_ES384,     /* ECDSA using P-256 and SHA-384 */
    CEL_JWT_ALG_ES512,     /* ECDSA using P-256 and SHA-512 */
    CEL_JWT_ALG_HS256,     /* HMAC using SHA-256 */
    CEL_JWT_ALG_HS384,     /* HMAC using SHA-384 */
    CEL_JWT_ALG_HS512,     /* HMAC using SHA-512 */
    CEL_JWT_ALG_RS256,     /* RSASSA-PKCS-v1_5 using SHA-256 */
    CEL_JWT_ALG_RS384,     /* RSASSA-PKCS-v1_5 using SHA-384 */
    CEL_JWT_ALG_RS512,     /* RSASSA-PKCS-v1_5 using SHA-512 */
    CEL_JWT_ALG_COUNT
}CelJwtAlg;

typedef struct _CelJwt
{
    CelJwtAlg alg;
    CelJson payload;
}CelJwt;

int cel_jwt_init(CelJwt *jwt);
void cel_jwt_destroy(CelJwt *jwt);

CelJwt *cel_jwt_new(void);
void cel_jwt_free(CelJwt *jwt);

char *cel_jwt_encode(CelJwt *jwt, char *token, size_t *token_len, 
                     const BYTE *key, int key_len);
int cel_jwt_decode(CelJwt *jwt, const char *token, size_t token_len,
                   const BYTE *key, int key_len);

static __inline void cel_jwt_alg_set(CelJwt *jwt, CelJwtAlg alg)
{
    jwt->alg = alg;
}
static __inline CelJwtAlg cel_jwt_alg_get(CelJwt *jwt)
{
    return jwt->alg;
}

static __inline void cel_jwt_payload_clear(CelJwt *jwt)
{
    cel_json_clear(&(jwt->payload));
}

static __inline 
void cel_jwt_payload_add_bool(CelJwt *jwt, const char *key, BOOL value)
{
    if (jwt->payload.root_node == NULL)
        jwt->payload.root_node = cel_json_object_new();
    cel_json_object_add_bool(jwt->payload.root_node, key, value);
}
static __inline 
void cel_jwt_payload_add_long(CelJwt *jwt, const char *key, long value)
{
    if (jwt->payload.root_node == NULL)
        jwt->payload.root_node = cel_json_object_new();
    cel_json_object_add_long(jwt->payload.root_node, key, value);
}
static __inline 
void cel_jwt_payload_add_double(CelJwt *jwt, const char *key, double value)
{
    if (jwt->payload.root_node == NULL)
        jwt->payload.root_node = cel_json_object_new();
    cel_json_object_add_double(jwt->payload.root_node, key, value);
}
static __inline 
void cel_jwt_payload_add_string(CelJwt *jwt, const char *key, const char *value)
{
    if (jwt->payload.root_node == NULL)
        jwt->payload.root_node = cel_json_object_new();
    cel_json_object_add_string(jwt->payload.root_node, key, value);
}

static __inline void cel_jwt_payload_iss_set(CelJwt *jwt, const char *iss)
{
    cel_jwt_payload_add_string(jwt, "iss", iss);
}

static __inline void cel_jwt_payload_exp_set(CelJwt *jwt, long seconds)
{
    CelDateTime dt;

    cel_datetime_init_now(&dt);
    cel_datetime_add_seconds(&dt, seconds);
    cel_jwt_payload_add_long(jwt, "exp", (long)dt);
}

static __inline 
int cel_jwt_payload_get_bool(CelJwt *jwt, const char *key, BOOL *value)
{
    return jwt->payload.root_node == NULL ?
        -1 : cel_json_object_get_bool(jwt->payload.root_node, key, value);
}
static __inline 
int cel_jwt_payload_get_int(CelJwt *jwt, const char *key, int *value)
{
    return jwt->payload.root_node == NULL ?
        -1 : cel_json_object_get_int(jwt->payload.root_node, key, value);
}
static __inline 
int cel_jwt_payload_get_double(CelJwt *jwt, const char *key, double *value)
{
    return jwt->payload.root_node == NULL ?
        -1 : cel_json_object_get_double(jwt->payload.root_node, key, value);
}
static __inline 
int cel_jwt_payload_get_string(CelJwt *jwt, const char *key, 
                               char *value, size_t size)
{
    return jwt->payload.root_node == NULL ?
        -1 : cel_json_object_get_string(jwt->payload.root_node,
        key, value, size);
}

static __inline 
void cel_jwt_payload_delete(CelJwt *jwt, const char *key)
{
    cel_json_object_delete(jwt->payload.root_node, key);
}


#ifdef __cplusplus
}
#endif

#endif
