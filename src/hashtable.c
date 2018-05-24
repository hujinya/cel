/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2016 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/hashtable.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

/* Debug defines */
#define Debug(args)   /*cel_log_debug args*/
#define Warning(args) CEL_SETERRSTR(args)/* cel_log_warning args */
#define Err(args)   CEL_SETERRSTR(args)/* cel_log_err args */

#define HASH_COLLISION                3

static __inline BOOL cel_hashtable_maybe_larger(CelHashTable *hash_table,
                                                size_t *new_capacity)
{
    if (hash_table->size >= (HASH_COLLISION * hash_table->capacity)
        && hash_table->capacity < CEL_CAPACITY_MAX)
    {
        *new_capacity = cel_capacity_get_min(hash_table->size);
        return TRUE;
    }
    return FALSE;
}

static __inline BOOL cel_hashtable_maybe_smaller(CelHashTable *hash_table,
                                                 size_t *new_capacity)
{
    if ((HASH_COLLISION * hash_table->size) <= hash_table->capacity
        && hash_table->capacity > CEL_CAPACITY_MIN)
    {
        *new_capacity = cel_capacity_get_min(hash_table->size);
        return TRUE;
    }
    return FALSE;
}

static int cel_hashtable_resize(CelHashTable *hash_table, size_t new_capacity)
{
    size_t i, hash_value;
    CelHashTableItem **new_items, *new_item, *old_item;

    Debug((_T("Hash table resize, "
        "new capacity %d, old capacity %d, crruent size %d"),
        new_capacity, hash_table->capacity, hash_table->size));
    if ((new_items = (CelHashTableItem **)cel_malloc(
        new_capacity * sizeof(CelHashTableItem *)
        + new_capacity * (sizeof(CelHashTableItem) * HASH_COLLISION))) == NULL)
    {
        return -1;
    }
    memset(new_items, 0 , sizeof(CelHashTableItem *) * new_capacity);
    hash_table->free = (CelHashTableItem *)(new_items + new_capacity);
    for (i = 0; i < (new_capacity * HASH_COLLISION - 1); i++)
    {
        hash_table->free[i].next = &hash_table->free[i+1];
    }
    if (hash_table->items != NULL)
    {
        for (i = 0; i < hash_table->capacity; i++)
        {
            old_item = hash_table->items[i];
            while (old_item != NULL)
            {
                hash_value = old_item->key_hash % new_capacity;
                new_item = hash_table->free;
                hash_table->free = hash_table->free->next;
                new_item->key = old_item->key;
                new_item->value = old_item->value;
                new_item->key_hash = old_item->key_hash;
                new_item->next = new_items[hash_value];
                new_items[hash_value] = new_item;
                old_item = old_item->next;
            }
        }
        cel_free(hash_table->items);
    }
    hash_table->capacity = new_capacity;
    hash_table->items = new_items;
    return 0;
}

int cel_hashtable_init(CelHashTable *hash_table, CelHashFunc hash_func, 
                       CelEqualFunc key_equal_func, 
                       CelFreeFunc key_free_func, CelFreeFunc value_free_func)
{
    hash_table->capacity = 0;
    hash_table->size = 0;
    hash_table->hash_func = hash_func;
    hash_table->key_equal_func = key_equal_func;
    hash_table->key_free_func = key_free_func;
    hash_table->value_free_func = value_free_func;
    hash_table->items = NULL;
    hash_table->free = NULL;

    return 0;
}

void cel_hashtable_destroy(CelHashTable *hash_table)
{
    if (hash_table == NULL)
        return;

    cel_hashtable_clear(hash_table);
    if (hash_table->items != NULL)
        cel_free(hash_table->items);
}

CelHashTable *cel_hashtable_new(CelHashFunc hash_func, 
                                CelEqualFunc key_equal_func, 
                                CelFreeFunc key_free_func, 
                                CelFreeFunc value_free_func)
{
    CelHashTable *hash_table;

    if ((hash_table = 
        (CelHashTable *)cel_malloc(sizeof(CelHashTable))) != NULL)
    {   
        if (cel_hashtable_init(hash_table,
            hash_func, key_equal_func, key_free_func, value_free_func) == 0)
            return hash_table;
        cel_free(hash_table);
    }
    return NULL;  
}

void cel_hashtable_free(CelHashTable *hash_table)
{
    cel_hashtable_destroy(hash_table);
    cel_free(hash_table);
}

void cel_hashtable_insert(CelHashTable *hash_table, void *key, void *value)
{
    size_t new_capacity;
    unsigned int hash_value, index;
    CelHashTableItem *item, *new_item;

    if (cel_hashtable_maybe_larger(hash_table, &new_capacity)
        && cel_hashtable_resize(hash_table, new_capacity) == -1)
        return ;
    hash_value = hash_table->hash_func(key);
    index = hash_value % hash_table->capacity;
    if ((item = hash_table->items[index]) == NULL)
    {
        new_item = hash_table->free;
        hash_table->free = hash_table->free->next;
        hash_table->items[index] = new_item;
    }
    else
    {
        do
        {
            if (item->key_hash == hash_value 
                && hash_table->key_equal_func(item->key, key))
            {
                if (hash_table->key_free_func != NULL)
                    hash_table->key_free_func(item->key);
                item->key = key;
                if (hash_table->value_free_func != NULL)
                    hash_table->value_free_func(item->value);
                item->value = value;
                Debug((_T("Hashkey %p already exists in the hashtable %p, ")
                    _T("replace it new value."), item->key, hash_table));
                return;
            }
            if (item->next == NULL)
            {
                new_item = hash_table->free;
                hash_table->free = hash_table->free->next;

                item->next = new_item;
                break;
            }
        }while ((item = item->next) != NULL);
    }
    new_item->next = NULL;
    new_item->key = key;
    new_item->value = value;
    new_item->key_hash = hash_value;
    hash_table->size++;
    /*Debug((_T("Insert a key %p and its associated value into hashtbale %p."), 
        new_item->key, hash_table));*/
}

void cel_hashtable_remove(CelHashTable *hash_table, const void *key)
{
    size_t new_capacity;
    unsigned int hash_value, index;
    CelHashTableItem *previous, *item;

    if (key == NULL || hash_table->size == 0) return;

    hash_value = hash_table->hash_func(key);
    index = hash_value % hash_table->capacity;
    previous = item = hash_table->items[index];
    while (item != NULL)
    {
        if (item->key_hash != hash_value 
            || !(hash_table->key_equal_func(item->key, key)))
        {
            previous = item;
            item = item->next;
            continue;
        }
        /* Free key */
        if (hash_table->key_free_func != NULL) 
            hash_table->key_free_func(item->key);
        /* Free value */
        if (hash_table->value_free_func != NULL)
            hash_table->value_free_func(item->value);
        if (previous == item)
            hash_table->items[index] = item->next;
        else
            previous->next = item->next;
        item->next = hash_table->free;
        hash_table->free = item;
        hash_table->size--;
        Debug((_T("Removes key %p,crruent size %d,  capacity %d."), 
            item->key, hash_table->size, hash_table->capacity));
        if (cel_hashtable_maybe_smaller(hash_table, &new_capacity))
            cel_hashtable_resize(hash_table, new_capacity);
        return;
    }
    Warning((_T("Hashkey %p not found in hashtable %p."), key, hash_table));
}

void *cel_hashtable_lookup(CelHashTable *hash_table, const void *key)
{
    CelHashTableItem *item;
    unsigned int hash_value;

    if (key == NULL || hash_table->size == 0) 
        return NULL;

    hash_value = hash_table->hash_func(key);
    item = hash_table->items[hash_value % hash_table->capacity];

    while (item != NULL)
    {
        if (item->key_hash == hash_value 
            && hash_table->key_equal_func(item->key, key))
        {
            break;
        }
        item = item->next;
    }
    return (item == NULL ? NULL : item->value);
}

int cel_hashtable_foreach(CelHashTable *hash_table, 
                          CelKeyValuePairEachFunc each_func, void *user_data)
{
    int ret;
    size_t i;
    CelHashTableItem *item, *next_item;

    if (each_func == NULL) 
        return -1;

    for (i = 0; i < hash_table->capacity; i++)
    {
        next_item = hash_table->items[i];
        while ((item =  next_item) != NULL)
        {
            next_item = item->next;
            if ((ret = each_func(item->key, item->value, user_data)) != 0)
                return ret;
        }
    }
    return 0;
}

void cel_hashtable_clear(CelHashTable *hash_table)
{
    size_t i, new_capacity;
    CelHashTableItem *item, *next;

    for (i = 0; i < hash_table->capacity; i++)
    {
        item = hash_table->items[i];
        while (item != NULL)
        {
            if (hash_table->key_free_func != NULL)
                hash_table->key_free_func(item->key);
            if (hash_table->value_free_func != NULL)
                hash_table->value_free_func(item->value);
            next = item->next;
            item->next = hash_table->free;
            hash_table->free = item;
            item = next;
        }
        hash_table->items[i] = NULL;
    }
    hash_table->size = 0;
    if (cel_hashtable_maybe_smaller(hash_table, &new_capacity))
        cel_hashtable_resize(hash_table, new_capacity);
}

BOOL cel_int_equal(const void *s_value, const void *d_value)
{
    return *((const int *)s_value) == *((const int *)d_value);
}

unsigned int cel_int_hash(const void *key)
{
    return *(const int *)key;
}

BOOL cel_long_equal(const void *s_value, const void *d_value)
{
    return *((const long* )s_value) == *((const long* )d_value);
}

unsigned int cel_long_hash(const void *key)
{
    return *(const int *)key;
}

BOOL cel_double_equal(const void *s_value, const void *d_value)
{
    return *((const double* )s_value) == *((const double* )d_value);
}

unsigned int cel_double_hash(const void *key)
{
    return *(const int*)key;
}

BOOL cel_str_equal(const void *s_value, const void *d_value)
{
    const TCHAR *string1 = (const TCHAR *)s_value;
    const TCHAR *string2 = (const TCHAR *)d_value;

    return (_tcscmp(string1, string2) == 0);
}

unsigned int cel_str_hash(const void *key)
{
    /* 31 bit hash function */
    const signed char *p = (const signed char *)key;
    unsigned int h = *p;
    if (h)
        for (p += 1; *p != '\0'; p++)
            h = (h << 5) - h + *p;
    return h;
}
