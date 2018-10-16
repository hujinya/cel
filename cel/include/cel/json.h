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
#ifndef __CEL_JSON_H__
#define __CEL_JSON_H__

#include "cel/types.h"
#include "cel/vstring.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CEL_JSON_LINE_SIZE  1024

/* Json node type */
typedef enum _CelJsonType
{
    CEL_JSONT_UNDEFINED,
    CEL_JSONT_STRING,
    CEL_JSONT_OBJECT,
    CEL_JSONT_ARRAY,
    CEL_JSONT_INT,
    CEL_JSONT_DOUBLE,
    CEL_JSONT_BOOL,
    CEL_JSONT_NULL
}CelJsonType;

/* Json deserialize and serialize machine state */
typedef enum _CelJsonState
{ 
    CEL_JSONDS_ROOT,           /* Deserialize root node */
    CEL_JSONDS_OBJECT,         /* Begin json node */
    CEL_JSONDS_NEXT,           /* Brother node */
    CEL_JSONDS_KEY,            /* For key */
    CEL_JSONDS_VALUE,          /* For value */
    CEL_JSONDS_OK,             /* Deserialize ok, finsh */
    CEL_JSONDS_ERROR,          /* Deserialize error */

    CEL_JSONSS_ERROR,          /* Serialize error */
    CEL_JSONSS_ROOT,           /* Serialize root node */
    CEL_JSONSS_NEXT,           /* Brother node */
    CEL_JSONSS_KEY,            /* For key */
    CEL_JSONSS_VALUE,          /* For value */
    CEL_JSONSS_OK              /* Serialize ok, finish */
}CelJsonState;

typedef struct _CelJsonNode
{
    CelJsonType type;
    char *key;
    size_t key_len;
    union {
        BOOL bvalue;
        long lvalue;
        double dvalue;
        struct {
            char *strvalue;
            size_t strvalue_len;
        };
        struct {
            size_t child_size;
            struct _CelJsonNode *child;
            struct _CelJsonNode *cursor;
        };
    };
    struct _CelJsonNode *parent, *next;
}CelJsonNode;

typedef struct _CelJson
{
    /* Deserialize and serialize machine state and data */
    CelJsonState state; 
    int indent, node_depth;
    int is_escape, escape_cursor;
    size_t colon;
    char escape_buf[4];
    CelVStringA vstr;
    CelJsonNode *cur_node;
    CelJsonNode *root_node;
}CelJson;

/* Json node */
CelJsonNode *cel_json_node_new(CelJsonType type);
void cel_json_node_free(CelJsonNode *node);

#define cel_json_null_new() cel_json_node_new(CEL_JSONT_NULL)
CelJsonNode *cel_json_bool_new(BOOL bool_value);
CelJsonNode *cel_json_long_new(long int_value);
CelJsonNode *cel_json_double_new(double double_value);
CelJsonNode *cel_json_string_new(const char *string_value);
/* CelJsonNode *cel_json_array_new(void); */
#define cel_json_array_new() cel_json_node_new(CEL_JSONT_ARRAY)
/* CelJsonNode *cel_json_object_new(void); */
#define cel_json_object_new() cel_json_node_new(CEL_JSONT_OBJECT)

/* Add item to the specified arrays/objects. */
void cel_json_array_add(CelJsonNode *_array, CelJsonNode *item);
void cel_json_object_add(CelJsonNode *object, 
                         const char *key, CelJsonNode *item);

/* Detatch/delete item from arrays/objects. */
CelJsonNode *cel_json_array_detach(CelJsonNode *_array, int which);
void cel_json_array_delete(CelJsonNode *_array, int which);
CelJsonNode *cel_json_object_detach(CelJsonNode *object, const char *key);
void cel_json_object_delete(CelJsonNode *object, const char *key);

/* Replace arrays/objects items. */
int cel_json_array_repalce(CelJsonNode *_array, int which, CelJsonNode *item);
int cel_json_object_replace(CelJsonNode *object, 
                            const char *key, CelJsonNode *item);

#define cel_json_array_get_size(_array) \
    ((_array)->type == CEL_JSONT_ARRAY ? (_array)->child_size : 0)
CelJsonNode *cel_json_array_get(CelJsonNode *_array, int which);
CelJsonNode *cel_json_object_get(CelJsonNode *object, const char *key);

/* Json init and destroy */
int cel_json_init(CelJson *json);
int cel_json_init_buffer(CelJson *json, char *buf, size_t size);
int cel_json_init_file(CelJson *json, const char *file);
void cel_json_destroy(CelJson *json);
/* Json new and free */
CelJson *cel_json_new(void);
CelJson *cel_json_new_buffer(char *buf, size_t size);
CelJson *cel_json_new_file(const char *file);
void cel_json_free(CelJson *json);

/* Json deserialize */
int cel_json_deserialize_starts(CelJson *json);
int cel_json_deserialize_update(CelJson *json, char *buf, size_t size);
int cel_json_deserialize_finish(CelJson *json);
int cel_json_deserialize_file(CelJson *json, const char *file);
/* Json serialize */
int cel_json_serialize_starts(CelJson *json, int is_indent);
int cel_json_serialize_update(CelJson *json, char *buf, size_t *size);
int cel_json_serialize_finish(CelJson *json);
int cel_json_serialize_file(CelJson *json, const char *file, int is_indent);

/* CelJsonNode *cel_json_root_node(CelJson *json) */
#define cel_json_root_node(json) ((json)->root_node)
static __inline void cel_json_clear(CelJson *json)
{
    if ((json)->root_node != NULL)  
        cel_json_node_free((json)->root_node);
}

#define cel_json_array_add_null(_array)  \
    cel_json_array_add(_array, cel_json_null_new())
#define cel_json_array_add_bool(_array, value) \
    cel_json_array_add(_array, cel_json_bool_new(value))
#define cel_json_array_add_int(_array, value) \
    cel_json_array_add(_array, cel_json_long_new(value))
#define cel_json_array_add_long cel_json_array_add_int
#define cel_json_array_add_double(_array, value) \
    cel_json_array_add(_array, cel_json_double_new(value))
#define cel_json_array_add_string(_array, value) \
    cel_json_array_add(_array, cel_json_string_new(value))

#define cel_json_object_add_null(object, key) \
    cel_json_object_add(object, key, cel_json_null_new())
#define cel_json_object_add_bool(object, key, value) \
    cel_json_object_add(object, key, cel_json_bool_new(value))
#define cel_json_object_add_int(object, key, value) \
    cel_json_object_add(object, key, cel_json_long_new(value))
#define cel_json_object_add_long cel_json_object_add_int
#define cel_json_object_add_double(object, key, value) \
    cel_json_object_add(object, key, cel_json_double_new(value))
#define cel_json_object_add_string(object, key, value) \
    cel_json_object_add(object, key, cel_json_string_new(value))

int cel_json_array_get_bool(CelJsonNode *_array, int which, BOOL *value);
int cel_json_array_get_int(CelJsonNode *_array, int which, int *value);
int cel_json_array_get_long(CelJsonNode *_array, int which, long *value);
int cel_json_array_get_double(CelJsonNode *_array, int which, double *value);
int cel_json_array_get_string(CelJsonNode *_array, 
                              int which, char *value, size_t size);
int cel_json_array_get_string_alloc(CelJsonNode *_array, 
                                    int which, char **value);

int cel_json_object_get_bool(CelJsonNode *object, 
                             const char *key, BOOL *value);
int cel_json_object_get_int(CelJsonNode *object, const char *key, int *value);
int cel_json_object_get_long(CelJsonNode *object, const char *key, long *value);
int cel_json_object_get_double(CelJsonNode *object, 
                               const char *key, double *value);
int cel_json_object_get_string(CelJsonNode *object,
                               const char *key, char *value, size_t size);
int cel_json_object_get_string_alloc(CelJsonNode *object, 
                                     const char *key, char **value);
int cel_json_object_get_string_pointer(CelJsonNode *obj, 
                                       const char *key, char **value);


#ifdef __cplusplus
}
#endif

#endif
