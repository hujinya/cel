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
#ifndef __CEL_PATTRIE_H__
#define __CEL_PATTRIE_H__

#include "cel/types.h"
#include "cel/list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _CelPatTrieNodeType
{
    CEL_PATTRIE_NODE_ROOT,
    CEL_PATTRIE_NODE_STATIC,
    CEL_PATTRIE_NODE_PARAM,
}CelPatTrieNodeType;

typedef struct _CelPatTrieNode
{
    CelListItem item;
    CelPatTrieNodeType type;
    size_t key_len;
    char *key;
    void *value;
    CelList *static_children;
    CelList *param_children;
    BOOL terminal;
}CelPatTrieNode;

typedef struct _CelPatTrie
{
    CelPatTrieNode *root;
    CelFreeFunc value_free_func;
}CelPatTrie;

CelPatTrieNode *cel_pattrie_node_new(CelPatTrieNodeType type, 
                                     char *key, size_t key_len, void *value);
void cel_pattrie_node_free(CelPatTrieNode *node, CelFreeFunc value_free_func);

int cel_pattrie_init(CelPatTrie *pat_trie, CelFreeFunc value_free_func);
void cel_pattrie_destroy(CelPatTrie *pat_trie);

CelPatTrie *cel_pattrie_new(CelFreeFunc value_free_func);
void cel_pattrie_free(CelPatTrie *pat_trie);

int _cel_pattrie_node_insert(CelPatTrieNode *node, 
                             char *key, size_t key_len, void *value);
static __inline 
void cel_pattrie_insert(CelPatTrie *pat_trie, char *key, void *value)
{
    if (pat_trie->root == NULL)
        pat_trie->root = cel_pattrie_node_new(
        CEL_PATTRIE_NODE_ROOT, NULL, 0, NULL);
    _cel_pattrie_node_insert(pat_trie->root, key, strlen(key), value);
}

void *_cel_pattrie_node_lookup(CelPatTrieNode *node,
                               char *key, size_t key_len);
#define cel_pattrie_lookup(pat_trie, key) \
    ((pat_trie)->root == NULL ? NULL \
    : _cel_pattrie_node_lookup((pat_trie)->root, key, strlen(key)))

void _cel_pattrie_node_remove(CelPatTrieNode *node, 
                              char *key, size_t key_len,
                              CelFreeFunc value_free_func);
#define cel_pattrie_remove(pat_trie, key) \
    ((pat_trie)->root == NULL ? NULL \
    : _cel_pattrie_node_remove((pat_trie)->root, \
    key, strlen(key), (pat_trie)->value_free_func))
void cel_pattrie_clear(CelPatTrie *pat_trie);

#ifdef __cplusplus
}
#endif
#endif
