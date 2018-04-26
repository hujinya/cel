/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_CONF_H__
#define __CEL_CONF_H__

#include "cel/types.h"
#include "cel/json.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelJsonNode CelConfItem;
typedef struct _CelJson CelConf;

int cel_conf_init(CelConf *conf);
void cel_conf_destroy(CelConf *conf);

CelConf *cel_conf_new(void);
void cel_conf_free(CelConf *conf);

int cel_conf_read_file(CelConf *conf, const TCHAR *path);
int cel_conf_write_file(CelConf *conf, const TCHAR *path);

#define cel_conf_root(conf) conf->root_node

#define cel_conf_object_get_bool(_obj, key, value) \
    cel_json_object_get_bool(_obj, key, value)
#define cel_conf_object_get_int(_obj, key, value)  \
    cel_json_object_get_int(_obj, key, value) 
#define cel_conf_object_get_double(_obj, key, value)  \
    cel_json_object_get_double(_obj, key, value) 
#define cel_conf_object_get_string(_obj, key, value, size)  \
    cel_json_object_get_string(_obj, key, value, size) 
#define cel_conf_object_get_item(_obj, key)  cel_json_object_get(_obj, key) 

#define cel_conf_get_size(_array) cel_json_array_get_size(_array)
#define cel_conf_array_get_bool(_array, which, value) \
    cel_json_array_get_bool(_array, which, value)
#define cel_conf_array_get_int(_array, which, value)  \
    cel_json_array_get_int(_array, which, value) 
#define cel_conf_array_get_double(_array, which, value)  \
    cel_json_array_get_double(_array, which, value) 
#define cel_conf_array_get_string(_array, which, value, size)  \
    cel_json_array_get_string(_array, which, value, size) 
#define cel_conf_array_get_item(_array, which)  \
    cel_json_array_get(_array, which) 

#define cel_conf_object_new() cel_json_object_new()
#define cel_conf_object_add_bool(_obj, key, value) \
    cel_json_object_add_bool(_obj, key, value)
#define cel_conf_object_add_int(_obj, key, value)  \
    cel_json_object_add_int(_obj, key, value) 
#define cel_conf_object_add_double(_obj, key, value)  \
    cel_json_object_add_double(_obj, key, value) 
#define cel_conf_object_add_string(_obj, key, value)  \
    cel_json_object_add_string(_obj, key, value) 
#define cel_conf_object_add_item(_obj, key)  \
    cel_json_object_get(_obj, key, item) 

#define cel_conf_array_new() cel_json_array_new()
#define cel_conf_array_add_bool(_array, value) \
    cel_json_array_add_bool(_array, value)
#define cel_conf_array_add_int(_array, value)  \
    cel_json_array_add_int(_array, value) 
#define cel_conf_array_add_double(_array, value)  \
    cel_json_array_add_double(_array, value) 
#define cel_conf_array_add_string(_array, value)  \
    cel_json_array_add_string(_array, value) 
#define cel_conf_array_add_item(_array, item)  cel_json_array_get(_array, item)

#ifdef __cplusplus
}
#endif

#endif
