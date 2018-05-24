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
#ifndef __CEL_HASHTABLE_H__
#define __CEL_HASHTABLE_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Hash function */
typedef unsigned int (* CelHashFunc) (const void *key);

typedef struct _CelHashTableItem
{
    struct _CelHashTableItem *next;
    unsigned int key_hash;
    void *key;
    void *value;
}CelHashTableItem;

typedef struct _CelHashTable
{
    size_t capacity;
    size_t size;
    CelHashFunc hash_func;
    CelEqualFunc key_equal_func;
    CelFreeFunc key_free_func, value_free_func;
    CelHashTableItem **items;
    CelHashTableItem *free;
}CelHashTable;

int cel_hashtable_init(CelHashTable *hash_table, CelHashFunc hash_func, 
                       CelEqualFunc key_equal_func, 
                       CelFreeFunc key_free, CelFreeFunc value_free_func);
void cel_hashtable_destroy(CelHashTable *hash_table);
CelHashTable *cel_hashtable_new(CelHashFunc hash_func, 
                                CelEqualFunc key_equal_func,
                                CelFreeFunc key_free_func, 
                                CelFreeFunc value_free_func);
void cel_hashtable_free(CelHashTable *hash_table);
 
void cel_hashtable_insert(CelHashTable *hash_table, void *key, void *value);

static __inline size_t cel_hashtable_get_capacity(CelHashTable *hash_table)
{
    return hash_table->capacity;
}
static __inline size_t cel_hashtable_get_size(CelHashTable *hash_table)
{
    return hash_table->size;
}
void *cel_hashtable_lookup(CelHashTable *hash_table, const void *key);
/*
 * \return     0 foreach successful; 1 each func return end; -1 error.
 */
int cel_hashtable_foreach(CelHashTable *hash_table, 
                          CelKeyValuePairEachFunc each_func, void *user_data);

void cel_hashtable_remove(CelHashTable *hash_table, const void *key);
void cel_hashtable_clear(CelHashTable *hash_table);

/* Common hash functions */
BOOL cel_int_equal(const void *s_value, const void *d_value);
unsigned int cel_int_hash(const void *key);

BOOL cel_long_equal(const void *s_value, const void *d_value);
unsigned int cel_long_hash(const void *key);

BOOL cel_double_equal(const void *s_value, const void *d_value);
unsigned int cel_double_hash(const void *key);

BOOL cel_str_equal(const void *s_value, const void *d_value);
unsigned int cel_str_hash(const void *key);

#ifdef __cplusplus
}
#endif

#endif
