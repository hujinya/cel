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

/* github.com/xujiajun/gorouter */

#include "cel/pattrie.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

int cel_pattrie_init(CelPatTrie *pat_trie, CelFreeFunc value_free_func)
{
    pat_trie->root = NULL;
    pat_trie->value_free_func = value_free_func;
    return 0;
}

void cel_pattrie_destroy(CelPatTrie *pat_trie)
{
    cel_pattrie_clear(pat_trie);
}

CelPatTrie *cel_pattrie_new(CelFreeFunc value_free_func)
{
    CelPatTrie *pat_trie;

    if ((pat_trie = 
        (CelPatTrie *)cel_malloc(sizeof(CelPatTrie))) != NULL)
    {
        if (cel_pattrie_init(pat_trie, value_free_func) == 0)
            return pat_trie;
        cel_free(pat_trie);
    }
    return NULL;
}

void cel_pattrie_free(CelPatTrie *pat_trie)
{
    cel_pattrie_destroy(pat_trie);
    cel_free(pat_trie);
}

CelPatTrieNode *cel_pattrie_node_new(CelPatTrieNodeType type, 
                                     const char *key, size_t key_len,
                                     void *value)
{
    CelPatTrieNode *new_node;

    if ((new_node = 
        (CelPatTrieNode *)cel_malloc(sizeof(CelPatTrieNode))) == NULL)
        return NULL;
    if (key != NULL)
    {
        new_node->key = cel_strdup_full(key, 0, key_len);
        new_node->key_len = key_len;
    }
    else
    {
        new_node->key = NULL;
        new_node->key_len = 0;
    }

    new_node->type = type;
    new_node->value = value;
    new_node->param_name = NULL;
    new_node->regexp = NULL;
    new_node->static_children = NULL;
    new_node->param_children = NULL;

   /* printf("new key %s, len %d\r\n", 
        new_node->key, (int)new_node->key_len);*/

    return new_node;
}

void cel_pattrie_node_free(CelPatTrieNode *node, CelFreeFunc value_free_func)
{
    CelList *list;
    CelPatTrieNode *child, *next;

    if ((list = node->static_children) != NULL)
    {
        child = (CelPatTrieNode *)(list->head.next);
        while (child != (CelPatTrieNode *)cel_list_get_tail(list))
        {
            //printf("child key %s\r\n", child->key);
            next = (CelPatTrieNode *)child->item.next;
            cel_pattrie_node_free(child, value_free_func);
            child = next;
        }
    }
    if ((list = node->param_children) != NULL)
    {
        child = (CelPatTrieNode *)(list->head.next);
        while (child != (CelPatTrieNode *)cel_list_get_tail(list))
        {
            //printf("child key %s\r\n", child->key);
            next = (CelPatTrieNode *)child->item.next;
            cel_pattrie_node_free(child, value_free_func);
            child = next;
        }
    }
    /*printf("free key %s\r\n", node->key);*/
    if (node->key != NULL)
        cel_free(node->key);
    if (node->value != NULL && value_free_func != NULL)
        value_free_func(node->value);
    if (node->param_name != NULL)
        cel_free(node->param_name);
    if (node->regexp != NULL)
        cel_free(node->regexp);
    cel_free(node);
}

int _cel_pattrie_child_node_insert(CelPatTrieNode *node, 
                                   const char *key, size_t key_len,
                                   void *value)
{
    int i, p0 = -1, p1 = -1, p2 = -1;
    int param_name_len, regexp_len;
    char *param_name, *regexp;
    CelPatTrieNode *new_node;

    /*printf("child node %s %d, key %s %d\r\n", 
        node->key, (int)node->key_len, key, (int)key_len);*/
    for (i = 0; i < (int)key_len; i++)
    {
        if (p0 < 0 && key[i] == '<')
            p0 = i;
        if (p0 >= 0)
        {
            if (key[i] == ':')
                p2 = i;
            else if (key[i] == '>')
            {
                p1 = i;
                break;
            }
        }
    }
    if ((p0 > 0 && p1 > 0) || p1 < 0)
    {
        if (p1 > 0)
        {
            new_node = cel_pattrie_node_new(
                CEL_PATTRIE_NODE_STATIC, key, p0, NULL);
            if (node->static_children == NULL)
                node->static_children = cel_list_new(NULL);
            cel_list_push_back(node->static_children, (CelListItem *)new_node);
            node = new_node;
        }
        else
        {
            new_node = cel_pattrie_node_new(
                CEL_PATTRIE_NODE_STATIC, key, key_len, value);
            if (node->static_children == NULL)
                node->static_children = cel_list_new(NULL);
            cel_list_push_back(node->static_children, (CelListItem *)new_node);
            return 0;
        }
    }
    /* Add param node */
    if (p2 == -1)
    {
        param_name_len = p1 - p0 - 1;
        param_name = cel_strdup_full(key, p0 + 1, p1);
        regexp = NULL;
        regexp_len = 0;
    }
    else
    {
        param_name_len = p2 - p0 - 1;
        param_name = cel_strdup_full(key, p0 + 1, p2);
        regexp_len = p1 - p2  - 1;
        regexp = cel_strdup_full(key, p2 + 1, p1);
    }
    if (p1 == key_len - 1)
    {
        new_node = cel_pattrie_node_new(
            CEL_PATTRIE_NODE_PARAM, &key[p0], key_len - p0, value);
        new_node->param_name = param_name;
        new_node->param_name_len = param_name_len;
        new_node->regexp = regexp;
        new_node->regexp_len = regexp_len;
        if (node->param_children == NULL)
            node->param_children = cel_list_new(NULL);
        cel_list_push_back(node->param_children, (CelListItem *)new_node);
        return 0;
    }
    else
    {
        new_node = cel_pattrie_node_new(
            CEL_PATTRIE_NODE_PARAM, &key[p0], p1 - p0 + 1, NULL);
        new_node->param_name = param_name;
        new_node->param_name_len = param_name_len;
        new_node->regexp = regexp;
        new_node->regexp_len = regexp_len;
        if (node->param_children == NULL)
            node->param_children = cel_list_new(NULL);
        cel_list_push_back(node->param_children, (CelListItem *)new_node);
        return _cel_pattrie_child_node_insert(
            new_node, &key[p1 + 1], key_len - p1 - 1, value);
    }
}

int _cel_pattrie_node_insert(CelPatTrieNode *node, 
                             const char *key, size_t key_len, void *value,
                             CelFreeFunc value_free_func)
{
    size_t matched, node_key_len, sub_key_len;
    const char *sub_key;
    CelPatTrieNode *child, *tail, *new_node;

    /*printf("node %s %d, key %s %d\r\n", 
        node->key, (int)node->key_len, key, (int)key_len);*/
    node_key_len = (node->key_len < key_len ? node->key_len : key_len);
    for (matched = 0 ;matched < node_key_len; matched++)
    {
        if (node->key[matched] != key[matched])
            break;
    }
    if (matched == node->key_len)
    {
        if (matched == key_len)
        {
            if (node->value != NULL
                && value_free_func != NULL)
                value_free_func(node->value);
            node->value = value;
            return 0;
        }
        sub_key = &key[matched];
        sub_key_len = key_len - matched;
        /* Try adding to a static child */
        if (node->static_children != NULL)
        {
            child = (CelPatTrieNode *)cel_list_get_head(node->static_children);
            tail = (CelPatTrieNode *)cel_list_get_tail(node->static_children);
            while ((child = (CelPatTrieNode *)child->item.next) != tail)
            {
                if (_cel_pattrie_node_insert(
                    child, sub_key, sub_key_len, value, value_free_func) == 0)
                    return 0;
            }
        }
        /* Try adding to a param child */
        if (node->param_children != NULL)
        {
            child = (CelPatTrieNode *)cel_list_get_head(node->param_children);
            tail = (CelPatTrieNode *)cel_list_get_tail(node->param_children);
            while ((child = (CelPatTrieNode *)child->item.next) != tail)
            {
                if (_cel_pattrie_node_insert(
                    child, sub_key, sub_key_len, value, value_free_func) == 0)
                    return 0;
            }
        }
        return _cel_pattrie_child_node_insert(
            node, sub_key, sub_key_len, value);
    }
    if (matched == 0 
        || node->type != CEL_PATTRIE_NODE_STATIC) 
    {
        /*
         * no common prefix, 
         * or partial common prefix with a non-static node: should skip this node
         */
        return -1;
    }
    /* The node key shares a partial prefix with the key: split the node key */
    new_node = cel_pattrie_node_new(node->type, 
        &(node->key[matched]), node->key_len - matched, node->value);
    new_node->static_children = node->static_children;
    new_node->param_children = node->param_children;

    node->key[matched] = '\0';
    node->key_len = matched;
    node->value = NULL;
    node->static_children = cel_list_new(NULL);
    node->param_children = NULL;
    cel_list_push_back(node->static_children, (CelListItem *)new_node);

    return _cel_pattrie_node_insert(
        node, key, key_len, value, value_free_func);
}

int _cel_pattrie_node_lookup(CelPatTrieNode *node, 
                             const char *key, size_t key_len,
                             void **value, CelPatTrieParams *params)
{
    int i;
    size_t child_key_len, sub_key_len;
    const char *sub_key;
    char *param_value;
    CelPatTrieNode *child, *tail;

   /* printf("node %s %d, key %s %d\r\n", 
        node->key, (int)node->key_len, key, (int)key_len);*/
    /*  Find a static child that can match the rest of the key */
    if (node->static_children != NULL)
    {
        child = (CelPatTrieNode *)cel_list_get_head(node->static_children);
        tail = (CelPatTrieNode *)cel_list_get_tail(node->static_children);
next_static_children:
        while ((child = (CelPatTrieNode *)child->item.next) != tail)
        {
            if ((child_key_len = child->key_len) > key_len)
                continue;
            for (i = (int)(child_key_len - 1); i >= 0; i--)
            {
                if (child->key[i] != key[i])
                    goto next_static_children;
            }
            if ((sub_key_len = key_len - child_key_len) == 0)
            {
                *value = child->value;
                return 0;
            }
            sub_key = &key[child_key_len];
            return _cel_pattrie_node_lookup(
                child, sub_key, sub_key_len, value, params);
        }
    }
    /* Try matching param children */
    if (node->param_children != NULL)
    {
        child = (CelPatTrieNode *)cel_list_get_head(node->param_children);
        tail = (CelPatTrieNode *)cel_list_get_tail(node->param_children);
        while ((child = (CelPatTrieNode *)child->item.next) != tail)
        {
            //if (child->regexp != NULL)
            //{
            //    // if n.regex.String() == "^.*" {
            //    // pvalues[n.pindex] = key
            //    //	key = ""
            //    //} else if match := n.regex.FindStringIndex(key); match != nil {
            //    //	pvalues[n.pindex] = key[0:match[1]]
            //    //	key = key[match[1]:]
            //    //} else {
            //    //	return
            //    //}
            //    return 0;
            //}
            //else
            {
                if (child->param_name[0] == '*')
                {
                    param_value = cel_strdup(key);
                    cel_rbtree_insert(params, child->param_name, param_value);
                    *value = child->value;
                    return 0;
                }
                for(i = 0; i < (int)key_len; i++)
                {
                    if (key[i] == '/')
                    {
                        if (params != NULL)
                        {
                            param_value = cel_strdup_full(key, 0, i);
                            cel_rbtree_insert(params, child->param_name, param_value);
                        }
                        //printf("param %s= %s\r\n", node->param_name, key);
                        sub_key = &key[i];
                        sub_key_len = key_len - i;
                        return _cel_pattrie_node_lookup(
                            child, sub_key, sub_key_len, value, params);
                    }
                }
                if (i == key_len)
                {
                    if (params != NULL)
                        cel_rbtree_insert(params, child->param_name, cel_strdup(key));
                    //printf("param %s= %s\r\n", node->param_name, key);
                    *value = child->value;
                    return 0;
                }
            }
        }
    }
    return -1;
}

void cel_pattrie_clear(CelPatTrie *pat_trie)
{
    if (pat_trie->root != NULL)
        cel_pattrie_node_free(pat_trie->root, pat_trie->value_free_func);
}
